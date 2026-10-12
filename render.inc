; Parses "/render <input> <output>" from the command line into g_rin and g_rout
parse_render PROC FRAME
    PROLOG
    call GetCommandLineW
    mov QWORD PTR [rbp-8], rax
pr_scan:
    mov rax, QWORD PTR [rbp-8]
    cmp WORD PTR [rax], 0
    je pr_no
    lea rcx, QWORD PTR [s_render_tok]
    mov rdx, rax
    mov r8d, 7
pr_cmp:
    test r8d, r8d
    jz pr_found
    movzx r9d, WORD PTR [rcx]
    cmp WORD PTR [rdx], r9w
    jne pr_next
    add rcx, 2
    add rdx, 2
    dec r8d
    jmp pr_cmp
pr_next:
    add QWORD PTR [rbp-8], 2
    jmp pr_scan
pr_found:
    mov rax, QWORD PTR [rbp-8]
    add rax, 14
    mov rcx, rax
    lea rdx, QWORD PTR [g_rin]
    call copy_token
    mov rcx, rax
    lea rdx, QWORD PTR [g_rout]
    call copy_token
    cmp WORD PTR [g_rin], 0
    je pr_no
    cmp WORD PTR [g_rout], 0
    je pr_no
    mov eax, 1
    EPILOG
pr_no:
    xor eax, eax
    EPILOG
parse_render ENDP


; ecx = UTF-16 unit appended to the RTF buffer
rtf_put PROC
    mov rax, QWORD PTR [g_rtfbuf]
    mov rdx, QWORD PTR [g_rtfpos]
    mov WORD PTR [rax + rdx*2], cx
    inc QWORD PTR [g_rtfpos]
    ret
rtf_put ENDP

; rcx = ASCII zero-terminated string
rtf_puts PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
rp_loop:
    mov rax, QWORD PTR [rbp-8]
    movzx ecx, BYTE PTR [rax]
    test ecx, ecx
    jz rp_done
    call rtf_put
    inc QWORD PTR [rbp-8]
    jmp rp_loop
rp_done:
    EPILOG
rtf_puts ENDP

; eax = signed value, written in decimal
rtf_putint PROC FRAME
    PROLOG
    mov DWORD PTR [rbp-8], eax
    test eax, eax
    jns ri_pos
    mov ecx, 2Dh
    call rtf_put
    neg DWORD PTR [rbp-8]
ri_pos:
    mov eax, DWORD PTR [rbp-8]
    lea r8, QWORD PTR [rbp-21]
    mov ecx, 10
ri_digit:
    xor edx, edx
    div ecx
    add dl, 30h
    dec r8
    mov BYTE PTR [r8], dl
    test eax, eax
    jnz ri_digit
    mov QWORD PTR [rbp-16], r8
ri_emit:
    mov rax, QWORD PTR [rbp-16]
    lea rdx, QWORD PTR [rbp-21]
    cmp rax, rdx
    jae ri_done
    movzx ecx, BYTE PTR [rax]
    call rtf_put
    inc QWORD PTR [rbp-16]
    jmp ri_emit
ri_done:
    EPILOG
rtf_putint ENDP

; ecx = UTF-16 unit: escaped for RTF
rtf_char PROC FRAME
    PROLOG
    mov DWORD PTR [rbp-8], ecx
    cmp ecx, 5Ch
    je rc_esc
    cmp ecx, 7Bh
    je rc_esc
    cmp ecx, 7Dh
    je rc_esc
    cmp ecx, 09h
    je rc_tab
    cmp ecx, 20h
    jb rc_done
    cmp ecx, 7Fh
    ja rc_uni
    call rtf_put
    jmp rc_done
rc_esc:
    mov ecx, 5Ch
    call rtf_put
    mov ecx, DWORD PTR [rbp-8]
    call rtf_put
    jmp rc_done
rc_tab:
    lea rcx, QWORD PTR [r_tab]
    call rtf_puts
    jmp rc_done
rc_uni:
    lea rcx, QWORD PTR [r_uni]
    call rtf_puts
    mov eax, DWORD PTR [rbp-8]
    movsx eax, ax
    call rtf_putint
    mov ecx, 3Fh
    call rtf_put
rc_done:
    EPILOG
rtf_char ENDP

; rcx = colour (COLORREF): "\redR\greenG\blueB;"
rtf_color PROC FRAME
    PROLOG
    mov DWORD PTR [rbp-8], ecx
    lea rcx, QWORD PTR [r_red]
    call rtf_puts
    mov eax, DWORD PTR [rbp-8]
    and eax, 0FFh
    call rtf_putint
    lea rcx, QWORD PTR [r_green]
    call rtf_puts
    mov eax, DWORD PTR [rbp-8]
    shr eax, 8
    and eax, 0FFh
    call rtf_putint
    lea rcx, QWORD PTR [r_blue]
    call rtf_puts
    mov eax, DWORD PTR [rbp-8]
    shr eax, 16
    and eax, 0FFh
    call rtf_putint
    mov ecx, 3Bh
    call rtf_put
    EPILOG
