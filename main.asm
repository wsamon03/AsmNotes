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
start PROC FRAME
    push rbp
    mov rbp, rsp
    sub rsp, 128                ; stack for msg struct and shadow space
    .endprolog

    push r12
    push r13

    mov r12, rcx                ; argc
    mov r13, rdx                ; argv

    ; Initialize heap
    call HeapInit

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

    mov rax, QWORD PTR [g_hMainWnd]
    test rax, rax
    jz start_exit                ; if window creation failed, exit

    ; Load accelerators (optional)
    mov rcx, QWORD PTR [g_hInstance]
    mov rdx, 102h               ; IDR_ACCEL
    call LoadAcceleratorsW
    mov QWORD PTR [g_hAccel], rax

    ; Message loop
    lea rdi, [rsp + 32]         ; address of MSG on stack

msg_loop:
    mov rcx, rdi
    xor edx, edx                ; hWnd = NULL (get all)
    xor r8d, r8d                ; wMsgFilterMin = 0
    xor r9d, r9d                ; wMsgFilterMax = 0
    call GetMessageW
    cmp eax, 0
    jle msg_done                ; GetMessageW returns 0 when WM_QUIT received

    ; Translate and dispatch
    mov rcx, rdi
    call TranslateMessage

    mov rcx, rdi
    call DispatchMessageW

    jmp msg_loop

msg_done:
    ; Save state on exit
    call StorageShutdown

start_exit:
    mov ecx, 0
    call ExitProcess

    pop r13
    pop r12
    mov rsp, rbp
    pop rbp
    ret

start ENDP

END
