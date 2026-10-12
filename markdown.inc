put_wc PROC
    mov rax, QWORD PTR [g_plain]
    mov rdx, QWORD PTR [g_plen]
    mov WORD PTR [rax + rdx*2], cx
    inc QWORD PTR [g_plen]
    ret
put_wc ENDP

; eax = run flags for the text that follows; ends the previous run first
flush_run PROC FRAME
    PROLOG
    mov rax, QWORD PTR [g_plen]
    sub rax, QWORD PTR [g_runstart]
    jz fr_done
    mov rcx, QWORD PTR [g_nspans]
    cmp rcx, QWORD PTR [g_spancap]
    jae fr_done
    shl rcx, 5
    add rcx, QWORD PTR [g_spans]
    mov rdx, QWORD PTR [g_runstart]
    mov DWORD PTR [rcx], edx
    mov DWORD PTR [rcx+4], eax
    mov r8d, DWORD PTR [g_iflags]
    or r8d, DWORD PTR [g_bflags]
    xor r10d, r10d
    xor r11d, r11d
    test r8d, 1
    jz fr_no_bold
    or r10d, 1
    or r11d, 1
fr_no_bold:
    test r8d, 2
    jz fr_no_italic
    or r10d, 2
    or r11d, 2
fr_no_italic:
    test r8d, 4
    jz fr_no_ul
    or r10d, 4
    or r11d, 4
fr_no_ul:
    test r8d, 8
    jz fr_no_strike
    or r10d, 8
    or r11d, 8
fr_no_strike:
    test r8d, 16
    jz fr_no_face
    or r10d, 20000000h
fr_no_face:
    test r8d, 16
    jnz fr_muted
    test r8d, 64
    jnz fr_muted
    test r8d, 32
    jz fr_color_set
    or r10d, 40000000h
    mov eax, DWORD PTR [g_accent]
    mov DWORD PTR [rcx+20], eax
    jmp fr_color_set
fr_muted:
    or r10d, 40000000h
    mov eax, DWORD PTR [g_muted]
    mov DWORD PTR [rcx+20], eax
fr_color_set:
    mov eax, DWORD PTR [g_bheight]
    test eax, eax
    jz fr_no_size
    or r10d, 80000000h
    mov DWORD PTR [rcx+16], eax
fr_no_size:
    test r10d, r10d
    jz fr_done
    mov DWORD PTR [rcx+8], r10d
    mov DWORD PTR [rcx+12], r11d
    inc QWORD PTR [g_nspans]
fr_done:
    mov rax, QWORD PTR [g_plen]
    mov QWORD PTR [g_runstart], rax
    EPILOG
flush_run ENDP

; eax = new inline flags
set_iflags PROC FRAME
    PROLOG
    cmp eax, DWORD PTR [g_iflags]
    je si_done
    mov DWORD PTR [rbp-8], eax
    call flush_run
    mov eax, DWORD PTR [rbp-8]
    mov DWORD PTR [g_iflags], eax
si_done:
    EPILOG
set_iflags ENDP

; ecx = block flags, edx = block height (twips), r8d = left indent, r9d = first-line offset
begin_block PROC FRAME
    PROLOG
    mov DWORD PTR [rbp-8], ecx
    mov DWORD PTR [rbp-16], edx
    mov DWORD PTR [rbp-24], r8d
    mov DWORD PTR [rbp-32], r9d
    xor eax, eax
    call flush_run
    mov eax, DWORD PTR [rbp-8]
    mov DWORD PTR [g_bflags], eax
    mov eax, DWORD PTR [rbp-16]
    mov DWORD PTR [g_bheight], eax
    mov eax, DWORD PTR [rbp-24]
    or eax, DWORD PTR [rbp-32]
    jz bb_done
    mov rcx, QWORD PTR [g_npars]
    cmp rcx, QWORD PTR [g_parcap]
    jae bb_done
    shl rcx, 4
    add rcx, QWORD PTR [g_pars]
    mov rdx, QWORD PTR [g_plen]
    mov DWORD PTR [rcx], edx
    mov eax, DWORD PTR [rbp-24]
    mov DWORD PTR [rcx+4], eax
    mov eax, DWORD PTR [rbp-32]
    mov DWORD PTR [rcx+8], eax
    inc QWORD PTR [g_npars]
