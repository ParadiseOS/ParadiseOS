format ELF

section ".text" executable

public jump_usermode
jump_usermode:
    mov ecx, [esp+0x4]
    mov ax, 0x23 ; user data GDT entry with RPL = 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, esp
    push 0x23
    push eax
    pushf
    push 0x1b
    push ecx
    iret
