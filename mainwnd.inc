; ecx = theme index
set_theme PROC FRAME
    PROLOG
    mov DWORD PTR [g_theme], ecx
    mov rcx, QWORD PTR [g_hEdit]
    call style_editor
    mov rcx, QWORD PTR [g_hMain]
    call GetMenu
    mov rcx, rax
    mov edx, IDM_THEME_0
    mov r8d, IDM_THEME_5
    mov r9d, IDM_THEME_0
    add r9d, DWORD PTR [g_theme]
    mov DWORD PTR [rsp+32], MF_BYCOMMAND
    call CheckMenuRadioItem
    mov DWORD PTR [rbp-16], 0
    mov ecx, DWORD PTR [g_theme]
    cmp ecx, 1
    je st_dark
    cmp ecx, 3
    je st_dark
    cmp ecx, 5
    jne st_light
st_dark:
    mov DWORD PTR [rbp-16], 1
st_light:
    mov rcx, QWORD PTR [g_hMain]
    mov edx, DWMWA_DARK_MODE
    lea r8, QWORD PTR [rbp-16]
    mov r9d, 4
    call DwmSetWindowAttribute
    mov rcx, QWORD PTR [g_hEdit]
    xor edx, edx
    mov r8d, 1
    call InvalidateRect
    mov rcx, QWORD PTR [g_hTabs]
    xor edx, edx
    mov r8d, 1
    call InvalidateRect
    mov rcx, QWORD PTR [g_hMain]
    call DrawMenuBar
    call update_stickies
    cmp DWORD PTR [g_view], 0
    je st_done
    call render_active
st_done:
    EPILOG
set_theme ENDP

; rcx = message: sends it to the editor (undo, redo, clipboard)
edit_command PROC FRAME
    PROLOG
    mov rax, rcx
    mov rcx, QWORD PTR [g_hEdit]
    mov edx, eax
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    EPILOG
edit_command ENDP

; Main window procedure
main_proc PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov eax, edx
    mov QWORD PTR [rbp-16], rax
    mov QWORD PTR [rbp-24], r8
    mov QWORD PTR [rbp-32], r9
    mov eax, DWORD PTR [rbp-16]
    cmp eax, WM_CREATE
    je mp_create
    cmp eax, WM_SIZE
    je mp_size
    cmp eax, WM_COMMAND
    je mp_command
    cmp eax, WM_NOTIFY
    je mp_notify
    cmp eax, WM_TIMER
    je mp_timer
    cmp eax, WM_SETFOCUS
    je mp_focus
    cmp eax, WM_DRAWITEM
    je mp_drawitem
    cmp eax, WM_MEASUREITEM
    je mp_measure
    cmp eax, WM_MENUCHAR
    je mp_menuchar
    cmp eax, WM_NCPAINT
    je mp_ncpaint
    cmp eax, WM_CLOSE
    je mp_close
    cmp eax, WM_DESTROY
    je mp_destroy
mp_default:
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rbp-16]
    mov r8, QWORD PTR [rbp-24]
    mov r9, QWORD PTR [rbp-32]
    call DefWindowProcW
    EPILOG
mp_zero:
    xor eax, eax
    EPILOG

