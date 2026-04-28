format ELF

section ".text" executable

extrn main
extrn system_page

public start as "_start"
start:
    pop ecx
    mov [system_page], ecx
    call main
    hlt
