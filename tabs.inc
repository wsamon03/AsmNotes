; rcx = tab index -> updates the tab's title in the tab strip from its first text line
update_title PROC FRAME
    PROLOG
    mov eax, ecx
    mov QWORD PTR [rbp-8], rax
    lea r11, QWORD PTR [g_tabs]
    mov rcx, QWORD PTR [r11 + rax*8]
    mov QWORD PTR [rbp-16], rcx
    mov rdx, QWORD PTR [rcx]
    mov QWORD PTR [rbp-24], rdx
    mov eax, DWORD PTR [rcx+8]
    mov QWORD PTR [rbp-32], rax
    xor r8d, r8d
ut_skip:
    cmp r8, QWORD PTR [rbp-32]
    jae ut_default
    movzx eax, WORD PTR [rdx + r8*2]
    cmp eax, 20h
    je ut_skip_next
    cmp eax, 09h
    je ut_skip_next
    cmp eax, 0Dh
    je ut_skip_next
    cmp eax, 0Ah
    je ut_skip_next
    cmp eax, 23h
    je ut_skip_next
    jmp ut_copy
ut_skip_next:
    inc r8
    jmp ut_skip
ut_copy:
    xor r9d, r9d
ut_copy_loop:
    cmp r8, QWORD PTR [rbp-32]
    jae ut_copy_done
    movzx eax, WORD PTR [rdx + r8*2]
    cmp eax, 0Dh
    je ut_copy_done
    cmp eax, 0Ah
    je ut_copy_done
    cmp r9, 24
    jae ut_copy_done
    lea r10, QWORD PTR [g_title]
    mov WORD PTR [r10 + r9*2], ax
    inc r9
    inc r8
    jmp ut_copy_loop
ut_copy_done:
    lea r10, QWORD PTR [g_title]
    mov WORD PTR [r10 + r9*2], 0
    jmp ut_set
ut_default:
    mov rax, QWORD PTR [rbp-8]
    inc eax
    mov r8d, eax
    lea rcx, QWORD PTR [g_title]
    lea rdx, QWORD PTR [fmt_note]
    call wsprintfW
ut_set:
    mov QWORD PTR [rbp-80], 1
    mov QWORD PTR [rbp-72], 0
    lea rax, QWORD PTR [g_title]
    mov QWORD PTR [rbp-64], rax
    mov QWORD PTR [rbp-56], 0
    mov QWORD PTR [rbp-48], 0
    mov rcx, QWORD PTR [g_hTabs]
    mov edx, TCM_SETITEMW
    mov r8, QWORD PTR [rbp-8]
    lea r9, QWORD PTR [rbp-80]
    call SendMessageW
    EPILOG
update_title ENDP

; -> rax = new tab (src buffer + length); tab layout: [0] src ptr, [8] cch
tab_create PROC FRAME
    PROLOG
    mov ecx, 16
    call heap_alloc
    mov QWORD PTR [rbp-8], rax
    mov ecx, MAX_CCH * 2
    call heap_alloc
    mov rcx, QWORD PTR [rbp-8]
    mov QWORD PTR [rcx], rax
    mov rax, rcx
    EPILOG
tab_create ENDP

; rcx = tab
tab_free PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov rcx, QWORD PTR [rcx]
    call heap_free
    mov rcx, QWORD PTR [rbp-8]
    call heap_free
    EPILOG
tab_free ENDP

; Copies the editor's text into the active tab (raw mode only; formatted mode keeps the source as-is)
sync_active PROC FRAME
    PROLOG
    cmp DWORD PTR [g_nTabs], 0
    je sy_done
    cmp DWORD PTR [g_view], 0
    jne sy_done
    mov eax, DWORD PTR [g_active]
    lea r11, QWORD PTR [g_tabs]
    mov rcx, QWORD PTR [r11 + rax*8]
    mov QWORD PTR [rbp-8], rcx
    mov rcx, QWORD PTR [g_hEdit]
    mov rdx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rdx]
    mov r8d, MAX_CCH
    call GetWindowTextW
    mov rcx, QWORD PTR [rbp-8]
    mov DWORD PTR [rcx+8], eax
    mov ecx, DWORD PTR [g_active]
    call update_title
sy_done:
    EPILOG
sync_active ENDP

; Loads the active tab's text into the editor
show_active PROC FRAME
    PROLOG
    cmp DWORD PTR [g_nTabs], 0
    je sa_done
    mov rcx, QWORD PTR [g_hEdit]
    mov edx, EM_SETREADONLY
    mov r8d, DWORD PTR [g_view]
    xor r9d, r9d
    call SendMessageW
    cmp DWORD PTR [g_view], 0
    jne sa_formatted
    mov eax, DWORD PTR [g_active]
    lea r11, QWORD PTR [g_tabs]
    mov rcx, QWORD PTR [r11 + rax*8]
    mov rdx, QWORD PTR [rcx]
    mov rcx, QWORD PTR [g_hEdit]
    call SetWindowTextW
    jmp sa_done
sa_formatted:
    call render_active
sa_done:
    EPILOG
show_active ENDP

