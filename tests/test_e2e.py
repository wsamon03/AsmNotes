"""End-to-end UI tests for AsmNotes.

Drives the real exe with keyboard and menu input (pywinauto), and reads state back
through Win32 messages. Run after build.bat:  python tests\\test_e2e.py
"""

import ctypes
import subprocess
import sys
import time
from ctypes import wintypes
from pathlib import Path

from pywinauto import Application
from pywinauto.keyboard import send_keys

user32 = ctypes.WinDLL("user32", use_last_error=True)
user32.FindWindowW.restype = wintypes.HWND
user32.GetClassNameW.argtypes = [wintypes.HWND, wintypes.LPWSTR, ctypes.c_int]
user32.SendMessageW.restype = ctypes.c_ssize_t
user32.SendMessageW.argtypes = [wintypes.HWND, ctypes.c_uint, ctypes.c_size_t, ctypes.c_void_p]
user32.GetDC.restype = wintypes.HDC
user32.GetDC.argtypes = [wintypes.HWND]
gdi32 = ctypes.WinDLL("gdi32")
gdi32.GetPixel.argtypes = [wintypes.HDC, ctypes.c_int, ctypes.c_int]
gdi32.GetPixel.restype = wintypes.DWORD
user32.ReleaseDC.argtypes = [wintypes.HWND, wintypes.HDC]
user32.GetClientRect.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.RECT)]

WM_GETTEXT = 0x000D
WM_GETTEXTLENGTH = 0x000E
TCM_GETITEMCOUNT = 0x1304
TCM_GETCURSEL = 0x130B
WM_CLOSE = 0x0010

EXE = Path(__file__).resolve().parent.parent / "AsmNotes.exe"

failures = []


def check(name, cond, detail=""):
    print(("PASS " if cond else "FAIL ") + name + (("  [" + str(detail) + "]") if detail != "" else ""))
    if not cond:
        failures.append(name)


def window_text(hwnd):
    n = user32.SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0)
    buf = ctypes.create_unicode_buffer(n + 1)
    user32.SendMessageW(hwnd, WM_GETTEXT, n + 1, ctypes.addressof(buf))
    return buf.value


def find_main():
    return user32.FindWindowW("AsmNotes", None) or 0


WNDENUMPROC = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)


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


def editor_bg_pixel(editor):
    rc = wintypes.RECT()
    user32.GetWindowRect(editor, ctypes.byref(rc))
    hdc = user32.GetDC(None)
    color = gdi32.GetPixel(hdc, rc.right - 12, rc.bottom - 12)
    user32.ReleaseDC(None, hdc)
    return color


def wait_for(pred, timeout=5.0):
    end = time.time() + timeout
    while time.time() < end:
        if pred():
            return True
        time.sleep(0.1)
    return False


def main():
    if not EXE.exists():
        print("FAIL exe not built: " + str(EXE))
        return 1

    proc = subprocess.Popen([str(EXE)])
    try:
        check("process starts and stays alive", True)
        ok = wait_for(lambda: find_main() != 0)
        check("main window (class AsmNotes) appears", ok)
        if not ok:
            return 1
        main = find_main()
        time.sleep(0.5)
        check("process still alive after startup", proc.poll() is None, proc.poll())

        app = Application(backend="win32").connect(process=proc.pid)
        win = app.window(handle=main)
        editor = child(main, "RICHEDIT50W")
        tabs = child(main, "SysTabControl32")
        check("editor control exists", bool(editor))
        check("tab control exists", bool(tabs))

        def tab_count():
            return user32.SendMessageW(tabs, TCM_GETITEMCOUNT, 0, 0)

        def tab_index():
            return user32.SendMessageW(tabs, TCM_GETCURSEL, 0, 0)

        check("starts with one tab", tab_count() == 1, tab_count())

        win.set_focus()
        time.sleep(0.3)
        send_keys("alpha", with_spaces=True, pause=0.05)
        time.sleep(0.3)
        check("typed text lands in editor", window_text(editor) == "alpha", repr(window_text(editor)))

        send_keys("^t")
        time.sleep(0.5)
        check("Ctrl+T adds a tab", tab_count() == 2, tab_count())
        check("new tab becomes active", tab_index() == 1, tab_index())
        check("new tab editor is empty", window_text(editor) == "", repr(window_text(editor)))

        send_keys("beta", pause=0.05)
        time.sleep(0.3)
        check("second tab holds its own text", window_text(editor) == "beta", repr(window_text(editor)))

        send_keys("^{TAB}")
        time.sleep(0.5)
        check("Ctrl+Tab returns to first tab", tab_index() == 0, tab_index())
        check("first tab text is restored", window_text(editor) == "alpha", repr(window_text(editor)))

        win.menu_select("View->Theme->Sepia")
        time.sleep(0.5)
        bg = editor_bg_pixel(editor)
        check("Sepia theme sets editor background (BGR 0x00D8ECF4)", (bg & 0xFFFFFF) == 0xD8ECF4, hex(bg))

        win.menu_select("View->Theme->Midnight")
        time.sleep(0.5)
        bg = editor_bg_pixel(editor)
        check("Midnight theme sets editor background (BGR 0x0020100B)", (bg & 0xFFFFFF) == 0x20100B, hex(bg))

        send_keys("^w")
        time.sleep(0.5)
        check("Ctrl+W closes the active tab", tab_count() == 1, tab_count())

        user32.SendMessageW(main, WM_CLOSE, 0, 0)
        exited = wait_for(lambda: proc.poll() is not None, timeout=5)
        check("WM_CLOSE exits the process cleanly", exited and proc.returncode == 0, proc.returncode)
    finally:
        if proc.poll() is None:
            proc.kill()

    print()
    if failures:
        print("FAILED: %d check(s): %s" % (len(failures), ", ".join(failures)))
        return 1
    print("ALL CHECKS PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
