; Theme management: 6 color schemes

; Color palette (RGB)
THEME_PAPER_BG = 0FFFBEDH
THEME_PAPER_FG = 01C1C1CH
THEME_PAPER_ACCENT = 0D32F2FH

THEME_SLATE_BG = 02B2B2BH
THEME_SLATE_FG = 0C8C8C8H
THEME_SLATE_ACCENT = 098D8D8H

THEME_SEPIA_BG = 0F5DEB3H
THEME_SEPIA_FG = 03D2817H
THEME_SEPIA_ACCENT = 08B6F47H

THEME_SOLAR_BG = 0002B36H
THEME_SOLAR_FG = 0839496H
THEME_SOLAR_ACCENT = 0268BD2H

THEME_FOREST_BG = 01B5E20H
THEME_FOREST_FG = 0C8E6C1H
THEME_FOREST_ACCENT = 0558B2FH

THEME_MIDNIGHT_BG = 0001429H
THEME_MIDNIGHT_FG = 0E0E1E8H
THEME_MIDNIGHT_ACCENT = 0FF5E1FH

g_iTheme DWORD 0

PUBLIC ThemesInit
ThemesInit PROC
    mov DWORD PTR [g_iTheme], 0
    ret
ThemesInit ENDP

PUBLIC ThemeApply
ThemeApply PROC
    ; rcx = hwnd
    ; rdx = theme index (0-5)
    ; Stub: just store the index
    cmp edx, 6
    jge theme_apply_ret
    mov DWORD PTR [g_iTheme], edx
theme_apply_ret:
    ret
ThemeApply ENDP

PUBLIC ThemeGetBgColor
ThemeGetBgColor PROC
    ; rcx = theme index
    ; Returns: eax = RGB color
    mov eax, THEME_PAPER_BG
    cmp ecx, 1
    jne tgbc_2
    mov eax, THEME_SLATE_BG
    ret
tgbc_2:
    cmp ecx, 2
    jne tgbc_3
    mov eax, THEME_SEPIA_BG
    ret
tgbc_3:
    cmp ecx, 3
    jne tgbc_4
    mov eax, THEME_SOLAR_BG
    ret
tgbc_4:
    cmp ecx, 4
    jne tgbc_5
    mov eax, THEME_FOREST_BG
    ret
tgbc_5:
    cmp ecx, 5
    jne tgbc_done
    mov eax, THEME_MIDNIGHT_BG
tgbc_done:
    ret
ThemeGetBgColor ENDP

PUBLIC ThemeGetFgColor
ThemeGetFgColor PROC
    ; rcx = theme index
    ; Returns: eax = RGB color
    mov eax, THEME_PAPER_FG
    cmp ecx, 1
    jne tgfc_2
    mov eax, THEME_SLATE_FG
    ret
tgfc_2:
    cmp ecx, 2
    jne tgfc_3
    mov eax, THEME_SEPIA_FG
    ret
tgfc_3:
    cmp ecx, 3
    jne tgfc_4
    mov eax, THEME_SOLAR_FG
    ret
tgfc_4:
    cmp ecx, 4
    jne tgfc_5
    mov eax, THEME_FOREST_FG
    ret
tgfc_5:
    cmp ecx, 5
    jne tgfc_done
    mov eax, THEME_MIDNIGHT_FG
tgfc_done:
    ret
ThemeGetFgColor ENDP
