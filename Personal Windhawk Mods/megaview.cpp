// ==WindhawkMod==
// @id              explorer-custom-mega-view
// @name            Explorer Custom Mega View
// @description     Opens a custom window with massive thumbnails for the current folder (Press Ctrl+Shift+M)
// @version         1.2
// @author          AI Assistant
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lcomctl32 -lole32 -loleaut32 -lshlwapi -lgdi32 -luuid
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Explorer Custom Mega View

Since Windows 11's modern File Explorer strictly caps thumbnail grid heights at 256 pixels, this mod bypasses the limitation entirely by creating a **custom, fully-functional viewer window**.

## How to use
1. Open any folder in File Explorer.
2. Press **Ctrl + Shift + M**.
3. A new "Mega View" window will instantly open, rendering the current folder's contents with massive, high-resolution thumbnails.

## Features
- Bypasses all Explorer grid limitations.
- Fully interactive: Double-click files to open them normally.
- Generates true high-resolution thumbnails (no blurry upscaling).
- Auto-arranges and scales perfectly.

## Settings
- **Target Size**: The resolution of the massive thumbnails (default: 640).
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- TargetSize: 640
  $name: Target Thumbnail Size
  $description: The resolution of the thumbnails in the custom view (e.g., 512, 640, 1024)
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <exdisp.h>
#include <commctrl.h>
#include <shellapi.h>
#include <propkey.h>
#include <string>
#include <thread>
#include <windhawk_api.h>

#define WM_APP_ADD_ITEM (WM_APP + 1)
#define WM_APP_NAVIGATE (WM_APP + 2)
#define WM_APP_ZOOM     (WM_APP + 3)
#define IDC_ADDRESSBAR  101

int g_TargetSize = 640;

struct ItemData {
    std::wstring fullPath;
    std::wstring fileName;
    HBITMAP hbmp;
};

struct LoadTask {
    std::wstring path;
    HWND hwndParent;
    int targetSize;
};

// Helper function to perfectly center the thumbnail on a transparent canvas
HBITMAP CreateCenteredThumbnail(HBITMAP hbmpOriginal, int targetSize) {
    BITMAP bm;
    if (!GetObject(hbmpOriginal, sizeof(bm), &bm)) return NULL;

    HDC hdcScreen = GetDC(NULL);
    HDC hdcDest = CreateCompatibleDC(hdcScreen);
    HDC hdcSrc = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = targetSize;
    bmi.bmiHeader.biHeight = -targetSize; // top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HBITMAP hbmpDest = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    
    if (hbmpDest) {
        // Fill with transparent pixels (0)
        memset(pBits, 0, targetSize * targetSize * 4);

        HBITMAP hOldDest = (HBITMAP)SelectObject(hdcDest, hbmpDest);
        HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, hbmpOriginal);

        // Calculate center position
        int x = (targetSize - bm.bmWidth) / 2;
        int y = (targetSize - bm.bmHeight) / 2;

        // Copy the original image into the center of the transparent canvas
        BitBlt(hdcDest, x, y, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);

        SelectObject(hdcDest, hOldDest);
        SelectObject(hdcSrc, hOldSrc);
    }

    DeleteDC(hdcSrc);
    DeleteDC(hdcDest);
    ReleaseDC(NULL, hdcScreen);

    return hbmpDest;
}

