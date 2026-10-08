#!/bin/bash
# Build verification script for AsmNotes

set -e

PROJ_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJ_DIR"

echo "=== AsmNotes Build Verification ==="
echo

# Check source files exist
echo "Checking source files..."
required_files=(
    "main.asm"
    "consts.inc"
    "util.inc"
    "themes.inc"
    "markdown.inc"
    "storage.inc"
    "sticky.inc"
    "mainwnd.inc"
    "notes.rc"
    "notes.manifest"
    "winconst.h"
    "build.bat"
)

for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        echo "ERROR: Missing source file: $file"
        exit 1
    fi
done
echo "✓ All source files present"
echo

# Check .gitignore
if [ ! -f ".gitignore" ]; then
    echo "ERROR: Missing .gitignore"
    exit 1
fi
echo "✓ .gitignore present"
echo

# Check executable
if [ ! -f "AsmNotes.exe" ]; then
    echo "ERROR: AsmNotes.exe not built"
    echo "Run: build.bat"
    exit 1
fi

exe_size=$(stat -f%z AsmNotes.exe 2>/dev/null || stat -c%s AsmNotes.exe 2>/dev/null || echo "unknown")
echo "✓ AsmNotes.exe exists (size: $exe_size bytes)"
echo

# Check PE format
if file AsmNotes.exe | grep -q "PE32+ executable"; then
    echo "✓ Valid x64 PE executable"
else
    echo "ERROR: Not a valid x64 PE executable"
    exit 1
fi
echo

# Check git repository
if [ ! -d ".git" ]; then
    echo "ERROR: Not a git repository"
    exit 1
fi
echo "✓ Git repository initialized"
echo

# Check commits
commits=$(git rev-list --count HEAD 2>/dev/null || echo 0)
if [ "$commits" -lt 2 ]; then
    echo "ERROR: Not enough commits (found: $commits, expected: >= 2)"
    exit 1
fi
echo "✓ Git history present ($commits commits)"
echo

echo "=== Verification Passed ==="
echo "Build status: OK"
echo "Next: Test on actual Windows (GUI functionality)"
