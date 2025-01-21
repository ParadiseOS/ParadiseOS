format ELF

section ".data" align 32

public cpu_interrupts
cpu_interrupts:
rept 32 interrupt:0 {
    dd interrupt_wrapper_#interrupt
}

section ".pages" writeable align 1024 * 4

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
    mov eax, [esp+0x4]
    mov [eax+0x04], esp
    mov word [eax+0x08], 0x10
    mov edx, cr3
    mov [eax+0x1C], edx
    xor edx, edx
    mov dword [eax+0x20], check_initialized
    pushf
    pop edx
    mov dword [eax+0x24], edx
    xor edx, edx
    mov dword [eax+0x28], 1
    mov [eax+0x2C], ecx
    mov [eax+0x30], edx
    mov [eax+0x34], ebx
    mov [eax+0x38], esp
    mov [eax+0x3C], ebp
    mov [eax+0x40], esi
    mov [eax+0x44], edi
    mov [eax+0x48], ES
    mov [eax+0x4C], CS
    mov [eax+0x50], SS
    mov [eax+0x54], DS
    mov [eax+0x58], FS
    mov [eax+0x5C], GS
    jmp 0x28:0x0
check_initialized:
    ret
