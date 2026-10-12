; AsmNotes - x64 MASM note-taking app for Windows
;
; Frame convention used by every procedure: push rbp / mov rbp,rsp / sub rsp,256.
; Outgoing calls use [rsp+0..95] (shadow space plus stack arguments); locals live
; at [rbp-8] .. [rbp-160], which never overlaps that area.

INCLUDE consts.inc

PROLOG MACRO
    push rbp
    .pushreg rbp
    mov rbp, rsp
    .setframe rbp, 0
    sub rsp, 256
    .allocstack 256
    .endprolog
ENDM

EPILOG MACRO
    mov rsp, rbp
    pop rbp
    ret
ENDM

EXTERN GetProcessHeap:PROC
EXTERN HeapAlloc:PROC
EXTERN HeapFree:PROC
EXTERN GetModuleHandleW:PROC
EXTERN InitCommonControlsEx:PROC
EXTERN LoadLibraryW:PROC
EXTERN LoadCursorW:PROC
EXTERN LoadAcceleratorsW:PROC
EXTERN RegisterClassExW:PROC
EXTERN CreateWindowExW:PROC
EXTERN DefWindowProcW:PROC
EXTERN ShowWindow:PROC
EXTERN GetMessageW:PROC
EXTERN TranslateAcceleratorW:PROC
EXTERN TranslateMessage:PROC
EXTERN DispatchMessageW:PROC
EXTERN PostQuitMessage:PROC
EXTERN DestroyWindow:PROC
EXTERN SendMessageW:PROC
EXTERN SetWindowTextW:PROC
EXTERN GetWindowTextW:PROC
EXTERN MoveWindow:PROC
EXTERN SetTimer:PROC
EXTERN KillTimer:PROC
EXTERN SetFocus:PROC
EXTERN GetMenu:PROC
EXTERN CheckMenuRadioItem:PROC
EXTERN DwmSetWindowAttribute:PROC
EXTERN InvalidateRect:PROC
EXTERN wsprintfW:PROC
EXTERN ExitProcess:PROC
EXTERN GetEnvironmentVariableW:PROC
EXTERN CreateDirectoryW:PROC
EXTERN GetPrivateProfileIntW:PROC
EXTERN WritePrivateProfileStringW:PROC
EXTERN CreateFileW:PROC
EXTERN ReadFile:PROC
EXTERN WriteFile:PROC
EXTERN GetFileSize:PROC
EXTERN CloseHandle:PROC
EXTERN MoveFileExW:PROC
EXTERN DeleteFileW:PROC
EXTERN WideCharToMultiByte:PROC
EXTERN MultiByteToWideChar:PROC

.DATA
ALIGN 8
g_hInst     QWORD 0
g_hHeap     QWORD 0
g_hMain     QWORD 0
g_hEdit     QWORD 0
g_hTabs     QWORD 0
g_hAccel    QWORD 0
g_nTabs     DWORD 0
g_active    DWORD 0
g_theme     DWORD 0
g_view      DWORD 0
g_tabs      QWORD MAX_TABS DUP(0)
g_title     WORD 64 DUP(0)
g_cf        BYTE 116 DUP(0)
g_icc       DWORD 8, ICC_TAB_CLASSES

; Theme palette: background, text, accent, muted as COLORREF (0x00BBGGRR)
g_palette   DWORD 0F1F8FBh, 02B2B2Bh, 0AD5F1Fh, 07A7A7Ah   ; 0 Paper
            DWORD 0030251Eh, 0F0EAE6h, 0FFB77Fh, 0B1A59Ah   ; 1 Slate
            DWORD 0D8ECF4h, 0036465Bh, 01A4A9Ch, 060758Ah   ; 2 Sepia
            DWORD 0362B00h, 0A1A193h, 0D28B26h, 00837B65h   ; 3 Solarized Dark
            DWORD 0ECF5EEh, 0243A1Fh, 0327D2Eh, 0627D5Eh   ; 4 Forest
            DWORD 0020100Bh, 0EEDED7h, 06BB8FFh, 0A8887Ch   ; 5 Midnight