mp_create:
    mov rax, QWORD PTR [rbp-8]
    mov QWORD PTR [g_hMain], rax
    mov rcx, rax
    call GetDpiForWindow
    mov DWORD PTR [g_dpi], eax
    imul eax, eax, 32
    xor edx, edx
    mov ecx, 96
    div ecx
    mov DWORD PTR [g_tabh], eax
    xor ecx, ecx
    lea rdx, QWORD PTR [s_tabclass]
    xor r8d, r8d
    mov r9d, WS_CHILD + WS_VISIBLE + TCS_OWNERDRAWFIXED + 0100h
    mov DWORD PTR [rsp+32], 0
    mov DWORD PTR [rsp+40], 0
    mov DWORD PTR [rsp+48], 0
    mov eax, DWORD PTR [g_tabh]
    mov DWORD PTR [rsp+56], eax
    mov rax, QWORD PTR [g_hMain]
    mov QWORD PTR [rsp+64], rax
    mov QWORD PTR [rsp+72], IDC_TABS
    mov rax, QWORD PTR [g_hInst]
    mov QWORD PTR [rsp+80], rax
    mov QWORD PTR [rsp+88], 0
    call CreateWindowExW
    mov QWORD PTR [g_hTabs], rax
    mov eax, 160
    call scale_px
    mov r8d, eax
    mov eax, DWORD PTR [g_tabh]
    shl eax, 16
    or eax, r8d
    mov r9, rax
    mov rcx, QWORD PTR [g_hTabs]
    mov edx, TCM_SETITEMSIZE
    xor r8d, r8d
    call SendMessageW
    mov rcx, QWORD PTR [g_hTabs]
    lea rdx, QWORD PTR [s_empty]
    lea r8, QWORD PTR [s_empty]
    call SetWindowTheme
    mov rcx, QWORD PTR [g_hTabs]
    lea rdx, QWORD PTR [tabs_subclass]
    mov r8d, 1
    xor r9d, r9d
    call SetWindowSubclass

    mov ecx, WS_EX_CLIENTEDGE
    lea rdx, QWORD PTR [s_richedit]
    xor r8d, r8d
    mov r9d, WS_CHILD + WS_VISIBLE + WS_VSCROLL + WS_TABSTOP + ES_MULTILINE + ES_AUTOVSCROLL + ES_WANTRETURN + ES_NOHIDESEL
    mov DWORD PTR [rsp+32], 0
    mov eax, DWORD PTR [g_tabh]
    mov DWORD PTR [rsp+40], eax
    mov DWORD PTR [rsp+48], 0
    mov DWORD PTR [rsp+56], 0
    mov rax, QWORD PTR [g_hMain]
    mov QWORD PTR [rsp+64], rax
    mov QWORD PTR [rsp+72], IDC_EDITOR
    mov rax, QWORD PTR [g_hInst]
    mov QWORD PTR [rsp+80], rax
    mov QWORD PTR [rsp+88], 0
    call CreateWindowExW
    mov QWORD PTR [g_hEdit], rax

    mov rcx, QWORD PTR [g_hEdit]
    mov edx, EM_SETEVENTMASK
    xor r8d, r8d
    mov r9d, ENM_CHANGE
    call SendMessageW

    call menu_font_init
    mov rcx, QWORD PTR [g_hMain]
    call GetMenu
    test rax, rax
    jz mp_nomenu
    mov rcx, rax
    xor edx, edx
    call menu_owner_draw
    mov rcx, QWORD PTR [g_hMain]
    call DrawMenuBar
mp_nomenu:
    call load_all
    jmp mp_zero

mp_size:
    mov eax, DWORD PTR [rbp-32]
    movzx eax, ax
    mov DWORD PTR [rbp-40], eax
    mov eax, DWORD PTR [rbp-32]
    shr eax, 16
    mov DWORD PTR [rbp-48], eax
    sub eax, DWORD PTR [g_tabh]
    jnc ms_h_ok
    xor eax, eax
ms_h_ok:
    mov DWORD PTR [rbp-56], eax
    mov rcx, QWORD PTR [g_hTabs]
    xor edx, edx
    xor r8d, r8d
    mov r9d, DWORD PTR [rbp-40]
    mov eax, DWORD PTR [g_tabh]
    mov DWORD PTR [rsp+32], eax
    mov DWORD PTR [rsp+40], 1
    call MoveWindow
    mov rcx, QWORD PTR [g_hEdit]
    xor edx, edx
    mov r8d, DWORD PTR [g_tabh]
    mov r9d, DWORD PTR [rbp-40]
    mov eax, DWORD PTR [rbp-56]
    mov DWORD PTR [rsp+32], eax
    mov DWORD PTR [rsp+40], 1
    call MoveWindow
    jmp mp_zero

mp_command:
    movzx eax, WORD PTR [rbp-24]
    cmp eax, IDM_NEW_TAB
    je mc_new
    cmp eax, IDM_CLOSE_TAB
    je mc_close
    cmp eax, IDM_NEXT_TAB
    je mc_next
    cmp eax, IDM_TOGGLE_VIEW
    je mc_toggle
    cmp eax, IDM_SEND_STICKY
    je mc_sticky
    cmp eax, IDM_EXIT
    je mc_exit
    cmp eax, IDM_UNDO
    je mc_undo
    cmp eax, IDM_REDO
    je mc_redo
    cmp eax, IDM_CUT
    je mc_cut
    cmp eax, IDM_COPY
    je mc_copy
    cmp eax, IDM_PASTE
    je mc_paste
    cmp eax, IDM_THEME_0
    jb mc_editor_check
    cmp eax, IDM_THEME_5
    ja mc_editor_check
    sub eax, IDM_THEME_0
    mov ecx, eax
    call set_theme
    jmp mp_zero
