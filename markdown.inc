; Markdown to RTF converter (simplified subset)
; Handles: headings, bold, italic, lists, code blocks

EXTERN lstrcpyW : PROC
EXTERN lstrcatW : PROC
EXTERN lstrlenW : PROC

; Max output size for converted RTF
MAX_RTF_SIZE EQU 020000h

; RTF state
g_rtfBuf QWORD 0
g_rtfPos DWORD 0

PUBLIC RenderMarkdownToRtf
RenderMarkdownToRtf PROC FRAME
    push rbp
    mov rbp, rsp
    sub rsp, 32
    .endprolog

    ; rcx = source text (UTF-16), wide chars
    ; rdx = text length
    ; r8 = theme index
    ; Returns: rax = RTF buffer or NULL

    push rbx
    push r12
    push r13

    mov r12, rcx                ; source
    mov r13, rdx                ; length
    mov r10, r8                 ; theme

    ; Allocate RTF buffer
    mov rcx, MAX_RTF_SIZE
    call HeapAlloc_
    mov QWORD PTR [g_rtfBuf], rax
    test rax, rax
    jz render_fail

    mov rbx, rax
    mov DWORD PTR [g_rtfPos], 0

    ; Write RTF header
    lea rcx, [rtf_prolog]
    mov rdx, rbx
    call lstrcpyW

    ; Skip the position update - lstrcatW will handle it

    ; For now, copy source as plain text
    ; (Full Markdown parsing would go here)
    mov rcx, rbx
    mov rdx, r12                    ; source
    call lstrcatW

    ; Write RTF trailer
    lea rcx, [rtf_epilog]
    mov rdx, rbx
    call lstrcatW

    mov rax, rbx
    jmp render_done

render_fail:
    xor eax, eax

render_done:
    pop r13
    pop r12
    pop rbx
    mov rsp, rbp
    pop rbp
    ret

RenderMarkdownToRtf ENDP

; RTF header preamble (wide-char string literals)
; Using direct WORD arrays to avoid string parsing issues
rtf_prolog:
    WORD '{'
    WORD 0
    WORD '\'
    WORD 'r'
    WORD 't'
    WORD 'f'
    WORD '1'
    WORD 0Ah
    WORD '\'
    WORD 'a'
    WORD 'n'
    WORD 's'
    WORD 'i'
    WORD 0Ah
    WORD '{'
    WORD 0ah
    WORD '\'
    WORD 'f'
    WORD 'o'
    WORD 'n'
    WORD 't'
    WORD 't'
    WORD 'b'
    WORD 'l'
    WORD '{'
    WORD 0ah
    WORD '\'
    WORD 'f'
    WORD '0'
    WORD ' '
    WORD '\'
    WORD 'f'
    WORD 'n'
    WORD 'i'
    WORD 'l'
    WORD ' '
    WORD 'C'
    WORD 'a'
    WORD 'l'
    WORD 'i'
    WORD 'b'
    WORD 'r'
    WORD 'i'
    WORD ';'
    WORD '}'
    WORD '}'
    WORD 0ah
    WORD 0

; RTF footer
rtf_epilog:
    WORD 0ah
    WORD '}'
    WORD 0