s_cls_main  WORD 'A','s','m','N','o','t','e','s',0
s_title     WORD 'A','s','m','N','o','t','e','s',0
s_tabclass  WORD 'S','y','s','T','a','b','C','o','n','t','r','o','l','3','2',0
s_richedit  WORD 'R','I','C','H','E','D','I','T','5','0','W',0
s_msftedit  WORD 'M','s','f','t','e','d','i','t','.','d','l','l',0
fmt_note    WORD 'N','o','t','e',' ','%','d',0
s_appdata   WORD 'A','P','P','D','A','T','A',0
fmt_dir     WORD '%','s',5Ch,'A','s','m','N','o','t','e','s',0
fmt_ini     WORD '%','s',5Ch,'s','e','t','t','i','n','g','s','.','i','n','i',0
fmt_tabpath WORD '%','s',5Ch,'t','a','b','%','d','.','m','d',0
fmt_tmp     WORD '%','s','.','t','m','p',0
fmt_int     WORD '%','d',0
s_sec_notes WORD 'N','o','t','e','s',0
k_tabs      WORD 'T','a','b','s',0
k_active    WORD 'A','c','t','i','v','e',0
k_theme     WORD 'T','h','e','m','e',0
k_view      WORD 'V','i','e','w',0
g_appdata   WORD 300 DUP(0)
g_dir       WORD 300 DUP(0)
g_path      WORD 320 DUP(0)
g_tmp       WORD 330 DUP(0)
g_ini       WORD 320 DUP(0)
g_num       WORD 32 DUP(0)

.CODE

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
    cmp DWORD PTR [g_view], 0
    jne sa_done
    mov eax, DWORD PTR [g_active]
    lea r11, QWORD PTR [g_tabs]
    mov rcx, QWORD PTR [r11 + rax*8]
    mov rdx, QWORD PTR [rcx]
    mov rcx, QWORD PTR [g_hEdit]
    call SetWindowTextW
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

; ecx = tab index: writes it as UTF-8 to a temp file, then renames over the real file
save_tab_file PROC FRAME
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
    mov ecx, 65001
    xor edx, edx
    mov r8, QWORD PTR [rbp-24]
    mov r9, QWORD PTR [rbp-32]
    mov QWORD PTR [rsp+32], 0
    mov QWORD PTR [rsp+40], 0
    mov QWORD PTR [rsp+48], 0
    mov QWORD PTR [rsp+56], 0
    call WideCharToMultiByte
    mov QWORD PTR [rbp-40], rax
    mov ecx, eax
    add ecx, 8
    call heap_alloc
    mov QWORD PTR [rbp-48], rax
    mov ecx, 65001
    xor edx, edx
    mov r8, QWORD PTR [rbp-24]
    mov r9, QWORD PTR [rbp-32]
    mov QWORD PTR [rsp+32], rax
    mov rax, QWORD PTR [rbp-40]
    mov QWORD PTR [rsp+40], rax
    mov QWORD PTR [rsp+48], 0
    mov QWORD PTR [rsp+56], 0
    call WideCharToMultiByte
    mov rcx, QWORD PTR [rbp-8]
    call path_for_tab
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
    je stf_free
    mov QWORD PTR [rbp-56], rax
    mov rcx, rax
    mov rdx, QWORD PTR [rbp-48]
    mov r8, QWORD PTR [rbp-40]
    lea r9, QWORD PTR [rbp-64]
    mov QWORD PTR [rsp+32], 0
    call WriteFile
    mov rcx, QWORD PTR [rbp-56]
    call CloseHandle
    lea rcx, QWORD PTR [g_tmp]
    lea rdx, QWORD PTR [g_path]
    mov r8d, 1
    call MoveFileExW
stf_free:
    mov rcx, QWORD PTR [rbp-48]
    call heap_free
    EPILOG
save_tab_file ENDP

