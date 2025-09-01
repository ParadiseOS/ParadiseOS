#include "processes.h"
#include "drivers/timer/timer.h"
#include "interrupts/interrupt.h"
#include "ipc/mailbox.h"
#include "kernel/kernel.h"
#include "lib/error.h"
#include "lib/libp.h"
#include "memory/heap.h"
#include "memory/mem.h"
#include "sun/sun.h"
#include "terminal/terminal.h"

#define MAX_PROCS    0x10000
#define STACK_SIZE   (4 * PAGE_SIZE)
#define STACK_TOP    ((void *) 0xbfc00000)
#define PROCESS_ORG  ((void *) 0x402000)
#define MAILBOX_ADDR (PROCESS_ORG - PAGE_SIZE)
#define PCB_ADDR     (MAILBOX_ADDR - PAGE_SIZE)

#define INIT_EFLAGS 0b1000000010

RbTree process_tree;
Queue run_queue;

Process processes[MAX_PROCS] = {0};
i32 running_pid = -1;
u32 process_count = 0;

__attribute__((noreturn)) extern void
jump_usermode(void (*f)(), void *stack, ProcessControlBlock *pcb);

u16 next_pid() {
    for (u32 i = 0; i < MAX_PROCS; ++i) {
        if (processes[i].page_dir_paddr == 0)
            return i;
    }

    KERNEL_ASSERT(FALSE);
}

#define RO_FLAGS PAGE_USER_MODE
#define RW_FLAGS (PAGE_WRITABLE | PAGE_USER_MODE)

void exec_sun(const char *name, int arg) {
    TableEntry *entry = sun_exe_lookup(name);

    KERNEL_ASSERT(entry->text_size);

    ProcessControlBlock *pcb = PCB_ADDR;
    Mailbox *mailbox = MAILBOX_ADDR;
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

    heap_init(&pcb->heap, heap, heap_pages);

    load_page_dir(kernel_page_dir);

    u16 pid = next_pid();
    processes[pid].page_dir_paddr = page_dir;
    ++process_count;
}

void schedule() {
    KERNEL_ASSERT(process_count);

    if (running_pid == -1)
        running_pid = 0;

    for (u16 i = running_pid;; ++i) {
        if (processes[i].page_dir_paddr != 0) {
            ProcessControlBlock *pcb = PCB_ADDR;
            load_page_dir(processes[i].page_dir_paddr);
            KERNEL_ASSERT(pcb->page_dir_paddr == processes[i].page_dir_paddr);
            running_pid = i;
            jump_usermode((void (*)()) pcb->eip, (void *) pcb->esp, pcb);
        }
    }
}

void syscall_handler(CpuContext *ctx) {

    // $eax determines what the syscall does
    switch (ctx->eax) {

    // Print string
    // $ebx -> pointer to the string
    // $ecx -> the size of the string
    case 1:
        for (u32 i = 0; i < ctx->ecx; i++) {
            terminal_putchar(*((char *) ctx->ebx + i));
        }
        terminal_putchar('\n');
        break;

    // Print null-terminated string
    // $ebx -> pointer to the string
    case 2:
        terminal_printf("%s\n", ctx->ebx);
        break;

    // Send message to another processes mailbox
    // $ah -> Flags (not implemented yet, assume indirect flag is always set for
    // now) $ebx -> Receiver PID $cl -> Data Size $edx -> Data $edi -> Data
    // (Depends on flag)
    case 3: {
        // Copy message
        u8 message_size = ctx->ecx & 0xFF;
        char message_cpy[255];
        for (u8 i = 0; i < message_size; i++) {
            message_cpy[i] = *((char *) ctx->edx + i);
        }
        message_cpy[message_size] = '\0';
        i32 org_pid = running_pid;

        // Switch address space
        load_page_dir(processes[ctx->ebx].page_dir_paddr);

        // Send message to mailbox
        send_message(MAILBOX_ADDR, org_pid, message_size, message_cpy);

        // Switch back address space
        load_page_dir(processes[org_pid].page_dir_paddr);
        break;
    }

    // Read message from own mailbox
    // $ebx -> pointer to where the message should be held (ensure 258 bytes are
    // allocated) $ecx -> status of the read request (0 if no message, 1 if
    // message written) $ecx is returned, no need to input anything
    case 4:
        ctx->ecx = read_message(MAILBOX_ADDR, (char *) ctx->ebx);
        break;

    default:
        terminal_printf("Unknown syscall :(\n");
        break;
    }
}

bool is_user_mode(u32 cs) {
    return (cs & 3) == 3;
}

void preempt(InterruptRegisters *regs) {
    if (is_user_mode(regs->cs)) {
        ProcessControlBlock *pcb = PCB_ADDR;

        pmemcpy(pcb, (u32 *) regs + 1, 8 * sizeof(u32));
        pcb->eflags = regs->eflags;
        pcb->eip = regs->eip;
        pcb->esp = regs->useresp;

        running_pid += 1;
        pic_eoi(regs->int_no - 32);
        schedule(); // no return
    }
    else {
        pic_eoi(regs->int_no - 32);
    }
}

void scheduler_init() {
    timer_callback = preempt;
    rb_init(&process_tree);
    queue_init(&run_queue);
}
