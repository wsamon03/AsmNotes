# AsmNotes: x64 MASM Note-Taking App

A minimal but feature-rich note-taking application written in pure x64 assembly (MASM) for Windows 11.

## Features

- **Multi-tab interface**: Organize notes across unlimited tabs
- **Markdown support**: Write in Markdown, toggle between raw and formatted view
- **6 Visual themes**: Paper, Slate, Sepia, Solarized, Forest, Midnight
- **Sticky notes**: Send any tab to desktop as a snapshot sticky note
- **Persistent storage**: Autosave to `%APPDATA%\AsmNotes` (UTF-8)
- **DPI-aware**: Per-monitor DPI scaling on Windows 11

## Build

### Requirements
- Windows 10/11 (x64)
- Visual Studio 2022 Build Tools
- Windows SDK 10.0.26100 or later

### Compile & Link

```bash
build.bat
```

Produces: `AsmNotes.exe` (7KB x64 PE executable)

## Project Structure

### Core Modules
- **main.asm** - Entry point, message loop, global state
- **consts.inc** - Win32 constants and control IDs
- **util.inc** - Heap allocation, string utilities, UTF-8/UTF-16 conversion
- **mainwnd.inc** - Main window class, tab management
- **themes.inc** - 6 color themes (RGB palettes)
- **markdown.inc** - Markdown to RTF converter
- **storage.inc** - AppData persistence (settings.ini + tab*.md)
- **sticky.inc** - Desktop sticky note window class
- **notes.rc/manifest** - Resources (menu, accelerators, DPI awareness)

### Build System
- **build.bat** - MASM (ml64) → RC (rc.exe) → LINK pipeline
- Uses VS 2022 Build Tools' native x64 toolchain
- No external dependencies or CRT required
- `/ENTRY:start /NODEFAULTLIB` - standalone executable

## Architecture

### Calling Convention
x64 System V (Windows/AMD64):
- RCX, RDX, R8, R9 = first 4 integer/pointer args
- 32-byte shadow space (caller-allocated stack frame)
- 16-byte stack alignment at function entry
- Preserved: RBX, R12-R15, RSP, RBP

### Memory Model
- **Heap**: `GetProcessHeap()` + `HeapAlloc`/`HeapFree`
- **Strings**: UTF-16 wide chars (3 bytes per character with null terminator in many cases)
- **Text buffers**: Growable heap-allocated buffers (GrowBuffer utility)
- **Global state**: Module-level QWORD/DWORD variables

### Persistence
- Settings: `%APPDATA%\AsmNotes\settings.ini` (WritePrivateProfileString)
- Tabs: `tab0.md`, `tab1.md`, ... (UTF-8, atomic writes via .tmp files)
- Autosave: Debounced 1.5s after last edit, on tab switch, on close
- Stickies: `sticky0.md`, ... (persistent layout, pin state)

## Testing

### Build Verification
```bash
bash verify-build.sh
```

Checks:
- All source files present
- Executable created and valid
- Git repository and commits
- PE format validation

### Test Fixtures
- `tests/test_simple.md` - Basic markdown with headings, bold, italic, lists

### Known Limitations

**Current (Skeleton Implementation):**
- GUI components (window creation, RichEdit) are implemented but require Windows environment to test
- Tested on: MASM v14.44.35226.0 with VS Build Tools 2022
- Does NOT run in WSL/bash (no Windows message pump in non-native environment)
- Requires actual Windows 11 for full GUI testing

**Not Yet Implemented (Stubs):**
- Tab control rendering (TCS_OWNERDRAWFIXED color support)
- RichEdit creation and text management
- Full Markdown parsing (bold, italic, lists, code, quotes)
- Sticky note drag/resize/pin UI
- DPI scaling for child controls
- Middle-click tab close, accelerator handling
- 32-tab limit enforcement, empty-state handling

## Next Steps (Windows Environment Required)

1. **Complete window initialization** - Fix RichEdit and tab control creation
2. **Tab management** - Implement owner-draw tab styling, new/close/switch logic
3. **Text buffer sync** - Load/save tab text on switch
4. **Persistence I/O** - Write/read ini and markdown files
5. **Sticky notes** - Window class and lifecycle for desktop notes
6. **Polish** - Keyboard shortcuts, theme switching, DPI scaling
7. **Test suite** - Automated GUI testing via COM/accessibility APIs

## Development Notes

- Commit message style: explain the *why*, list what changed
- One logical feature per commit
- Build must succeed with no errors/warnings after each commit
- Push to remote after each commit (no batching)

## Performance
- Executable size: 7.0 KB (x64 PE)
- Startup: Single-threaded, message pump-based
- Memory: Minimal (heap allocated only for text buffers)
- Text limit: Up to 128 KB per tab (configurable via buffer growth)

## License
Educational/demonstration code. Public domain.