bb_done:
    mov rax, QWORD PTR [g_plen]
    mov QWORD PTR [g_runstart], rax
    EPILOG
begin_block ENDP

end_block PROC FRAME
    PROLOG
    xor eax, eax
    call flush_run
    mov ecx, 0Dh
    call put_wc
    mov DWORD PTR [g_iflags], 0
    mov DWORD PTR [g_bflags], 0
    mov DWORD PTR [g_bheight], 0
    mov rax, QWORD PTR [g_plen]
    mov QWORD PTR [g_runstart], rax
    EPILOG
end_block ENDP

; rcx = start (UTF-16 pointer), rdx = end (exclusive): emits inline text
md_inline PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
mi_loop:
    mov rax, QWORD PTR [rbp-8]
    cmp rax, QWORD PTR [rbp-16]
    jae mi_done
    movzx ecx, WORD PTR [rax]
    cmp ecx, 5Ch
    je mi_escape
    cmp ecx, 60h
    je mi_code
    cmp ecx, 2Ah
    je mi_star
    cmp ecx, 5Fh
    je mi_star
    cmp ecx, 7Eh
    je mi_tilde
    cmp ecx, 5Bh
    je mi_link
mi_plain:
    mov rax, QWORD PTR [rbp-8]
    movzx ecx, WORD PTR [rax]
    call put_wc
    add QWORD PTR [rbp-8], 2
    jmp mi_loop

mi_escape:
    lea rdx, QWORD PTR [rax+4]
    cmp rdx, QWORD PTR [rbp-16]
    ja mi_plain
    movzx ecx, WORD PTR [rax+2]
    call put_wc
    add QWORD PTR [rbp-8], 4
    jmp mi_loop

mi_code:
    mov eax, DWORD PTR [g_iflags]
    xor eax, 10h
    call set_iflags
    add QWORD PTR [rbp-8], 2
    jmp mi_loop

mi_star:
    mov rax, QWORD PTR [rbp-8]
    movzx ecx, WORD PTR [rax]
    lea rdx, QWORD PTR [rax+4]
    cmp rdx, QWORD PTR [rbp-16]
    ja mi_star_single
    movzx edx, WORD PTR [rax+2]
    cmp edx, ecx
    jne mi_star_single
    mov eax, DWORD PTR [g_iflags]
    xor eax, 1
    call set_iflags
    add QWORD PTR [rbp-8], 4
    jmp mi_loop
mi_star_single:
    cmp ecx, 5Fh
    je mi_plain
    mov eax, DWORD PTR [g_iflags]
    xor eax, 2
    call set_iflags
    add QWORD PTR [rbp-8], 2
    jmp mi_loop

mi_tilde:
    mov rax, QWORD PTR [rbp-8]
    lea rdx, QWORD PTR [rax+4]
    cmp rdx, QWORD PTR [rbp-16]
    ja mi_plain
    movzx edx, WORD PTR [rax+2]
    cmp edx, 7Eh
    jne mi_plain
    mov eax, DWORD PTR [g_iflags]
    xor eax, 8
    call set_iflags
    add QWORD PTR [rbp-8], 4
    jmp mi_loop

mi_link:
    mov rax, QWORD PTR [rbp-8]
    add rax, 2
    mov rdx, QWORD PTR [rbp-16]
ml_find_close:
    cmp rax, rdx
    jae mi_plain
    cmp WORD PTR [rax], 5Dh
    je ml_found_close
    add rax, 2
    jmp ml_find_close
ml_found_close:
    mov QWORD PTR [rbp-24], rax
    lea r8, QWORD PTR [rax+2]
    cmp r8, rdx
    jae mi_plain
    cmp WORD PTR [rax+2], 28h
    jne mi_plain
    lea rax, QWORD PTR [rax+4]
ml_find_paren:
    cmp rax, rdx
    jae mi_plain
    cmp WORD PTR [rax], 29h
    je ml_found_paren
    add rax, 2
    jmp ml_find_paren
ml_found_paren:
    mov QWORD PTR [rbp-32], rax
    mov eax, DWORD PTR [g_iflags]
    mov DWORD PTR [rbp-40], eax
    or eax, 36
    call set_iflags
    mov rcx, QWORD PTR [rbp-8]
    add rcx, 2
    mov rdx, QWORD PTR [rbp-24]
    call md_inline
    mov eax, DWORD PTR [rbp-40]
    call set_iflags
    mov rax, QWORD PTR [rbp-32]
    add rax, 2
    mov QWORD PTR [rbp-8], rax
    jmp mi_loop

