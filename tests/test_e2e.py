"""End-to-end UI tests for AsmNotes.

Drives the real exe with keyboard and menu input (pywinauto) and reads state back
through Win32 messages. Each run uses a throwaway %APPDATA% so the user's notes are
never touched. Run after build.bat:  python tests\\test_e2e.py
"""

import ctypes
import os
import subprocess
import sys
import tempfile
import time
from ctypes import wintypes
from pathlib import Path

from pywinauto import Application
from pywinauto import mouse
from pywinauto.keyboard import send_keys

user32 = ctypes.WinDLL("user32", use_last_error=True)
gdi32 = ctypes.WinDLL("gdi32")

user32.FindWindowW.restype = wintypes.HWND
user32.GetClassNameW.argtypes = [wintypes.HWND, wintypes.LPWSTR, ctypes.c_int]
user32.SendMessageW.restype = ctypes.c_ssize_t
user32.SendMessageW.argtypes = [wintypes.HWND, ctypes.c_uint, ctypes.c_size_t, ctypes.c_void_p]
user32.PostMessageW.argtypes = [wintypes.HWND, ctypes.c_uint, ctypes.c_size_t, ctypes.c_ssize_t]
user32.GetDC.restype = wintypes.HDC
user32.GetDC.argtypes = [wintypes.HWND]
user32.ReleaseDC.argtypes = [wintypes.HWND, wintypes.HDC]
user32.GetWindowRect.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.RECT)]
gdi32.GetPixel.argtypes = [wintypes.HDC, ctypes.c_int, ctypes.c_int]
gdi32.GetPixel.restype = wintypes.DWORD

WM_GETTEXT = 0x000D
WM_GETTEXTLENGTH = 0x000E
WM_CHAR = 0x0102
WM_CLOSE = 0x0010
TCM_GETITEMCOUNT = 0x1304
TCM_GETCURSEL = 0x130B
MIDNIGHT_BG = 0x20100B
SEPIA_BG = 0xD8ECF4

EXE = Path(__file__).resolve().parent.parent / "AsmNotes.exe"
WNDENUMPROC = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)

failures = []


def check(name, cond, detail=""):
    print(("PASS " if cond else "FAIL ") + name + (("  [" + str(detail) + "]") if detail != "" else ""))
    if not cond:
        failures.append(name)


def wait_for(pred, timeout=5.0):
    end = time.time() + timeout
    while time.time() < end:
        if pred():
            return True
        time.sleep(0.1)
    return False


def find_main():
    return user32.FindWindowW("AsmNotes", None) or 0


def child(parent, cls):
    found = []

    def cb(hwnd, _lp):
        buf = ctypes.create_unicode_buffer(256)
        user32.GetClassNameW(hwnd, buf, 256)
        if buf.value == cls:
            found.append(hwnd)
        return True

    user32.EnumChildWindows(parent, WNDENUMPROC(cb), 0)
    return found[0] if found else 0


def window_text(hwnd):
    n = user32.SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0)
    buf = ctypes.create_unicode_buffer(n + 1)
    user32.SendMessageW(hwnd, WM_GETTEXT, n + 1, ctypes.addressof(buf))
    return buf.value


def editor_bg_pixel(editor):
    rc = wintypes.RECT()
    user32.GetWindowRect(editor, ctypes.byref(rc))
    hdc = user32.GetDC(None)
    color = gdi32.GetPixel(hdc, rc.right - 12, rc.bottom - 12)
    user32.ReleaseDC(None, hdc)
    return color & 0xFFFFFF


