# AsmNotes

A note-taking app for Windows 11, written in x64 assembly (MASM) against the Win32 API.

## Features

- Multiple tabs (Ctrl+T new, Ctrl+W close, Ctrl+Tab next), up to 32, drawn in the theme colours
- Markdown: Ctrl+M switches between raw source and a formatted, read-only view, rendered as RTF
  (headings, bold, italic, strikethrough, code, lists, quotes, rules, links, escapes)
- Six themes under View > Theme (Paper, Slate, Sepia, Solarized Dark, Forest, Midnight)
- Sticky notes: Ctrl+Shift+S opens the active tab as a snapshot window (up to 32).
  Each sticky has its own caption with a title, a Pin/Unpin button and a close button,
  which show hover and pressed colours.
- Autosave to `%APPDATA%\AsmNotes` (UTF-8 files and `settings.ini`), restored on launch
- DPI-aware tab strip and sticky caption

## Build

Requires the Visual Studio 2022 Build Tools (MSVC x64 and Windows SDK).

    build.bat

This produces `AsmNotes.exe`.

## Test

With the exe built, run:

    python tests\test_e2e.py

The suite has nine sessions. It starts the real exe and drives it with keyboard and
mouse input (pywinauto), reads state back through Win32 messages, checks the files
written to a temporary `%APPDATA%`, and compares the `/render` output with golden files.
It also samples pixels to check the editor and tab strip colours in all six themes,
and it checks the 32-tab and 32-sticky limits. Python 3 with `pywinauto` is required.

Headless renderer check:

    AsmNotes.exe /render input.md output.txt

This writes the formatted text followed by its formatting runs (see
`tests\fixtures\*.expected.txt`).

## Layout

- `main.asm` - entry point, command line handling, includes
- `consts.inc` - Win32 equates checked against the SDK headers
- `globals.inc` - data: state, palette, strings
- `util.inc` - heap, colours and drawing helpers
- `storage.inc` - file I/O, settings, tab and sticky persistence
- `markdown.inc` - Markdown parsing into text and formatting runs, and their application
- `tabs.inc` - tab strip, tab drawing, switching and closing
- `sticky.inc` - sticky windows and their custom caption
- `render.inc` - the `/render` headless renderer
- `mainwnd.inc` - themes and the main window procedure
- `notes.rc`, `winconst.h` - menu, accelerators and resource IDs
- `notes.manifest` - Common Controls v6 and per-monitor DPI awareness

## Known limitations

- The menu bar is the native Windows menu bar, so it keeps the system colours. This was a
  decision: it cannot be recoloured reliably without drawing our own menu bar.
- Markdown is a subset: no tables, images or nested lists.
- A note is limited to 65,535 characters.
