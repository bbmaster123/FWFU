// ==WindhawkMod==
// @id              remove-bg-batch-paint
// @name            Remove Background with Paint (Batch)
// @description     Adds a right-click menu item that invokes Paint's background removal via ms-paint URI
// @version         0.1
// @author          bbmaster123
// @include         explorer.exe
// @compilerOptions -ladvapi32 -luser32 -lshell32
// ==/WindhawkMod==

#include <windows.h>
#include <shlobj.h>
#include <string>

// Registry path (HKCU) for all files' context menu
// Scope is HKCU so it doesn't require admin and is per-user.
static const wchar_t* kVerbKey      = L"Software\\Classes\\*\\shell\\RemoveBackgroundWithPaint";
static const wchar_t* kVerbCmdKey   = L"Software\\Classes\\*\\shell\\RemoveBackgroundWithPaint\\command";
static const wchar_t* kVerbName     = L"Remove Background with Paint";

// Command will call powershell to open ms-paint://backgroundRemoval with the selected file.
// Using -NoProfile and -ExecutionPolicy Bypass for reliability.
// %1 is the selected file path provided by Explorer.
static std::wstring BuildCommandLine() {
    return L"rundll32.exe url.dll,FileProtocolHandler "
           L"\"ms-paint://backgroundRemoval?isTemporaryPath=false&file=%1\"";
}

static bool CreateRegKeyAndSetDefault(HKEY root, const wchar_t* subkey, const wchar_t* defaultValue) {
    HKEY hKey = nullptr;
    DWORD disp = 0;
    LONG st = RegCreateKeyExW(root, subkey, 0, nullptr, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, nullptr, &hKey, &disp);
    if (st != ERROR_SUCCESS) {
        return false;
    }
    st = RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(defaultValue),
                        static_cast<DWORD>((wcslen(defaultValue) + 1) * sizeof(wchar_t)));
    RegCloseKey(hKey);
    return st == ERROR_SUCCESS;
}

static bool RemoveRegKey(HKEY root, const wchar_t* subkey) {
    // Try to delete subkey tree; fall back to RegDeleteKeyW if needed.
    // On older systems, SHDeleteKeyW is available via shlwapi, but we keep to advapi32 only.
    // Attempt recursive delete using RegDeleteTreeW.
    LONG st = RegDeleteTreeW(root, subkey);
    if (st == ERROR_SUCCESS) return true;
    // If tree delete failed, try deleting the key directly (if empty).
    st = RegDeleteKeyW(root, subkey);
    return st == ERROR_SUCCESS;
}

BOOL Wh_ModInit() {
    // Create the verb key with a friendly display name
    if (!CreateRegKeyAndSetDefault(HKEY_CURRENT_USER, kVerbKey, kVerbName)) {
        MessageBoxW(GetActiveWindow(), L"Failed to create context menu verb key in HKCU.", L"Windhawk", MB_OK | MB_ICONWARNING);
        return FALSE;
    }

    // Create the command subkey with our PowerShell launcher
    std::wstring cmd = BuildCommandLine();
    if (!CreateRegKeyAndSetDefault(HKEY_CURRENT_USER, kVerbCmdKey, cmd.c_str())) {
        MessageBoxW(GetActiveWindow(), L"Failed to create verb command key in HKCU.", L"Windhawk", MB_OK | MB_ICONWARNING);
        // Attempt cleanup
        RemoveRegKey(HKEY_CURRENT_USER, kVerbKey);
        return FALSE;
    }

    // Optional: force Explorer to refresh its verb cache by broadcasting settings change
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    // Informative toast
    Wh_Log(L"[remove-bg-batch-paint] Context menu verb registered in HKCU. Item should appear on next right-click.");
    return TRUE;
}

void Wh_ModUninit() {
    // Remove the command key first, then the verb key
    RemoveRegKey(HKEY_CURRENT_USER, kVerbCmdKey);
    RemoveRegKey(HKEY_CURRENT_USER, kVerbKey);

    // Refresh Explorer associations
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    Wh_Log(L"[remove-bg-batch-paint] Context menu verb removed from HKCU.");
}