class App:
    """A running AsmNotes process with its window and child controls."""

    def __init__(self, appdata):
        env = dict(os.environ)
        env["APPDATA"] = str(appdata)
        self.proc = subprocess.Popen([str(EXE)], env=env)
        if not wait_for(lambda: find_main() != 0):
            raise RuntimeError("main window did not appear")
        time.sleep(0.6)
        self.main = find_main()
        self.editor = child(self.main, "RICHEDIT50W")
        self.tabs = child(self.main, "SysTabControl32")
        self.win = Application(backend="win32").connect(process=self.proc.pid).window(handle=self.main)

    def alive(self):
        return self.proc.poll() is None

    def tab_count(self):
        return user32.SendMessageW(self.tabs, TCM_GETITEMCOUNT, 0, 0)

    def tab_index(self):
        return user32.SendMessageW(self.tabs, TCM_GETCURSEL, 0, 0)

    def text(self):
        return window_text(self.editor)

    def focus(self):
        # Windows may refuse SetForegroundWindow from a background process; a click on
        # the editor activates the window just the same.
        try:
            self.win.set_focus()
        except Exception:
            rect = rect_of(self.editor)
            mouse.click(button="left", coords=((rect[0] + rect[2]) // 2, (rect[1] + rect[3]) // 2))
        time.sleep(0.3)

    def close(self):
        user32.SendMessageW(self.main, WM_CLOSE, 0, 0)
        exited = wait_for(lambda: self.proc.poll() is not None, timeout=5)
        return exited, self.proc.returncode

    def kill(self):
        if self.proc.poll() is None:
            self.proc.kill()


def session_one(data):
    print("\n== Session 1: fresh profile ==")
    app = App(data)
    try:
        check("process starts and stays alive", app.alive())
        check("editor control exists", bool(app.editor))
        check("tab control exists", bool(app.tabs))
        check("starts with one tab", app.tab_count() == 1, app.tab_count())
        check("first tab starts empty", app.text() == "", repr(app.text()))

        app.focus()
        send_keys("alpha", pause=0.05)
        time.sleep(0.3)
        check("typed text lands in editor", app.text() == "alpha", repr(app.text()))

        send_keys("^t")
        time.sleep(0.5)
        check("Ctrl+T adds a tab", app.tab_count() == 2, app.tab_count())
        check("new tab becomes active", app.tab_index() == 1, app.tab_index())
        check("new tab editor is empty", app.text() == "", repr(app.text()))

        send_keys("beta", pause=0.05)
        user32.PostMessageW(app.editor, WM_CHAR, 0xE9, 0)
        time.sleep(0.3)
        check("non-ASCII input lands in editor", app.text() == "betaé", repr(app.text()))

        send_keys("^{TAB}")
        time.sleep(0.5)
        check("Ctrl+Tab returns to first tab", app.tab_index() == 0, app.tab_index())
        check("first tab text is restored", app.text() == "alpha", repr(app.text()))

        choose_theme(app, "Sepia")
        time.sleep(0.5)
        check("Sepia theme sets editor background", editor_bg_pixel(app.editor) == SEPIA_BG,
              hex(editor_bg_pixel(app.editor)))

        choose_theme(app, "Midnight")
        time.sleep(0.5)
        check("Midnight theme sets editor background", editor_bg_pixel(app.editor) == MIDNIGHT_BG,
              hex(editor_bg_pixel(app.editor)))

        time.sleep(2.5)
        exited, code = app.close()
        check("WM_CLOSE exits the process cleanly", exited and code == 0, code)
    finally:
        app.kill()

    notes = data / "AsmNotes"
    tab0 = (notes / "tab0.md").read_bytes()
    tab1 = (notes / "tab1.md").read_bytes()
    check("tab0.md holds the first tab as UTF-8", tab0 == b"alpha", tab0)
    check("tab1.md holds non-ASCII as UTF-8", tab1 == "betaé".encode("utf-8"), tab1)
    ini = (notes / "settings.ini").read_bytes().decode("latin-1")
    check("settings.ini records 2 tabs", "Tabs=2" in ini, ini.strip())
    check("settings.ini records the Midnight theme", "Theme=5" in ini, ini.strip())


def session_two(data):
    print("\n== Session 2: relaunch restores the previous session ==")
    app = App(data)
    try:
        check("restored tab count is 2", app.tab_count() == 2, app.tab_count())
        check("restored active tab is the first", app.tab_index() == 0, app.tab_index())
        check("restored first tab text", app.text() == "alpha", repr(app.text()))
        check("restored theme is Midnight", editor_bg_pixel(app.editor) == MIDNIGHT_BG,
              hex(editor_bg_pixel(app.editor)))

        app.focus()
        send_keys("^{TAB}")
        time.sleep(0.5)
        check("restored second tab text", app.text() == "betaé", repr(app.text()))

        send_keys("^w")
        time.sleep(0.5)
        check("Ctrl+W closes the active tab", app.tab_count() == 1, app.tab_count())
        time.sleep(1.5)
        exited, code = app.close()
        check("second session exits cleanly", exited and code == 0, code)
    finally:
        app.kill()

    check("closed tab's file is removed", not (data / "AsmNotes" / "tab1.md").exists())


def session_three(data):
    print("\n== Session 3: the closed tab stays closed ==")
    app = App(data)
    try:
        check("tab count is 1 after relaunch", app.tab_count() == 1, app.tab_count())
        check("remaining tab text kept", app.text() == "alpha", repr(app.text()))

        app.focus()
        send_keys("^w")
        time.sleep(0.5)
        check("closing the last tab leaves one blank tab", app.tab_count() == 1 and app.text() == "",
              (app.tab_count(), repr(app.text())))
        time.sleep(1.5)
        app.close()
    finally:
        app.kill()


EM_GETCHARFORMAT = 0x043A
EM_SETSEL = 0x00B1
SCF_SELECTION = 1
CFE_BOLD = 0x0001
CFE_ITALIC = 0x0002


class CHARFORMAT2W(ctypes.Structure):
    _fields_ = [
        ("cbSize", wintypes.UINT),
        ("dwMask", wintypes.DWORD),
        ("dwEffects", wintypes.DWORD),
        ("yHeight", wintypes.LONG),
        ("yOffset", wintypes.LONG),
        ("crTextColor", wintypes.DWORD),
        ("bCharSet", ctypes.c_ubyte),
        ("bPitchAndFamily", ctypes.c_ubyte),
        ("szFaceName", wintypes.WCHAR * 32),
        ("wWeight", wintypes.WORD),
        ("sSpacing", ctypes.c_short),
        ("crBackColor", wintypes.DWORD),
        ("lcid", wintypes.DWORD),
        ("dwReserved", wintypes.DWORD),
        ("sStyle", ctypes.c_short),
        ("wKerning", wintypes.WORD),
        ("bUnderlineType", ctypes.c_ubyte),
        ("bAnimation", ctypes.c_ubyte),
        ("bRevAuthor", ctypes.c_ubyte),
        ("bReserved1", ctypes.c_ubyte),
    ]


kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
kernel32.OpenProcess.restype = wintypes.HANDLE
kernel32.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
kernel32.VirtualAllocEx.restype = ctypes.c_void_p
kernel32.VirtualAllocEx.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_size_t, wintypes.DWORD, wintypes.DWORD]
kernel32.WriteProcessMemory.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p]
kernel32.ReadProcessMemory.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p]

