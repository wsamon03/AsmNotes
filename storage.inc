; rcx = key name, edx = default -> eax = value from settings.ini
get_int PROC FRAME
    PROLOG
    mov r8d, edx
    mov rdx, rcx
    lea rcx, QWORD PTR [s_sec_notes]
    lea r9, QWORD PTR [g_ini]
    call GetPrivateProfileIntW
    EPILOG
get_int ENDP

; rcx = key name, edx = value
set_int PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov r8d, edx
    lea rdx, QWORD PTR [fmt_int]
    lea rcx, QWORD PTR [g_num]
    call wsprintfW
    mov rdx, QWORD PTR [rbp-8]
    lea r8, QWORD PTR [g_num]
    lea rcx, QWORD PTR [s_sec_notes]
    lea r9, QWORD PTR [g_ini]
    call WritePrivateProfileStringW
    EPILOG
set_int ENDP

storage_init PROC FRAME
    PROLOG
    lea rcx, QWORD PTR [s_appdata]
    lea rdx, QWORD PTR [g_appdata]
    mov r8d, 260
    call GetEnvironmentVariableW
    lea rcx, QWORD PTR [g_dir]
    lea rdx, QWORD PTR [fmt_dir]
    lea r8, QWORD PTR [g_appdata]
    call wsprintfW
    lea rcx, QWORD PTR [g_dir]
    xor edx, edx
    call CreateDirectoryW
    lea rcx, QWORD PTR [g_ini]
    lea rdx, QWORD PTR [fmt_ini]
    lea r8, QWORD PTR [g_dir]
    call wsprintfW
    EPILOG
storage_init ENDP

; ecx = tab index -> g_path = that tab's file
path_for_tab PROC FRAME
    PROLOG
    mov eax, ecx
    mov QWORD PTR [rbp-8], rax
    lea rcx, QWORD PTR [g_path]
    lea rdx, QWORD PTR [fmt_tabpath]
    lea r8, QWORD PTR [g_dir]
    mov r9, QWORD PTR [rbp-8]
    call wsprintfW
    EPILOG
path_for_tab ENDP

; Writes every open tab, removes files of closed tabs, and saves the settings
save_all PROC FRAME
    PROLOG
    call sync_active
    mov DWORD PTR [rbp-8], 0
sa_save:
    mov eax, DWORD PTR [rbp-8]
    cmp eax, DWORD PTR [g_nTabs]
    jae sa_delete
    mov ecx, eax
    call save_tab_file
    inc DWORD PTR [rbp-8]
    jmp sa_save
sa_delete:
    mov eax, DWORD PTR [rbp-8]
    cmp eax, MAX_TABS
    jae sa_settings
    mov ecx, eax
    call path_for_tab
    lea rcx, QWORD PTR [g_path]
    call DeleteFileW
    inc DWORD PTR [rbp-8]
    jmp sa_delete
sa_settings:
    call save_stickies
    lea rcx, QWORD PTR [k_tabs]
    mov edx, DWORD PTR [g_nTabs]
    call set_int
    lea rcx, QWORD PTR [k_active]
    mov edx, DWORD PTR [g_active]
    call set_int
    lea rcx, QWORD PTR [k_theme]
    mov edx, DWORD PTR [g_theme]
    call set_int
    lea rcx, QWORD PTR [k_view]
    mov edx, DWORD PTR [g_view]
    call set_int
    EPILOG
save_all ENDP

; Restores settings and tabs from the previous session, or starts with one blank tab
load_all PROC FRAME
    PROLOG
    lea rcx, QWORD PTR [k_theme]
    xor edx, edx
    call get_int
    cmp eax, 5
    jbe la_theme_ok
    xor eax, eax
la_theme_ok:
    mov DWORD PTR [rbp-8], eax
    lea rcx, QWORD PTR [k_view]
    xor edx, edx
    call get_int
    and eax, 1
    mov DWORD PTR [g_view], eax
    lea rcx, QWORD PTR [k_tabs]
    mov edx, 1
    call get_int
    cmp eax, 0
    jne la_count_nonzero
    mov eax, 1
la_count_nonzero:
    cmp eax, MAX_TABS
    jbe la_count_ok
    mov eax, MAX_TABS
la_count_ok:
    mov DWORD PTR [rbp-16], eax
    mov ecx, DWORD PTR [rbp-8]
    call set_theme
    mov DWORD PTR [rbp-24], 0
