; rcx = bytes -> rax = zeroed heap block
heap_alloc PROC FRAME
    PROLOG
    mov r8, rcx
    mov rcx, QWORD PTR [g_hHeap]
    mov edx, 8
    call HeapAlloc
    EPILOG
heap_alloc ENDP

; rcx = block
heap_free PROC FRAME
    PROLOG
    mov r8, rcx
    mov rcx, QWORD PTR [g_hHeap]
    xor edx, edx
    call HeapFree
    EPILOG
heap_free ENDP

; rcx = editor: applies the current theme's background and default text colour
style_editor PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov eax, DWORD PTR [g_theme]
    shl eax, 4
    lea rdx, QWORD PTR [g_palette]
    add rdx, rax
    mov QWORD PTR [rbp-16], rdx
    mov r9d, DWORD PTR [rdx]
    mov rcx, QWORD PTR [rbp-8]
    mov edx, EM_SETBKGNDCOLOR
    xor r8d, r8d
    call SendMessageW
    mov rdx, QWORD PTR [rbp-16]
    mov eax, DWORD PTR [rdx+4]
    mov DWORD PTR [g_cf+20], eax
    mov DWORD PTR [g_cf], 116
    mov DWORD PTR [g_cf+4], CFM_COLOR
    mov rcx, QWORD PTR [rbp-8]
    mov edx, EM_SETCHARFORMAT
    xor r8d, r8d
    lea r9, QWORD PTR [g_cf]
    call SendMessageW
    mov rcx, QWORD PTR [rbp-8]
    mov edx, EM_SETCHARFORMAT
    mov r8d, SCF_ALL
    lea r9, QWORD PTR [g_cf]
    call SendMessageW
    EPILOG
style_editor ENDP

; eax = value in pixels at 96 dpi -> eax scaled to the window's dpi
scale_px PROC
    imul eax, DWORD PTR [g_dpi]
    xor edx, edx
    mov ecx, 96
    div ecx
    ret
scale_px ENDP

; -> rax = pointer to the palette entry for the active theme
palette_ptr PROC
    mov eax, DWORD PTR [g_theme]
    shl eax, 4
    lea rdx, QWORD PTR [g_palette]
    add rax, rdx
    ret
palette_ptr ENDP

; rcx = device context, rdx = RECT*, r8d = colour
fill_rect_color PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    mov ecx, r8d
    call CreateSolidBrush
    mov QWORD PTR [rbp-24], rax
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rbp-16]
    mov r8, QWORD PTR [rbp-24]
    call FillRect
    mov rcx, QWORD PTR [rbp-24]
    call DeleteObject
    EPILOG
fill_rect_color ENDP

; rcx = device context, rdx = RECT*, r8d = colour: draws a one-pixel outline
frame_rect_color PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    mov ecx, r8d
    call CreateSolidBrush
    mov QWORD PTR [rbp-24], rax
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rbp-16]
    mov r8, QWORD PTR [rbp-24]
    call FrameRect
    mov rcx, QWORD PTR [rbp-24]
    call DeleteObject
    EPILOG
frame_rect_color ENDP

; rcx = source, rdx = destination: copies one space-separated token, returns the next source position
copy_token PROC
ct_skip:
    cmp WORD PTR [rcx], 20h
    jne ct_copy
    add rcx, 2
    jmp ct_skip
ct_copy:
    movzx eax, WORD PTR [rcx]
    cmp eax, 20h
    jbe ct_end
    mov WORD PTR [rdx], ax
    add rcx, 2
    add rdx, 2
    jmp ct_copy
ct_end:
    mov WORD PTR [rdx], 0
    mov rax, rcx
    ret
copy_token ENDP

; rcx = destination, rdx = source: copies a zero-terminated string, returns its length in eax
copy_wstr PROC
    xor eax, eax
cw_loop:
    movzx r8d, WORD PTR [rdx + rax*2]
    mov WORD PTR [rcx + rax*2], r8w
    test r8d, r8d
    jz cw_done
    inc eax
    jmp cw_loop
cw_done:
    ret
copy_wstr ENDP