; rcx = index: makes it the active tab
activate PROC FRAME
    PROLOG
    mov DWORD PTR [g_active], ecx
    mov rcx, QWORD PTR [g_hTabs]
    mov edx, TCM_SETCURSEL
    mov r8d, DWORD PTR [g_active]
    xor r9d, r9d
    call SendMessageW
    call show_active
    EPILOG
activate ENDP

; Appends an empty tab and returns its index in eax
append_tab PROC FRAME
    PROLOG
    call tab_create
    mov ecx, DWORD PTR [g_nTabs]
    mov QWORD PTR [rbp-8], rcx
    lea r11, QWORD PTR [g_tabs]
    mov QWORD PTR [r11 + rcx*8], rax
    mov ecx, DWORD PTR [rbp-8]
    call update_title
    mov QWORD PTR [rbp-80], 1
    mov QWORD PTR [rbp-72], 0
    lea rax, QWORD PTR [g_title]
    mov QWORD PTR [rbp-64], rax
    mov QWORD PTR [rbp-56], 0
    mov QWORD PTR [rbp-48], 0
    mov rcx, QWORD PTR [g_hTabs]
    mov edx, TCM_INSERTITEMW
    mov r8d, DWORD PTR [rbp-8]
    lea r9, QWORD PTR [rbp-80]
    call SendMessageW
    inc DWORD PTR [g_nTabs]
    mov eax, DWORD PTR [rbp-8]
    EPILOG
append_tab ENDP

new_tab PROC FRAME
    PROLOG
    mov eax, DWORD PTR [g_nTabs]
    cmp eax, MAX_TABS
    jae nt_done
    call sync_active
    call append_tab
    mov ecx, eax
    call activate
    call save_all
nt_done:
    EPILOG
new_tab ENDP

close_tab PROC FRAME
    PROLOG
    cmp DWORD PTR [g_nTabs], 1
    ja ct_multi
    mov rcx, QWORD PTR [g_tabs]
    mov rdx, QWORD PTR [rcx]
    mov WORD PTR [rdx], 0
    mov DWORD PTR [rcx+8], 0
    xor ecx, ecx
    call update_title
    call show_active
    jmp ct_done
ct_multi:
    call sync_active
    mov eax, DWORD PTR [g_active]
    mov QWORD PTR [rbp-8], rax
    lea r11, QWORD PTR [g_tabs]
    mov rcx, QWORD PTR [r11 + rax*8]
    call tab_free
    mov ecx, DWORD PTR [rbp-8]
ct_shift:
    mov eax, DWORD PTR [g_nTabs]
    dec eax
    cmp ecx, eax
    jae ct_shifted
    lea r11, QWORD PTR [g_tabs]
    mov rdx, QWORD PTR [r11 + rcx*8 + 8]
    lea r11, QWORD PTR [g_tabs]
    mov QWORD PTR [r11 + rcx*8], rdx
    inc ecx
    jmp ct_shift
ct_shifted:
    dec DWORD PTR [g_nTabs]
    mov rcx, QWORD PTR [g_hTabs]
    mov edx, TCM_DELETEITEM
    mov r8d, DWORD PTR [rbp-8]
    xor r9d, r9d
    call SendMessageW
    mov eax, DWORD PTR [g_nTabs]
    dec eax
    mov ecx, DWORD PTR [rbp-8]
    cmp ecx, eax
    jbe ct_keep
    mov ecx, eax
ct_keep:
    call activate
ct_done:
    call save_all
    EPILOG
close_tab ENDP

next_tab PROC FRAME
    PROLOG
    mov eax, DWORD PTR [g_nTabs]
    cmp eax, 2
    jb nx_done
    call sync_active
    mov eax, DWORD PTR [g_active]
    inc eax
    xor edx, edx
    div DWORD PTR [g_nTabs]
    mov ecx, edx
    call activate
    call save_all
nx_done:
    EPILOG
next_tab ENDP

toggle_view PROC FRAME
    PROLOG
    call sync_active
    xor DWORD PTR [g_view], 1
    call show_active
    call save_all
    EPILOG
toggle_view ENDP

; rcx = DRAWITEMSTRUCT* for one tab of the tab strip
draw_tab PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov eax, DWORD PTR [rcx+8]
    mov DWORD PTR [rbp-16], eax
    mov QWORD PTR [rbp-120], 1
    mov QWORD PTR [rbp-112], 0
    lea rax, QWORD PTR [g_dtitle]
    mov QWORD PTR [rbp-104], rax
    mov DWORD PTR [rbp-96], 64
    mov DWORD PTR [rbp-92], 0
    mov QWORD PTR [rbp-88], 0
    mov rcx, QWORD PTR [g_hTabs]
    mov edx, TCM_GETITEMW
    mov r8d, DWORD PTR [rbp-16]
    lea r9, QWORD PTR [rbp-120]
    call SendMessageW
    call palette_ptr
    mov rdx, rax
    mov ecx, DWORD PTR [rdx]
    mov DWORD PTR [rbp-24], ecx
    mov ecx, DWORD PTR [rdx+4]
    mov DWORD PTR [rbp-32], ecx
    mov ecx, DWORD PTR [rdx+8]
    mov DWORD PTR [rbp-40], ecx
    mov ecx, DWORD PTR [rdx+12]
    mov DWORD PTR [rbp-48], ecx
    mov eax, DWORD PTR [rbp-16]
    xor ecx, ecx
    cmp eax, DWORD PTR [g_active]
    jne dt_unselected
    mov ecx, 1
