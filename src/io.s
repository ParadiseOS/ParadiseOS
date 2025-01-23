format ELF

section ".text" executable

public inb
inb:
    mov dx, [esp+0x04]
    in al, dx
    ret

public outb
outb:
    mov dx, [esp+0x04]
    mov al, [esp+0x08]
    out dx, al
    ret