mi_done:
    EPILOG
md_inline ENDP

; Renders one source line delimited by g_ls (start) and g_le (end, exclusive)
md_block PROC FRAME
    PROLOG
    mov rax, QWORD PTR [g_ls]
    mov QWORD PTR [rbp-8], rax
mb_sp:
    mov rax, QWORD PTR [rbp-8]
    cmp rax, QWORD PTR [g_le]
    jae mb_blank
    cmp WORD PTR [rax], 20h
    jne mb_fence
    add QWORD PTR [rbp-8], 2
    jmp mb_sp

mb_fence:
    lea rdx, QWORD PTR [rax+6]
    cmp rdx, QWORD PTR [g_le]
    ja mb_code_check
    cmp WORD PTR [rax], 60h
    jne mb_code_check
    cmp WORD PTR [rax+2], 60h
    jne mb_code_check
    cmp WORD PTR [rax+4], 60h
    jne mb_code_check
    xor QWORD PTR [g_incode], 1
    jmp mb_ret

mb_code_check:
    cmp QWORD PTR [g_incode], 0
    je mb_heading
    mov ecx, 50h
    mov edx, 0
    xor r8d, r8d
    xor r9d, r9d
    call begin_block
    mov rax, QWORD PTR [g_ls]
    mov QWORD PTR [rbp-16], rax
mb_code_loop:
    mov rax, QWORD PTR [rbp-16]
    cmp rax, QWORD PTR [g_le]
    jae mb_code_end
    movzx ecx, WORD PTR [rax]
    call put_wc
    add QWORD PTR [rbp-16], 2
    jmp mb_code_loop
mb_code_end:
    call end_block
    jmp mb_ret

mb_heading:
    mov rax, QWORD PTR [rbp-8]
    cmp WORD PTR [rax], 23h
    jne mb_quote
    xor r8d, r8d
mb_h_count:
    mov rax, QWORD PTR [rbp-8]
    lea rdx, QWORD PTR [rax + r8*2]
    cmp rdx, QWORD PTR [g_le]
    jae mb_h_end
    cmp WORD PTR [rdx], 23h
    jne mb_h_end
    inc r8
    cmp r8, 7
    jb mb_h_count
mb_h_end:
    cmp r8, 6
    ja mb_quote
    mov rax, QWORD PTR [rbp-8]
    lea rdx, QWORD PTR [rax + r8*2]
    cmp rdx, QWORD PTR [g_le]
    jae mb_quote
    cmp WORD PTR [rdx], 20h
    jne mb_quote
mb_h_skip:
    add rdx, 2
    cmp rdx, QWORD PTR [g_le]
    jae mb_h_text
    cmp WORD PTR [rdx], 20h
    je mb_h_skip
mb_h_text:
    mov QWORD PTR [rbp-16], rdx
    mov QWORD PTR [rbp-24], r8
    mov rax, r8
    lea rdx, QWORD PTR [hsize]
    mov edx, DWORD PTR [rdx + rax*4]
    mov ecx, 1
    xor r8d, r8d
    xor r9d, r9d
    call begin_block
    mov rcx, QWORD PTR [rbp-16]
    mov rdx, QWORD PTR [g_le]
    call md_inline
    call end_block
    jmp mb_ret

mb_quote:
    mov rax, QWORD PTR [rbp-8]
    cmp WORD PTR [rax], 3Eh
    jne mb_rule
    add rax, 2
    cmp rax, QWORD PTR [g_le]
    jae mb_quote_go
    cmp WORD PTR [rax], 20h
    jne mb_quote_go
    add rax, 2
mb_quote_go:
    mov QWORD PTR [rbp-16], rax
    mov ecx, 42h
    xor edx, edx
    mov r8d, 480
    xor r9d, r9d
    call begin_block
    mov rcx, QWORD PTR [rbp-16]
    mov rdx, QWORD PTR [g_le]
    call md_inline
    call end_block
    jmp mb_ret

