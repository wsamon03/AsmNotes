@echo off
setlocal enabledelayedexpansion

REM Paths
set MSVC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64
set SDK_PATH=C:\Program Files (x86)\Windows Kits\10
set SDK_VER=10.0.26100.0
set SDK_BIN=%SDK_PATH%\bin\%SDK_VER%\x64

REM Compile resource file
echo Compiling resources...
"%SDK_BIN%\rc.exe" /d NDEBUG /l 0x409 /r notes.rc
if errorlevel 1 (
    echo RC.EXE failed
    exit /b 1
)

REM Assemble
echo Assembling...
"%MSVC_PATH%\ml64.exe" /c /Fo main.obj main.asm
if errorlevel 1 (
    echo ML64.EXE failed
    exit /b 1
)

REM Link
echo Linking...
"%MSVC_PATH%\link.exe" /ENTRY:start /NODEFAULTLIB /SUBSYSTEM:WINDOWS ^
    /LIBPATH:"%SDK_PATH%\Lib\%SDK_VER%\um\x64" ^
    /LIBPATH:"%SDK_PATH%\Lib\%SDK_VER%\ucrt\x64" ^
    kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib ^
    ole32.lib advapi32.lib dwmapi.lib uxtheme.lib shell32.lib ^
    notes.res main.obj /OUT:AsmNotes.exe
if errorlevel 1 (
    echo LINK.EXE failed
    exit /b 1
)

echo Build complete: AsmNotes.exe