la_tab:
    mov eax, DWORD PTR [rbp-24]
    cmp eax, DWORD PTR [rbp-16]
    jae la_tabs_done
    call append_tab
    mov ecx, eax
    call load_tab_file
    inc DWORD PTR [rbp-24]
    jmp la_tab
la_tabs_done:
    lea rcx, QWORD PTR [k_active]
    xor edx, edx
    call get_int
    cmp eax, DWORD PTR [g_nTabs]
    jb la_active_ok
    xor eax, eax
la_active_ok:
    mov ecx, eax
    call activate
    call load_stickies
    EPILOG
load_all ENDP

; rcx = source (UTF-16), rdx = length: writes g_path as UTF-8 via g_tmp, then renames
write_src PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    mov ecx, 65001
    xor edx, edx
    mov r8, QWORD PTR [rbp-8]
    mov r9, QWORD PTR [rbp-16]
    mov QWORD PTR [rsp+32], 0
    mov QWORD PTR [rsp+40], 0
    mov QWORD PTR [rsp+48], 0
    mov QWORD PTR [rsp+56], 0
    call WideCharToMultiByte
    mov QWORD PTR [rbp-24], rax
    mov ecx, eax
    add ecx, 8
    call heap_alloc
    mov QWORD PTR [rbp-32], rax
    mov ecx, 65001
    xor edx, edx
    mov r8, QWORD PTR [rbp-8]
    mov r9, QWORD PTR [rbp-16]
    mov QWORD PTR [rsp+32], rax
    mov rax, QWORD PTR [rbp-24]
    mov QWORD PTR [rsp+40], rax
    mov QWORD PTR [rsp+48], 0
    mov QWORD PTR [rsp+56], 0
    call WideCharToMultiByte
    lea rcx, QWORD PTR [g_tmp]
    lea rdx, QWORD PTR [fmt_tmp]
    lea r8, QWORD PTR [g_path]
    call wsprintfW
    lea rcx, QWORD PTR [g_tmp]
    mov edx, 40000000h
    xor r8d, r8d
    xor r9d, r9d
    mov DWORD PTR [rsp+32], 2
    mov DWORD PTR [rsp+40], 80h
    mov QWORD PTR [rsp+48], 0
    call CreateFileW
    cmp rax, -1
    je ws_free
    mov QWORD PTR [rbp-40], rax
    mov rcx, rax
    mov rdx, QWORD PTR [rbp-32]
    mov r8, QWORD PTR [rbp-24]
    lea r9, QWORD PTR [rbp-48]
    mov QWORD PTR [rsp+32], 0
    call WriteFile
    mov rcx, QWORD PTR [rbp-40]
    call CloseHandle
    lea rcx, QWORD PTR [g_tmp]
    lea rdx, QWORD PTR [g_path]
    mov r8d, 1
    call MoveFileExW
ws_free:
    mov rcx, QWORD PTR [rbp-32]
    call heap_free
    EPILOG
write_src ENDP

; rcx = destination, rdx = capacity in characters -> eax = characters read (0 if no file)
read_src PROC FRAME
    PROLOG
    mov QWORD PTR [rbp-8], rcx
    mov QWORD PTR [rbp-16], rdx
    mov DWORD PTR [rbp-56], 0
    lea rcx, QWORD PTR [g_path]
    mov edx, 80000000h
    mov r8d, 1
    xor r9d, r9d
    mov DWORD PTR [rsp+32], 3
    mov DWORD PTR [rsp+40], 80h
    mov QWORD PTR [rsp+48], 0
    call CreateFileW
    cmp rax, -1
    je rs_fail
    mov QWORD PTR [rbp-24], rax
    mov rcx, rax
    xor edx, edx
    call GetFileSize
    cmp eax, -1
    je rs_close
    cmp eax, 262144
    ja rs_close
    mov DWORD PTR [rbp-32], eax
    mov ecx, eax
    add ecx, 4
    call heap_alloc
    mov QWORD PTR [rbp-40], rax
    mov rcx, QWORD PTR [rbp-24]
    mov rdx, QWORD PTR [rbp-40]
    mov r8d, DWORD PTR [rbp-32]
    lea r9, QWORD PTR [rbp-48]
    mov QWORD PTR [rsp+32], 0
    call ReadFile
    mov rcx, QWORD PTR [rbp-24]
    call CloseHandle
    mov rax, QWORD PTR [rbp-8]
    mov QWORD PTR [rsp+32], rax
    mov eax, DWORD PTR [rbp-16]
    mov DWORD PTR [rsp+40], eax
    mov ecx, 65001
    xor edx, edx
    mov r8, QWORD PTR [rbp-40]
    mov r9d, DWORD PTR [rbp-32]
    call MultiByteToWideChar
    mov DWORD PTR [rbp-56], eax
    mov rdx, QWORD PTR [rbp-8]
    movsxd rax, DWORD PTR [rbp-56]
    mov WORD PTR [rdx + rax*2], 0
    mov rcx, QWORD PTR [rbp-40]
    call heap_free
    jmp rs_ret
