; ecx = pinned flag: sets the window's always-on-top state and its system menu check
apply_pin PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov DWORD PTR [rbp-16], edx
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, -2
    cmp DWORD PTR [rbp-16], 0
    je ap_set
    mov rdx, -1
ap_set:
    xor r8d, r8d
    xor r9d, r9d
    mov DWORD PTR [rsp+32], 0
    mov DWORD PTR [rsp+40], 0
    mov DWORD PTR [rsp+48], 3
    call SetWindowPos
    mov rcx, QWORD PTR [rbp-8]
    xor edx, edx
    mov r8d, 1
    call InvalidateRect
    EPILOG
apply_pin ENDP

; rcx = source buffer (owned by the sticky), rdx = length, r8d = x, r9d = y,
; [rbp+48] = width, [rbp+56] = height, [rbp+64] = pinned
create_sticky PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    mov DWORD PTR [rbp-24], r8d
    mov DWORD PTR [rbp-32], r9d
    mov eax, DWORD PTR [rbp+48]
    mov DWORD PTR [rbp-40], eax
    mov eax, DWORD PTR [rbp+56]
    mov DWORD PTR [rbp-48], eax
    mov eax, DWORD PTR [rbp+64]
    mov DWORD PTR [rbp-56], eax
    mov ecx, 48
    call heap_alloc
    mov QWORD PTR [rbp-64], rax
    mov rcx, QWORD PTR [rbp-8]
    mov QWORD PTR [rax], rcx
    mov rdx, QWORD PTR [rbp-16]
    mov DWORD PTR [rax+8], edx
    mov edx, DWORD PTR [rbp-56]
    mov DWORD PTR [rax+32], edx
    mov rcx, QWORD PTR [rbp-64]
    mov eax, DWORD PTR [g_nstk]
    lea r11, QWORD PTR [g_stk]
    mov QWORD PTR [r11 + rax*8], rcx
    inc DWORD PTR [g_nstk]
    mov ecx, WS_EX_TOOLWINDOW
    lea rdx, QWORD PTR [s_cls_sticky]
    lea r8, QWORD PTR [s_sticky_title]
    mov r9d, WS_POPUP + WS_THICKFRAME + WS_VISIBLE
    mov eax, DWORD PTR [rbp-24]
    mov DWORD PTR [rsp+32], eax
    mov eax, DWORD PTR [rbp-32]
    mov DWORD PTR [rsp+40], eax
    mov eax, DWORD PTR [rbp-40]
    mov DWORD PTR [rsp+48], eax
    mov eax, DWORD PTR [rbp-48]
    mov DWORD PTR [rsp+56], eax
    mov QWORD PTR [rsp+64], 0
    mov QWORD PTR [rsp+72], 0
    mov rax, QWORD PTR [g_hInst]
    mov QWORD PTR [rsp+80], rax
    mov rax, QWORD PTR [rbp-64]
    mov QWORD PTR [rsp+88], rax
    call CreateWindowExW
    test rax, rax
    jnz cs_ok
    dec DWORD PTR [g_nstk]
    mov rcx, QWORD PTR [rbp-8]
    call heap_free
    mov rcx, QWORD PTR [rbp-64]
    call heap_free
cs_ok:
    EPILOG
create_sticky ENDP

; Takes a snapshot of the active tab into a new sticky window
send_sticky PROC FRAME
    PROLOG
    cmp DWORD PTR [g_nstk], MAX_STK
    jae ss_done
    cmp DWORD PTR [g_nTabs], 0
    je ss_done
    call sync_active
    mov ecx, MAX_CCH * 2
    call heap_alloc
    mov QWORD PTR [rbp-8], rax
    mov eax, DWORD PTR [g_active]
    lea r11, QWORD PTR [g_tabs]
    mov rcx, QWORD PTR [r11 + rax*8]
    mov rdx, QWORD PTR [rcx]
    mov QWORD PTR [rbp-16], rdx
    mov eax, DWORD PTR [rcx+8]
    mov QWORD PTR [rbp-24], rax
    xor ecx, ecx
