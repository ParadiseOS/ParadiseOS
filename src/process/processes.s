format ELF

section ".text" executable

;; NOTE: This assumes the layout of the PCB so it will break if that layout is
;; changed.
public jump_usermode
jump_usermode:
    mov ax, 0x23 ; user data GDT entry with RPL = 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ecx, [esp+0x4]
    mov eax, [esp+0x8]
    mov edx, [esp+0xC]
    push 0x23
    push eax
    mov eax, [edx+0x20]
    or eax, (1 shl 9)
    push eax
    push 0x1b
    push ecx

    mov edi, [edx+0x00]
    mov esi, [edx+0x04]
    mov ebp, [edx+0x08]
    mov ebx, [edx+0x10]
    mov ecx, [edx+0x18]
    mov eax, [edx+0x1C]
    mov edx, [edx+0x14]

    iret
