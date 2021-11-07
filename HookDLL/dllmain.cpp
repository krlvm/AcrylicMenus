#include "pch.h"
#include <Windows.h>
#include <tchar.h>
#include "AcrylicHelper.h"

#define ACRYLIC_OPACITY 192
#define WINDOWS_11

#ifdef WINDOWS_11
#define DELAY_TIME 250
#define CONTEXTM_COLORKEY RGB(249, 249, 249)
#define EXPLORER_COLORKEY CONTEXTM_COLORKEY
#define CONTEXTM_TINT 0xF9F9F9
#define EXPLORER_TINT CONTEXTM_TINT
#define EXPLORER_COLORKEY_DARK RGB(43, 43, 43)
#define EXPLORER_TINT_DARK 0x2B2B2B
#else
#define DELAY_TIME 200
#define CONTEXTM_COLORKEY RGB(242, 242, 242)
#define CONTEXTM_TINT 0xEEEEEE
#define EXPLORER_COLORKEY RGB(238, 238, 238)
#define EXPLORER_TINT 0xEEEEEE
#define EXPLORER_COLORKEY_DARK RGB(43, 43, 43)
#define EXPLORER_TINT_DARK 0x2B2B2B
#endif

BOOL bIsExplorer = FALSE;
HKEY hKeyPersonalization;

int IsExplorerDarkTheme()
{
    DWORD dwBufferSize(sizeof(DWORD));
    DWORD nResult(0);
    LONG nError = RegQueryValueEx(
        hKeyPersonalization,
        L"AppsUseLightTheme",
        0,
        NULL,
        reinterpret_cast<LPBYTE>(&nResult),
        &dwBufferSize
    );
    return ERROR_SUCCESS == nError ? !nResult : FALSE;
}

DWORD WINAPI HandleMenu(LPVOID lpParameter)
{
    Sleep(DELAY_TIME);
    HWND hwnd = (HWND)lpParameter;

    int bIsExplorerDark = bIsExplorer && IsExplorerDarkTheme();

	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP);
    SetLayeredWindowAttributes(hwnd, bIsExplorer ? (bIsExplorerDark ? EXPLORER_COLORKEY_DARK : EXPLORER_COLORKEY) : CONTEXTM_COLORKEY, 0, LWA_COLORKEY);
    ///Sleep(100);
    AcrylicHelper::ApplyAcrylic(hwnd, ACRYLIC_OPACITY, bIsExplorer ? (bIsExplorerDark ? EXPLORER_TINT_DARK : EXPLORER_TINT) : CONTEXTM_TINT);
    Sleep(1);
    InvalidateRect(hwnd, nullptr, true);

    ExitThread(0);
}

__declspec(dllexport) LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= HC_ACTION)
    {
        LPCWPSTRUCT cwps = (LPCWPSTRUCT)lParam;

        if (cwps->message == WM_CREATE)
        {
            WCHAR szClass[128];
            GetClassName(cwps->hwnd, szClass, 127);
            if (wcscmp(szClass, L"#32768") == 0)
            {
                CreateThread(NULL, 0, &HandleMenu, cwps->hwnd, 0, NULL);
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        WCHAR exePath[MAX_PATH + 1];
        DWORD len = GetModuleFileNameW(NULL, exePath, MAX_PATH);
        //MessageBox(NULL, exePath, L"bIsExplorer", MB_OK);
        if (len > 0 && _wcsicmp(exePath, L"c:\\windows\\explorer.exe") == 0) {
            bIsExplorer = TRUE;
            RegOpenKeyEx(
                HKEY_CURRENT_USER,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                0, KEY_READ, &hKeyPersonalization
            );
        }
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