mb_rule:
    mov rax, QWORD PTR [rbp-8]
    movzx ecx, WORD PTR [rax]
    cmp ecx, 2Dh
    je mb_rule_chk
    cmp ecx, 2Ah
    je mb_rule_chk
    cmp ecx, 5Fh
    je mb_rule_chk
    jmp mb_bullet
mb_rule_chk:
    xor r8d, r8d
mb_rule_loop:
    cmp rax, QWORD PTR [g_le]
    jae mb_rule_eval
    movzx edx, WORD PTR [rax]
    cmp edx, ecx
    jne mb_rule_other
    inc r8
    add rax, 2
    jmp mb_rule_loop
mb_rule_other:
    cmp edx, 20h
    jne mb_bullet
    add rax, 2
    jmp mb_rule_loop
mb_rule_eval:
    cmp r8, 3
    jb mb_bullet
    mov ecx, 40h
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    call begin_block
    mov ecx, 2500h
    mov DWORD PTR [rbp-16], 0
mb_rule_emit:
    cmp DWORD PTR [rbp-16], 24
    jae mb_rule_done
    call put_wc
    inc DWORD PTR [rbp-16]
    jmp mb_rule_emit
mb_rule_done:
    call end_block
    jmp mb_ret

mb_bullet:
    mov rax, QWORD PTR [rbp-8]
    movzx ecx, WORD PTR [rax]
    cmp ecx, 2Dh
    je mb_bullet_chk
    cmp ecx, 2Ah
    je mb_bullet_chk
    cmp ecx, 2Bh
    je mb_bullet_chk
    jmp mb_number
mb_bullet_chk:
    lea rdx, QWORD PTR [rax+2]
    cmp rdx, QWORD PTR [g_le]
    jae mb_number
    cmp WORD PTR [rax+2], 20h
    jne mb_number
    lea rdx, QWORD PTR [rax+4]
    mov QWORD PTR [rbp-16], rdx
    xor eax, eax
    mov ecx, eax
    xor edx, edx
    mov r8d, 360
    mov r9d, -360
    call begin_block
    mov ecx, 2022h
    call put_wc
    mov ecx, 09h
    call put_wc
    mov rcx, QWORD PTR [rbp-16]
    mov rdx, QWORD PTR [g_le]
    call md_inline
    call end_block
    jmp mb_ret

mb_number:
    mov rax, QWORD PTR [rbp-8]
    xor r8d, r8d
mb_num_digits:
    lea rdx, QWORD PTR [rax + r8*2]
    cmp rdx, QWORD PTR [g_le]
    jae mb_num_end
    movzx ecx, WORD PTR [rdx]
    cmp ecx, 30h
    jb mb_num_end
    cmp ecx, 39h
    ja mb_num_end
    inc r8
    cmp r8, 9
    jb mb_num_digits
    jmp mb_paragraph
mb_num_end:
    test r8, r8
    jz mb_paragraph
    lea rdx, QWORD PTR [rax + r8*2]
    lea rcx, QWORD PTR [rdx+4]
    cmp rcx, QWORD PTR [g_le]
    ja mb_paragraph
    cmp WORD PTR [rdx], 2Eh
    jne mb_paragraph
    cmp WORD PTR [rdx+2], 20h
    jne mb_paragraph
    lea rdx, QWORD PTR [rdx+4]
    mov QWORD PTR [rbp-16], rdx
    mov QWORD PTR [rbp-32], r8
    mov QWORD PTR [rbp-40], 0
    xor eax, eax
    mov ecx, eax
    xor edx, edx
    mov r8d, 360
    mov r9d, -360
    call begin_block
mb_num_emit:
    mov rax, QWORD PTR [rbp-40]
    cmp rax, QWORD PTR [rbp-32]
    jae mb_num_emitted
    mov rdx, QWORD PTR [rbp-8]
    movzx ecx, WORD PTR [rdx + rax*2]
    call put_wc
    inc QWORD PTR [rbp-40]
    jmp mb_num_emit
mb_num_emitted:
    mov ecx, 2Eh
    call put_wc
    mov ecx, 09h
    call put_wc
    mov rcx, QWORD PTR [rbp-16]
    mov rdx, QWORD PTR [g_le]
    call md_inline
    call end_block
    jmp mb_ret

mb_paragraph:
    xor ecx, ecx
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    call begin_block
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [g_le]
    call md_inline
    call end_block
    jmp mb_ret

