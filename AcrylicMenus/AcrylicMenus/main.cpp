#include <iostream>
#include <Windows.h>

#include "AcrylicHelper.h"
#include "ThemeHelper.h"

#define ACRYLIC_OPACITY_LIGHT 196
#define EXPLORER_TINT_LIGHT 0xEEEEEE

#define ACRYLIC_OPACITY_DARK 196
#define EXPLORER_TINT_DARK 0x2B2B2B

HWINEVENTHOOK g_hook;
HKEY hKeyPersonalization;

bool IsExplorerDarkTheme()
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

void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime)
{
    WCHAR szClass[256];
    GetClassName(hwnd, szClass, sizeof(szClass));
    if (wcscmp(szClass, L"#32768") == 0)
    {
        bool bIsExplorerDark = IsExplorerDarkTheme();
        AcrylicHelper::ApplyAcrylic(hwnd,
            bIsExplorerDark ? ACRYLIC_OPACITY_DARK : ACRYLIC_OPACITY_LIGHT,
            bIsExplorerDark ? EXPLORER_TINT_DARK : EXPLORER_TINT_LIGHT);
    }
}

void RegisterEventHook(DWORD pid)
{
    g_hook = SetWinEventHook(
        EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE,
        NULL,
        HandleWinEvent,
        pid, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
}

void UnregisterEventHook()
{
    UnhookWinEvent(g_hook);
}

bool GetExplorerPID(DWORD &pid)
{
    HWND hwnd = FindWindow(L"Shell_TrayWnd", NULL);
    return GetWindowThreadProcessId(hwnd, &pid);
}

int main()
{
    std::cout << "AcrylicMenus v0.5 Preview" << std::endl;
    std::cout << "https://github.com/krlvm/AcrylicMenus" << std::endl;
    std::cout << "(c) krlvm, 2021" << std::endl;
    std::cout << std::endl;

    DWORD pid;
    if (!GetExplorerPID(pid)) {
        std::cout << "Please, start a File Explorer process to attach" << std::endl;
        return -1;
    }

    UpdateThemeData();
    RegOpenKeyEx(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKeyPersonalization
    );

    RegisterEventHook(pid);
    std::cout << "Attached to File Explorer (PID=" << pid << ")" << std::endl;
    std::cout << "AcrylicMenus should be restarted after Explorer restart to take effect" << std::endl;
    std::cout << "You need to restart your PC if context menus became black after stop" << std::endl;
    std::cout <<  std::endl;
    std::cout << "Close this window to unregister event hook" << std::endl;

    MSG msg;
    while (true) {
        GetMessage(&msg, NULL, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterEventHook();
}