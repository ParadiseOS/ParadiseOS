format ELF

section ".data" align 32

public cpu_interrupts
cpu_interrupts:
rept 48 interrupt:0 {
    dd interrupt_wrapper_#interrupt
}

section ".text" executable

extrn isr_handler
rept 48 interrupt:0 {
    interrupt_wrapper_#interrupt:
        push interrupt
        call isr_handler
        add esp, 0x04
        iret
}

extrn keyboard_handler
public keyboard_wrapper
keyboard_wrapper:
    call keyboard_handler
    iret

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