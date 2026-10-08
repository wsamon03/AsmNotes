; Sticky note window class and lifecycle

EXTERN CreateWindowExW : PROC
EXTERN RegisterClassExW : PROC
EXTERN DefWindowProcW : PROC
EXTERN PostQuitMessage : PROC
EXTERN ShowWindow : PROC
EXTERN DestroyWindow : PROC

; Sticky note window class
szStickyClassName:
    WORD 'A', 's', 'm', 'S', 't', 'i', 'c', 'k', 'y', 0

; Sticky instance data (stored per window)
g_cStickies DWORD 0

PUBLIC RegisterStickyClass
RegisterStickyClass PROC
    ; Register the sticky note window class
    ret
RegisterStickyClass ENDP

PUBLIC StickyCreate
StickyCreate PROC
    ; rcx = text (UTF-16)
    ; rdx = text length in wide chars
    ; r8 = theme index
    ; Returns: rax = hwnd or NULL
    xor eax, eax
    ret
StickyCreate ENDP

PUBLIC StickyProc
StickyProc PROC FRAME
    .endprolog
    ; Window procedure for sticky notes
    ; rcx = hwnd
    ; edx = message
    ; r8 = wParam
    ; r9 = lParam

    cmp edx, 2                  ; WM_DESTROY
    je sticky_destroy

    call DefWindowProcW
    ret

sticky_destroy:
    xor eax, eax
    ret

StickyProc ENDP