ss_copy:
    cmp rcx, QWORD PTR [rbp-24]
    jae ss_copied
    mov rdx, QWORD PTR [rbp-16]
    mov ax, WORD PTR [rdx + rcx*2]
    mov rdx, QWORD PTR [rbp-8]
    mov WORD PTR [rdx + rcx*2], ax
    inc rcx
    jmp ss_copy
ss_copied:
    mov rdx, QWORD PTR [rbp-8]
    mov rax, QWORD PTR [rbp-24]
    mov WORD PTR [rdx + rax*2], 0
    mov eax, DWORD PTR [g_nstk]
    imul eax, eax, 32
    add eax, 120
    mov DWORD PTR [rbp-32], eax
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rbp-24]
    mov r8d, DWORD PTR [rbp-32]
    mov r9d, DWORD PTR [rbp-32]
    mov DWORD PTR [rsp+32], 320
    mov DWORD PTR [rsp+40], 360
    mov DWORD PTR [rsp+48], 0
    call create_sticky
    call save_all
ss_done:
    EPILOG
send_sticky ENDP

; rcx = sticky record: removes it from the list and frees it
sticky_remove PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    lea r11, QWORD PTR [g_stk]
    xor ecx, ecx
sr_find:
    cmp ecx, DWORD PTR [g_nstk]
    jae sr_done
    mov rax, QWORD PTR [r11 + rcx*8]
    cmp rax, QWORD PTR [rbp-8]
    je sr_found
    inc ecx
    jmp sr_find
sr_found:
    mov eax, DWORD PTR [g_nstk]
    dec eax
sr_shift:
    cmp ecx, eax
    jae sr_shifted
    mov rdx, QWORD PTR [r11 + rcx*8 + 8]
    mov QWORD PTR [r11 + rcx*8], rdx
    inc ecx
    jmp sr_shift
sr_shifted:
    dec DWORD PTR [g_nstk]
    mov rcx, QWORD PTR [rbp-8]
    mov rcx, QWORD PTR [rcx]
    call heap_free
    mov rcx, QWORD PTR [rbp-8]
    call heap_free
sr_done:
    EPILOG
sticky_remove ENDP

; Re-renders every sticky in the current theme
update_stickies PROC FRAME
    PROLOG
    mov DWORD PTR [rbp-8], 0
us_loop:
    mov eax, DWORD PTR [rbp-8]
    cmp eax, DWORD PTR [g_nstk]
    jae us_done
    lea r11, QWORD PTR [g_stk]
    mov rax, QWORD PTR [r11 + rax*8]
    mov QWORD PTR [rbp-16], rax
    mov rcx, QWORD PTR [rax+24]
    call style_editor
    mov rax, QWORD PTR [rbp-16]
    mov rcx, QWORD PTR [rax+24]
    mov rdx, QWORD PTR [rax]
    mov r8d, DWORD PTR [rax+8]
    call render_into
    inc DWORD PTR [rbp-8]
    jmp us_loop
us_done:
    EPILOG
update_stickies ENDP

; rcx = sticky window, rdx = screen point: hit-test code for the custom frame
sticky_hit PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    lea rdx, QWORD PTR [rbp-56]
    mov rcx, QWORD PTR [rbp-8]
    call GetWindowRect
    movsx eax, WORD PTR [rbp-16]
    sub eax, DWORD PTR [rbp-56]
    mov DWORD PTR [rbp-24], eax
    movsx eax, WORD PTR [rbp-14]
    sub eax, DWORD PTR [rbp-52]
    mov DWORD PTR [rbp-28], eax
    mov eax, DWORD PTR [rbp-48]
    sub eax, DWORD PTR [rbp-56]
    mov DWORD PTR [rbp-32], eax
    mov eax, DWORD PTR [rbp-44]
    sub eax, DWORD PTR [rbp-52]
    mov DWORD PTR [rbp-36], eax
    mov eax, 6
    call scale_px
    mov DWORD PTR [rbp-40], eax
    mov eax, 26
    call scale_px
    mov DWORD PTR [rbp-88], eax
    mov eax, DWORD PTR [rbp-24]
    mov ecx, DWORD PTR [rbp-40]
    xor r8d, r8d
    cmp eax, ecx
    jge sh_nl
    mov r8d, 1
