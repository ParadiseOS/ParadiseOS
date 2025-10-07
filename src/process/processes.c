#include "processes.h"
#include "drivers/timer/timer.h"
#include "interrupts/interrupt.h"
#include "ipc/mailbox.h"
#include "kernel/kernel.h"
#include "lib/error.h"
#include "lib/libp.h"
#include "memory/heap.h"
#include "memory/mem.h"
#include "process/pool.h"
#include "process/queue.h"
#include "process/rb_tree.h"
#include "sun/sun.h"
#include "syscall/syscall.h"
#include "terminal/terminal.h"

#define STACK_SIZE   (4 * PAGE_SIZE)
#define STACK_TOP    ((void *) 0xbfc00000)
#define PROCESS_ORG  ((void *) 0x402000)
#define MAILBOX_ADDR (PROCESS_ORG - PAGE_SIZE)
#define PCB_ADDR     (MAILBOX_ADDR - PAGE_SIZE)

#define INIT_EFLAGS 0b1000000010

ProcessPool process_pool;
RbTree process_tree;
Queue run_queue;

Process *current = NULL;
CpuContext *current_ctx = NULL;
u16 pid_counter = 0;

ProcessControlBlock *pcb = PCB_ADDR;
Mailbox *mailbox = MAILBOX_ADDR;

__attribute__((noreturn)) extern void
jump_usermode(void (*f)(), void *stack, ProcessControlBlock *pcb);

#define FIELD_PARENT_PTR(parent_type, field_name, field_ptr)                   \
    ((parent_type *) ((u8 *) field_ptr - offsetof(parent_type, field_name)))

Process *get_process(u16 pid) {
    RbNode *node = rb_find(&process_tree, pid);
    if (node)
        return FIELD_PARENT_PTR(Process, rb_node, node);
    else
        return NULL;
}

static Process *run_queue_next() {
    QueueNode *node = queue_poll(&run_queue);
    if (node)
        return FIELD_PARENT_PTR(Process, queue_node, node);
    else
        return NULL;
}

static u16 next_free_pid() {
    u16 pid = pid_counter;

    do {
        if (!rb_find(&process_tree, pid)) {
            pid_counter = pid + 1;
            return pid;
        }

        pid += 1;
    } while (pid != pid_counter);

    KERNEL_ASSERT(false); // Too many processes
}

#define RO_FLAGS PAGE_USER_MODE
#define RW_FLAGS (PAGE_WRITABLE | PAGE_USER_MODE)

void exec_sun(const char *name, int arg) {
    TableEntry *entry = sun_exe_lookup(name);

    KERNEL_ASSERT(entry->text_size);

    void *text = PROCESS_ORG;
    void *rodata = align_next_page(text + entry->text_size - 1);
    void *data = align_next_page(rodata + entry->rodata_size - 1);
    void *bss = align_next_page(data + entry->data_size - 1);
    void *heap = align_next_page(bss + entry->bss_size - 1);
    void *stack = STACK_TOP;

    u32 heap_pages = ((u32) STACK_TOP - (u32) heap) / PAGE_SIZE;

    u32 page_dir = new_page_dir();
    load_page_dir(page_dir);

    alloc_pages(text, RO_FLAGS, (rodata - text) / PAGE_SIZE);
    if (entry->rodata_size)
        alloc_pages(rodata, RO_FLAGS, (data - rodata) / PAGE_SIZE);
    if (entry->data_size)
        alloc_pages(data, RW_FLAGS, (bss - data) / PAGE_SIZE);
    if (entry->bss_size)
        alloc_pages(bss, RW_FLAGS, (heap - bss) / PAGE_SIZE);
    alloc_pages(stack - STACK_SIZE, RW_FLAGS, STACK_SIZE / PAGE_SIZE);
    alloc_pages(mailbox, PAGE_WRITABLE, 1);
    alloc_pages(pcb, PAGE_WRITABLE, 1);

    sun_load_text(entry, text);
    sun_load_rodata(entry, rodata);
    sun_load_data(entry, data);
    pmemset(bss, 0, entry->bss_size);
    mailbox_init_temp(mailbox);

    pcb->prog_brk = heap;
    pcb->eip = (u32) entry->entry_point;
    pcb->esp = (u32) stack;
    pcb->eflags = INIT_EFLAGS;
    pcb->page_dir_paddr = page_dir;
    pcb->eax = arg;

    pmemset(pcb->fpu_regs, 0, /*fpu_regs size*/ 512);

    heap_init(&pcb->heap, heap, heap_pages);

    load_page_dir(kernel_page_dir);

    u16 pid = next_free_pid();
    Process *p = pool_create(&process_pool);
    p->page_dir_paddr = page_dir;
    p->blocked = false;
    rb_insert(&process_tree, &p->rb_node, pid);
    queue_add(&run_queue, &p->queue_node);
}

