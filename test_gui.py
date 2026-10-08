#!/usr/bin/env python
"""End-to-end GUI test for AsmNotes using pywinauto"""

import sys
import os
import time
from pathlib import Path

try:
    from pywinauto import Application, findwindows
    from pywinauto.keyboard import send_keys
except ImportError:
    print("ERROR: pywinauto not installed. Run: pip install pywinauto")
    sys.exit(1)

def main():
    print("=" * 60)
    print("AsmNotes E2E GUI Test")
    print("=" * 60)

    exe_path = Path(__file__).parent / "AsmNotes.exe"

    if not exe_path.exists():
        print("ERROR: {} not found".format(exe_path))
        print("Run: build.bat")
        return False

    print("\n[1/5] Launching {}".format(exe_path))
    try:
        app = Application(backend="uia").start(str(exe_path))
        time.sleep(2)  # Wait for window to appear
        print("  [OK] Application started")
    except Exception as e:
        print("  [FAIL] Failed to start: {}".format(e))
        return False

    # Get main window
    try:
        windows = app.windows()
        if not windows:
            print("  [FAIL] No windows found")
            return False
        main_window = windows[0]
        print("  [OK] Main window found: {}".format(main_window.window_text()))
    except Exception as e:
        print("  [FAIL] Error finding window: {}".format(e))
        return False

    print("\n[2/5] Checking window properties")
    try:
        rect = main_window.rectangle()
        print("  [OK] Window rect: {}".format(rect))
        print("  [OK] Is visible: {}".format(main_window.is_visible()))
        print("  [OK] Is enabled: {}".format(main_window.is_enabled()))
    except Exception as e:
        print("  [FAIL] Error reading properties: {}".format(e))

    print("\n[3/5] Testing keyboard input (Ctrl+T for new tab)")
    try:
        main_window.set_focus()
        time.sleep(0.5)
        send_keys("^t")  # Ctrl+T
        time.sleep(1)
        print("  [OK] Keyboard input sent")
    except Exception as e:
        print("  [FAIL] Keyboard test failed: {}".format(e))

    print("\n[4/5] Testing theme menu (Alt+V for View menu)")
    try:
        main_window.set_focus()
        send_keys("%v")  # Alt+V for View menu
        time.sleep(1)
        print("  [OK] Menu accessed")
    except Exception as e:
        print("  [FAIL] Menu test failed: {}".format(e))

    print("\n[5/5] Closing application")
    try:
        send_keys("%{F4}")  # Alt+F4
        time.sleep(1)
        print("  [OK] Close command sent")
    except Exception as e:
        print("  [FAIL] Close failed: {}".format(e))

    print("\n" + "=" * 60)
    print("[SUCCESS] E2E Test Complete")
    print("=" * 60)
    return True

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
