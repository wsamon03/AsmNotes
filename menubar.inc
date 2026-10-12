; ---- Owner-drawn menu bar -----------------------------------------------------
; Every menu item is MF_OWNERDRAW, so the bar and its drop-downs use the theme colours.
; The menu tree itself stays native, so accelerators and WM_COMMAND work unchanged.
; Item data points to a record: [0] text (UTF-16, '&' marks a mnemonic, a tab
; separates the shortcut), [8] flags: 1 top-level bar item, 2 opens a submenu.
; A separator has item data 0.

; -> creates the menu font: Segoe UI 9pt at the window's DPI
menu_font_init PROC FRAME
    PROLOG
    mov eax, 9
    imul eax, DWORD PTR [g_dpi]
    add eax, 36
    xor edx, edx
    mov ecx, 72
    div ecx
    neg eax
    mov DWORD PTR [rbp-160], eax
    mov DWORD PTR [rbp-156], 0
    mov DWORD PTR [rbp-152], 0
    mov DWORD PTR [rbp-148], 0
    mov DWORD PTR [rbp-144], 400
    mov DWORD PTR [rbp-140], 01000000h
    mov DWORD PTR [rbp-136], 22050000h
    lea rcx, QWORD PTR [rbp-132]
    lea rdx, QWORD PTR [s_segoe]
    call copy_wstr
    lea rcx, QWORD PTR [rbp-160]
    call CreateFontIndirectW
    mov QWORD PTR [g_menufont], rax
    EPILOG
menu_font_init ENDP

; rcx = menu, edx = depth: makes every item owner-drawn and attaches its record
menu_owner_draw PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov DWORD PTR [rbp-16], edx
    call GetMenuItemCount
    mov DWORD PTR [rbp-24], eax
    mov DWORD PTR [rbp-32], 0
mo_loop:
    mov eax, DWORD PTR [rbp-32]
    cmp eax, DWORD PTR [rbp-24]
    jge mo_done
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rbp-32]
    call GetSubMenu
    mov QWORD PTR [rbp-40], rax
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rbp-32]
    call GetMenuItemID
    mov DWORD PTR [rbp-48], eax
    cmp QWORD PTR [rbp-40], 0
    jne mo_text
    cmp DWORD PTR [rbp-48], 0
    jne mo_text
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rbp-32]
    mov r8d, MF_BYPOSITION + MF_SEPARATOR + MF_OWNERDRAW
    xor r9d, r9d
    mov QWORD PTR [rsp+32], 0
    call ModifyMenuW
    jmp mo_next
mo_text:
    mov ecx, 512
    call heap_alloc
    mov QWORD PTR [rbp-56], rax
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rbp-32]
    mov r8, QWORD PTR [rbp-56]
    mov r9d, 256
    mov DWORD PTR [rsp+32], MF_BYPOSITION
    call GetMenuStringW
    mov ecx, 16
    call heap_alloc
    mov QWORD PTR [rbp-64], rax
    mov rdx, QWORD PTR [rbp-56]
    mov QWORD PTR [rax], rdx
    xor ecx, ecx
    cmp DWORD PTR [rbp-16], 0
    jne mo_flags_sub
    mov ecx, 1
mo_flags_sub:
    cmp QWORD PTR [rbp-40], 0
    je mo_flags_store
    or ecx, 2
mo_flags_store:
    mov QWORD PTR [rax+8], rcx
    mov rcx, QWORD PTR [rbp-8]
    mov edx, DWORD PTR [rbp-32]
    mov r8d, MF_BYPOSITION + MF_OWNERDRAW
    mov r9d, DWORD PTR [rbp-48]
    cmp QWORD PTR [rbp-40], 0
    je mo_modify
    mov r8d, MF_BYPOSITION + MF_OWNERDRAW + MF_POPUP
    mov r9, QWORD PTR [rbp-40]
mo_modify:
    mov rax, QWORD PTR [rbp-64]
    mov QWORD PTR [rsp+32], rax
    call ModifyMenuW
    cmp QWORD PTR [rbp-40], 0
    je mo_next
    mov rcx, QWORD PTR [rbp-40]
    mov edx, DWORD PTR [rbp-16]
    inc edx
    call menu_owner_draw