mb_blank:
    xor ecx, ecx
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    call begin_block
    call end_block
mb_ret:
    EPILOG
md_block ENDP

; Builds g_plain, g_spans and g_pars from g_rsrc[0..g_rcch)
md_build PROC FRAME
    PROLOG
    mov QWORD PTR [g_plen], 0
    mov QWORD PTR [g_nspans], 0
    mov QWORD PTR [g_npars], 0
    mov QWORD PTR [g_runstart], 0
    mov DWORD PTR [g_iflags], 0
    mov DWORD PTR [g_bflags], 0
    mov DWORD PTR [g_bheight], 0
    mov QWORD PTR [g_incode], 0
    mov QWORD PTR [rbp-16], 0
mr_line:
    mov rax, QWORD PTR [rbp-16]
    cmp rax, QWORD PTR [g_rcch]
    jae mr_end
    mov rdx, QWORD PTR [g_rsrc]
    lea rcx, QWORD PTR [rdx + rax*2]
    mov QWORD PTR [g_ls], rcx
    mov r8, rax
mr_find:
    cmp r8, QWORD PTR [g_rcch]
    jae mr_eol
    cmp WORD PTR [rdx + r8*2], 0Ah
    je mr_eol
    inc r8
    jmp mr_find
mr_eol:
    lea rcx, QWORD PTR [rdx + r8*2]
    cmp rcx, QWORD PTR [g_ls]
    jbe mr_no_cr
    cmp WORD PTR [rcx-2], 0Dh
    jne mr_no_cr
    sub rcx, 2
mr_no_cr:
    mov QWORD PTR [g_le], rcx
    mov QWORD PTR [rbp-24], r8
    call md_block
    mov rax, QWORD PTR [rbp-24]
    inc rax
    mov QWORD PTR [rbp-16], rax
    jmp mr_line
mr_end:
    xor eax, eax
    call flush_run
    mov rax, QWORD PTR [g_plain]
    mov rdx, QWORD PTR [g_plen]
    mov WORD PTR [rax + rdx*2], 0
    EPILOG
md_build ENDP

; Applies one paragraph record (rcx = pointer to record)
apply_par PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov DWORD PTR [g_pf], 188
    mov DWORD PTR [g_pf+4], 5
    mov eax, DWORD PTR [rcx+4]
    mov DWORD PTR [g_pf+12], eax
    mov eax, DWORD PTR [rcx+8]
    mov DWORD PTR [g_pf+20], eax
    mov rdx, QWORD PTR [rbp-8]
    mov eax, DWORD PTR [rdx]
    mov QWORD PTR [rbp-16], rax
    mov rcx, QWORD PTR [g_rhEdit]
    mov edx, EM_SETSEL
    mov r8, QWORD PTR [rbp-16]
    mov r9, QWORD PTR [rbp-16]
    call SendMessageW
    mov rcx, QWORD PTR [g_rhEdit]
    mov edx, EM_SETPARAFORMAT
    xor r8d, r8d
    lea r9, QWORD PTR [g_pf]
    call SendMessageW
    EPILOG
apply_par ENDP

; Applies one span record (rcx = pointer to record)
apply_span PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov eax, DWORD PTR [rcx]
    mov QWORD PTR [rbp-16], rax
    mov eax, DWORD PTR [rcx+4]
    add rax, QWORD PTR [rbp-16]
    mov QWORD PTR [rbp-24], rax
    mov rcx, QWORD PTR [g_rhEdit]
    mov edx, EM_SETSEL
    mov r8, QWORD PTR [rbp-16]
    mov r9, QWORD PTR [rbp-24]
    call SendMessageW
    mov rdx, QWORD PTR [rbp-8]
    mov eax, DWORD PTR [rdx+8]
    mov DWORD PTR [g_cf2+4], eax
    mov eax, DWORD PTR [rdx+12]
    mov DWORD PTR [g_cf2+8], eax
    mov eax, DWORD PTR [rdx+16]
    mov DWORD PTR [g_cf2+12], eax
    mov eax, DWORD PTR [rdx+20]
    mov DWORD PTR [g_cf2+20], eax
    mov DWORD PTR [g_cf2], 116
    test DWORD PTR [rdx+8], 20000000h
    jz as_send
    lea rcx, QWORD PTR [g_cf2+26]
    lea rdx, QWORD PTR [s_consolas]
    mov r8d, 9