sh_nl:
    mov edx, DWORD PTR [rbp-32]
    sub edx, ecx
    xor r9d, r9d
    cmp eax, edx
    jl sh_nr
    mov r9d, 1
sh_nr:
    mov eax, DWORD PTR [rbp-28]
    xor r10d, r10d
    cmp eax, ecx
    jge sh_nt
    mov r10d, 1
sh_nt:
    mov edx, DWORD PTR [rbp-36]
    sub edx, ecx
    xor r11d, r11d
    cmp eax, edx
    jl sh_nb
    mov r11d, 1
sh_nb:
    test r10d, r10d
    jz sh_b_check
    test r8d, r8d
    jz sh_t_r
    mov eax, 13
    EPILOG
sh_t_r:
    test r9d, r9d
    jz sh_t_only
    mov eax, 14
    EPILOG
sh_t_only:
    mov eax, 12
    EPILOG
sh_b_check:
    test r11d, r11d
    jz sh_side
    test r8d, r8d
    jz sh_b_r
    mov eax, 16
    EPILOG
sh_b_r:
    test r9d, r9d
    jz sh_b_only
    mov eax, 17
    EPILOG
sh_b_only:
    mov eax, 15
    EPILOG
sh_side:
    test r8d, r8d
    jz sh_r
    mov eax, 10
    EPILOG
sh_r:
    test r9d, r9d
    jz sh_inside
    mov eax, 11
    EPILOG
sh_inside:
    mov eax, DWORD PTR [rbp-28]
    mov ecx, DWORD PTR [rbp-40]
    add ecx, DWORD PTR [rbp-88]
    cmp eax, ecx
    jge sh_client
    mov edx, DWORD PTR [rbp-32]
    sub edx, DWORD PTR [rbp-40]
    sub edx, DWORD PTR [rbp-88]
    mov ecx, DWORD PTR [rbp-24]
    cmp ecx, edx
    jge sh_client
    sub edx, DWORD PTR [rbp-88]
    cmp ecx, edx
    jge sh_client
    mov eax, 2
    EPILOG
sh_client:
    mov eax, 1
    EPILOG
sticky_hit ENDP

; rcx = sticky window: paints the custom caption, the border and the three controls
sticky_paint PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-24], rcx
    lea rdx, QWORD PTR [rbp-160]
    call BeginPaint
    mov QWORD PTR [rbp-32], rax
    mov rcx, QWORD PTR [rbp-24]
    lea rdx, QWORD PTR [rbp-88]
    call GetClientRect
    mov rcx, QWORD PTR [rbp-24]
    mov edx, GWLP_USERDATA
    call GetWindowLongPtrW
    mov QWORD PTR [rbp-16], rax
    mov eax, 6
    call scale_px
    mov DWORD PTR [rbp-40], eax
    mov eax, 26
    call scale_px
    mov DWORD PTR [rbp-36], eax
    call palette_ptr
    mov r8d, DWORD PTR [rax+8]
    mov rcx, QWORD PTR [rbp-32]
    lea rdx, QWORD PTR [rbp-88]
    call fill_rect_color
    mov eax, DWORD PTR [rbp-40]
    mov DWORD PTR [rbp-72], eax
    mov DWORD PTR [rbp-68], eax
    mov ecx, DWORD PTR [rbp-80]
    sub ecx, DWORD PTR [rbp-40]
    mov DWORD PTR [rbp-64], ecx
    mov ecx, DWORD PTR [rbp-40]
    add ecx, DWORD PTR [rbp-36]
    mov DWORD PTR [rbp-60], ecx
    call palette_ptr
    mov r8d, DWORD PTR [rax]
    mov rcx, QWORD PTR [rbp-32]
    lea rdx, QWORD PTR [rbp-72]
    call fill_rect_color
    call palette_ptr
    mov edx, DWORD PTR [rax+4]
    mov rcx, QWORD PTR [rbp-32]
    call SetTextColor
    mov rcx, QWORD PTR [rbp-32]
    mov edx, 1
    call SetBkMode
    mov eax, DWORD PTR [rbp-80]
    sub eax, DWORD PTR [rbp-40]
    sub eax, DWORD PTR [rbp-36]
    sub eax, DWORD PTR [rbp-36]
    mov edx, eax
    mov eax, DWORD PTR [rbp-40]
    add eax, 8
    mov DWORD PTR [rbp-56], eax
    mov eax, DWORD PTR [rbp-40]
    mov DWORD PTR [rbp-52], eax
    mov DWORD PTR [rbp-48], edx
    mov eax, DWORD PTR [rbp-40]
    add eax, DWORD PTR [rbp-36]
    mov DWORD PTR [rbp-44], eax
    mov rcx, QWORD PTR [rbp-32]
    lea rdx, QWORD PTR [s_sticky_title]
    mov r8d, -1
    lea r9, QWORD PTR [rbp-56]
    mov DWORD PTR [rsp+32], 8024h
    call DrawTextW
    mov eax, DWORD PTR [rbp-80]
    sub eax, DWORD PTR [rbp-40]
    sub eax, DWORD PTR [rbp-36]
    sub eax, DWORD PTR [rbp-36]
    mov DWORD PTR [rbp-72], eax
    add eax, DWORD PTR [rbp-36]
    mov DWORD PTR [rbp-64], eax
    mov eax, DWORD PTR [rbp-40]
    mov DWORD PTR [rbp-68], eax
    mov ecx, DWORD PTR [rbp-40]
    add ecx, DWORD PTR [rbp-36]
    mov DWORD PTR [rbp-60], ecx
    mov rcx, QWORD PTR [rbp-32]
    lea rdx, QWORD PTR [rbp-72]
    mov r8d, 1
    mov r9, QWORD PTR [rbp-16]
    call paint_button
    mov rax, QWORD PTR [rbp-16]
    lea rdx, QWORD PTR [s_pin_txt]
    cmp DWORD PTR [rax+32], 0
    je sp_pin_label
    lea rdx, QWORD PTR [s_unpin_txt]