mo_next:
    inc DWORD PTR [rbp-32]
    jmp mo_loop
mo_done:
    EPILOG
menu_owner_draw ENDP

; rcx = text -> eax = position of the first tab, or -1 when there is none
menu_tab_index PROC
    xor eax, eax
mti_loop:
    movzx edx, WORD PTR [rcx + rax*2]
    test edx, edx
    jz mti_none
    cmp edx, 9
    je mti_done
    inc eax
    jmp mti_loop
mti_none:
    mov eax, -1
mti_done:
    ret
menu_tab_index ENDP

; rcx = hdc, rdx = text, r8d = character count (-1 for the whole string) -> eax = width
menu_text_width PROC FRAME
    PROLOG
    mov DWORD PTR [rbp-24], 0
    mov DWORD PTR [rbp-20], 0
    mov DWORD PTR [rbp-16], 10000
    mov DWORD PTR [rbp-12], 100
    lea r9, QWORD PTR [rbp-24]
    mov DWORD PTR [rsp+32], DT_CALCRECT + DT_SINGLELINE + DT_HIDEPREFIX
    call DrawTextW
    mov eax, DWORD PTR [rbp-16]
    sub eax, DWORD PTR [rbp-24]
    EPILOG
menu_text_width ENDP

; rcx = MEASUREITEMSTRUCT: sets the width and height of one menu item
menu_measure PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov rax, QWORD PTR [rcx+24]
    mov QWORD PTR [rbp-16], rax
    test rax, rax
    jnz mm_item
    mov eax, 7
    call scale_px
    mov rcx, QWORD PTR [rbp-8]
    mov DWORD PTR [rcx+16], eax
    mov DWORD PTR [rcx+12], 0
    EPILOG
mm_item:
    mov rcx, QWORD PTR [g_hMain]
    call GetDC
    mov QWORD PTR [rbp-24], rax
    mov rcx, rax
    mov rdx, QWORD PTR [g_menufont]
    call SelectObject
    mov QWORD PTR [rbp-32], rax
    mov rax, QWORD PTR [rbp-16]
    mov rcx, QWORD PTR [rax]
    mov QWORD PTR [rbp-40], rcx
    mov rcx, QWORD PTR [rbp-40]
    call menu_tab_index
    mov DWORD PTR [rbp-48], eax
    mov rax, QWORD PTR [rbp-16]
    mov eax, DWORD PTR [rax+8]
    mov DWORD PTR [rbp-56], eax
    mov rcx, QWORD PTR [rbp-24]
    mov rdx, QWORD PTR [rbp-40]
    mov r8d, DWORD PTR [rbp-48]
    call menu_text_width
    mov DWORD PTR [rbp-64], eax
    mov DWORD PTR [rbp-72], 0
    cmp DWORD PTR [rbp-48], 0
    jl mm_no_shortcut
    mov rcx, QWORD PTR [rbp-24]
    mov rdx, QWORD PTR [rbp-40]
    movsxd rax, DWORD PTR [rbp-48]
    lea rdx, QWORD PTR [rdx + rax*2 + 2]
    mov r8d, -1
    call menu_text_width
    mov DWORD PTR [rbp-72], eax
mm_no_shortcut:
    test DWORD PTR [rbp-56], 1
    jz mm_dropdown
    mov eax, 24
    call scale_px
    add eax, DWORD PTR [rbp-64]
    jmp mm_store
mm_dropdown:
    mov eax, 62
    call scale_px
    add eax, DWORD PTR [rbp-64]
    add eax, DWORD PTR [rbp-72]
    test DWORD PTR [rbp-56], 2
    jz mm_store
    mov DWORD PTR [rbp-80], eax
    mov eax, 12
    call scale_px
    add eax, DWORD PTR [rbp-80]
mm_store:
    mov rcx, QWORD PTR [rbp-8]
    mov DWORD PTR [rcx+12], eax
    mov eax, 26
    call scale_px
    mov rcx, QWORD PTR [rbp-8]
    mov DWORD PTR [rcx+16], eax
    mov rcx, QWORD PTR [rbp-24]
    mov rdx, QWORD PTR [rbp-32]
    call SelectObject
    mov rcx, QWORD PTR [g_hMain]
    mov rdx, QWORD PTR [rbp-24]
    call ReleaseDC
    EPILOG
