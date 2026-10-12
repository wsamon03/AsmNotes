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
    mov rcx, QWORD PTR [rbp-32]
    mov rdx, QWORD PTR [rbp-56]
    call write_src
    xor ecx, ecx
    call ExitProcess
render_file ENDP