sp_pin_label:
    mov rcx, QWORD PTR [rbp-32]
    mov r8d, -1
    lea r9, QWORD PTR [rbp-72]
    mov DWORD PTR [rsp+32], 25h
    call DrawTextW
    mov eax, DWORD PTR [rbp-64]
    mov DWORD PTR [rbp-72], eax
    mov eax, DWORD PTR [rbp-80]
    sub eax, DWORD PTR [rbp-40]
    mov DWORD PTR [rbp-64], eax
    mov rcx, QWORD PTR [rbp-32]
    lea rdx, QWORD PTR [rbp-72]
    mov r8d, 2
    mov r9, QWORD PTR [rbp-16]
    call paint_button
    mov rcx, QWORD PTR [rbp-32]
    lea rdx, QWORD PTR [s_close_txt]
    mov r8d, -1
    lea r9, QWORD PTR [rbp-72]
    mov DWORD PTR [rsp+32], 25h
    call DrawTextW
    mov rcx, QWORD PTR [rbp-24]
    lea rdx, QWORD PTR [rbp-160]
    call EndPaint
    EPILOG
sticky_paint ENDP

; rcx = sticky window, rdx = client point: presses the pin or close button
sticky_click PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    lea rdx, QWORD PTR [rbp-88]
    mov rcx, QWORD PTR [rbp-8]
    call GetClientRect
    mov eax, 6
    call scale_px
    mov DWORD PTR [rbp-40], eax
    mov eax, 26
    call scale_px
    mov DWORD PTR [rbp-36], eax
    movzx eax, WORD PTR [rbp-16]
    mov DWORD PTR [rbp-24], eax
    movzx eax, WORD PTR [rbp-14]
    mov DWORD PTR [rbp-28], eax
    cmp eax, DWORD PTR [rbp-40]
    jl sc_none
    mov ecx, DWORD PTR [rbp-40]
    add ecx, DWORD PTR [rbp-36]
    cmp eax, ecx
    jge sc_none
    mov edx, DWORD PTR [rbp-80]
    sub edx, DWORD PTR [rbp-40]
    sub edx, DWORD PTR [rbp-36]
    cmp DWORD PTR [rbp-24], edx
    jl sc_pin
    mov rcx, QWORD PTR [rbp-8]
    mov edx, WM_CLOSE
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    jmp sc_none
sc_pin:
    sub edx, DWORD PTR [rbp-36]
    cmp DWORD PTR [rbp-24], edx
    jl sc_none
    mov rcx, QWORD PTR [rbp-8]
    mov edx, GWLP_USERDATA
    call GetWindowLongPtrW
    mov QWORD PTR [rbp-48], rax
    mov rax, QWORD PTR [rbp-48]
    xor DWORD PTR [rax+32], 1
    mov rax, QWORD PTR [rbp-48]
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rax+32]
    call apply_pin
    call save_all