; ecx = tab index: reads its file (if any) into the tab
load_tab_file PROC FRAME
    PROLOG
    mov eax, ecx
    mov QWORD PTR [rbp-8], rax
    call path_for_tab
    lea rcx, QWORD PTR [g_path]
    mov edx, 80000000h
    mov r8d, 1
    xor r9d, r9d
    mov DWORD PTR [rsp+32], 3
    mov DWORD PTR [rsp+40], 80h
    mov QWORD PTR [rsp+48], 0
    call CreateFileW
    cmp rax, -1
    je ltf_done
    mov QWORD PTR [rbp-16], rax
    mov rcx, rax
    xor edx, edx
    call GetFileSize
    cmp eax, -1
    je ltf_close_only
    cmp eax, 262144
    ja ltf_close_only
    mov DWORD PTR [rbp-24], eax
    mov ecx, eax
    add ecx, 4
    call heap_alloc
    mov QWORD PTR [rbp-32], rax
    mov rcx, QWORD PTR [rbp-16]
    mov rdx, QWORD PTR [rbp-32]
    mov r8d, DWORD PTR [rbp-24]
    lea r9, QWORD PTR [rbp-40]
    mov QWORD PTR [rsp+32], 0
    call ReadFile
    mov rcx, QWORD PTR [rbp-16]
    call CloseHandle
    mov rax, QWORD PTR [rbp-8]
    lea r11, QWORD PTR [g_tabs]
    mov rcx, QWORD PTR [r11 + rax*8]
    mov QWORD PTR [rbp-48], rcx
    mov rax, QWORD PTR [rcx]
    mov QWORD PTR [rsp+32], rax
    mov DWORD PTR [rsp+40], MAX_CCH - 1
    mov ecx, 65001
    xor edx, edx
    mov r8, QWORD PTR [rbp-32]
    mov r9d, DWORD PTR [rbp-24]
    call MultiByteToWideChar
    mov rcx, QWORD PTR [rbp-48]
    mov DWORD PTR [rcx+8], eax
    mov rdx, QWORD PTR [rcx]
    mov eax, DWORD PTR [rcx+8]
    mov WORD PTR [rdx + rax*2], 0
    mov rcx, QWORD PTR [rbp-32]
    call heap_free
    jmp ltf_title
ltf_close_only:
    mov rcx, QWORD PTR [rbp-16]
    call CloseHandle
ltf_title:
    mov ecx, DWORD PTR [rbp-8]
    call update_title
ltf_done:
    EPILOG
load_tab_file ENDP

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
    EPILOG
load_all ENDP

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

; ecx = theme index
set_theme PROC FRAME
    PROLOG
    mov DWORD PTR [g_theme], ecx
    mov eax, ecx
    shl eax, 4
    lea rdx, QWORD PTR [g_palette]
    add rdx, rax
    mov QWORD PTR [rbp-8], rdx
    mov r9d, DWORD PTR [rdx]
    mov rcx, QWORD PTR [g_hEdit]
    mov edx, EM_SETBKGNDCOLOR
    xor r8d, r8d
    call SendMessageW
    mov rdx, QWORD PTR [rbp-8]
    mov eax, DWORD PTR [rdx+4]
    mov DWORD PTR [g_cf+20], eax
    mov DWORD PTR [g_cf], 116
    mov DWORD PTR [g_cf+4], CFM_COLOR
    mov rcx, QWORD PTR [g_hEdit]
    mov edx, EM_SETCHARFORMAT
    xor r8d, r8d
    lea r9, QWORD PTR [g_cf]
    call SendMessageW
    mov rcx, QWORD PTR [g_hEdit]
    mov edx, EM_SETCHARFORMAT
    mov r8d, SCF_ALL
    lea r9, QWORD PTR [g_cf]
    call SendMessageW
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
    xor ecx, ecx
    lea rdx, QWORD PTR [s_tabclass]
    xor r8d, r8d
    mov r9d, WS_CHILD + WS_VISIBLE + WS_TABSTOP
    mov DWORD PTR [rsp+32], 0
    mov DWORD PTR [rsp+40], 0
    mov DWORD PTR [rsp+48], 0
    mov DWORD PTR [rsp+56], TABH
    mov rax, QWORD PTR [g_hMain]
    mov QWORD PTR [rsp+64], rax
    mov QWORD PTR [rsp+72], IDC_TABS
    mov rax, QWORD PTR [g_hInst]
    mov QWORD PTR [rsp+80], rax
    mov QWORD PTR [rsp+88], 0
    call CreateWindowExW
    mov QWORD PTR [g_hTabs], rax

    mov ecx, WS_EX_CLIENTEDGE
    lea rdx, QWORD PTR [s_richedit]
    xor r8d, r8d
    mov r9d, WS_CHILD + WS_VISIBLE + WS_VSCROLL + WS_TABSTOP + ES_MULTILINE + ES_AUTOVSCROLL + ES_WANTRETURN + ES_NOHIDESEL
    mov DWORD PTR [rsp+32], 0
    mov DWORD PTR [rsp+40], TABH
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

    call load_all
    jmp mp_zero

