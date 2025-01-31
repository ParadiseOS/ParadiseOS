format ELF

section ".pages" writeable align 1024 * 4

; Create a page directory and page table, both must be page-aligned
public page_directory_start
page_directory_start:
    rd 1024
public page_tables_start
page_tables_start:
    rd 1024 * 12 ; Just allocate space for 12 tables for now

section ".text" executable

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