sc_none:
    xor eax, eax
    EPILOG
sticky_click ENDP


; rcx = sticky window, rdx = client point: 0 none, 1 pin button, 2 close button
caption_button_at PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    lea rdx, QWORD PTR [rbp-88]
    mov rcx, QWORD PTR [rbp-8]
    call GetClientRect
    mov eax, 6
    call scale_px
    mov DWORD PTR [rbp-40], eax
    mov eax, 26
    call scale_px
    mov DWORD PTR [rbp-36], eax
    movzx eax, WORD PTR [rbp-16]
    mov DWORD PTR [rbp-24], eax
    movzx eax, WORD PTR [rbp-14]
    cmp eax, DWORD PTR [rbp-40]
    jl cba_none
    mov ecx, DWORD PTR [rbp-40]
    add ecx, DWORD PTR [rbp-36]
    cmp eax, ecx
    jge cba_none
    mov edx, DWORD PTR [rbp-80]
    sub edx, DWORD PTR [rbp-40]
    sub edx, DWORD PTR [rbp-36]
    cmp DWORD PTR [rbp-24], edx
    jl cba_pin
    mov eax, 2
    EPILOG
cba_pin:
    sub edx, DWORD PTR [rbp-36]
    cmp DWORD PTR [rbp-24], edx
    jl cba_none
    mov eax, 1
    EPILOG
cba_none:
    xor eax, eax
    EPILOG
caption_button_at ENDP

; rcx = sticky window, edx = hovered button, r8d = pressed button: redraws when either changes
sticky_setstate PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov DWORD PTR [rbp-12], edx
    mov DWORD PTR [rbp-16], r8d
    mov rcx, QWORD PTR [rbp-8]
    mov edx, GWLP_USERDATA
    call GetWindowLongPtrW
    mov QWORD PTR [rbp-24], rax
    mov ecx, DWORD PTR [rbp-12]
    cmp ecx, DWORD PTR [rax+40]
    jne ss_set
    mov ecx, DWORD PTR [rbp-16]
    cmp ecx, DWORD PTR [rax+44]
    je ss_done
ss_set:
    mov ecx, DWORD PTR [rbp-12]
    mov DWORD PTR [rax+40], ecx
    mov ecx, DWORD PTR [rbp-16]
    mov DWORD PTR [rax+44], ecx
    mov rcx, QWORD PTR [rbp-8]
    xor edx, edx
    mov r8d, 1
    call InvalidateRect
ss_done:
    EPILOG
sticky_setstate ENDP

sticky_mousemove PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rbp-16]
    call caption_button_at
    mov DWORD PTR [rbp-24], eax
    mov DWORD PTR [rbp-88], 24
    mov DWORD PTR [rbp-84], 2
    mov rax, QWORD PTR [rbp-8]
    mov QWORD PTR [rbp-80], rax
    mov DWORD PTR [rbp-72], 0
    lea rcx, QWORD PTR [rbp-88]
    call TrackMouseEvent
    mov rcx, QWORD PTR [rbp-8]
    mov edx, GWLP_USERDATA
    call GetWindowLongPtrW
    mov r8d, DWORD PTR [rax+44]
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rbp-24]
    call sticky_setstate
    EPILOG
sticky_mousemove ENDP

; rcx = sticky window: the pointer left the window
sticky_mouseleave PROC FRAME
    PROLOG
    xor edx, edx
    xor r8d, r8d
    call sticky_setstate
    EPILOG
sticky_mouseleave ENDP

; rcx = sticky window, rdx = client point: a button was pressed
sticky_mousedown PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rbp-16]
    call caption_button_at
    mov DWORD PTR [rbp-24], eax
    mov rcx, QWORD PTR [rbp-8]
    mov edx, GWLP_USERDATA
    call GetWindowLongPtrW
    mov edx, DWORD PTR [rax+40]
    mov r8d, DWORD PTR [rbp-24]
    mov rcx, QWORD PTR [rbp-8]
    call sticky_setstate
    EPILOG
