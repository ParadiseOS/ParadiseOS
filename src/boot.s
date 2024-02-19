format ELF

ALIGNPAGE = 1 shl 0 ; Align boot modules to page boundaries
MEMINFO =   1 shl 1 ; Get info on available memory and memory map
VIDEOINFO = 1 shl 2 ; Get video info mode

TEXTMODE =   1 ; EGA-standard text-mode
SIZENOPREF = 1 ; Don't care value for size

FLAGS = VIDEOINFO or MEMINFO or ALIGNPAGE

MAGIC =    0x1BADB002 ; Communicate that our kernel is indeed multiboot compatible
CHECKSUM = -(MAGIC + FLAGS)


; So multiboot can actually load our kernel.
; https://www.gnu.org/software/grub/manual/multiboot/multiboot.html 
section ".multiboot" align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    rb 20
    dd TEXTMODE
    dd SIZENOPREF ; width
    dd SIZENOPREF ; height
    dd 0          ; depth (always 0 in text mode)



section ".bss" writeable align 16

public multiboot_info
multiboot_info:
    rd 1

; Create a 16 KiB, word-aligned stack so our C kernel can function
stack_bottom:
    align 16
    rb 1024 * 16
public stack_top
stack_top:


section ".text" executable
extrn kernel_main
public start as "_start"
start:
    mov esp, stack_top        ; Set up the stack
    mov [multiboot_info], ebx ; Save multiboot info structure for our kernel

    call kernel_main

    cli ; If we ever leave the kernel, just sit in an idle loop
    @@:
        hlt
        jmp @b

public load_gdt
load_gdt:
    ret
    pop eax; get GDT address
    lgdt [eax]
    jmp 0x8:flush_gdt

flush_gdt:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

    
