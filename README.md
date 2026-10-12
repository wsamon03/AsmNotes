# AsmNotes

A note-taking app for Windows 11, written in x64 assembly (MASM) against the Win32 API.

## Features

- Multiple tabs (Ctrl+T new, Ctrl+W close, Ctrl+Tab next), up to 32
- Markdown: Ctrl+M switches between raw source and a formatted, read-only view
  (headings, bold, italic, strikethrough, code, lists, quotes, rules, links, escapes)
- Six themes under View > Theme (Paper, Slate, Sepia, Solarized Dark, Forest, Midnight)
- Sticky notes: Ctrl+Shift+S opens the active tab as a snapshot window (up to 32).
  Stickies can be pinned on top from their system menu.
- Autosave to `%APPDATA%\AsmNotes` (UTF-8 files and `settings.ini`), restored on launch

## Build

Requires the Visual Studio 2022 Build Tools (MSVC x64 and Windows SDK).

    build.bat

This produces `AsmNotes.exe`.

## Test

With the exe built, run:

    python tests\test_e2e.py

The tests start the real exe, drive it with keyboard and menu input (pywinauto),
read state back through Win32 messages, and check the files written to a
temporary `%APPDATA%`. They need Python 3 with `pywinauto` installed.

## Layout

- `main.asm` - all code: window procedures, tabs, persistence, Markdown renderer, stickies
- `consts.inc` - Win32 equates checked against the SDK headers
- `notes.rc`, `winconst.h` - menu, accelerators and the application manifest reference
- `notes.manifest` - Common Controls v6 and per-monitor DPI awareness

## Known limitations

- Tab strip and menu bar use the system colours; only the editor area follows the theme.
- The tab strip height is fixed in pixels and does not scale with DPI.
- Sticky windows use the standard Windows caption rather than a custom one.
- Markdown is a subset: no tables, images or nested lists.
- A note is limited to 65,535 characters.