PROCESS_VM_ALL = 0x0008 | 0x0010 | 0x0020 | 0x0400
MEM_COMMIT_RESERVE = 0x3000
PAGE_READWRITE = 0x04

_remote = {}


def remote_buffer(pid, size=256):
    if pid not in _remote:
        h = kernel32.OpenProcess(PROCESS_VM_ALL, False, pid)
        addr = kernel32.VirtualAllocEx(h, None, size, MEM_COMMIT_RESERVE, PAGE_READWRITE)
        _remote[pid] = (h, addr)
    return _remote[pid]


def char_format_at(editor, index, pid):
    """Reads the character format of one character.

    EM_GETCHARFORMAT writes through a pointer, so the buffer must live in the app's
    address space, which is why it is allocated remotely and copied back.
    """
    user32.SendMessageW(editor, EM_SETSEL, index, index + 1)
    h, addr = remote_buffer(pid)
    cf = CHARFORMAT2W()
    cf.cbSize = ctypes.sizeof(CHARFORMAT2W)
    kernel32.WriteProcessMemory(h, addr, ctypes.addressof(cf), ctypes.sizeof(CHARFORMAT2W), None)
    user32.SendMessageW(editor, EM_GETCHARFORMAT, SCF_SELECTION, addr)
    out = CHARFORMAT2W()
    kernel32.ReadProcessMemory(h, addr, ctypes.addressof(out), ctypes.sizeof(CHARFORMAT2W), None)
    return out


