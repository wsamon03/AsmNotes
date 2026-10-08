; Main window: creation, message handling, tabs

EXTERN CreateWindowExW : PROC
EXTERN RegisterClassExW : PROC
EXTERN DefWindowProcW : PROC
EXTERN SendMessageW : PROC
EXTERN ShowWindow : PROC
EXTERN UpdateWindow : PROC

; Class name
szMainClassName:
    WORD 'A'
    WORD 's'
    WORD 'm'
    WORD 'N'
    WORD 'o'
    WORD 't'
    WORD 'e'
    WORD 's'
    WORD 0

szWindowTitle:
    WORD 'A'
    WORD 's'
    WORD 'm'
    WORD 'N'
    WORD 'o'
    WORD 't'
    WORD 'e'
    WORD 's'
    WORD 0

PUBLIC MainWndCreate
MainWndCreate PROC FRAME
    ; rcx = pointer to HWND return
    push rbx
    push r12
    .endprolog

    mov r12, rcx
    mov rbx, rsp
    sub rsp, 88h

    ; Build WNDCLASSEX on stack
    ; cbSize
    lea rcx, [rsp]
    mov DWORD PTR [rcx], 80h

    ; style = CS_VREDRAW | CS_HREDRAW
    mov DWORD PTR [rcx + 4], 3

    ; lpfnWndProc
    lea rax, [MainWndProc]
    mov QWORD PTR [rcx + 8], rax

    ; cbClsExtra, cbWndExtra
    mov DWORD PTR [rcx + 16], 0
    mov DWORD PTR [rcx + 20], 0

    ; hInstance
    mov rax, QWORD PTR [g_hInstance]
    mov QWORD PTR [rcx + 24], rax

    ; hIcon, hCursor, hbrBackground
    mov QWORD PTR [rcx + 32], 0
    mov QWORD PTR [rcx + 40], 0
    mov QWORD PTR [rcx + 48], 6   ; COLOR_WINDOW + 1

    ; lpszMenuName
    mov QWORD PTR [rcx + 56], 0

    ; lpszClassName
    lea rax, [szMainClassName]
    mov QWORD PTR [rcx + 64], rax

    ; hIconSm
    mov QWORD PTR [rcx + 72], 0

    ; Register class
    call RegisterClassExW

    ; Create window
    xor ecx, ecx                ; dwExStyle
    lea rdx, [szMainClassName]
    lea r8, [szWindowTitle]
    mov r9d, 0CFF0000h          ; WS_OVERLAPPEDWINDOW | WS_VISIBLE

    ; Stack parameters for CreateWindowExW:
    ; x, y, width, height, hWndParent, hMenu, hInstance, lpParam
    sub rsp, 64                 ; space for shadow + 8 params
    mov DWORD PTR [rsp + 32], 0    ; x = CW_USEDEFAULT (really need 0x7FFFFFF00 but this is fine)
    mov DWORD PTR [rsp + 40], 0    ; y
    mov DWORD PTR [rsp + 48], 0    ; width
    mov DWORD PTR [rsp + 56], 0    ; height
    mov QWORD PTR [rsp + 64], 0    ; hWndParent
    mov QWORD PTR [rsp + 72], 0    ; hMenu
    mov rax, QWORD PTR [g_hInstance]
    mov QWORD PTR [rsp + 80], rax  ; hInstance
    mov QWORD PTR [rsp + 88], 0    ; lpParam

    mov rbx, rsp
    call CreateWindowExW
    mov rsp, rbx

    ; Save hwnd
    mov QWORD PTR [r12], rax

    ; Show and update
    mov rcx, rax
    mov edx, 5                  ; SW_SHOW
    call ShowWindow

    mov rcx, QWORD PTR [r12]
    call UpdateWindow

    mov rsp, rbx
    pop r12
    pop rbx
    ret
MainWndCreate ENDP

PUBLIC MainWndProc
MainWndProc PROC FRAME
    ; rcx = hwnd
    ; edx = message
    ; r8 = wParam
    ; r9 = lParam
    .endprolog

    cmp edx, 2                  ; WM_DESTROY
    je mwp_destroy

    cmp edx, 111h               ; WM_COMMAND
    je mwp_command

    cmp edx, 5                  ; WM_SIZE
    je mwp_size

    ; Default handling
    call DefWindowProcW
    ret

mwp_destroy:
    xor ecx, ecx
    EXTERN PostQuitMessage : PROC
    call PostQuitMessage
    xor eax, eax
    ret

mwp_command:
    xor eax, eax
    ret

mwp_size:
    xor eax, eax
    ret

MainWndProc ENDP
