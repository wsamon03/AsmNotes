; Markdown to RTF renderer (stub for now)

PUBLIC RenderMarkdownToRtf
RenderMarkdownToRtf PROC
    ; rcx = source text (UTF-16)
    ; rdx = text length in wide chars
    ; r8 = theme index
    ; Returns: rax = RTF buffer (allocated on heap) or NULL
    ;
    ; Stub: just return NULL for now

    xor eax, eax
    ret
RenderMarkdownToRtf ENDP
