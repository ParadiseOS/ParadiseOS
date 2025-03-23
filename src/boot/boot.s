format ELF

ALIGNPAGE = 1 shl 0 ; Align boot modules to page boundaries
MEMINFO =   1 shl 1 ; Get info on available memory and memory map
VIDEOINFO = 1 shl 2 ; Get video info mode

TEXTMODE =   1 ; EGA-standard text-mode
SIZENOPREF = 1 ; Don't care value for size

FLAGS = VIDEOINFO or MEMINFO or ALIGNPAGE

MAGIC =    0x1BADB002 ; Communicate that our kernel is indeed multiboot
                      ; compatible
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

section ".data"

public kernel_stack
kernel_stack:
    dd stack_top

section ".bss" writable align 16

public multiboot_info
multiboot_info:
    rd 1

; Create a 16 KiB, 16 byte aligned stack so our C kernel can function
stack_bottom:
    align 16
    rb 1024 * 16
stack_top:

section ".paging" writeable align 4096

KERNELOFFSET = 0xC0000000
MAXKERNELPAGES = 0x300 ; Since we only reserve one page table

; Create a page-aligned page directory
page_directory:
    rd 1024

; Create a page-aligned page table. One should be enough as long as our kernel
; is smaller than 3 MiB.
boot_page_table:
    rd 1024

section ".boottext" executable

extrn _kernel_start_paddr
extrn _kernel_end_vaddr

extrn kernel_start
extrn panic_handler
public start as "_start"
start:
    ; Clear the initial page directory and page table
    mov ecx, 1024
    @@:
        dec ecx
        mov DWORD [page_directory - KERNELOFFSET + 4*ecx], 0
        mov DWORD [boot_page_table - KERNELOFFSET + 4*ecx], 0
        test ecx, ecx
        jnz @b

    ; self reference last page directory entry
    mov DWORD [page_directory - KERNELOFFSET + 4*1023], \
        page_directory - KERNELOFFSET + 1
    ; map page table for kernel virtual addresses
    mov DWORD [page_directory - KERNELOFFSET + 4*(KERNELOFFSET shr 22)], \
        boot_page_table - KERNELOFFSET + 1
    ; map page table for lower addresses
    mov DWORD [page_directory - KERNELOFFSET], \
        boot_page_table - KERNELOFFSET + 1

    mov ecx, 0 ; identity map lower memory
    mov edx, 1 ; present bit
    @@:
        mov DWORD [boot_page_table - KERNELOFFSET + 4*ecx], edx
        add edx, 4096
        add ecx, 1
        cmp ecx, 256 ; Number of lower memory pages that we should map. This
                     ; may be allowed to be smaller.
        jl @b

    mov ecx, _kernel_end_vaddr + 4095 - KERNELOFFSET ; find kernel size in pages
    sub ecx, _kernel_start_paddr
    shr ecx, 12

    cmp ecx, MAXKERNELPAGES ; size check
    jg panic_handler

    mov esi, _kernel_start_paddr ; create page table entry
    or esi, 1

    mov edi, _kernel_start_paddr + KERNELOFFSET ; page table index of kernel
    shr edi, 12
    and edi, 0x3FF

    ; Map kernel and boot section. This works since we mapped the same page
    ; table to both entries above. If the boot section is not mapped, we will
    ; immediately page fault after enabling paging.
    @@:
        dec ecx ; TODO: maybe check that we don't overflow the page table
        mov DWORD [boot_page_table - KERNELOFFSET + 4*edi], esi
        inc edi
        add esi, 4096
        test ecx, ecx
        jnz @b

    mov eax, cr4 ; enable global pages
    or eax, 1 shl 7
    mov cr4, eax

    mov eax, page_directory - KERNELOFFSET ; load page directory
    mov cr3, eax

    mov eax, cr0 ; enable paging
    or eax, 0x80000000
    mov cr0, eax

    mov esp, stack_top        ; Set up the stack
    mov [multiboot_info], ebx ; Save multiboot info structure for our kernel

    jmp kernel_start
