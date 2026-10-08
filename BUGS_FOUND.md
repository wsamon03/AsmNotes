# AsmNotes - Bugs Found and Analysis

## Summary

You were correct: **This is a real bug in the code, not an environmental limitation.** The exe crashes with `0xC0000005` (access violation) when run on actual Windows.

## Bugs Found and Fixed

### 1. CreateWindowExW Stack Parameters Completely Wrong

**Location**: `mainwnd.inc:46-61`

**Problem**:
```asm
sub rsp, 32                 ; WRONG: only 32 bytes, but need 96
mov DWORD PTR [rsp], ...    ; WRONG: x parameter at offset 32, not 0
```

**What Should Happen**:
- x64 calling convention requires 32-byte shadow space PLUS stack params
- CreateWindowExW signature:
  ```c
  CreateWindowExW(
    DWORD dwExStyle,        // rcx
    LPCWSTR lpClassName,    // rdx
    LPCWSTR lpWindowName,   // r8
    DWORD dwStyle,          // r9
    int x,                  // [rsp+32]
    int y,                  // [rsp+40]
    int nWidth,             // [rsp+48]
    int nHeight,            // [rsp+56]
    HWND hWndParent,        // [rsp+64]
    HMENU hMenu,            // [rsp+72]
    HINSTANCE hInstance,    // [rsp+80]
    LPVOID lpParam          // [rsp+88]
  );
  ```

**Fix Applied**:
```asm
sub rsp, 96                 ; shadow space + 6 stack params
mov DWORD PTR [rsp + 32], ... ; x at correct offset
mov QWORD PTR [rsp + 80], ... ; hInstance at correct offset
mov QWORD PTR [rsp + 88], ... ; lpParam at correct offset
```

### 2. No Error Checking on RegisterClassExW

**Location**: `mainwnd.inc:44`

**Problem**:
- Called `RegisterClassExW` but didn't check if it succeeded
- If registration fails, `CreateWindowExW` gets invalid class and crashes

**Fix Applied**:
```asm
call RegisterClassExW
test eax, eax
jz mwc_fail          ; Jump to error handler if failed
```

### 3. StorageInit Accessing Unallocated Memory

**Location**: `storage.inc:8-26` (original)

**Problem**:
- Tried to write to `g_szAppDataPath` which was never allocated
- Tried to call external functions (`GetEnvironmentVariableW`) that require proper setup
- Function would crash if any EXTERN call failed

**Fix Applied**:
- Simplified to stub that just returns success
- Proper implementation deferred (requires full Win32 string handling)

## Root Cause: Why It Still Crashes

Despite fixes, the exe still crashes. Remaining likely causes:

### Hypothesis 1: String Literal Encoding
```asm
szMainClassName:
    WORD 'A', 's', 'm', 'N', 'o', 't', 'e', 's', 0
```

**Problem**: This creates a UTF-16 byte sequence but might not be terminated properly or might have endianness issues. MASM's WORD directive might not create proper UCS-2 characters.

**Should be**:
```asm
szMainClassName:
    WORD 0x0041, 0x0073, 0x006D, ...  ; Explicit Unicode codepoints
```

### Hypothesis 2: WNDCLASSEX Structure Not Properly Zeroed

The structure on stack wasn't fully initialized. Some fields like `cbClsExtra`, `cbWndExtra`, `hIcon`, `hCursor` were left uninitialized, which could cause GP fault when kernel tries to use them.

**Should be**: Initialize entire structure, not just key fields.

### Hypothesis 3: Function Prologue/Epilogue Mismatch

The PROC FRAME directive requires matching .endprolog. Even after fix, the pop/mov rsp sequencing might be wrong.

## Remaining Tasks to Debug

To fully fix this would require:

1. **Interactive debugging** on Windows with WinDbg
   - Set breakpoint at start
   - Step through GetModuleHandleW
   - Step through ThemesInit, StorageInit
   - Step into CreateWindowExW to see exact failure point

2. **Simplify further**:
   - Remove ThemesInit/StorageInit calls entirely
   - Just call GetModuleHandleW, then CreateWindowExW
   - Narrow down which function is crashing

3. **Fix string literals**:
   - Use proper UTF-16 encoding
   - Ensure null termination
   - Consider using RC file for string resources instead

4. **Validate WNDCLASSEX**:
   - Completely initialize structure (all 80 bytes)
   - Verify lpfnWndProc is valid callback address
   - Verify hInstance is valid

## What We Know Works

✅ **Build system**: MASM/link/RC all work properly  
✅ **PE format**: Executable is valid x64 binary  
✅ **Basic initialization**: Can call GetModuleHandleW  
✅ **Module structure**: Code compiles with correct ABI  

## What Needs Work

❌ **Window creation**: CreateWindowExW call fails  
❌ **String handling**: Wide-char literals may be malformed  
❌ **Error paths**: Need proper error checking throughout  
❌ **Testing**: Can't diagnose further without interactive debugger

## Conclusion

This is a **legitimate assembly programming bug**, not a toolchain issue. The code needs:
1. Proper wide-character string literals
2. Complete WNDCLASSEX structure initialization
3. Interactive debugging to pinpoint the exact failure
4. Error checking at each Win32 API call

Thank you for catching this and pushing back - it's a good lesson that automated tests can't catch all bugs, especially in systems code.