menu_measure ENDP

; rcx = DRAWITEMSTRUCT: paints one menu item in the active theme's colours
menu_draw PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov rax, QWORD PTR [rcx+56]
    mov QWORD PTR [rbp-16], rax
    mov rax, QWORD PTR [rcx+32]
    mov QWORD PTR [rbp-24], rax
    mov eax, DWORD PTR [rcx+16]
    mov DWORD PTR [rbp-32], eax
    mov rax, QWORD PTR [rcx+40]
    mov QWORD PTR [rbp-80], rax
    mov rax, QWORD PTR [rcx+48]
    mov QWORD PTR [rbp-72], rax
    call palette_ptr
    mov QWORD PTR [rbp-136], rax
    cmp QWORD PTR [rbp-16], 0
    jne md_item

    ; separator: background, then a one-pixel rule in the muted colour
    mov rax, QWORD PTR [rbp-136]
    mov rcx, QWORD PTR [rbp-24]
    lea rdx, QWORD PTR [rbp-80]
    mov r8d, DWORD PTR [rax]
    call fill_rect_color
    mov eax, 10
    call scale_px
    mov ecx, DWORD PTR [rbp-80]
    add ecx, eax
    mov DWORD PTR [rbp-96], ecx
    mov ecx, DWORD PTR [rbp-72]
    sub ecx, eax
    mov DWORD PTR [rbp-88], ecx
    mov ecx, DWORD PTR [rbp-76]
    add ecx, DWORD PTR [rbp-68]
    sar ecx, 1
    mov DWORD PTR [rbp-92], ecx
    inc ecx
    mov DWORD PTR [rbp-84], ecx
    mov rax, QWORD PTR [rbp-136]
    mov rcx, QWORD PTR [rbp-24]
    lea rdx, QWORD PTR [rbp-96]
    mov r8d, DWORD PTR [rax+12]
    call fill_rect_color
    mov eax, 1
    EPILOG

md_item:
    mov rdx, QWORD PTR [rbp-136]
    mov r8d, DWORD PTR [rdx]
    mov r9d, DWORD PTR [rdx+4]
    mov eax, DWORD PTR [rbp-32]
    test eax, 1
    jz md_colours
    mov r8d, DWORD PTR [rdx+8]
    mov r9d, DWORD PTR [rdx]
md_colours:
    mov DWORD PTR [rbp-40], r8d
    mov DWORD PTR [rbp-48], r9d
    test eax, 1
    jnz md_colours_done
    test eax, 6
    jz md_colours_done
    mov ecx, DWORD PTR [rdx+12]
    mov DWORD PTR [rbp-48], ecx
md_colours_done:
    mov rcx, QWORD PTR [rbp-24]
    lea rdx, QWORD PTR [rbp-80]
    mov r8d, DWORD PTR [rbp-40]
    call fill_rect_color
    mov rcx, QWORD PTR [rbp-24]
    mov edx, 1
    call SetBkMode
    mov rcx, QWORD PTR [rbp-24]
    mov edx, DWORD PTR [rbp-48]
    call SetTextColor
    mov rcx, QWORD PTR [rbp-24]
    mov rdx, QWORD PTR [g_menufont]
    call SelectObject
    mov QWORD PTR [rbp-56], rax
    mov rax, QWORD PTR [rbp-16]
    mov rcx, QWORD PTR [rax]
    mov QWORD PTR [rbp-64], rcx
    test DWORD PTR [rax+8], 1
    jz md_dropdown

    ; top-level bar item: centred label
    mov rcx, QWORD PTR [rbp-24]
    mov rdx, QWORD PTR [rbp-64]
    mov r8d, -1
    lea r9, QWORD PTR [rbp-80]
    mov DWORD PTR [rsp+32], DT_CENTER + DT_VCENTER + DT_SINGLELINE + DT_HIDEPREFIX
    call DrawTextW
    jmp md_restore

