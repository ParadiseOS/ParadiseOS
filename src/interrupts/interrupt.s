format ELF

section ".data" align 32

public cpu_interrupts
cpu_interrupts:
dd interrupt_wrapper_no_error_code_0
dd interrupt_wrapper_no_error_code_1
dd interrupt_wrapper_no_error_code_2
dd interrupt_wrapper_no_error_code_3
dd interrupt_wrapper_no_error_code_4
dd interrupt_wrapper_no_error_code_5
dd interrupt_wrapper_no_error_code_6
dd interrupt_wrapper_no_error_code_7
dd interrupt_wrapper_error_code_8
dd interrupt_wrapper_no_error_code_9 
dd interrupt_wrapper_error_code_10
dd interrupt_wrapper_error_code_11
dd interrupt_wrapper_error_code_12
dd interrupt_wrapper_error_code_13
dd interrupt_wrapper_error_code_14
dd interrupt_wrapper_no_error_code_15
dd interrupt_wrapper_no_error_code_16
dd interrupt_wrapper_no_error_code_17
dd interrupt_wrapper_no_error_code_18
dd interrupt_wrapper_no_error_code_19
dd interrupt_wrapper_no_error_code_20
dd interrupt_wrapper_no_error_code_21
dd interrupt_wrapper_no_error_code_22
dd interrupt_wrapper_no_error_code_23
dd interrupt_wrapper_no_error_code_24
dd interrupt_wrapper_no_error_code_25
dd interrupt_wrapper_no_error_code_26
dd interrupt_wrapper_no_error_code_27
dd interrupt_wrapper_no_error_code_28
dd interrupt_wrapper_no_error_code_29
dd interrupt_wrapper_no_error_code_30
dd interrupt_wrapper_no_error_code_31
dd interrupt_wrapper_no_error_code_32
dd interrupt_wrapper_no_error_code_33
dd interrupt_wrapper_no_error_code_34
dd interrupt_wrapper_no_error_code_35
dd interrupt_wrapper_no_error_code_36
dd interrupt_wrapper_no_error_code_37
dd interrupt_wrapper_no_error_code_38
dd interrupt_wrapper_no_error_code_39
dd interrupt_wrapper_no_error_code_40
dd interrupt_wrapper_no_error_code_41
dd interrupt_wrapper_no_error_code_42
dd interrupt_wrapper_no_error_code_43
dd interrupt_wrapper_no_error_code_44
dd interrupt_wrapper_no_error_code_45
dd interrupt_wrapper_no_error_code_46
dd interrupt_wrapper_no_error_code_47

section ".text" executable

extrn isr_handler

rept 48 int_no:0 {
    interrupt_wrapper_error_code_#int_no:
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
    
    interrupt_wrapper_no_error_code_#int_no:
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