void schedule() {
    current = run_queue_next();
    KERNEL_ASSERT(current);

    load_page_dir(current->page_dir_paddr);
    KERNEL_ASSERT(pcb->page_dir_paddr == current->page_dir_paddr);
    fpu_restore(pcb->fpu_regs);
    jump_usermode((void (*)()) pcb->eip, (void *) pcb->esp, pcb);
}

// TODO: unify the below two functions. It's not really great that we have two
// different cpu context info formats for syscalls vs other interrupts.

static void save_context_int(InterruptRegisters *regs) {
    pmemcpy(pcb, (u32 *) regs + 1, 8 * sizeof(u32));
    pcb->eflags = regs->eflags;
    pcb->eip = regs->eip;
    pcb->esp = regs->useresp;
    fpu_save(pcb->fpu_regs);
}

static void save_context_syscall(CpuContext *ctx) {
    pmemcpy(pcb, (u32 *) ctx, 8 * sizeof(u32));
    pcb->eflags = ctx->eflags;
    pcb->eip = ctx->eip;
    pcb->esp = ctx->useresp;
    fpu_save(pcb->fpu_regs);
}

void syscall_handler(CpuContext *ctx) {
    current_ctx = ctx;
    if (!dispatch_syscall(ctx))
        terminal_printf("Unknown syscall :(\n");
    current_ctx = NULL;
}

bool is_user_mode(u32 cs) {
    return (cs & 3) == 3;
}

void preempt(InterruptRegisters *regs) {
    if (is_user_mode(regs->cs)) {
        KERNEL_ASSERT(pcb->page_dir_paddr == current->page_dir_paddr);
        save_context_int(regs);
        queue_add(&run_queue, &current->queue_node);
        pic_eoi(regs->int_no - 32);
        schedule();
    }
    else {
        pic_eoi(regs->int_no - 32);
    }
}

SyscallResult syscall_send_message(u32 pid, u32 len, char *data) {
    u8 message_size = len & 0xFF;
    char message_cpy[256];
    pmemcpy(message_cpy, data, message_size);
    message_cpy[message_size] = '\0';
    i32 org_pid = GET_PID(current);

    // Switch address space
    Process *dst = get_process(pid);
    KERNEL_ASSERT(dst);
    load_page_dir(dst->page_dir_paddr);

    // Send message to mailbox
    send_message(mailbox, org_pid, message_size, message_cpy);

    if (dst->blocked) {
        dst->blocked = false;
        pcb->eax = read_message(mailbox, (void *) pcb->ebx);
        KERNEL_ASSERT(pcb->eax);
        queue_add(&run_queue, &dst->queue_node);
    }

    // Switch back address space
    load_page_dir(current->page_dir_paddr);
    SYSCALL_RETURN(0, 0);
}

SyscallResult syscall_read_message(char *out, u32 blocking) {
    bool res = read_message(mailbox, out);
    if (blocking && !res) {
        KERNEL_ASSERT(!current->blocked); // Can't issue syscall while blocked
        current->blocked = true;
        save_context_syscall(current_ctx);
        schedule();
    }
    SYSCALL_RETURN(res, 0);
}

void processes_init() {
    timer_callback = preempt;
    pool_init(&process_pool);
    rb_init(&process_tree);
    queue_init(&run_queue);

    register_syscall(3, syscall_send_message);
    register_syscall(4, syscall_read_message);
}