md_dropdown:
    mov rcx, QWORD PTR [rbp-64]
    call menu_tab_index
    mov DWORD PTR [rbp-144], eax

    ; rcL: the label, starting after the check column
    mov rax, QWORD PTR [rbp-80]
    mov QWORD PTR [rbp-96], rax
    mov rax, QWORD PTR [rbp-72]
    mov QWORD PTR [rbp-88], rax
    mov eax, 18
    call scale_px
    add DWORD PTR [rbp-96], eax

    ; rcR: shortcut and submenu arrow, inset from the right edge
    mov rax, QWORD PTR [rbp-80]
    mov QWORD PTR [rbp-112], rax
    mov rax, QWORD PTR [rbp-72]
    mov QWORD PTR [rbp-104], rax
    mov eax, 10
    call scale_px
    sub DWORD PTR [rbp-104], eax

    ; check bullet in the left column
    test DWORD PTR [rbp-32], 8
    jz md_text
    mov rax, QWORD PTR [rbp-80]
    mov QWORD PTR [rbp-128], rax
    mov rax, QWORD PTR [rbp-72]
    mov QWORD PTR [rbp-120], rax
    mov eax, 18
    call scale_px
    add eax, DWORD PTR [rbp-80]
    mov DWORD PTR [rbp-120], eax
    mov rcx, QWORD PTR [rbp-24]
    lea rdx, QWORD PTR [s_bullet]
    mov r8d, -1
    lea r9, QWORD PTR [rbp-128]
    mov DWORD PTR [rsp+32], DT_CENTER + DT_VCENTER + DT_SINGLELINE + DT_HIDEPREFIX
    call DrawTextW

md_text:
    mov rcx, QWORD PTR [rbp-24]
    mov rdx, QWORD PTR [rbp-64]
    mov r8d, DWORD PTR [rbp-144]
    lea r9, QWORD PTR [rbp-96]
    mov DWORD PTR [rsp+32], DT_LEFT + DT_VCENTER + DT_SINGLELINE + DT_HIDEPREFIX
    call DrawTextW
    mov rax, QWORD PTR [rbp-16]
    test DWORD PTR [rax+8], 2
    jz md_no_arrow
    mov rcx, QWORD PTR [rbp-24]
    lea rdx, QWORD PTR [s_arrow]
    mov r8d, 1
    lea r9, QWORD PTR [rbp-112]
    mov DWORD PTR [rsp+32], DT_RIGHT + DT_VCENTER + DT_SINGLELINE + DT_HIDEPREFIX
    call DrawTextW
md_no_arrow:
    cmp DWORD PTR [rbp-144], 0
    jl md_restore
    mov rax, QWORD PTR [rbp-64]
    movsxd rdx, DWORD PTR [rbp-144]
    lea rdx, QWORD PTR [rax + rdx*2 + 2]
    mov rcx, QWORD PTR [rbp-24]
    mov r8d, -1
    lea r9, QWORD PTR [rbp-112]
    mov DWORD PTR [rsp+32], DT_RIGHT + DT_VCENTER + DT_SINGLELINE + DT_HIDEPREFIX
    call DrawTextW

md_restore:
    mov rcx, QWORD PTR [rbp-24]
    mov rdx, QWORD PTR [rbp-56]
    call SelectObject
    mov eax, 1
    EPILOG
menu_draw ENDP

; ecx = character, rdx = menu -> eax = MAKELRESULT(item, MNC_*) for its mnemonic, or 0
menu_mnemonic PROC FRAME
    PROLOG
    movzx eax, cx
    cmp eax, 61h
    jb mk_wanted
    cmp eax, 7Ah
    ja mk_wanted
    sub eax, 20h
mk_wanted:
    mov DWORD PTR [rbp-8], eax
    mov QWORD PTR [rbp-16], rdx
    mov rcx, rdx
    call GetMenuItemCount
    mov DWORD PTR [rbp-24], eax
    mov DWORD PTR [rbp-32], 0