void LoadThumbnailsThread(LoadTask* task) {
    CoInitialize(NULL);
    
    std::wstring searchPath = task->path + L"\\*";
    WIN32_FIND_DATAW ffd;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &ffd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!IsWindow(task->hwndParent)) break;

            if (wcscmp(ffd.cFileName, L".") == 0 || wcscmp(ffd.cFileName, L"..") == 0) continue;
            
            std::wstring fullPath = task->path + L"\\" + ffd.cFileName;
            
            HBITMAP hbmp = NULL;
            IShellItem* psi = nullptr;
            if (SUCCEEDED(SHCreateItemFromParsingName(fullPath.c_str(), nullptr, IID_PPV_ARGS(&psi)))) {
                IShellItemImageFactory* pImageFactory = nullptr;
                if (SUCCEEDED(psi->QueryInterface(IID_PPV_ARGS(&pImageFactory)))) {
                    SIZE size = { task->targetSize, task->targetSize };
                    
                    HRESULT hr = pImageFactory->GetImage(size, SIIGBF_RESIZETOFIT, &hbmp);
                    if (FAILED(hr)) {
                        pImageFactory->GetImage(size, SIIGBF_RESIZETOFIT | SIIGBF_ICONONLY, &hbmp);
                    }
                    
                    pImageFactory->Release();
                }
                psi->Release();
            }
            
            if (hbmp) {
                // Center the image before adding it to the list
                HBITMAP hbmpCentered = CreateCenteredThumbnail(hbmp, task->targetSize);
                DeleteObject(hbmp); // Free the original uncentered image
                
                if (hbmpCentered) {
                    ItemData* itemData = new ItemData{ fullPath, std::wstring(ffd.cFileName), hbmpCentered };
                    PostMessageW(task->hwndParent, WM_APP_ADD_ITEM, 0, (LPARAM)itemData);
                }
            }
            
        } while (FindNextFileW(hFind, &ffd));
        FindClose(hFind);
    }
    
    delete task;
    CoUninitialize();
}

void ShowContextMenu(HWND hwnd, HWND hwndLV, POINT pt) {
    int iItem = ListView_GetNextItem(hwndLV, -1, LVNI_SELECTED);
    if (iItem == -1) return;

    LVITEMW lvi = {0};
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    ListView_GetItem(hwndLV, &lvi);
    std::wstring* pathPtr = (std::wstring*)lvi.lParam;

    IShellItem* psi;
    if (SUCCEEDED(SHCreateItemFromParsingName(pathPtr->c_str(), NULL, IID_PPV_ARGS(&psi)))) {
        IContextMenu* pcm;
        if (SUCCEEDED(psi->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARGS(&pcm)))) {
            HMENU hMenu = CreatePopupMenu();
            if (hMenu) {
                pcm->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_NORMAL);
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
                if (cmd > 0) {
                    CMINVOKECOMMANDINFOEX ici = { sizeof(ici) };
                    ici.cbSize = sizeof(ici);
                    ici.fMask = CMIC_MASK_UNICODE;
                    ici.hwnd = hwnd;
                    ici.lpVerb = MAKEINTRESOURCEA(cmd - 1);
                    ici.lpVerbW = MAKEINTRESOURCEW(cmd - 1);
                    ici.nShow = SW_SHOWNORMAL;
                    pcm->InvokeCommand((CMINVOKECOMMANDINFO*)&ici);
                }
                DestroyMenu(hMenu);
            }
            pcm->Release();
        }
        psi->Release();
    }
}

