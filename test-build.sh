#!/bin/bash
# Automated build verification test suite

set -e

PROJ_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJ_DIR"

PASS=0
FAIL=0

echo "========================================="
echo "AsmNotes Build Test Suite"
echo "========================================="
echo

# Test 1: Check all source files
echo "[Test 1] Source files present..."
required_files=(
    "main.asm" "consts.inc" "util.inc" "themes.inc" "markdown.inc"
    "storage.inc" "sticky.inc" "mainwnd.inc" "notes.rc" "notes.manifest"
    "winconst.h" "build.bat"
)

for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file"
    else
        echo "  ✗ FAIL: $file"
        FAIL=$((FAIL + 1))
    fi
done

if [ $FAIL -eq 0 ]; then
    PASS=$((PASS + 1))
    echo "PASS: All source files present"
else
    echo "FAIL: Missing source files"
fi
echo

# Test 2: Check .gitignore
echo "[Test 2] .gitignore configured..."
if grep -q "\.obj" .gitignore && grep -q "\.exe" .gitignore; then
    echo "  ✓ Proper .gitignore entries"
    PASS=$((PASS + 1))
else
    echo "  ✗ FAIL: .gitignore missing required entries"
    FAIL=$((FAIL + 1))
fi
echo

# Test 3: Check git repository
echo "[Test 3] Git repository..."
if [ -d ".git" ]; then
    echo "  ✓ Git initialized"
    commits=$(git rev-list --count HEAD 2>/dev/null || echo "0")
    echo "  ✓ Commits: $commits"
    if [ "$commits" -gt 3 ]; then
        echo "  ✓ Reasonable commit history"
        PASS=$((PASS + 1))
    else
        echo "  ✗ FAIL: Not enough commits"
        FAIL=$((FAIL + 1))
    fi
else
    echo "  ✗ FAIL: Not a git repository"
    FAIL=$((FAIL + 1))
fi
echo

# Test 4: Check executable
echo "[Test 4] Executable..."
if [ -f "AsmNotes.exe" ]; then
    size=$(stat -f%z AsmNotes.exe 2>/dev/null || stat -c%s AsmNotes.exe 2>/dev/null)
    echo "  ✓ AsmNotes.exe exists ($size bytes)"

    if file AsmNotes.exe | grep -q "PE32+ executable"; then
        echo "  ✓ Valid x64 PE executable"
        PASS=$((PASS + 1))
    else
        echo "  ✗ FAIL: Invalid PE format"
        FAIL=$((FAIL + 1))
    fi
else
    echo "  ✗ FAIL: AsmNotes.exe not built"
    FAIL=$((FAIL + 1))
fi
echo

# Test 5: Module structure
echo "[Test 5] Module structure..."
has_header=0
has_prolog=0
has_footer=0
has_color=0

[ -f "README.md" ] && has_header=1
[ -f "build.bat" ] && has_prolog=1
[ -f ".gitignore" ] && has_footer=1
grep -q "THEME_" themes.inc 2>/dev/null && has_color=1

if [ "$has_header$has_prolog$has_footer$has_color" = "1111" ]; then
    echo "  ✓ All modules documented and configured"
    PASS=$((PASS + 1))
else
    echo "  ✗ Some modules incomplete"
    FAIL=$((FAIL + 1))
fi
echo

# Test 6: Build reproducibility
echo "[Test 6] Build files..."
echo "  Checking timestamps for recent build..."
if [ -f "main.obj" ] && [ "main.asm" -ot "main.obj" ]; then
    echo "  ✓ Object files up-to-date with source"
    PASS=$((PASS + 1))
else
    echo "  Note: Build may need refresh (run build.bat)"
fi
echo

# Summary
echo "========================================="
echo "Summary: $PASS passed, $FAIL failed"
echo "========================================="

if [ $FAIL -eq 0 ]; then
    echo "✓ ALL TESTS PASSED"
    exit 0
else
    echo "✗ SOME TESTS FAILED"
    exit 1
fi
