.DATA
ALIGN 8
g_hInst     QWORD 0
g_hHeap     QWORD 0
g_hMain     QWORD 0
g_hEdit     QWORD 0
g_hTabs     QWORD 0
g_hAccel    QWORD 0
g_nTabs     DWORD 0
g_active    DWORD 0
g_theme     DWORD 0
g_view      DWORD 0
g_tabs      QWORD MAX_TABS DUP(0)
g_title     WORD 64 DUP(0)
g_cf        BYTE 116 DUP(0)
g_icc       DWORD 8, ICC_TAB_CLASSES

; Theme palette: background, text, accent, muted as COLORREF (0x00BBGGRR)
g_palette   DWORD 0F1F8FBh, 02B2B2Bh, 0AD5F1Fh, 07A7A7Ah   ; 0 Paper
            DWORD 0030251Eh, 0F0EAE6h, 0FFB77Fh, 0B1A59Ah   ; 1 Slate
            DWORD 0D8ECF4h, 0036465Bh, 01A4A9Ch, 060758Ah   ; 2 Sepia
            DWORD 0362B00h, 0A1A193h, 0D28B26h, 00837B65h   ; 3 Solarized Dark
            DWORD 0ECF5EEh, 0243A1Fh, 0327D2Eh, 0627D5Eh   ; 4 Forest
            DWORD 0020100Bh, 0EEDED7h, 06BB8FFh, 0A8887Ch   ; 5 Midnight

s_cls_main  WORD 'A','s','m','N','o','t','e','s',0
s_title     WORD 'A','s','m','N','o','t','e','s',0
s_tabclass  WORD 'S','y','s','T','a','b','C','o','n','t','r','o','l','3','2',0
s_richedit  WORD 'R','I','C','H','E','D','I','T','5','0','W',0
s_msftedit  WORD 'M','s','f','t','e','d','i','t','.','d','l','l',0
fmt_note    WORD 'N','o','t','e',' ','%','d',0
s_appdata   WORD 'A','P','P','D','A','T','A',0
fmt_dir     WORD '%','s',5Ch,'A','s','m','N','o','t','e','s',0
fmt_ini     WORD '%','s',5Ch,'s','e','t','t','i','n','g','s','.','i','n','i',0
fmt_tabpath WORD '%','s',5Ch,'t','a','b','%','d','.','m','d',0
fmt_tmp     WORD '%','s','.','t','m','p',0
fmt_int     WORD '%','d',0
s_sec_notes WORD 'N','o','t','e','s',0
k_tabs      WORD 'T','a','b','s',0
k_active    WORD 'A','c','t','i','v','e',0
k_theme     WORD 'T','h','e','m','e',0
k_view      WORD 'V','i','e','w',0
g_appdata   WORD 300 DUP(0)
g_dir       WORD 300 DUP(0)
g_path      WORD 320 DUP(0)
g_tmp       WORD 330 DUP(0)
g_ini       WORD 320 DUP(0)
g_num       WORD 32 DUP(0)

g_rsrc      QWORD 0
g_rcch      QWORD 0
g_plain     QWORD 0
g_plen      QWORD 0
g_plainCap  QWORD 0
g_spans     QWORD 0
g_nspans    QWORD 0
g_spancap   QWORD 0
g_pars      QWORD 0
g_npars     QWORD 0
g_parcap    QWORD 0
g_runstart  QWORD 0
g_ls        QWORD 0
g_le        QWORD 0
g_incode    QWORD 0
g_iflags    DWORD 0
g_bflags    DWORD 0
g_bheight   DWORD 0
g_accent    DWORD 0
g_muted     DWORD 0
g_pf        BYTE 188 DUP(0)
g_cf2       BYTE 116 DUP(0)
hsize       DWORD 0, 360, 300, 260, 240, 240, 240
s_consolas  WORD 'C','o','n','s','o','l','a','s',0
g_key       WORD 32 DUP(0)
g_stk       QWORD MAX_STK DUP(0)
g_nstk      DWORD 0
g_dpi       DWORD 96
g_tabh      DWORD 32
g_dtitle    WORD 64 DUP(0)
s_unpin_txt WORD 'U','n','p','i','n',0
s_empty    WORD 0
s_render_tok WORD '/','r','e','n','d','e','r',0
fmt_span   WORD '%','u',' ','%','u',' ','%','u',' ','%','u',' ','%','u',0Dh,0Ah,0
fmt_par    WORD '%','u',' ','%','d',' ','%','d',0Dh,0Ah,0
s_sep1     WORD 0Dh,0Ah,'=','=','s','p','a','n','s','=','=',0Dh,0Ah,0
s_sep2     WORD '=','=','p','a','r','s','=','=',0Dh,0Ah,0
g_rin      WORD 300 DUP(0)
g_rout     WORD 300 DUP(0)
s_pin_txt   WORD 'P','i','n',0
s_close_txt WORD 'X',0
g_rhEdit    QWORD 0
fmt_stpath  WORD '%','s',5Ch,'s','t','i','c','k','y','%','d','.','m','d',0
fmt_key     WORD '%','s','%','d',0
s_kx        WORD 'X',0
s_ky        WORD 'Y',0
s_kw        WORD 'W',0
s_kh        WORD 'H',0
s_kp        WORD 'P',0
k_stickies  WORD 'S','t','i','c','k','i','e','s',0
s_cls_sticky WORD 'A','s','m','S','t','i','c','k','y',0
s_sticky_title WORD 'S','t','i','c','k','y',' ','n','o','t','e',0
s_pin_text  WORD 'A','l','w','a','y','s',' ','o','n',' ','t','o','p',0

g_rtfbuf    QWORD 0
g_rtfpos    QWORD 0
r_head      DB "{\rtf1\ansi\deff0{\fonttbl{\f0\fswiss Segoe UI;}{\f1\fmodern Consolas;}}{\colortbl;", 0
r_mid       DB "}\f0\fs22\cf1 ", 0
r_close_doc DB "}", 0
r_red       DB "\red", 0
r_green     DB "\green", 0
r_blue      DB "\blue", 0
r_pard      DB "\pard", 0
r_li        DB "\li", 0
r_fi        DB "\fi", 0
r_par_crlf  DB "\par", 0Ah, 0
r_tab       DB "\tab ", 0
r_uni       DB "\u", 0
r_b         DB "\b", 0
r_i         DB "\i", 0
r_ul        DB "\ul", 0
r_strike    DB "\strike", 0
r_f1        DB "\f1", 0
r_cf2       DB "\cf2", 0
r_cf3       DB "\cf3", 0
r_fs        DB "\fs", 0
g_sptr      QWORD 0
g_srem      QWORD 0
g_menufont  QWORD 0
s_segoe     WORD 'S','e','g','o','e',' ','U','I',0
s_bullet    WORD 2022h,0
s_arrow     WORD 203Ah,0