// Subclass for the Address Bar to catch the Enter key
LRESULT CALLBACK AddressBarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    if (uMsg == WM_KEYDOWN && wParam == VK_RETURN) {
        wchar_t path[MAX_PATH];
        GetWindowTextW(hWnd, path, MAX_PATH);
        HWND hwndParent = GetParent(hWnd);
        SendMessageW(hwndParent, WM_APP_NAVIGATE, 0, (LPARAM)new std::wstring(path));
        return 0;
    }
    if (uMsg == WM_SETFOCUS) {
        LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        SendMessage(hWnd, EM_SETSEL, 0, -1); // Select all text when clicked
        return res;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// Subclass for the ListView to catch Ctrl + Mouse Wheel for Dynamic Zooming
LRESULT CALLBACK ListViewSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    if (uMsg == WM_MOUSEWHEEL && (GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL)) {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        HWND hwndParent = GetParent(hWnd);
        PostMessageW(hwndParent, WM_APP_ZOOM, delta, 0);
        return 0; // Handled
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK MegaViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Fix Taskbar Grouping
            IPropertyStore* pps = nullptr;
            if (SUCCEEDED(SHGetPropertyStoreForWindow(hwnd, IID_PPV_ARGS(&pps)))) {
                PROPVARIANT pv;
                PropVariantInit(&pv);
                pv.vt = VT_LPWSTR;
                pv.pwszVal = (LPWSTR)CoTaskMemAlloc(128);
                wcscpy(pv.pwszVal, L"MegaView.App");
                pps->SetValue(PKEY_AppUserModel_ID, pv);
                PropVariantClear(&pv);
                pps->Release();
            }

            // Create Address Bar
            HWND hwndEdit = CreateWindowExW(0, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                0, 0, 0, 0, hwnd, (HMENU)IDC_ADDRESSBAR, GetModuleHandle(NULL), NULL);
            SetWindowSubclass(hwndEdit, AddressBarProc, 0, 0);
            
            // Set nice font for Address Bar
            HFONT hFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                      DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            SendMessageW(hwndEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)hFont);

            // Create ListView
            HWND hwndLV = CreateWindowExW(0, WC_LISTVIEWW, L"",
                WS_CHILD | WS_VISIBLE | LVS_ICON | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS,
                0, 0, 0, 0, hwnd, (HMENU)2, GetModuleHandle(NULL), NULL);
            SetWindowSubclass(hwndLV, ListViewSubclassProc, 0, 0);
            
            // Apply Dark Mode to ListView
            ListView_SetBkColor(hwndLV, RGB(32, 32, 32));
            ListView_SetTextBkColor(hwndLV, RGB(32, 32, 32));
            ListView_SetTextColor(hwndLV, RGB(255, 255, 255));

            return 0;
        }
        case WM_SIZE: {
            HWND hwndEdit = GetDlgItem(hwnd, IDC_ADDRESSBAR);
            HWND hwndLV = GetDlgItem(hwnd, 2);
            if (hwndEdit && hwndLV) {
                MoveWindow(hwndEdit, 0, 0, LOWORD(lParam), 28, TRUE);
                MoveWindow(hwndLV, 0, 28, LOWORD(lParam), HIWORD(lParam) - 28, TRUE);
            }
            return 0;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            HWND hwndControl = (HWND)lParam;
            if (GetDlgCtrlID(hwndControl) == IDC_ADDRESSBAR) {
                SetTextColor(hdc, RGB(255, 255, 255));
                SetBkColor(hdc, RGB(45, 45, 45));
                static HBRUSH hbrBkgnd = CreateSolidBrush(RGB(45, 45, 45));
                return (INT_PTR)hbrBkgnd;
            }
            break;
        }
        case WM_APP_ZOOM: {
            int delta = (int)wParam;
            int oldSize = g_TargetSize;
            
            if (delta > 0) g_TargetSize += 32;
            else g_TargetSize -= 32;
            
            if (g_TargetSize < 64) g_TargetSize = 64;
            if (g_TargetSize > 1024) g_TargetSize = 1024;
            
            if (oldSize != g_TargetSize) {
                // Debounce the reload so we don't spam the disk while scrolling
                SetTimer(hwnd, 1, 300, NULL); 
            }
            return 0;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                KillTimer(hwnd, 1);
                wchar_t path[MAX_PATH];
                HWND hwndEdit = GetDlgItem(hwnd, IDC_ADDRESSBAR);
                GetWindowTextW(hwndEdit, path, MAX_PATH);
                SendMessageW(hwnd, WM_APP_NAVIGATE, 0, (LPARAM)new std::wstring(path));
            }
            return 0;
        }
        case WM_APP_NAVIGATE: {
            std::wstring* newPath = (std::wstring*)lParam;
            HWND hwndLV = GetDlgItem(hwnd, 2);
            HWND hwndEdit = GetDlgItem(hwnd, IDC_ADDRESSBAR);
            
            SetWindowTextW(hwndEdit, newPath->c_str());
            
            // Free existing items
            int count = ListView_GetItemCount(hwndLV);
            for (int i = 0; i < count; i++) {
                LVITEMW lvi = {0};
                lvi.mask = LVIF_PARAM;
                lvi.iItem = i;
                if (ListView_GetItem(hwndLV, &lvi)) {
                    delete (std::wstring*)lvi.lParam;
                }
            }
            ListView_DeleteAllItems(hwndLV);
            
            // Clear image list
            HIMAGELIST himl = ListView_GetImageList(hwndLV, LVSIL_NORMAL);
            if (himl) {
                ImageList_Destroy(himl);
            }

            himl = ImageList_Create(g_TargetSize, g_TargetSize, ILC_COLOR32 | ILC_MASK, 0, 100);
            ListView_SetImageList(hwndLV, himl, LVSIL_NORMAL);
            ListView_SetIconSpacing(hwndLV, g_TargetSize + 32, g_TargetSize + 64);

            std::wstring title = L"Mega View - " + *newPath;
            SetWindowTextW(hwnd, title.c_str());

            LoadTask* task = new LoadTask{ *newPath, hwnd, g_TargetSize };
            std::thread(LoadThumbnailsThread, task).detach();
            
            delete newPath;
            return 0;
        }
        case WM_APP_ADD_ITEM: {
            ItemData* data = (ItemData*)lParam;
            HWND hwndLV = GetDlgItem(hwnd, 2);
            HIMAGELIST himl = ListView_GetImageList(hwndLV, LVSIL_NORMAL);
            
            int imgIndex = ImageList_Add(himl, data->hbmp, NULL);
            DeleteObject(data->hbmp);
            
            LVITEMW lvi = {0};
            lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
            lvi.iItem = ListView_GetItemCount(hwndLV);
            lvi.iImage = imgIndex;
            lvi.pszText = (LPWSTR)data->fileName.c_str();
            
            std::wstring* pathPtr = new std::wstring(data->fullPath);
            lvi.lParam = (LPARAM)pathPtr;
            
            ListView_InsertItem(hwndLV, &lvi);
            
            delete data;
            return 0;
        }
        case WM_NOTIFY: {
            LPNMHDR lpnmh = (LPNMHDR)lParam;
            if (lpnmh->code == NM_DBLCLK) {
                LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
                if (lpnmia->iItem >= 0) {
                    LVITEMW lvi = {0};
                    lvi.mask = LVIF_PARAM;
                    lvi.iItem = lpnmia->iItem;
                    if (ListView_GetItem(lpnmh->hwndFrom, &lvi)) {
                        std::wstring* pathPtr = (std::wstring*)lvi.lParam;
                        
                        DWORD attr = GetFileAttributesW(pathPtr->c_str());
                        if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
                            SendMessageW(hwnd, WM_APP_NAVIGATE, 0, (LPARAM)new std::wstring(*pathPtr));
                        } else {
                            ShellExecuteW(NULL, L"open", pathPtr->c_str(), NULL, NULL, SW_SHOWNORMAL);
                        }
                    }
                }
            }
            else if (lpnmh->code == NM_RCLICK) {
                LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
                if (lpnmia->iItem >= 0) {
                    POINT pt = lpnmia->ptAction;
                    ClientToScreen(lpnmh->hwndFrom, &pt);
                    ShowContextMenu(hwnd, lpnmh->hwndFrom, pt);
                }
            }
            else if (lpnmh->code == LVN_KEYDOWN) {
                LPNMLVKEYDOWN pnkd = (LPNMLVKEYDOWN)lParam;
                if (pnkd->wVKey == VK_BACK) {
                    wchar_t path[MAX_PATH];
                    HWND hwndEdit = GetDlgItem(hwnd, IDC_ADDRESSBAR);
                    GetWindowTextW(hwndEdit, path, MAX_PATH);
                    std::wstring currentPath = path;
                    size_t lastSlash = currentPath.find_last_of(L"\\/");
                    if (lastSlash != std::wstring::npos && lastSlash > 0) {
                        std::wstring parentPath = currentPath.substr(0, lastSlash);
                        if (parentPath.back() == L':') parentPath += L"\\";
                        SendMessageW(hwnd, WM_APP_NAVIGATE, 0, (LPARAM)new std::wstring(parentPath));
                    }
                }
            }
            break;
        }
        case WM_DESTROY: {
            HWND hwndLV = GetDlgItem(hwnd, 2);
            if (hwndLV) {
                int count = ListView_GetItemCount(hwndLV);
                for (int i = 0; i < count; i++) {
                    LVITEMW lvi = {0};
                    lvi.mask = LVIF_PARAM;
                    lvi.iItem = i;
                    if (ListView_GetItem(hwndLV, &lvi)) {
                        delete (std::wstring*)lvi.lParam;
                    }
                }
                HIMAGELIST himl = ListView_GetImageList(hwndLV, LVSIL_NORMAL);
                if (himl) ImageList_Destroy(himl);
            }
            HFONT hFont = (HFONT)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
            if (hFont) DeleteObject(hFont);
            
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

DWORD WINAPI MegaViewThread(LPVOID lpParam) {
    std::wstring* pathPtr = (std::wstring*)lpParam;
    std::wstring path = *pathPtr;
    delete pathPtr;

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = MegaViewWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"MegaThumbViewerClass";
    wc.hbrBackground = CreateSolidBrush(RGB(32, 32, 32)); // Dark Mode Window Background
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    std::wstring title = L"Mega View";
    HWND hwnd = CreateWindowExW(0, L"MegaThumbViewerClass", title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 800,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    // Trigger the initial navigation
    SendMessageW(hwnd, WM_APP_NAVIGATE, 0, (LPARAM)new std::wstring(path));

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

std::wstring GetActiveExplorerPath(HWND hwndActive) {
    std::wstring result = L"";
    IShellWindows* psw = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_ALL, IID_IShellWindows, (void**)&psw))) {
        long count = 0;
        psw->get_Count(&count);
        for (long i = 0; i < count; i++) {
            VARIANT v;
            v.vt = VT_I4;
            v.lVal = i;
            IDispatch* pdisp = nullptr;
            if (SUCCEEDED(psw->Item(v, &pdisp)) && pdisp) {
                IWebBrowserApp* pwa = nullptr;
                if (SUCCEEDED(pdisp->QueryInterface(IID_IWebBrowserApp, (void**)&pwa))) {
                    HWND hwndExp = NULL;
                    pwa->get_HWND((SHANDLE_PTR*)&hwndExp);
                    if (hwndExp == hwndActive) {
                        IDispatch* pdoc = nullptr;
                        if (SUCCEEDED(pwa->get_Document(&pdoc)) && pdoc) {
                            IServiceProvider* psp = nullptr;
                            if (SUCCEEDED(pdoc->QueryInterface(IID_PPV_ARGS(&psp)))) {
                                IFolderView* pfv = nullptr;
                                if (SUCCEEDED(psp->QueryService(SID_SFolderView, IID_PPV_ARGS(&pfv)))) {
                                    IPersistFolder2* ppf2 = nullptr;
                                    if (SUCCEEDED(pfv->GetFolder(IID_PPV_ARGS(&ppf2)))) {
                                        LPITEMIDLIST pidl = nullptr;
                                        if (SUCCEEDED(ppf2->GetCurFolder(&pidl))) {
                                            IShellItem* psi = nullptr;
                                            if (SUCCEEDED(SHCreateItemFromIDList(pidl, IID_PPV_ARGS(&psi)))) {
                                                LPWSTR name = nullptr;
                                                if (SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &name))) {
                                                    result = name;
                                                    CoTaskMemFree(name);
                                                }
                                                psi->Release();
                                            }
                                            CoTaskMemFree(pidl);
                                        }
                                        ppf2->Release();
                                    }
                                    pfv->Release();
                                }
                                psp->Release();
                            }
                            pdoc->Release();
                        }
                    }
                    pwa->Release();
                }
                pdisp->Release();
            }
            if (!result.empty()) break;
        }
        psw->Release();
    }
    return result;
}

DWORD WINAPI HotkeyThread(LPVOID) {
    CoInitialize(NULL);
    if (!RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_SHIFT, 'M')) {
        Wh_Log(L"Failed to register hotkey Ctrl+Shift+M");
    }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            HWND hwndActive = GetForegroundWindow();
            std::wstring path = GetActiveExplorerPath(hwndActive);
            if (!path.empty()) {
                std::wstring* pathPtr = new std::wstring(path);
                CreateThread(NULL, 0, MegaViewThread, pathPtr, 0, NULL);
            }
        }
    }
    CoUninitialize();
    return 0;
}

void LoadSettings() {
    g_TargetSize = Wh_GetIntSetting(L"TargetSize");
}

BOOL Wh_ModInit() {
    LoadSettings();
    Wh_Log(L"Initializing Explorer Custom Mega View...");
    
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    CreateThread(NULL, 0, HotkeyThread, NULL, 0, NULL);

    Wh_Log(L"Explorer Custom Mega View initialized successfully.");
    return TRUE;
}

void Wh_ModUninit() {
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
