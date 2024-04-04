ENTRY(_start)

SECTIONS {
    /* Start physical address */
    . = 2M;

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

    .pages BLOCK(4k) : ALIGN(4k) {
        *(.pages)
    }
}