def type_text(editor, text):
    for ch in text:
        if ch == "\n":
            continue
        if ch == "\r":
            send_keys("{ENTER}")
        else:
            user32.PostMessageW(editor, WM_CHAR, ord(ch), 0)
        time.sleep(0.02)
    time.sleep(0.3)


MD_SOURCE = (
    "# Title\r\n"
    "*it* and **bold** and `code`\r\n"
    "- one\r\n"
    "- two\r\n"
    "\\*not italic\\* [site](http://x)"
)


def session_markdown(data):
    print("\n== Session 4: Markdown raw and formatted views ==")
    app = App(data)
    try:
        app.focus()
        type_text(app.editor, MD_SOURCE)
        raw = app.text()
        check("raw view keeps the Markdown source", raw == MD_SOURCE, repr(raw))

        send_keys("^m")
        time.sleep(0.8)
        fmt = app.text()
        check("formatted view strips heading markers", "#" not in fmt, repr(fmt))
        check("formatted view strips bold markers", "**" not in fmt)
        check("formatted view strips code markers", "`" not in fmt)
        check("formatted view drops link URLs", "http" not in fmt and "(" not in fmt)
        check("formatted view keeps link text", "site" in fmt)
        check("formatted view shows bullets", "•" in fmt)
        check("escaped asterisks stay literal", "*not italic*" in fmt)

        fmt_i = fmt.replace("\r\n", "\r")
        title = char_format_at(app.editor, 0, app.proc.pid)
        check("heading is larger than body text", title.yHeight > 300 and (title.dwEffects & CFE_BOLD),
              (title.yHeight, hex(title.dwEffects)))
        bold_at = fmt_i.index("bold")
        check("**bold** renders bold", bool(char_format_at(app.editor, bold_at, app.proc.pid).dwEffects & CFE_BOLD))
        it_at = fmt_i.index("it and")
        check("*it* renders italic", bool(char_format_at(app.editor, it_at, app.proc.pid).dwEffects & CFE_ITALIC))
        code_at = fmt_i.index("code")
        check("`code` uses the monospace font", char_format_at(app.editor, code_at, app.proc.pid).szFaceName == "Consolas",
              char_format_at(app.editor, code_at, app.proc.pid).szFaceName)

        before = app.text()
        app.focus()
        send_keys("zz", pause=0.05)
        time.sleep(0.3)
        check("formatted view is read-only", app.text() == before, repr(app.text()))

        send_keys("^m")
        time.sleep(0.6)
        check("toggling back restores the exact source", app.text() == MD_SOURCE, repr(app.text()))

        send_keys("^m")
        time.sleep(1.5)
        exited, code = app.close()
        check("Markdown session exits cleanly", exited and code == 0, code)
    finally:
        app.kill()

    saved = (data / "AsmNotes" / "tab0.md").read_bytes().decode("utf-8")
    check("the file keeps Markdown source, not rendered text", saved == MD_SOURCE, repr(saved[:60]))


def session_formatted_restore(data):
    print("\n== Session 5: formatted view is restored on relaunch ==")
    app = App(data)
    try:
        check("formatted view survives a restart", "•" in app.text() and "**" not in app.text(),
              repr(app.text()))
        app.focus()
        send_keys("^m")
        time.sleep(0.6)
        check("toggling from the restored view shows the raw source", app.text().startswith("# Title"),
              repr(app.text()[:20]))
        send_keys("^m")
        time.sleep(1.5)
        app.close()
    finally:
        app.kill()


WS_EX_TOPMOST = 0x00000008
GWL_EXSTYLE = -20
WM_SYSCOMMAND = 0x0112
IDM_PIN = 6000
user32.GetWindowLongW.argtypes = [wintypes.HWND, ctypes.c_int]
user32.GetWindowLongW.restype = wintypes.LONG
user32.GetWindowRect.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.RECT)]
user32.IsWindow.argtypes = [wintypes.HWND]
user32.GetWindowThreadProcessId.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.DWORD)]


