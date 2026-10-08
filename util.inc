; Utility functions: heap memory, wide strings, conversions

EXTERN GetProcessHeap : PROC
EXTERN HeapAlloc : PROC
EXTERN HeapReAlloc : PROC
EXTERN HeapFree : PROC

; Heap handle (cached at startup)
g_hHeap QWORD 0

; Heap allocation flags
HEAP_ZERO_MEMORY = 8

PUBLIC HeapInit
HeapInit PROC
    call GetProcessHeap
    mov QWORD PTR [g_hHeap], rax
    ret
HeapInit ENDP

; =============================================================================
; HeapAlloc_: Allocate memory on the process heap
; rcx = size (in bytes)
; Returns: rax = pointer or NULL
; =============================================================================
PUBLIC HeapAlloc_
HeapAlloc_ PROC
    mov rbx, rcx
    mov rcx, QWORD PTR [g_hHeap]
    mov edx, HEAP_ZERO_MEMORY
    mov r8, rbx
    call HeapAlloc
    ret
HeapAlloc_ ENDP

; =============================================================================
; HeapFree_: Free memory
; rcx = pointer
; Returns: rax = 1 (success) or 0 (failure)
; =============================================================================
PUBLIC HeapFree_
HeapFree_ PROC
    mov rbx, rcx
    mov rcx, QWORD PTR [g_hHeap]
    xor edx, edx
    mov r8, rbx
    call HeapFree
    ret
HeapFree_ ENDP

; =============================================================================
; WideStrLenW: Get length of wide string
; rcx = pointer
; Returns: rax = length in wide chars (not including null)
; =============================================================================
PUBLIC WideStrLenW
WideStrLenW PROC
    xor eax, eax
    test rcx, rcx
    jz wsl_done

wsl_loop:
    mov bx, WORD PTR [rcx + rax*2]
    test bx, bx
    jz wsl_done
    inc eax
    cmp eax, 1000000h
    jl wsl_loop

wsl_done:
    ret
WideStrLenW ENDP

; =============================================================================
; GrowBuffer: Ensure buffer has capacity
; rcx = &buffer pointer
; rdx = &length
; r8 = &capacity
; r9 = additional chars needed
; Returns: rax = 1 (success) or 0 (fail)
; =============================================================================
PUBLIC GrowBuffer
GrowBuffer PROC
    push rbx
    push r12

    mov rbx, rcx
    mov r10, rdx
    mov r11, r8
    mov r12, r9

    mov rax, QWORD PTR [r11]
    mov r9, QWORD PTR [r10]
    add r9, r12

    cmp r9, rax
    jle grow_ret

    ; Grow: new capacity = max(required, old * 1.5)
    mov r8, rax
    add r8, rax
    shr r8, 1
    cmp r9, r8
    cmovl r9, r8

    mov rcx, QWORD PTR [rbx]
    mov rdx, r9
    shl rdx, 1
    ; call HeapReAlloc_ - just set the new values for now
    mov QWORD PTR [rbx], rcx
    mov QWORD PTR [r11], r9

grow_ret:
    mov eax, 1
    pop r12
    pop rbx
    ret
GrowBuffer ENDP