mp_size:
    mov eax, DWORD PTR [rbp-32]
    movzx eax, ax
    mov DWORD PTR [rbp-40], eax
    mov eax, DWORD PTR [rbp-32]
    shr eax, 16
    mov DWORD PTR [rbp-48], eax
    sub eax, TABH
    jnc ms_h_ok
    xor eax, eax
ms_h_ok:
    mov DWORD PTR [rbp-56], eax
    mov rcx, QWORD PTR [g_hTabs]
    xor edx, edx
    xor r8d, r8d
    mov r9d, DWORD PTR [rbp-40]
    mov DWORD PTR [rsp+32], TABH
    mov DWORD PTR [rsp+40], 1
    call MoveWindow
    mov rcx, QWORD PTR [g_hEdit]
    xor edx, edx
    mov r8d, TABH
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

mp_destroy:
    xor ecx, ecx
    call PostQuitMessage
    jmp mp_zero
main_proc ENDP

PUBLIC start
start PROC FRAME
    PROLOG
    call GetProcessHeap
    mov QWORD PTR [g_hHeap], rax
    call storage_init
    xor ecx, ecx
    call GetModuleHandleW
    mov QWORD PTR [g_hInst], rax
    lea rcx, QWORD PTR [g_icc]
    call InitCommonControlsEx
    lea rcx, QWORD PTR [s_msftedit]
    call LoadLibraryW

    mov DWORD PTR [rbp-96], 80
    mov DWORD PTR [rbp-92], CS_HREDRAW + CS_VREDRAW
    lea rax, QWORD PTR [main_proc]
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
    mov QWORD PTR [rbp-40], IDR_MAINMENU
    lea rax, QWORD PTR [s_cls_main]
    mov QWORD PTR [rbp-32], rax
    mov QWORD PTR [rbp-24], 0
    lea rcx, QWORD PTR [rbp-96]
    call RegisterClassExW

    mov rcx, QWORD PTR [g_hInst]
    mov edx, IDR_ACCEL
    call LoadAcceleratorsW
    mov QWORD PTR [g_hAccel], rax

    xor ecx, ecx
    lea rdx, QWORD PTR [s_cls_main]
    lea r8, QWORD PTR [s_title]
    mov r9d, WS_OVERLAPPEDWINDOW + WS_VISIBLE
    mov DWORD PTR [rsp+32], CW_USEDEFAULT
    mov DWORD PTR [rsp+40], CW_USEDEFAULT
    mov DWORD PTR [rsp+48], 1000
    mov DWORD PTR [rsp+56], 700
    mov QWORD PTR [rsp+64], 0
    mov QWORD PTR [rsp+72], 0
    mov rax, QWORD PTR [g_hInst]
    mov QWORD PTR [rsp+80], rax
    mov QWORD PTR [rsp+88], 0
    call CreateWindowExW
    test rax, rax
    jz st_exit

msg_loop:
    lea rcx, QWORD PTR [rbp-160]
    xor edx, edx
    xor r8d, r8d
    xor r9d, r9d
    call GetMessageW
    test eax, eax
    jle st_exit
    mov rcx, QWORD PTR [g_hMain]
    mov rdx, QWORD PTR [g_hAccel]
    lea r8, QWORD PTR [rbp-160]
    call TranslateAcceleratorW
    test eax, eax
    jnz msg_loop
    lea rcx, QWORD PTR [rbp-160]
    call TranslateMessage
    lea rcx, QWORD PTR [rbp-160]
    call DispatchMessageW
    jmp msg_loop

st_exit:
    xor ecx, ecx
    call ExitProcess
start ENDP

END