rs_close:
    mov rcx, QWORD PTR [rbp-24]
    call CloseHandle
rs_fail:
    mov rdx, QWORD PTR [rbp-8]
    mov WORD PTR [rdx], 0
rs_ret:
    mov eax, DWORD PTR [rbp-56]
    EPILOG
read_src ENDP

; ecx = index -> g_path = that sticky's file
path_for_sticky PROC FRAME
    PROLOG
    mov eax, ecx
    mov QWORD PTR [rbp-8], rax
    lea rcx, QWORD PTR [g_path]
    lea rdx, QWORD PTR [fmt_stpath]
    lea r8, QWORD PTR [g_dir]
    mov r9, QWORD PTR [rbp-8]
    call wsprintfW
    EPILOG
path_for_sticky ENDP

; ecx = tab index: writes the tab's text to its file
save_tab_file PROC FRAME
    PROLOG
    mov eax, ecx
    mov QWORD PTR [rbp-8], rax
    mov ecx, eax
    call path_for_tab
    mov rax, QWORD PTR [rbp-8]
    lea r11, QWORD PTR [g_tabs]
    mov rcx, QWORD PTR [r11 + rax*8]
    mov rdx, QWORD PTR [rcx]
    mov QWORD PTR [rbp-16], rdx
    mov eax, DWORD PTR [rcx+8]
    mov QWORD PTR [rbp-24], rax
    mov rcx, QWORD PTR [rbp-16]
    mov rdx, QWORD PTR [rbp-24]
    call write_src
    EPILOG
save_tab_file ENDP

; ecx = tab index: reads the tab's file (if any) into the tab
load_tab_file PROC FRAME
    PROLOG
    mov eax, ecx
    mov QWORD PTR [rbp-8], rax
    mov ecx, eax
    call path_for_tab
    mov rax, QWORD PTR [rbp-8]
    lea r11, QWORD PTR [g_tabs]
    mov rcx, QWORD PTR [r11 + rax*8]
    mov QWORD PTR [rbp-16], rcx
    mov rcx, QWORD PTR [rcx]
    mov edx, MAX_CCH - 1
    call read_src
    mov rcx, QWORD PTR [rbp-16]
    mov DWORD PTR [rcx+8], eax
    mov ecx, DWORD PTR [rbp-8]
    call update_title
    EPILOG
load_tab_file ENDP

; ecx = key prefix pointer, edx = index -> g_key
key_build PROC FRAME
    PROLOG
    mov r8, rcx
    mov r9d, edx
    lea rcx, QWORD PTR [g_key]
    lea rdx, QWORD PTR [fmt_key]
    call wsprintfW
    EPILOG
key_build ENDP

; Writes every sticky's text and window placement, and removes files of closed stickies
save_stickies PROC FRAME
    PROLOG
    mov DWORD PTR [rbp-8], 0
sv_loop:
    mov eax, DWORD PTR [rbp-8]
    cmp eax, DWORD PTR [g_nstk]
    jae sv_del
    lea r11, QWORD PTR [g_stk]
    mov rax, QWORD PTR [r11 + rax*8]
    mov QWORD PTR [rbp-16], rax
    mov ecx, DWORD PTR [rbp-8]
    call path_for_sticky
    mov rax, QWORD PTR [rbp-16]
    mov rcx, QWORD PTR [rax]
    mov rdx, QWORD PTR [rax+8]
    call write_src
    mov rax, QWORD PTR [rbp-16]
    mov rcx, QWORD PTR [rax+16]
    lea rdx, QWORD PTR [rbp-72]
    call GetWindowRect
    lea rcx, QWORD PTR [s_kx]
    mov edx, DWORD PTR [rbp-8]
    call key_build
    lea rcx, QWORD PTR [g_key]
    mov edx, DWORD PTR [rbp-72]
    call set_int
    lea rcx, QWORD PTR [s_ky]
    mov edx, DWORD PTR [rbp-8]
    call key_build
    lea rcx, QWORD PTR [g_key]
    mov edx, DWORD PTR [rbp-68]
    call set_int
    lea rcx, QWORD PTR [s_kw]
    mov edx, DWORD PTR [rbp-8]
    call key_build
    lea rcx, QWORD PTR [g_key]
    mov edx, DWORD PTR [rbp-64]
    sub edx, DWORD PTR [rbp-72]
    call set_int
    lea rcx, QWORD PTR [s_kh]
    mov edx, DWORD PTR [rbp-8]
    call key_build
    lea rcx, QWORD PTR [g_key]
    mov edx, DWORD PTR [rbp-60]
    sub edx, DWORD PTR [rbp-68]
    call set_int
    lea rcx, QWORD PTR [s_kp]
    mov edx, DWORD PTR [rbp-8]
    call key_build
    mov rax, QWORD PTR [rbp-16]
    lea rcx, QWORD PTR [g_key]
    mov edx, DWORD PTR [rax+32]
    call set_int
    inc DWORD PTR [rbp-8]
    jmp sv_loop
