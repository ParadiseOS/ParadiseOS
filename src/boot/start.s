format ELF

section ".text"

extrn kernel_main
public kernel_start
kernel_start:
    cli ; We never want maskable interrupts in the kernel
    call kernel_main

public panic_handler
panic_handler:
    @@: ; If we ever leave the kernel, just sit in an idle loop
        hlt
        jmp @b
