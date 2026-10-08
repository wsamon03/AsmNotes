; Main window: creation, message handling, tabs

EXTERN CreateWindowExW : PROC
EXTERN RegisterClassExW : PROC
EXTERN DefWindowProcW : PROC
EXTERN PostQuitMessage : PROC
EXTERN ShowWindow : PROC
EXTERN UpdateWindow : PROC


; Window class names
szMainClassName:
    WORD 'A', 's', 'm', 'N', 'o', 't', 'e', 's', 0

szWindowTitle:
    WORD 'A', 's', 'm', 'N', 'o', 't', 'e', 's', 0

PUBLIC MainWndCreate
MainWndCreate PROC FRAME
    push rbp
    mov rbp, rsp
    sub rsp, 80h
    .endprolog

    push rbx
    push r12

    mov r12, rcx                ; save hwnd return pointer

    ; Register window class (simplified)
    lea rax, [rsp]
    mov DWORD PTR [rax], 80h    ; cbSize
    mov DWORD PTR [rax + 4], 3  ; style
    lea rbx, [MainWndProc]
    mov QWORD PTR [rax + 8], rbx   ; lpfnWndProc

    mov rbx, QWORD PTR [g_hInstance]
    mov QWORD PTR [rax + 24], rbx  ; hInstance
    mov QWORD PTR [rax + 48], 6    ; hbrBackground
    lea rbx, [szMainClassName]
    mov QWORD PTR [rax + 64], rbx  ; lpszClassName

    mov rcx, rax
    call RegisterClassExW

    ; Create window
    xor ecx, ecx                ; exStyle
    lea rdx, [szMainClassName]
    lea r8, [szWindowTitle]
    mov r9d, 0CF0000h          ; WS_OVERLAPPEDWINDOW

    sub rsp, 32
    mov DWORD PTR [rsp], 080000000h  ; x
    mov DWORD PTR [rsp + 4], 080000000h  ; y
    mov DWORD PTR [rsp + 8], 080000000h  ; width
    mov DWORD PTR [rsp + 12], 080000000h ; height
    mov QWORD PTR [rsp + 16], 0     ; hWndParent
    mov QWORD PTR [rsp + 24], 0     ; hMenu

    call CreateWindowExW
    add rsp, 32

    ; Save hwnd
    mov QWORD PTR [r12], rax

    ; Show window
    mov rcx, rax
    mov edx, 5                  ; SW_SHOW
    call ShowWindow

    mov rcx, QWORD PTR [r12]
    call UpdateWindow

    pop r12
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

MainWndCreate ENDP

PUBLIC MainWndProc
MainWndProc PROC FRAME
    push rbp
    mov rbp, rsp
    .endprolog

    cmp edx, 2                  ; WM_DESTROY
    je mwp_destroy

    ; Default
    call DefWindowProcW
    jmp mwp_end

mwp_destroy:
    xor ecx, ecx
    call PostQuitMessage
    xor eax, eax

mwp_end:
    mov rsp, rbp
    pop rbp
    ret

MainWndProc ENDP