#include "kernel.h"
#include "boot/multiboot.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/serial/io.h"
#include "drivers/timer/timer.h"
#include "init.h"
#include "interrupts/interrupt.h"
#include "lib/error.h"
#include "lib/logging.h"
#include "lib/strings.h"
#include "memory/mem.h"
#include "process/processes.h"
#include "sun/sun.h"
#include "terminal/terminal.h"
#include "tests/testing.h"
#include "syscall/syscall.h"

const u32 kernel_start_paddr = (u32) &_kernel_start_paddr;
const void *kernel_start_vaddr = &_kernel_start_vaddr;
const u32 kernel_end_paddr = (u32) &_kernel_end_paddr;
const void *kernel_end_vaddr = &_kernel_end_vaddr;

extern void *syscall_table[100];
typedef SyscallResult (*Syscall)(u32 a, u32 b, u32 c, u32 d, u32 e);

void kernel_main(void) {
    if (!(multiboot_info->flags & MB_FLAG_FRAMEBUFFER)) {
        // No terminal info is available. Something went wrong and there's no
        // way to report it.
        kernel_panic();
    }

    terminal_init(
        multiboot_info->framebuffer_width, multiboot_info->framebuffer_height,
        (u16 *) multiboot_info->framebuffer_addr_lo
    );

    KERNEL_ASSERT(multiboot_info->framebuffer_addr_hi == 0);

    printk(
        DEBUG, "Kernel Size: %u KB\n",
        (kernel_end_paddr - kernel_start_paddr) / 1024
    );

    printk(DEBUG, "Initializing GDT...\n");
    init_gdt();

    printk(DEBUG, "Initializing IDT...\n");
    init_idt();

    printk(DEBUG, "Initializing TSS...\n");
    init_tss();

    printk(DEBUG, "Initializing FPU...\n");
    init_fpu();

    printk(DEBUG, "Initializing memory...\n");
    mem_init();

    printk(DEBUG, "Initializing timer\n");
    init_timer();

    printk(DEBUG, "Initializing keyboard\n");
    init_keyboard();

    printk(DEBUG, "Initializing serial io...\n");
    serial_init();

    printk(DEBUG, "Initializing the sun...\n");
    sun_init();

#ifdef TESTS_ENABLED // Test Flag should be passed to build script
    kernel_test();
#endif

    processes_init();

    // Add your processes here
    // ex. exec_sun("binary.out", 0)
    // "hello_world.out, 0"
    // "screamer.o"

    asm("sti");

    //exec_sun("hello_world.out", 0);

    //Syscall syscall  = syscall_table[ctx->eax];
    Syscall reg_proc = syscall_table[2];
    Syscall del_proc = syscall_table[3];
    Syscall jmp_proc =syscall_table[4];

    // This works regardless of the number of actual expected parameters since
    // the callee is responsible for cleaning up the stack.
    //SyscallResult res =
        //syscall(ctx->ebx, ctx->ecx, ctx->edx, ctx->esi, ctx->edi);

    SyscallResult reg_ret = reg_proc(0, 0, 0, 0, 0);
    printk(DEBUG, "Register 1 Ret <%u> Err <%u>\n", reg_ret.ret, reg_ret.err);

    reg_ret = reg_proc(0, 0, 0, 0, 0);
    printk(DEBUG, "REGISTER 2 Ret <%u> Err <%u>\n", reg_ret.ret, reg_ret.err);

    SyscallResult del_ret = del_proc(9, 0, 0, 0, 0);
    printk(DEBUG, "DELETE BAD PID Ret <%u> Err <%u>\n", del_ret.ret, del_ret.err);

    exec_sun("hello_world.out", 1);
    exec_sun("screamer.out", 2);
    jmp_proc(1, 0, 0, 0 ,0);



    schedule();

    for (;;) {
        asm("hlt");
    }
}