sticky_mousedown ENDP

; rcx = sticky window: the press is over (the click handler runs next)
sticky_release PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov rcx, QWORD PTR [rbp-8]
    mov edx, GWLP_USERDATA
    call GetWindowLongPtrW
    mov edx, DWORD PTR [rax+40]
    xor r8d, r8d
    mov rcx, QWORD PTR [rbp-8]
    call sticky_setstate
    EPILOG
sticky_release ENDP

; rcx = hdc, rdx = button rectangle, r8d = button id, r9 = sticky record
paint_button PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    mov DWORD PTR [rbp-24], r8d
    mov QWORD PTR [rbp-32], r9
    mov rax, QWORD PTR [rbp-32]
    mov ecx, DWORD PTR [rax+44]
    cmp ecx, DWORD PTR [rbp-24]
    jne pb_hover
    call palette_ptr
    mov r8d, DWORD PTR [rax+8]
    jmp pb_fill
pb_hover:
    mov rax, QWORD PTR [rbp-32]
    mov ecx, DWORD PTR [rax+40]
    cmp ecx, DWORD PTR [rbp-24]
    jne pb_done
    call palette_ptr
    mov r8d, DWORD PTR [rax+12]
pb_fill:
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rbp-16]
    call fill_rect_color
pb_done:
    EPILOG
paint_button ENDP

; Sticky window procedure
sticky_proc PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov eax, edx
    mov QWORD PTR [rbp-16], rax
    mov QWORD PTR [rbp-24], r8
    mov QWORD PTR [rbp-32], r9
    mov eax, DWORD PTR [rbp-16]
    cmp eax, WM_CREATE
    je sp_create
    cmp eax, WM_SIZE
    je sp_size
    cmp eax, WM_NCCALCSIZE
    je sp_nccalc
    cmp eax, WM_NCHITTEST
    je sp_hit
    cmp eax, WM_PAINT
    je sp_paint
    cmp eax, WM_LBUTTONUP
    je sp_click
    cmp eax, WM_MOUSEMOVE
    je sp_mousemove
    cmp eax, WM_MOUSELEAVE
    je sp_mouseleave
    cmp eax, WM_LBUTTONDOWN
    je sp_mousedown
    cmp eax, WM_EXITSIZEMOVE
    je sp_moved
    cmp eax, WM_CLOSE
    je sp_close
sp_default:
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rbp-16]
    mov r8, QWORD PTR [rbp-24]
    mov r9, QWORD PTR [rbp-32]
    call DefWindowProcW
    EPILOG
sp_zero:
    xor eax, eax
    EPILOG

sp_nccalc:
    cmp QWORD PTR [rbp-24], 0
    je sp_default
    jmp sp_zero

sp_hit:
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rbp-32]
    call sticky_hit
    EPILOG

sp_paint:
    mov rcx, QWORD PTR [rbp-8]
    call sticky_paint
    jmp sp_zero

sp_click:
    mov rcx, QWORD PTR [rbp-8]
    call sticky_release
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rbp-32]
    call sticky_click
    jmp sp_zero
sp_mousemove:
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rbp-32]
    call sticky_mousemove
    jmp sp_zero
sp_mouseleave:
    mov rcx, QWORD PTR [rbp-8]
    call sticky_mouseleave
    jmp sp_zero
sp_mousedown:
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rbp-32]
    call sticky_mousedown
    jmp sp_zero

