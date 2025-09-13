format ELF

section ".data" align 32

public cpu_interrupts
cpu_interrupts:
rept 48 int_no:0 {
    dd interrupt_wrapper_#int_no
}

section ".text" executable

extrn isr_handler

rept 48 int_no:0 {
    if int_no = 8 | (10 <= int_no & int_no <= 14)
        ;; interrupts with error codes (8, 10, 11, 12, 13, 14)
        interrupt_wrapper_#int_no:
            cli
            push int_no
            pushad
            mov eax, cr2
            push eax

            push esp
            call isr_handler

            add esp, 8
            popad
            add esp, 4
            sti
            iret
    else
        ;; interrupts without error codes
        interrupt_wrapper_#int_no:
            cli
            push 0
            push int_no
            pushad
            mov eax, cr2
            push eax

            push esp
            call isr_handler

            add esp, 8
            popad
            add esp, 8
            sti
            iret
    end if
}

extrn syscall_handler
public syscall_wrapper
syscall_wrapper:
    cli
    pushad

    push esp
    call syscall_handler

    add esp, 4
    popad
    sti
    iret

public load_idt
load_idt:
    mov eax, [esp+0x4]
    lidt [eax]
    ret
