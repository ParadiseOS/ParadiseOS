    .set ALIGNPAGE, 1 << 0 // Align boot module to page boundaries
    .set MEMINFO,   1 << 1 // Get info on available memory and memory map
    .set VIDEOINFO, 1 << 2 // Get video mode info

    .set TEXTMODE,   1 // EGA-standard text mode.
    .set SIZENOPREF, 0 // Don't care value for graphics size

    .set FLAGS, VIDEOINFO | MEMINFO | ALIGNPAGE

    // To adhere to the multiboot specification.
    .set MAGIC,      0x1BADB002
    .set CHECKSUM,   -(MAGIC + FLAGS)

    // So we can boot from mutliboot.
    // https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
    .section .multiboot
    .align 4
    .long MAGIC
    .long FLAGS
    .long CHECKSUM
    .skip 20
    .long TEXTMODE
    .long SIZENOPREF // width
    .long SIZENOPREF // height
    .long 0          // depth (always 0 in text mode)

    .section .bss
    .global multiboot_info
    .align 16
multiboot_info:
    .long 0
    // Create a 16 KiB stack so our C kernel can function.
stack_bottom:
    .skip 1024 * 16
stack_top:

    .section .text
    .global _start
    .type _start, @function
_start:
    mov $stack_top, %esp        // Set up a stack.
    mov %ebx, [multiboot_info]  // Store multiboot info for the kernel.

    // TODO: Enable paging.
    // TODO: Handle interrupts

    call kernel_main            // Ensure stack is 16-byte aligned before this

    cli                         // Disable interrupts
idle_loop:
    hlt                         // Wait for non-maskable interrupt
    jmp idle_loop