mk_item:
    mov eax, DWORD PTR [rbp-32]
    cmp eax, DWORD PTR [rbp-24]
    jge mk_none
    mov DWORD PTR [rbp-160], 80
    mov DWORD PTR [rbp-156], 20h
    mov rcx, QWORD PTR [rbp-16]
    mov edx, DWORD PTR [rbp-32]
    mov r8d, 1
    lea r9, QWORD PTR [rbp-160]
    call GetMenuItemInfoW
    test eax, eax
    jz mk_next
    mov rax, QWORD PTR [rbp-112]
    test rax, rax
    jz mk_next
    mov QWORD PTR [rbp-40], rax
    mov rdx, QWORD PTR [rax]
    mov QWORD PTR [rbp-48], rdx
    xor r8d, r8d
mk_scan:
    movzx eax, WORD PTR [rdx + r8*2]
    test eax, eax
    jz mk_next
    cmp eax, 26h
    jne mk_scan_next
    movzx eax, WORD PTR [rdx + r8*2 + 2]
    test eax, eax
    jz mk_next
    cmp eax, 26h
    je mk_scan_skip
    cmp eax, 61h
    jb mk_cand
    cmp eax, 7Ah
    ja mk_cand
    sub eax, 20h
mk_cand:
    cmp eax, DWORD PTR [rbp-8]
    je mk_hit
mk_scan_skip:
    add r8, 2
    jmp mk_scan
mk_scan_next:
    inc r8
    jmp mk_scan
mk_hit:
    mov rax, QWORD PTR [rbp-40]
    mov edx, 2
    test DWORD PTR [rax+8], 2
    jz mk_leaf
    mov edx, 3
mk_leaf:
    shl edx, 16
    or edx, DWORD PTR [rbp-32]
    mov eax, edx
    EPILOG
mk_next:
    inc DWORD PTR [rbp-32]
    jmp mk_item
mk_none:
    xor eax, eax
    EPILOG
menu_mnemonic ENDP

; Paints the part of the menu bar to the right of the last item, which Windows
; leaves in the system colour
menu_gap_paint PROC FRAME
    PROLOG
    mov rcx, QWORD PTR [g_hMain]
    call GetMenu
    test rax, rax
    jz mg_done
    mov QWORD PTR [rbp-8], rax
    mov rcx, rax
    call GetMenuItemCount
    test eax, eax
    jle mg_done
    mov rcx, QWORD PTR [g_hMain]
    mov rdx, QWORD PTR [rbp-8]
    mov r8d, eax
    dec r8d
    lea r9, QWORD PTR [rbp-24]
    call GetMenuItemRect
    mov DWORD PTR [rbp-80], 48
    mov rcx, QWORD PTR [g_hMain]
    mov edx, 0FFFFFFFDh
    xor r8d, r8d
    lea r9, QWORD PTR [rbp-80]
    call GetMenuBarInfo
    mov rcx, QWORD PTR [g_hMain]
    lea rdx, QWORD PTR [rbp-112]
    call GetWindowRect

    ; gap = (last item's right edge, bar top) .. (bar right, bar bottom), in window coordinates
    mov eax, DWORD PTR [rbp-16]
    sub eax, DWORD PTR [rbp-112]
    mov DWORD PTR [rbp-128], eax
    mov eax, DWORD PTR [rbp-72]
    sub eax, DWORD PTR [rbp-108]
    mov DWORD PTR [rbp-124], eax
    mov eax, DWORD PTR [rbp-68]
    sub eax, DWORD PTR [rbp-112]
    mov DWORD PTR [rbp-120], eax
    mov eax, DWORD PTR [rbp-64]
    sub eax, DWORD PTR [rbp-108]
    mov DWORD PTR [rbp-116], eax

    call palette_ptr
    mov eax, DWORD PTR [rax]
    mov DWORD PTR [rbp-144], eax
    mov rcx, QWORD PTR [g_hMain]
    call GetWindowDC
    mov QWORD PTR [rbp-136], rax
    mov rcx, rax
    lea rdx, QWORD PTR [rbp-128]
    mov r8d, DWORD PTR [rbp-144]
    call fill_rect_color
    mov rcx, QWORD PTR [g_hMain]
    mov rdx, QWORD PTR [rbp-136]
    call ReleaseDC
mg_done:
    EPILOG
menu_gap_paint ENDP
