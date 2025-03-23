format ELF

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
