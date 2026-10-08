#!/usr/bin/env python
"""Simpler GUI test: launch exe and monitor for window"""

import sys
import subprocess
import time
import ctypes
from pathlib import Path

def find_window_by_class(class_name):
    """Find window by class name using Win32 API"""
    FindWindow = ctypes.windll.user32.FindWindowW
    hwnd = FindWindow(class_name, None)
    return hwnd if hwnd else None

def main():
    print("=" * 60)
    print("AsmNotes E2E Test v2 - Simple Launch")
    print("=" * 60)

    exe_path = Path(__file__).parent / "AsmNotes.exe"
    if not exe_path.exists():
        print("ERROR: {} not found".format(exe_path))
        return False

    print("\n[1/3] Launching exe: {}".format(exe_path))

    try:
        # Start process without waiting
        proc = subprocess.Popen(str(exe_path),
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
        pid = proc.pid
        print("  [OK] Process started (PID: {})".format(pid))
    except Exception as e:
        print("  [FAIL] Could not start: {}".format(e))
        return False

    print("\n[2/3] Waiting for window to appear (5 seconds)...")

    # Wait for window to appear
    for i in range(10):
        time.sleep(0.5)

        # Try to find our window class "AsmNotes"
        hwnd = find_window_by_class("AsmNotes")
        if hwnd:
            print("  [OK] Window found! HWND: {}".format(hwnd))

            # Try to get window title
            GetWindowText = ctypes.windll.user32.GetWindowTextW
            GetWindowTextLength = ctypes.windll.user32.GetWindowTextLengthW

            text_len = GetWindowTextLength(hwnd)
            if text_len > 0:
                text = ctypes.create_unicode_buffer(text_len + 1)
                GetWindowText(hwnd, text, text_len + 1)
                print("  [OK] Window title: {}".format(text.value))

            print("\n[3/3] Closing application...")
            PostMessage = ctypes.windll.user32.PostMessageW
            WM_CLOSE = 0x0010
            PostMessage(hwnd, WM_CLOSE, 0, 0)
            time.sleep(1)

            print("  [OK] Close signal sent")
            print("\n" + "=" * 60)
            print("[SUCCESS] Window appeared and closed successfully")
            print("=" * 60)
            return True

    print("  [FAIL] Window did not appear within 5 seconds")

    # Check if process is still running
    retcode = proc.poll()
    if retcode is None:
        print("  [INFO] Process still running, terminating...")
        proc.terminate()
        try:
            proc.wait(timeout=2)
        except:
            proc.kill()
    else:
        print("  [INFO] Process exited with code: {}".format(retcode))

    print("\n" + "=" * 60)
    print("[FAIL] No window created")
    print("=" * 60)
    return False

if __name__ == "__main__":
    try:
        success = main()
        sys.exit(0 if success else 1)
    except Exception as e:
        print("FATAL: {}".format(e))
        import traceback
        traceback.print_exc()
        sys.exit(1)
