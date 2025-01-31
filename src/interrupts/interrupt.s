format ELF

section ".data" align 32

public cpu_interrupts
cpu_interrupts:
rept 32 interrupt:0 {
    dd interrupt_wrapper_#interrupt
}

section ".text" executable

extrn interrupt_handler
rept 32 interrupt:0 {
    public interrupt_wrapper_#interrupt
    interrupt_wrapper_#interrupt:
        push interrupt
        call interrupt_handler
        add esp, 0x04
        iret
}

extrn syscall_handler
public syscall_wrapper
syscall_wrapper:
    call syscall_handler
    iret

public load_idt
load_idt:
    mov eax, [esp+0x4]
    lidt [eax]
    ret