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
        self.win.set_focus()
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

        app.win.menu_select("View->Theme->Sepia")
        time.sleep(0.5)
        check("Sepia theme sets editor background", editor_bg_pixel(app.editor) == SEPIA_BG,
              hex(editor_bg_pixel(app.editor)))

        app.win.menu_select("View->Theme->Midnight")
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

    print()
    if failures:
        print("FAILED: %d check(s): %s" % (len(failures), ", ".join(failures)))
        return 1
    print("ALL CHECKS PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