def stickies_of(pid):
    found = []

    def cb(hwnd, _lp):
        buf = ctypes.create_unicode_buffer(64)
        user32.GetClassNameW(hwnd, buf, 64)
        owner = wintypes.DWORD(0)
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(owner))
        if buf.value == "AsmSticky" and owner.value == pid:
            found.append(hwnd)
        return True

    user32.EnumWindows(WNDENUMPROC(cb), 0)
    return found


user32.GetDpiForSystem.restype = wintypes.UINT


def caption_button(hwnd, which):
    """Screen coordinates of the pin ('pin') or close ('close') button of a sticky."""
    user32.SetWindowPos(hwnd, 0, 0, 0, 0, 0, 3)
    time.sleep(0.2)
    left, top, right, bottom = rect_of(hwnd)
    dpi = user32.GetDpiForSystem()
    bord = 6 * dpi // 96
    cap = 26 * dpi // 96
    w = right - left
    if which == "close":
        x = w - bord - cap // 2
    else:
        x = w - bord - cap - cap // 2
    return left + x, top + bord + cap // 2


def caption_button_inside(hwnd, which):
    """A point inside a caption button, away from its label."""
    user32.SetWindowPos(hwnd, 0, 0, 0, 0, 0, 3)
    time.sleep(0.2)
    left, top, right, bottom = rect_of(hwnd)
    dpi = user32.GetDpiForSystem()
    bord = 6 * dpi // 96
    cap = 26 * dpi // 96
    w = right - left
    buttons = 2 if which == "pin" else 1
    x0 = left + w - bord - buttons * cap
    return x0 + 3, top + bord + 3


def rect_of(hwnd):
    rc = wintypes.RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rc))
    return (rc.left, rc.top, rc.right, rc.bottom)


def session_stickies(_unused):
    print("\n== Session 6: sticky notes ==")
    data = Path(tempfile.mkdtemp())
    app = App(data)
    try:
        app.focus()
        type_text(app.editor, "# Sticky\r\n**hello** world")
        send_keys("^+s")
        wait_for(lambda: len(stickies_of(app.proc.pid)) == 1, timeout=3)
        found = stickies_of(app.proc.pid)
        check("Ctrl+Shift+S opens one sticky window", len(found) == 1, len(found))
        if not found:
            return
        st = found[0]
        se = child(st, "RICHEDIT50W")
        check("sticky has a formatted editor", bool(se))
        text = window_text(se)
        check("sticky shows formatted text", "Sticky" in text and "#" not in text and "**" not in text, repr(text))
        fmt_i = text.replace("\r\n", "\r")
        bold_at = fmt_i.index("hello")
        check("sticky renders bold text", bool(char_format_at(se, bold_at, app.proc.pid).dwEffects & CFE_BOLD))

        app.focus()
        type_text(app.editor, "XYZ")
        time.sleep(0.3)
        check("sticky is a snapshot of the tab", "XYZ" not in window_text(se), repr(window_text(se)))

        px, py = caption_button_inside(st, "pin")
        mouse.move(coords=(px, py))
        time.sleep(0.4)
        check("pin button shows hover colour", gdi_pixel(px, py) == 0x7A7A7A, hex(gdi_pixel(px, py)))
        mouse.press(button="left", coords=(px, py))
        time.sleep(0.4)
        check("pin button shows pressed colour", gdi_pixel(px, py) == 0xAD5F1F, hex(gdi_pixel(px, py)))
        mouse.release(button="left", coords=(px, py))
        time.sleep(0.4)
        pinned = bool(user32.GetWindowLongW(st, GWL_EXSTYLE) & WS_EX_TOPMOST)
        check("system menu pins the sticky on top", pinned)

        choose_theme(app, "Midnight")
        time.sleep(0.6)
        check("sticky follows the theme", editor_bg_pixel(se) == MIDNIGHT_BG, hex(editor_bg_pixel(se)))

        before = rect_of(st)
        time.sleep(1.0)
        exited, code = app.close()
        check("main window closes with stickies open", exited and code == 0, code)
    finally:
        app.kill()

    app = App(data)
    try:
        wait_for(lambda: len(stickies_of(app.proc.pid)) == 1, timeout=3)
        found = stickies_of(app.proc.pid)
        check("sticky is restored on relaunch", len(found) == 1, len(found))
        if found:
            st = found[0]
            se = child(st, "RICHEDIT50W")
            check("restored sticky keeps its text", "Sticky" in window_text(se) and "XYZ" not in window_text(se),
                  repr(window_text(se)))
            check("restored sticky keeps its pin", bool(user32.GetWindowLongW(st, GWL_EXSTYLE) & WS_EX_TOPMOST))
            check("restored sticky keeps its position", rect_of(st) == before, (rect_of(st), before))
            check("restored sticky follows the restored theme", editor_bg_pixel(se) == MIDNIGHT_BG,
                  hex(editor_bg_pixel(se)))
            mouse.click(button="left", coords=caption_button(st, "close"))
            time.sleep(0.8)
            check("the caption close button removes the sticky", len(stickies_of(app.proc.pid)) == 0)
        exited, code = app.close()
        check("relaunched app exits cleanly", exited and code == 0, code)
    finally:
        app.kill()

    check("closed sticky's file is removed", not (data / "AsmNotes" / "sticky0.md").exists())


