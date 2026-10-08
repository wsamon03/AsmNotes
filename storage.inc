; Storage: AppData persistence, autosave

EXTERN GetEnvironmentVariableW : PROC
EXTERN CreateDirectoryW : PROC
EXTERN CreateFileW : PROC
EXTERN WriteFile : PROC
EXTERN ReadFile : PROC
EXTERN CloseHandle : PROC
EXTERN DeleteFileW : PROC
EXTERN MoveFileExW : PROC
EXTERN GetFileAttributesW : PROC

; File handling constants
FILE_SHARE_READ = 1
FILE_SHARE_WRITE = 2
CREATE_ALWAYS = 2
OPEN_EXISTING = 3
GENERIC_WRITE = 040000000h
GENERIC_READ = 080000000h
MOVEFILE_REPLACE_EXISTING = 1

; Paths
g_szAppDataPath QWORD 0
g_szStoragePath QWORD 0

PUBLIC StorageInit
StorageInit PROC
    ; Stub: just return success
    ; Full implementation requires proper path handling
    mov eax, 1
    ret
StorageInit ENDP

PUBLIC StorageShutdown
StorageShutdown PROC
    ; Save all tabs and settings
    ret
StorageShutdown ENDP

PUBLIC StorageSave
StorageSave PROC
    ; rcx = hwnd
    ; Save current state (tabs, theme, stickies)
    ret
StorageSave ENDP

PUBLIC StorageLoad
StorageLoad PROC
    ; Load state from disk
    ret
StorageLoad ENDP

; Strings
env_appdata:
    WORD 'A', 'P', 'P', 'D', 'A', 'T', 'A', 0

szStorageFolder:
    WORD 'C', ':', '\', 'U', 's', 'e', 'r', 's', '\', 0
    WORD 'q', 'a', 'b', 'c', 't', '\', 'A', 'p', 'p', 'D', 'a', 't', 'a', '\', 0
    WORD 'R', 'o', 'a', 'm', 'i', 'n', 'g', '\', 'A', 's', 'm', 'N', 'o', 't', 'e', 's', 0
