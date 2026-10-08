; Sticky note window class and lifecycle

PUBLIC RegisterStickyClass
RegisterStickyClass PROC
    ; Register the sticky note window class
    ret
RegisterStickyClass ENDP

PUBLIC StickyCreate
StickyCreate PROC
    ; rcx = text (UTF-16)
    ; rdx = text length
    ; r8 = theme
    ; Returns: rax = hwnd or NULL
    xor eax, eax
    ret
StickyCreate ENDP

PUBLIC StickyProc
StickyProc PROC
    ; Window procedure for sticky notes
    ; rcx = hwnd
    ; edx = message
    ; r8 = wParam
    ; r9 = lParam
    xor eax, eax
    ret
StickyProc ENDP