rtf_color ENDP

; rcx = span record: opens a group with the run's formatting
rtf_span_open PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov ecx, 7Bh
    call rtf_put
    mov rax, QWORD PTR [rbp-8]
    mov ecx, DWORD PTR [rax+12]
    mov DWORD PTR [rbp-16], ecx
    mov ecx, DWORD PTR [rax+8]
    mov DWORD PTR [rbp-24], ecx
    mov ecx, DWORD PTR [rax+16]
    mov DWORD PTR [rbp-32], ecx
    mov ecx, DWORD PTR [rax+20]
    mov DWORD PTR [rbp-40], ecx
    test DWORD PTR [rbp-16], 1
    jz rso_i
    lea rcx, QWORD PTR [r_b]
    call rtf_puts
rso_i:
    test DWORD PTR [rbp-16], 2
    jz rso_ul
    lea rcx, QWORD PTR [r_i]
    call rtf_puts
rso_ul:
    test DWORD PTR [rbp-16], 4
    jz rso_strike
    lea rcx, QWORD PTR [r_ul]
    call rtf_puts
rso_strike:
    test DWORD PTR [rbp-16], 8
    jz rso_face
    lea rcx, QWORD PTR [r_strike]
    call rtf_puts
rso_face:
    test DWORD PTR [rbp-24], 20000000h
    jz rso_color
    lea rcx, QWORD PTR [r_f1]
    call rtf_puts
rso_color:
    test DWORD PTR [rbp-24], 40000000h
    jz rso_size
    mov eax, DWORD PTR [rbp-40]
    cmp eax, DWORD PTR [g_accent]
    jne rso_muted
    lea rcx, QWORD PTR [r_cf2]
    call rtf_puts
    jmp rso_size
rso_muted:
    lea rcx, QWORD PTR [r_cf3]
    call rtf_puts
rso_size:
    test DWORD PTR [rbp-24], 80000000h
    jz rso_space
    lea rcx, QWORD PTR [r_fs]
    call rtf_puts
    mov eax, DWORD PTR [rbp-32]
    xor edx, edx
    mov ecx, 10
    div ecx
    call rtf_putint
rso_space:
    mov ecx, 20h
    call rtf_put
    EPILOG
rtf_span_open ENDP

; Writes the whole document from g_plain, g_spans and g_pars into the RTF buffer
rtf_build PROC FRAME
    PROLOG
    lea rcx, QWORD PTR [r_head]
    call rtf_puts
    call palette_ptr
    mov ecx, DWORD PTR [rax+4]
    call rtf_color
    mov eax, DWORD PTR [g_accent]
    mov ecx, eax
    call rtf_color
    mov eax, DWORD PTR [g_muted]
    mov ecx, eax
    call rtf_color
    lea rcx, QWORD PTR [r_mid]
    call rtf_puts
    mov QWORD PTR [rbp-8], 0
    mov QWORD PTR [rbp-16], 0
    mov QWORD PTR [rbp-24], 0
    mov QWORD PTR [rbp-32], 0
    mov QWORD PTR [rbp-40], 0
    mov QWORD PTR [rbp-48], 1
rb_loop:
    mov rax, QWORD PTR [rbp-8]
    cmp rax, QWORD PTR [g_plen]
    jae rb_end
    cmp DWORD PTR [rbp-32], 0
    je rb_par
    mov rax, QWORD PTR [rbp-8]
    cmp rax, QWORD PTR [rbp-40]
    jne rb_chk_open
    mov ecx, 7Dh
    call rtf_put
    mov DWORD PTR [rbp-32], 0
rb_par:
    cmp QWORD PTR [rbp-48], 0
    je rb_chk_open
    mov QWORD PTR [rbp-48], 0
    mov DWORD PTR [rbp-64], 0
    mov rax, QWORD PTR [rbp-24]
    cmp rax, QWORD PTR [g_npars]
    jae rb_pemit
    shl rax, 4
    add rax, QWORD PTR [g_pars]
    mov QWORD PTR [rbp-72], rax
    mov edx, DWORD PTR [rax]
    cmp rdx, QWORD PTR [rbp-8]
    jne rb_pemit
    mov DWORD PTR [rbp-64], 1
    inc QWORD PTR [rbp-24]