sv_del:
    mov eax, DWORD PTR [rbp-8]
    cmp eax, MAX_STK
    jae sv_count
    mov ecx, eax
    call path_for_sticky
    lea rcx, QWORD PTR [g_path]
    call DeleteFileW
    inc DWORD PTR [rbp-8]
    jmp sv_del
sv_count:
    lea rcx, QWORD PTR [k_stickies]
    mov edx, DWORD PTR [g_nstk]
    call set_int
    EPILOG
save_stickies ENDP

; Restores stickies from the previous session
load_stickies PROC FRAME
    PROLOG
    lea rcx, QWORD PTR [k_stickies]
    xor edx, edx
    call get_int
    cmp eax, MAX_STK
    jbe ls_count_ok
    mov eax, MAX_STK
ls_count_ok:
    mov DWORD PTR [rbp-8], eax
    mov DWORD PTR [rbp-16], 0
ls_loop:
    mov eax, DWORD PTR [rbp-16]
    cmp eax, DWORD PTR [rbp-8]
    jae ls_done
    mov ecx, eax
    call path_for_sticky
    mov ecx, MAX_CCH * 2
    call heap_alloc
    mov QWORD PTR [rbp-24], rax
    mov rcx, rax
    mov edx, MAX_CCH - 1
    call read_src
    mov DWORD PTR [rbp-32], eax
    mov eax, DWORD PTR [rbp-16]
    imul eax, eax, 32
    add eax, 120
    mov DWORD PTR [rbp-40], eax
    lea rcx, QWORD PTR [s_kx]
    mov edx, DWORD PTR [rbp-16]
    call key_build
    lea rcx, QWORD PTR [g_key]
    mov edx, DWORD PTR [rbp-40]
    call get_int
    mov DWORD PTR [rbp-48], eax
    lea rcx, QWORD PTR [s_ky]
    mov edx, DWORD PTR [rbp-16]
    call key_build
    lea rcx, QWORD PTR [g_key]
    mov edx, DWORD PTR [rbp-40]
    call get_int
    mov DWORD PTR [rbp-56], eax
    lea rcx, QWORD PTR [s_kw]
    mov edx, DWORD PTR [rbp-16]
    call key_build
    lea rcx, QWORD PTR [g_key]
    mov edx, 320
    call get_int
    mov DWORD PTR [rbp-64], eax
    lea rcx, QWORD PTR [s_kh]
    mov edx, DWORD PTR [rbp-16]
    call key_build
    lea rcx, QWORD PTR [g_key]
    mov edx, 360
    call get_int
    mov DWORD PTR [rbp-72], eax
    lea rcx, QWORD PTR [s_kp]
    mov edx, DWORD PTR [rbp-16]
    call key_build
    lea rcx, QWORD PTR [g_key]
    xor edx, edx
    call get_int
    mov DWORD PTR [rbp-80], eax
    mov rcx, QWORD PTR [rbp-24]
    mov edx, DWORD PTR [rbp-32]
    mov r8d, DWORD PTR [rbp-48]
    mov r9d, DWORD PTR [rbp-56]
    mov eax, DWORD PTR [rbp-64]
    mov DWORD PTR [rsp+32], eax
    mov eax, DWORD PTR [rbp-72]
    mov DWORD PTR [rsp+40], eax
    mov eax, DWORD PTR [rbp-80]
    mov DWORD PTR [rsp+48], eax
    call create_sticky
    inc DWORD PTR [rbp-16]
    jmp ls_loop
ls_done:
    EPILOG
load_stickies ENDP
