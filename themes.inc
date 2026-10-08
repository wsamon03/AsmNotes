; Theme management

; Color table: RGB values
THEME_COLOR_0_BG = 0FFFBEDh      ; Paper (light cream)
THEME_COLOR_0_FG = 01C1C1Ch
THEME_COLOR_0_ACCENT = 0D32F2Fh

THEME_COLOR_1_BG = 02B2B2Bh      ; Slate (dark)
THEME_COLOR_1_FG = 0C8C8C8h
THEME_COLOR_1_ACCENT = 098D8D8h

; Default theme index
g_iTheme DWORD 0

PUBLIC ThemesInit
ThemesInit PROC
    mov DWORD PTR [g_iTheme], 0
    ret
ThemesInit ENDP

PUBLIC ThemeApply
ThemeApply PROC
    ; rcx = hwnd
    ; rdx = theme index
    ; Stub: just store the theme
    mov DWORD PTR [g_iTheme], edx
    ret
ThemeApply ENDP

PUBLIC ThemeGetBgColor
ThemeGetBgColor PROC
    ; rcx = theme index
    ; Returns: eax = RGB color
    mov eax, THEME_COLOR_0_BG
    test ecx, ecx
    jz tgbc_done
    mov eax, THEME_COLOR_1_BG
tgbc_done:
    ret
ThemeGetBgColor ENDP

PUBLIC ThemeGetFgColor
ThemeGetFgColor PROC
    ; rcx = theme index
    ; Returns: eax = RGB color
    mov eax, THEME_COLOR_0_FG
    test ecx, ecx
    jz tgfc_done
    mov eax, THEME_COLOR_1_FG
tgfc_done:
    ret
ThemeGetFgColor ENDP