rb_pemit:
    lea rcx, QWORD PTR [r_pard]
    call rtf_puts
    cmp DWORD PTR [rbp-64], 0
    je rb_pspace
    lea rcx, QWORD PTR [r_li]
    call rtf_puts
    mov rax, QWORD PTR [rbp-72]
    mov eax, DWORD PTR [rax+4]
    call rtf_putint
    lea rcx, QWORD PTR [r_fi]
    call rtf_puts
    mov rax, QWORD PTR [rbp-72]
    mov eax, DWORD PTR [rax+8]
    call rtf_putint
rb_pspace:
    mov ecx, 20h
    call rtf_put
rb_chk_open:
    cmp DWORD PTR [rbp-32], 0
    jne rb_char
    mov rax, QWORD PTR [rbp-16]
    cmp rax, QWORD PTR [g_nspans]
    jae rb_char
    mov rax, QWORD PTR [rbp-16]
    shl rax, 5
    add rax, QWORD PTR [g_spans]
    mov QWORD PTR [rbp-56], rax
    mov edx, DWORD PTR [rax]
    cmp rdx, QWORD PTR [rbp-8]
    jne rb_char
    mov rcx, QWORD PTR [rbp-56]
    call rtf_span_open
    mov DWORD PTR [rbp-32], 1
    mov rax, QWORD PTR [rbp-56]
    mov ecx, DWORD PTR [rax]
    add ecx, DWORD PTR [rax+4]
    mov eax, ecx
    mov QWORD PTR [rbp-40], rax
    inc QWORD PTR [rbp-16]
rb_char:
    mov rax, QWORD PTR [g_plain]
    mov rdx, QWORD PTR [rbp-8]
    movzx ecx, WORD PTR [rax + rdx*2]
    cmp ecx, 0Dh
    je rb_cr
    call rtf_char
    inc QWORD PTR [rbp-8]
    jmp rb_loop
rb_cr:
    lea rcx, QWORD PTR [r_par_crlf]
    call rtf_puts
    mov QWORD PTR [rbp-48], 1
    inc QWORD PTR [rbp-8]
    jmp rb_loop
rb_end:
    cmp DWORD PTR [rbp-32], 0
    je rb_close
    mov ecx, 7Dh
    call rtf_put
rb_close:
    lea rcx, QWORD PTR [r_close_doc]
    call rtf_puts
    EPILOG
rtf_build ENDP

; Renders the input file and writes the formatted text and its runs to the output file
render_file PROC FRAME
    PROLOG
    lea rcx, QWORD PTR [g_path]
    lea rdx, QWORD PTR [g_rin]
    call copy_wstr
    mov ecx, MAX_CCH * 2
    call heap_alloc
    mov QWORD PTR [rbp-8], rax
    mov rcx, rax
    mov edx, MAX_CCH - 1
    call read_src
    mov DWORD PTR [rbp-16], eax
    mov eax, DWORD PTR [rbp-16]
    mov QWORD PTR [g_rcch], rax
    mov rax, QWORD PTR [rbp-8]
    mov QWORD PTR [g_rsrc], rax
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
    call palette_ptr
    mov rdx, rax
    mov eax, DWORD PTR [rdx+8]
    mov DWORD PTR [g_accent], eax
    mov eax, DWORD PTR [rdx+12]
    mov DWORD PTR [g_muted], eax
    call md_build
    mov DWORD PTR [rbp-60], 0
    lea rcx, QWORD PTR [g_rout]
    xor eax, eax
rf_len:
    cmp WORD PTR [rcx + rax*2], 0
    je rf_len_done
    inc eax
    jmp rf_len
rf_len_done:
    cmp eax, 4
    jb rf_fmt_done
    lea rdx, QWORD PTR [rcx + rax*2 - 8]
    cmp WORD PTR [rdx], 2Eh
    jne rf_fmt_done
    cmp WORD PTR [rdx+2], 72h
    jne rf_fmt_done
    cmp WORD PTR [rdx+4], 74h
    jne rf_fmt_done
    cmp WORD PTR [rdx+6], 66h
    jne rf_fmt_done
    mov DWORD PTR [rbp-60], 1
rf_fmt_done:
    mov rax, QWORD PTR [g_nspans]
    imul rax, rax, 64
    mov rcx, QWORD PTR [g_npars]
    imul rcx, rcx, 64
    add rax, rcx
    add rax, QWORD PTR [g_plen]
    add rax, 512
    mov QWORD PTR [rbp-24], rax
    shl rax, 1
    mov rcx, rax
    call heap_alloc
    mov QWORD PTR [rbp-32], rax
    mov QWORD PTR [rbp-40], 0