sp_create:
    mov rax, QWORD PTR [rbp-32]
    mov rax, QWORD PTR [rax]
    mov QWORD PTR [rbp-40], rax
    mov rcx, QWORD PTR [rbp-8]
    mov edx, GWLP_USERDATA
    mov r8, QWORD PTR [rbp-40]
    call SetWindowLongPtrW
    mov rax, QWORD PTR [rbp-40]
    mov rcx, QWORD PTR [rbp-8]
    mov QWORD PTR [rax+16], rcx
    xor ecx, ecx
    lea rdx, QWORD PTR [s_richedit]
    xor r8d, r8d
    mov r9d, WS_CHILD + WS_VISIBLE + WS_VSCROLL + ES_MULTILINE + ES_AUTOVSCROLL + ES_NOHIDESEL
    mov DWORD PTR [rsp+32], 0
    mov DWORD PTR [rsp+40], 0
    mov DWORD PTR [rsp+48], 0
    mov DWORD PTR [rsp+56], 0
    mov rax, QWORD PTR [rbp-8]
    mov QWORD PTR [rsp+64], rax
    mov QWORD PTR [rsp+72], IDC_STICKY_EDITOR
    mov rax, QWORD PTR [g_hInst]
    mov QWORD PTR [rsp+80], rax
    mov QWORD PTR [rsp+88], 0
    call CreateWindowExW
    mov rcx, QWORD PTR [rbp-40]
    mov QWORD PTR [rcx+24], rax
    mov rcx, rax
    call style_editor
    mov rax, QWORD PTR [rbp-40]
    mov rcx, QWORD PTR [rax+24]
    mov rdx, QWORD PTR [rax]
    mov r8d, DWORD PTR [rax+8]
    call render_into
    mov rax, QWORD PTR [rbp-40]
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rax+32]
    call apply_pin
    jmp sp_zero

sp_size:
    mov rcx, QWORD PTR [rbp-8]
    mov edx, GWLP_USERDATA
    call GetWindowLongPtrW
    mov QWORD PTR [rbp-40], rax
    mov rcx, QWORD PTR [rax+24]
    mov QWORD PTR [rbp-48], rcx
    movzx eax, WORD PTR [rbp-32]
    mov DWORD PTR [rbp-56], eax
    movzx eax, WORD PTR [rbp-30]
    mov DWORD PTR [rbp-64], eax
    mov eax, 6
    call scale_px
    mov DWORD PTR [rbp-72], eax
    mov eax, 26
    call scale_px
    mov DWORD PTR [rbp-80], eax
    mov rcx, QWORD PTR [rbp-48]
    mov edx, DWORD PTR [rbp-72]
    mov r8d, DWORD PTR [rbp-72]
    add r8d, DWORD PTR [rbp-80]
    mov r9d, DWORD PTR [rbp-56]
    sub r9d, DWORD PTR [rbp-72]
    sub r9d, DWORD PTR [rbp-72]
    mov eax, DWORD PTR [rbp-64]
    sub eax, DWORD PTR [rbp-72]
    sub eax, DWORD PTR [rbp-72]
    sub eax, DWORD PTR [rbp-80]
    mov DWORD PTR [rsp+32], eax
    mov DWORD PTR [rsp+40], 1
    call MoveWindow
    jmp sp_zero

sp_moved:
    call save_all
    jmp sp_zero

sp_close:
    mov rcx, QWORD PTR [rbp-8]
    mov edx, GWLP_USERDATA
    call GetWindowLongPtrW
    mov rcx, rax
    call sticky_remove
    mov rcx, QWORD PTR [rbp-8]
    call DestroyWindow
    call save_all
    jmp sp_zero
sticky_proc ENDP

; Registers the sticky window class
reg_sticky_class PROC FRAME
    PROLOG
    mov DWORD PTR [rbp-96], 80
    mov DWORD PTR [rbp-92], CS_HREDRAW + CS_VREDRAW
    lea rax, QWORD PTR [sticky_proc]
    mov QWORD PTR [rbp-88], rax
    mov DWORD PTR [rbp-80], 0
    mov DWORD PTR [rbp-76], 0
    mov rax, QWORD PTR [g_hInst]
    mov QWORD PTR [rbp-72], rax
    mov QWORD PTR [rbp-64], 0
    xor ecx, ecx
    mov edx, IDC_ARROW
    call LoadCursorW
    mov QWORD PTR [rbp-56], rax
    mov QWORD PTR [rbp-48], COLOR_WINDOW_PLUS1
    mov QWORD PTR [rbp-40], 0
    lea rax, QWORD PTR [s_cls_sticky]
    mov QWORD PTR [rbp-32], rax
    mov QWORD PTR [rbp-24], 0
    lea rcx, QWORD PTR [rbp-96]
    call RegisterClassExW
    EPILOG
reg_sticky_class ENDP
