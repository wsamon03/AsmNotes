.CODE

EXTERN GetModuleHandleW : PROC
EXTERN ExitProcess : PROC

g_hInstance QWORD 0

PUBLIC start
start PROC FRAME
    push rbp
    mov rbp, rsp
    .endprolog

    ; Get module handle
    xor ecx, ecx
    call GetModuleHandleW
    mov QWORD PTR [g_hInstance], rax

    ; For now, just exit (no window)
    mov ecx, 0
    call ExitProcess

    pop rbp
    ret
start ENDP

END
