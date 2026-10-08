.CODE

INCLUDE consts.inc
INCLUDE util.inc
INCLUDE themes.inc
INCLUDE markdown.inc
INCLUDE storage.inc
INCLUDE sticky.inc
INCLUDE mainwnd.inc

EXTERN GetModuleHandleW : PROC
EXTERN LoadAcceleratorsW : PROC
EXTERN GetMessageW : PROC
EXTERN TranslateMessage : PROC
EXTERN DispatchMessageW : PROC
EXTERN PostQuitMessage : PROC
EXTERN ExitProcess : PROC

; Global state
g_hInstance QWORD 0
g_hMainWnd QWORD 0
g_hAccel QWORD 0
g_pTabNotes QWORD 0
g_cTabs DWORD 0
g_iActiveTab DWORD 0

PUBLIC start
start PROC
    push rbp
    mov rbp, rsp
    sub rsp, 80h                ; stack for msg struct

    push r12
    push r13

    mov r12, rcx                ; argc
    mov r13, rdx                ; argv

    ; Get module handle
    xor ecx, ecx
    call GetModuleHandleW
    mov QWORD PTR [g_hInstance], rax

    ; Initialize themes
    call ThemesInit

    ; Initialize storage
    call StorageInit

    ; Create main window
    lea rcx, QWORD PTR [g_hMainWnd]
    call MainWndCreate

    ; Load accelerators
    mov rcx, QWORD PTR [g_hInstance]
    mov rdx, 101h               ; IDR_ACCEL
    call LoadAcceleratorsW
    mov QWORD PTR [g_hAccel], rax

    ; Message loop
    lea rdi, [rsp + 16]

msg_loop:
    mov rcx, rdi
    xor edx, edx                ; hWnd = NULL
    xor r8d, r8d                ; wMsgFilterMin = 0
    xor r9d, r9d                ; wMsgFilterMax = 0
    call GetMessageW
    cmp eax, 0
    jle msg_done

    mov rcx, rdi
    call TranslateMessage

    mov rcx, rdi
    call DispatchMessageW

    jmp msg_loop

msg_done:
    ; Save state on exit
    call StorageShutdown

    mov ecx, 0
    call ExitProcess

start ENDP

END