as_face:
    mov ax, WORD PTR [rdx]
    mov WORD PTR [rcx], ax
    add rdx, 2
    add rcx, 2
    dec r8d
    jnz as_face
as_send:
    mov rcx, QWORD PTR [g_rhEdit]
    mov edx, EM_SETCHARFORMAT
    mov r8d, 1
    lea r9, QWORD PTR [g_cf2]
    call SendMessageW
    EPILOG
apply_span ENDP

; rcx = editor, rdx = source, r8 = length: renders the Markdown into the editor (read-only)
render_into PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    mov QWORD PTR [rbp-24], r8
    mov rax, QWORD PTR [rbp-8]
    mov QWORD PTR [g_rhEdit], rax
    mov rax, QWORD PTR [rbp-16]
    mov QWORD PTR [g_rsrc], rax
    mov rax, QWORD PTR [rbp-24]
    mov QWORD PTR [g_rcch], rax
    mov rax, QWORD PTR [g_rcch]
    imul rax, rax, 3
    add rax, 16
    mov QWORD PTR [g_spancap], rax
    shl rax, 5
    mov rcx, rax
    call heap_alloc
    mov QWORD PTR [g_spans], rax
    mov rax, QWORD PTR [g_rcch]
    inc rax
    mov QWORD PTR [g_parcap], rax
    shl rax, 4
    mov rcx, rax
    call heap_alloc
    mov QWORD PTR [g_pars], rax
    mov rax, QWORD PTR [g_rcch]
    shl rax, 3
    add rax, 1024
    mov QWORD PTR [g_plainCap], rax
    shl rax, 1
    mov rcx, rax
    call heap_alloc
    mov QWORD PTR [g_plain], rax
    mov eax, DWORD PTR [g_theme]
    shl eax, 4
    lea rdx, QWORD PTR [g_palette]
    add rdx, rax
    mov eax, DWORD PTR [rdx+8]
    mov DWORD PTR [g_accent], eax
    mov eax, DWORD PTR [rdx+12]
    mov DWORD PTR [g_muted], eax
    call md_build
    mov rcx, QWORD PTR [g_rhEdit]
    mov edx, EM_SETREADONLY
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    mov rcx, QWORD PTR [g_rhEdit]
    mov rdx, QWORD PTR [g_plain]
    call SetWindowTextW
    mov QWORD PTR [rbp-16], 0
ri_pars:
    mov rax, QWORD PTR [rbp-16]
    cmp rax, QWORD PTR [g_npars]
    jae ri_pars_done
    shl rax, 4
    add rax, QWORD PTR [g_pars]
    mov rcx, rax
    call apply_par
    inc QWORD PTR [rbp-16]
    jmp ri_pars
ri_pars_done:
    mov QWORD PTR [rbp-16], 0
ri_spans:
    mov rax, QWORD PTR [rbp-16]
    cmp rax, QWORD PTR [g_nspans]
    jae ri_spans_done
    shl rax, 5
    add rax, QWORD PTR [g_spans]
    mov rcx, rax
    call apply_span
    inc QWORD PTR [rbp-16]
    jmp ri_spans
ri_spans_done:
    mov rcx, QWORD PTR [g_rhEdit]
    mov edx, EM_SETSEL
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    mov rcx, QWORD PTR [g_rhEdit]
    mov edx, EM_SETREADONLY
    mov r8d, 1
    xor r9d, r9d
    call SendMessageW
    mov rcx, QWORD PTR [g_plain]
    call heap_free
    mov rcx, QWORD PTR [g_spans]
    call heap_free
    mov rcx, QWORD PTR [g_pars]
    call heap_free
    EPILOG
render_into ENDP

; Shows the active tab as formatted text in the editor
render_active PROC FRAME
    PROLOG
    cmp DWORD PTR [g_nTabs], 0
    je ra_done
    mov eax, DWORD PTR [g_active]
    lea r11, QWORD PTR [g_tabs]
    mov rcx, QWORD PTR [r11 + rax*8]
    mov rdx, QWORD PTR [rcx]
    mov r8d, DWORD PTR [rcx+8]
    mov rcx, QWORD PTR [g_hEdit]
    call render_into
ra_done:
    EPILOG
render_active ENDP
