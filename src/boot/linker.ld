ENTRY(_start)

SECTIONS {
    /* Start physical address */
    . = 1M;
    _kernel_start = .;

    .text BLOCK(4K) : ALIGN(4K) {
        *(.multiboot) /* Multiboot header must come first */
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
    }

    .pagedir BLOCK(4K) : ALIGN(4K) {
        *(.pagedir)
    }

    _kernel_end = .;
}
