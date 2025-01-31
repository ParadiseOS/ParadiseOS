format ELF

section ".data"

public cpu_interrupts
cpu_interrupts:
rept 32 interrupt:0 {
    dd interrupt_wrapper_#interrupt
}

section ".pages" writeable align 4096

; Create a page directory and page table, both must be page-aligned
public page_directory_start
page_directory_start:
    rd 1024
public page_tables_start
page_tables_start:
    rd 1024 * 12 ; Just allocate space for 12 tables for now

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

public load_gdt
load_gdt:
    mov eax, [esp+0x4]
    lgdt [eax]
    jmp 0x08:flush_gdt

flush_gdt:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

public enable_paging
enable_paging:
    mov eax, page_directory_start
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret

public load_tss
load_tss:
    mov ax, [esp+0x4]
    ltr ax
    ret

public get_privilege_level
get_privilege_level:
    mov ax, cs
    and ax, 3
    ret
