ENTRY(_start)

SECTIONS {
    /* Start physical address */
    . = 1M;
    _kernel_start_paddr = .;
    _kernel_start_vaddr = . + 0xC0000000;

    .multiboot : {
        *(.multiboot) /* Multiboot header must come first */
        *(.boottext)
    }

    . += 0xC0000000;

    .text ALIGN(4K) : AT(ADDR (.text) - 0xC0000000) {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR (.rodata) - 0xC0000000) {
        *(.rodata)
    }

    .data ALIGN(4K) : AT(ADDR (.data) - 0xC0000000) {
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR (.bss) - 0xC0000000) {
        *(COMMON)
        *(.bss)
    }

    .paging ALIGN(4K) : AT(ADDR (.paging) - 0xC0000000) {
        *(.paging)
    }

    _kernel_end_paddr = . - 0xC0000000;
    _kernel_end_vaddr = .;
}
