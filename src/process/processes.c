#include "processes.h"
#include "drivers/timer/timer.h"
#include "interrupts/interrupt.h"
#include "kernel/kernel.h"
#include "lib/error.h"
#include "lib/libp.h"
#include "memory/heap.h"
#include "memory/mem.h"
#include "sun/sun.h"
#include "terminal/terminal.h"
#include "ipc/mailbox.h"

#define MAX_PROCS 0x10000
#define STACK_SIZE (4 * PAGE_SIZE)
#define STACK_TOP ((void *) 0xbfc00000)
#define PROCESS_ORG ((void *) 0x401000)
#define PCB_ADDR (PROCESS_ORG - PAGE_SIZE)
#define MAILBOX_ADDR (PCB_ADDR - PAGE_SIZE)

#define INIT_EFLAGS 0b1000000010

Process processes[MAX_PROCS] = {0};
i32 running_pid = -1;
u32 process_count = 0;

__attribute__((noreturn))
extern void jump_usermode(void (*f)(), void *stack, ProcessControlBlock *pcb);

u16 next_pid() {
    for (u32 i = 0; i < MAX_PROCS; ++i) {
        if (processes[i].page_dir_paddr == 0)
            return i;
    }

    KERNEL_ASSERT(FALSE);
}

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

    alloc_pages(text, PAGE_USER_MODE, (rodata - text) / PAGE_SIZE);
    if (entry->rodata_size) alloc_pages(rodata, PAGE_USER_MODE, (data - rodata) / PAGE_SIZE);
    if (entry->data_size) alloc_pages(data, PAGE_WRITABLE | PAGE_USER_MODE, (bss - data) / PAGE_SIZE);
    if (entry->bss_size) alloc_pages(bss, PAGE_WRITABLE | PAGE_USER_MODE, (heap - bss) / PAGE_SIZE);
    alloc_pages(stack - STACK_SIZE, PAGE_WRITABLE | PAGE_USER_MODE, STACK_SIZE / PAGE_SIZE);
    alloc_pages(pcb, PAGE_WRITABLE, 1);
    alloc_pages(mailbox, PAGE_WRITABLE, 1);

    sun_load_text(entry, text);
    sun_load_rodata(entry, rodata);
    sun_load_data(entry, data);
    pmemset(bss, 0, entry->bss_size);

    pcb->prog_brk = heap;
    pcb->eip = (u32) entry->entry_point;
    pcb->esp = (u32) stack;
    pcb->eflags = INIT_EFLAGS;
    pcb->page_dir_paddr = page_dir;
    pcb->eax = arg;

    heap_init(&pcb->heap, heap, heap_pages);

    load_page_dir(kernel_page_dir);
    Mailbox_Init_Temp(mailbox);

    u16 pid = next_pid();
    processes[pid].page_dir_paddr = page_dir;
    ++process_count;
}

void schedule() {
    KERNEL_ASSERT(process_count);

    if (running_pid == -1) running_pid = 0;

    for (u16 i = running_pid; ; ++i) {
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
    // Come up with some syscall format for the print server
    // Maybe set something like eax to 1 to set for printing
    // Put a pointer to the data in ecx
    // Then the size is edx

    if (ctx->eax == 1) { 
        for (u32 len = 0; len < ctx->edx; len++) {
            terminal_putchar(*((char*)ctx->ecx + len));
        }
        terminal_putchar('\n');
    } else if (ctx->eax == 2) { // Expects null terminated string as the pointer.
        terminal_printf("%s\n", ctx->ecx);
    } else {
        terminal_printf("Unknown syscall :(\n");
    }
}

bool is_user_mode(u32 cs) {
    return (cs & 3) == 3;
}

void preempt(InterruptRegisters *regs) {
    if (is_user_mode(regs->cs)) {
        ProcessControlBlock *pcb = PCB_ADDR;

        pmemcpy(pcb, (u32 *)regs + 1, 8 * sizeof(u32));
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
}
