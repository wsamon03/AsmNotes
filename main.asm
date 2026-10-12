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
EXTERN GetCommandLineW:PROC
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
EXTERN GetSystemMenu:PROC
EXTERN GetDpiForWindow:PROC
EXTERN TrackMouseEvent:PROC
EXTERN FrameRect:PROC
EXTERN SetWindowTheme:PROC
EXTERN GetDC:PROC
EXTERN ReleaseDC:PROC
EXTERN DefSubclassProc:PROC
EXTERN SetWindowSubclass:PROC
EXTERN GetProcAddress:PROC
EXTERN DrawMenuBar:PROC
EXTERN EndPaint:PROC
EXTERN BeginPaint:PROC
EXTERN GetClientRect:PROC
EXTERN DeleteObject:PROC
EXTERN CreateSolidBrush:PROC
EXTERN SetTextColor:PROC
EXTERN SetBkMode:PROC
EXTERN DrawTextW:PROC
EXTERN FillRect:PROC
EXTERN AppendMenuW:PROC
EXTERN CheckMenuItem:PROC
EXTERN SetWindowLongPtrW:PROC
EXTERN GetWindowLongPtrW:PROC
EXTERN GetWindowRect:PROC
EXTERN SetWindowPos:PROC
EXTERN GetMenuItemCount:PROC
EXTERN GetSubMenu:PROC
EXTERN GetMenuItemID:PROC
EXTERN ModifyMenuW:PROC
EXTERN GetMenuStringW:PROC
EXTERN GetMenuItemInfoW:PROC
EXTERN GetMenuItemRect:PROC
EXTERN GetMenuBarInfo:PROC
EXTERN GetWindowDC:PROC
EXTERN CreateFontIndirectW:PROC
EXTERN SelectObject:PROC
















; ecx = tab index: writes it as UTF-8 to a temp file, then renames over the real file







; ---- Markdown rendering -----------------------------------------------------
; The formatted view is built as plain UTF-16 text plus formatting runs. Runs are
; applied to the editor with EM_SETCHARFORMAT and EM_SETPARAFORMAT.
;
; Run flags: 1 bold, 2 italic, 4 underline, 8 strike, 16 code font, 32 accent colour,
; 64 muted colour.
; Span record (32 bytes): start, length, mask, effects, height (twips), colour, pad, pad.
; Paragraph record (16 bytes): start, left indent (twips), first-line offset (twips).











; Shows the active tab as formatted text in the editor
; ---- Files --------------------------------------------------------------------






; ---- Editor styling and rendering ---------------------------------------------





; ---- Stickies -----------------------------------------------------------------
; Sticky record (48 bytes): [0] source buffer, [8] length, [16] window, [24] editor,
; [32] pinned flag
































INCLUDE globals.inc
.CODE

INCLUDE util.inc
INCLUDE storage.inc
INCLUDE markdown.inc
INCLUDE tabs.inc
INCLUDE menubar.inc
INCLUDE sticky.inc
INCLUDE render.inc
INCLUDE mainwnd.inc

PUBLIC start
start PROC FRAME
    PROLOG
    call GetProcessHeap
    mov QWORD PTR [g_hHeap], rax
    call storage_init
    call parse_render
    test eax, eax
    jz st_normal
    call render_file
st_normal:
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

    call reg_sticky_class
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