def session_limits(_unused):
    print("\n== Session 7: 32-tab and 32-sticky limits ==")
    data = Path(tempfile.mkdtemp())
    app = App(data)
    try:
        app.focus()
        for _ in range(31):
            send_keys("^t")
            time.sleep(0.12)
        time.sleep(0.5)
        check("32 tabs can be opened", app.tab_count() == 32, app.tab_count())
        send_keys("^t")
        time.sleep(0.5)
        check("a 33rd tab is refused", app.tab_count() == 32, app.tab_count())

        for _ in range(32):
            send_keys("^+s")
            time.sleep(0.25)
        wait_for(lambda: len(stickies_of(app.proc.pid)) == 32, timeout=5)
        check("32 stickies can be opened", len(stickies_of(app.proc.pid)) == 32,
              len(stickies_of(app.proc.pid)))
        send_keys("^+s")
        time.sleep(0.8)
        check("a 33rd sticky is refused", len(stickies_of(app.proc.pid)) == 32,
              len(stickies_of(app.proc.pid)))
        exited, code = app.close()
        check("app exits cleanly with 32 tabs and stickies", exited and code == 0, code)
    finally:
        app.kill()


THEME_MENU = ["Paper", "Slate", "Sepia", "Solarized Dark", "Forest", "Midnight"]
THEME_BG = [0xF1F8FB, 0x30251E, 0xD8ECF4, 0x362B00, 0xECF5EE, 0x20100B]


