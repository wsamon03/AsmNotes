; Storage: AppData persistence, autosave

PUBLIC StorageInit
StorageInit PROC
    ; Initialize storage subsystem
    ; Load previous state from %APPDATA%\AsmNotes
    ret
StorageInit ENDP

PUBLIC StorageShutdown
StorageShutdown PROC
    ; Finalize: save all state atomically
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
