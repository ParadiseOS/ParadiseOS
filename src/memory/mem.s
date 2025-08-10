format ELF

section ".text" executable

public fpu_save
fpu_save:
    mov eax, [esp+0x04]
    fxsave [eax]
    ret

public fpu_restore
fpu_restore:
    mov eax, [esp+0x04]
    fxrstor [eax]
    ret

;; NOTE: This won't work for cpu families earlier than i486. We can use
;; conditional compilation at some point later to support more platforms.
public invalidate_page
invalidate_page:
    mov eax, [esp+0x04]
    invlpg [eax]
    ret

public flush_tlb
flush_tlb:
    mov eax, cr3
    mov cr3, eax
    ret

public load_page_dir
load_page_dir:
    mov eax, [esp+0x04]
    mov cr3, eax
    ret

public get_page_dir_paddr
get_page_dir_paddr:
    mov eax, cr3
    ret