rf_copy:
    mov rax, QWORD PTR [rbp-40]
    cmp rax, QWORD PTR [g_plen]
    jae rf_copied
    mov rdx, QWORD PTR [g_plain]
    movzx r8d, WORD PTR [rdx + rax*2]
    mov rdx, QWORD PTR [rbp-32]
    mov WORD PTR [rdx + rax*2], r8w
    inc QWORD PTR [rbp-40]
    jmp rf_copy
rf_copied:
    mov rax, QWORD PTR [rbp-32]
    mov rcx, QWORD PTR [g_plen]
    lea rax, QWORD PTR [rax + rcx*2]
    mov QWORD PTR [rbp-48], rax
    mov rcx, rax
    lea rdx, QWORD PTR [s_sep1]
    call copy_wstr
    mov rcx, QWORD PTR [rbp-48]
    lea rcx, QWORD PTR [rcx + rax*2]
    mov QWORD PTR [rbp-48], rcx
    mov QWORD PTR [rbp-40], 0
rf_span:
    mov rax, QWORD PTR [rbp-40]
    cmp rax, QWORD PTR [g_nspans]
    jae rf_span_done
    shl rax, 5
    add rax, QWORD PTR [g_spans]
    mov QWORD PTR [rbp-56], rax
    mov rcx, QWORD PTR [rbp-48]
    lea rdx, QWORD PTR [fmt_span]
    mov r8d, DWORD PTR [rax]
    mov r9d, DWORD PTR [rax+4]
    mov r10d, DWORD PTR [rax+8]
    mov DWORD PTR [rsp+32], r10d
    mov r10d, DWORD PTR [rax+12]
    mov DWORD PTR [rsp+40], r10d
    mov r10d, DWORD PTR [rax+16]
    mov DWORD PTR [rsp+48], r10d
    call wsprintfW
    mov ecx, eax
    mov rax, QWORD PTR [rbp-48]
    lea rax, QWORD PTR [rax + rcx*2]
    mov QWORD PTR [rbp-48], rax
    inc QWORD PTR [rbp-40]
    jmp rf_span
rf_span_done:
    mov rcx, QWORD PTR [rbp-48]
    lea rdx, QWORD PTR [s_sep2]
    call copy_wstr
    mov rcx, QWORD PTR [rbp-48]
    lea rcx, QWORD PTR [rcx + rax*2]
    mov QWORD PTR [rbp-48], rcx
    mov QWORD PTR [rbp-40], 0
rf_par:
    mov rax, QWORD PTR [rbp-40]
    cmp rax, QWORD PTR [g_npars]
    jae rf_par_done
    shl rax, 4
    add rax, QWORD PTR [g_pars]
    mov rdx, rax
    mov rcx, QWORD PTR [rbp-48]
    lea rdx, QWORD PTR [fmt_par]
    mov r8d, DWORD PTR [rax]
    mov r9d, DWORD PTR [rax+4]
    mov r10d, DWORD PTR [rax+8]
    mov DWORD PTR [rsp+32], r10d
    call wsprintfW
    mov ecx, eax
    mov rax, QWORD PTR [rbp-48]
    lea rax, QWORD PTR [rax + rcx*2]
    mov QWORD PTR [rbp-48], rax
    inc QWORD PTR [rbp-40]
    jmp rf_par
rf_par_done:
    mov rax, QWORD PTR [rbp-48]
    mov rcx, QWORD PTR [rbp-32]
    sub rax, rcx
    shr rax, 1
    mov QWORD PTR [rbp-56], rax
    lea rcx, QWORD PTR [g_path]
    lea rdx, QWORD PTR [g_rout]
    call copy_wstr
    lea rcx, QWORD PTR [g_path]
    lea rdx, QWORD PTR [g_rout]
    call copy_wstr
    cmp DWORD PTR [rbp-60], 0
    je rf_write_dump
    mov rax, QWORD PTR [g_plen]
    imul rax, rax, 12
    mov rcx, QWORD PTR [g_nspans]
    imul rcx, rcx, 64
    add rax, rcx
    mov rcx, QWORD PTR [g_npars]
    imul rcx, rcx, 48
    add rax, rcx
    add rax, 2048
    shl rax, 1
    mov rcx, rax
    call heap_alloc
    mov QWORD PTR [g_rtfbuf], rax
    mov QWORD PTR [g_rtfpos], 0
    call rtf_build
    mov rcx, QWORD PTR [g_rtfbuf]
    mov rdx, QWORD PTR [g_rtfpos]
    call write_src
    xor ecx, ecx
    call ExitProcess
rf_write_dump:
    mov rcx, QWORD PTR [rbp-32]
    mov rdx, QWORD PTR [rbp-56]
    call write_src
    xor ecx, ecx
    call ExitProcess
render_file ENDP