def session_themes(_unused):
    print("\n== Session 8: every theme ==")
    data = Path(tempfile.mkdtemp())
    shots = Path(tempfile.mkdtemp())
    app = App(data)
    try:
        app.focus()
        type_text(app.editor, "Theme check")
        send_keys("^t")
        time.sleep(0.4)
        for idx, name in enumerate(THEME_MENU):
            choose_theme(app, name)
            time.sleep(0.6)
            bg = editor_bg_pixel(app.editor)
            check("editor follows theme %s" % name, bg == THEME_BG[idx], hex(bg))
            bar = menu_bar_rect(app.main)
            bar_mid = (bar.top + bar.bottom) // 2
            item = gdi_pixel(bar.left + 3, bar.top + 2)
            gap = gdi_pixel(bar.right - 6, bar_mid)
            check("menu bar item follows theme %s" % name, item == THEME_BG[idx], hex(item))
            check("menu bar gap follows theme %s" % name, gap == THEME_BG[idx], hex(gap))
            left, top, right, bottom = rect_of(app.tabs)
            strip = gdi_pixel(right - 20, top + (bottom - top) // 2)
            check("tab strip follows theme %s" % name, strip == THEME_BG[idx], hex(strip))
            crop_screen(left, top, min(right, left + 700), top + 180, shots / ("theme%d.png" % idx))
        print("screenshots in", shots)
        exited, code = app.close()
        check("theme session exits cleanly", exited and code == 0, code)
    finally:
        app.kill()


class MENUBARINFO(ctypes.Structure):
    _fields_ = [("cbSize", wintypes.DWORD), ("rcBar", wintypes.RECT), ("hMenu", wintypes.HANDLE),
                ("hwndMenu", wintypes.HWND), ("fBarFocused", wintypes.DWORD), ("fFocused", wintypes.DWORD)]


user32.GetMenuBarInfo.argtypes = [wintypes.HWND, wintypes.LONG, wintypes.LONG, ctypes.POINTER(MENUBARINFO)]
OBJID_MENU = -3
THEME_KEYS = {"Paper": "p", "Slate": "s", "Sepia": "i", "Solarized Dark": "o", "Forest": "f", "Midnight": "m"}


def menu_bar_rect(hwnd):
    """Screen rectangle of the menu bar (owner-drawn items fill it)."""
    info = MENUBARINFO()
    info.cbSize = ctypes.sizeof(info)
    user32.GetMenuBarInfo(hwnd, OBJID_MENU, 0, ctypes.byref(info))
    return info.rcBar


def choose_theme(app, name):
    """View > Theme > name, driven by mnemonics because the menu is owner-drawn."""
    app.focus()
    send_keys("%v")
    time.sleep(0.3)
    send_keys("t")
    time.sleep(0.3)
    send_keys(THEME_KEYS[name])


def gdi_pixel(x, y):
    hdc = user32.GetDC(None)
    color = gdi32.GetPixel(hdc, x, y)
    user32.ReleaseDC(None, hdc)
    return color & 0xFFFFFF


def crop_screen(left, top, right, bottom, path):
    import subprocess
    ps = ("Add-Type -AssemblyName System.Drawing; $b = New-Object System.Drawing.Bitmap %d, %d; "
          "$g = [System.Drawing.Graphics]::FromImage($b); $g.CopyFromScreen(%d, %d, 0, 0, $b.Size); "
          "$b.Save('%s')") % (right - left, bottom - top, left, top, str(path))
    subprocess.run(["powershell", "-NoProfile", "-Command", ps], check=False)


def session_render(_unused):
    print("\n== Session 9: /render seam against golden files ==")
    fixtures = Path(__file__).resolve().parent / "fixtures"
    out_dir = Path(tempfile.mkdtemp())
    for src in sorted(fixtures.glob("*.md")):
        expected = fixtures / (src.stem + ".expected.txt")
        out = out_dir / (src.stem + ".out.txt")
        before = src.read_bytes()
        result = subprocess.run([str(EXE), "/render", str(src), str(out)], timeout=20)
        check("%s: render exits cleanly" % src.name, result.returncode == 0, result.returncode)
        check("%s: input file is left untouched" % src.name, src.read_bytes() == before)
        got = out.read_bytes() if out.exists() else b""
        check("%s: output matches the golden file" % src.name, got == expected.read_bytes(),
              len(got))
        rtf_out = out_dir / (src.stem + ".out.rtf")
        rtf_expected = fixtures / (src.stem + ".expected.rtf")
        subprocess.run([str(EXE), "/render", str(src), str(rtf_out)], timeout=20)
        got_rtf = rtf_out.read_bytes() if rtf_out.exists() else b""
        check("%s: RTF output matches the golden file" % src.name, got_rtf == rtf_expected.read_bytes(),
              len(got_rtf))
        check("%s: RTF output is a document" % src.name, got_rtf.startswith(b"{\\rtf1"))


def main():
    if not EXE.exists():
        print("FAIL exe not built: " + str(EXE))
        return 1
    with tempfile.TemporaryDirectory() as tmp:
        data = Path(tmp)
        session_one(data)
        session_two(data)
        session_three(data)
        session_markdown(data)
        session_formatted_restore(data)
        session_stickies(data)
        session_limits(data)
        session_themes(data)
        session_render(data)

    print()
    if failures:
        print("FAILED: %d check(s): %s" % (len(failures), ", ".join(failures)))
        return 1
    print("ALL CHECKS PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