dt_unselected:
    mov DWORD PTR [rbp-56], ecx
    mov rcx, QWORD PTR [rbp-8]
    mov rax, QWORD PTR [rcx+32]
    mov QWORD PTR [rbp-64], rax
    lea rdx, QWORD PTR [rcx+40]
    mov r8d, DWORD PTR [rbp-24]
    mov rcx, QWORD PTR [rbp-64]
    call fill_rect_color
    mov rcx, QWORD PTR [rbp-64]
    mov rax, QWORD PTR [rbp-8]
    lea rdx, QWORD PTR [rax+40]
    mov r8d, DWORD PTR [rbp-48]
    call frame_rect_color
    cmp DWORD PTR [rbp-56], 0
    je dt_text
    mov rcx, QWORD PTR [rbp-8]
    mov eax, DWORD PTR [rcx+40]
    mov DWORD PTR [rbp-160], eax
    mov eax, DWORD PTR [rcx+48]
    mov DWORD PTR [rbp-152], eax
    mov eax, DWORD PTR [rcx+52]
    mov DWORD PTR [rbp-148], eax
    mov DWORD PTR [rbp-84], eax
    mov eax, 3
    call scale_px
    mov edx, DWORD PTR [rbp-84]
    sub edx, eax
    mov DWORD PTR [rbp-156], edx
    mov rcx, QWORD PTR [rbp-64]
    lea rdx, QWORD PTR [rbp-160]
    mov r8d, DWORD PTR [rbp-40]
    call fill_rect_color
dt_text:
    mov rcx, QWORD PTR [rbp-64]
    mov edx, 1
    call SetBkMode
    mov rcx, QWORD PTR [rbp-64]
    mov edx, DWORD PTR [rbp-32]
    cmp DWORD PTR [rbp-56], 0
    jne dt_color
    mov edx, DWORD PTR [rbp-48]
dt_color:
    call SetTextColor
    mov rcx, QWORD PTR [rbp-64]
    lea rdx, QWORD PTR [g_dtitle]
    mov r8d, -1
    mov rax, QWORD PTR [rbp-8]
    lea r9, QWORD PTR [rax+40]
    mov DWORD PTR [rsp+32], 8025h
    call DrawTextW
    EPILOG
draw_tab ENDP

; Subclass for the tab strip: paints the background in the theme colour
tabs_subclass PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov eax, edx
    mov QWORD PTR [rbp-16], rax
    mov QWORD PTR [rbp-24], r8
    mov QWORD PTR [rbp-32], r9
    cmp eax, WM_PAINT
    je ts_paint
    cmp eax, WM_ERASEBKGND
    jne ts_default
    lea rdx, QWORD PTR [rbp-56]
    mov rcx, QWORD PTR [rbp-8]
    call GetClientRect
    call palette_ptr
    mov r8d, DWORD PTR [rax]
    mov rcx, QWORD PTR [rbp-24]
    lea rdx, QWORD PTR [rbp-56]
    call fill_rect_color
    mov eax, 1
    EPILOG
ts_paint:
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rbp-16]
    mov r8, QWORD PTR [rbp-24]
    mov r9, QWORD PTR [rbp-32]
    call DefSubclassProc
    mov DWORD PTR [rbp-40], eax
    mov rcx, QWORD PTR [rbp-8]
    call GetDC
    mov QWORD PTR [rbp-48], rax
    mov rcx, QWORD PTR [rbp-8]
    mov edx, TCM_GETITEMCOUNT
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    mov QWORD PTR [rbp-64], rax
    mov QWORD PTR [rbp-56], 0
ts_loop:
    mov rax, QWORD PTR [rbp-56]
    cmp rax, QWORD PTR [rbp-64]
    jae ts_done
    mov rcx, QWORD PTR [rbp-8]
    mov edx, TCM_GETITEMRECT
    mov r8, QWORD PTR [rbp-56]
    lea r9, QWORD PTR [rbp-88]
    call SendMessageW
    call palette_ptr
    mov r8d, DWORD PTR [rax+12]
    mov rcx, QWORD PTR [rbp-48]
    lea rdx, QWORD PTR [rbp-88]
    call frame_rect_color
    inc QWORD PTR [rbp-56]
    jmp ts_loop
ts_done:
    mov rcx, QWORD PTR [rbp-8]
    mov rdx, QWORD PTR [rbp-48]
    call ReleaseDC
    mov eax, DWORD PTR [rbp-40]
    EPILOG
ts_default:
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rbp-16]
    mov r8, QWORD PTR [rbp-24]
    mov r9, QWORD PTR [rbp-32]
    call DefSubclassProc
    EPILOG
tabs_subclass ENDP
