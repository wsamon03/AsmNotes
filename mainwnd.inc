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

    ; Check if class registration succeeded
    test eax, eax
    jz mwc_fail

    ; Create window
    ; Stack layout for CreateWindowExW:
    ; [rsp+0-31]   = shadow space (32 bytes)
    ; [rsp+32]     = x (CW_USEDEFAULT)
    ; [rsp+40]     = y (CW_USEDEFAULT)
    ; [rsp+48]     = nWidth (CW_USEDEFAULT)
    ; [rsp+56]     = nHeight (CW_USEDEFAULT)
    ; [rsp+64]     = hWndParent (NULL)
    ; [rsp+72]     = hMenu (NULL)
    ; [rsp+80]     = hInstance
    ; [rsp+88]     = lpParam (NULL)

    xor ecx, ecx                ; exStyle = 0
    lea rdx, [szMainClassName]
    lea r8, [szWindowTitle]
    mov r9d, 0CF0000h          ; WS_OVERLAPPEDWINDOW | WS_VISIBLE

    sub rsp, 96                 ; shadow space + 6 stack params

    ; Set window creation parameters
    mov DWORD PTR [rsp + 32], 080000000h  ; x = CW_USEDEFAULT
    mov DWORD PTR [rsp + 40], 080000000h  ; y = CW_USEDEFAULT
    mov DWORD PTR [rsp + 48], 080000000h  ; nWidth = CW_USEDEFAULT
    mov DWORD PTR [rsp + 56], 080000000h  ; nHeight = CW_USEDEFAULT
    mov QWORD PTR [rsp + 64], 0           ; hWndParent = NULL
    mov QWORD PTR [rsp + 72], 0           ; hMenu = NULL
    mov rax, QWORD PTR [g_hInstance]
    mov QWORD PTR [rsp + 80], rax         ; hInstance
    mov QWORD PTR [rsp + 88], 0           ; lpParam = NULL

    call CreateWindowExW
    add rsp, 96

    ; Save hwnd
    mov QWORD PTR [r12], rax

    ; Show window
    mov rcx, rax
    mov edx, 5                  ; SW_SHOW
    call ShowWindow

    mov rcx, QWORD PTR [r12]
    call UpdateWindow

    jmp mwc_done

mwc_fail:
    mov QWORD PTR [r12], 0      ; return NULL on failure

mwc_done:
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