mc_editor_check:
    mov rax, QWORD PTR [rbp-32]
    cmp rax, QWORD PTR [g_hEdit]
    jne mp_zero
    mov ecx, DWORD PTR [rbp-24]
    shr ecx, 16
    cmp ecx, EN_CHANGE
    jne mp_zero
    mov rcx, QWORD PTR [g_hMain]
    mov edx, IDT_AUTOSAVE
    mov r8d, AUTOSAVE_MS
    xor r9d, r9d
    call SetTimer
    jmp mp_zero
mc_new:
    call new_tab
    jmp mp_zero
mc_close:
    call close_tab
    jmp mp_zero
mc_next:
    call next_tab
    jmp mp_zero
mc_toggle:
    call toggle_view
    jmp mp_zero
mc_sticky:
    call send_sticky
    jmp mp_zero
mc_exit:
    mov rcx, QWORD PTR [g_hMain]
    mov edx, WM_CLOSE
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    jmp mp_zero
mc_undo:
    mov ecx, EM_UNDO
    call edit_command
    jmp mp_zero
mc_redo:
    mov ecx, EM_REDO
    call edit_command
    jmp mp_zero
mc_cut:
    mov ecx, WM_CUT
    call edit_command
    jmp mp_zero
mc_copy:
    mov ecx, WM_COPY
    call edit_command
    jmp mp_zero
mc_paste:
    mov ecx, WM_PASTE
    call edit_command
    jmp mp_zero

mp_notify:
    mov rax, QWORD PTR [rbp-32]
    mov eax, DWORD PTR [rax+16]
    cmp eax, TCN_SELCHANGE_U
    jne mp_zero
    call sync_active
    mov rcx, QWORD PTR [g_hTabs]
    mov edx, TCM_GETCURSEL
    xor r8d, r8d
    xor r9d, r9d
    call SendMessageW
    mov ecx, eax
    call activate
    call save_all
    jmp mp_zero

mp_timer:
    mov rcx, QWORD PTR [rbp-8]
    mov edx, IDT_AUTOSAVE
    call KillTimer
    call save_all
    jmp mp_zero

mp_focus:
    mov rcx, QWORD PTR [g_hEdit]
    call SetFocus
    jmp mp_zero

mp_close:
    call save_all
    mov rcx, QWORD PTR [rbp-8]
    call DestroyWindow
    jmp mp_zero

mp_drawitem:
    mov rax, QWORD PTR [rbp-32]
    cmp DWORD PTR [rax], ODT_MENU
    jne mp_drawctl
    mov rcx, QWORD PTR [rbp-32]
    call menu_draw
    mov eax, 1
    EPILOG
mp_drawctl:
    cmp DWORD PTR [rbp-24], IDC_TABS
    jne mp_zero
    mov rcx, QWORD PTR [rbp-32]
    call draw_tab
    mov eax, 1
    EPILOG
mp_measure:
    mov rax, QWORD PTR [rbp-32]
    cmp DWORD PTR [rax], ODT_MENU
    jne mp_zero
    mov rcx, QWORD PTR [rbp-32]
    call menu_measure
    mov eax, 1
    EPILOG
mp_menuchar:
    movzx ecx, WORD PTR [rbp-24]
    mov rdx, QWORD PTR [rbp-32]
    call menu_mnemonic
    EPILOG
mp_ncpaint:
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rbp-16]
    mov r8, QWORD PTR [rbp-24]
    mov r9, QWORD PTR [rbp-32]
    call DefWindowProcW
    mov QWORD PTR [rbp-40], rax
    call menu_gap_paint
    mov rax, QWORD PTR [rbp-40]
    EPILOG
mp_destroy:
    xor ecx, ecx
    call PostQuitMessage
    jmp mp_zero
main_proc ENDP
