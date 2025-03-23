format ELF

section ".text" executable

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
