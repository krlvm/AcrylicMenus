#include <iostream>
#include <Windows.h>

using namespace std;
int main()
{
    if (0)
    {
        Sleep(5000);
        WCHAR buf[256];
        GetClassName(GetForegroundWindow(), buf, 256);
        wcout << buf << endl;
        return 0;
    }

    cout << "AcrylicMenus v0.1 Preview" << endl << "https://github.com/krlvm/AcrylicMenus" << endl << "(c) krlvm, 2021" << endl << endl;

    cout << "Installing hook... ";

    HINSTANCE hDLL = LoadLibrary(TEXT(".\\HookDLL.dll"));

    if (!hDLL)
    {
        cout << "Failed to load library: " << GetLastError();
        return 1;
    }
    
    // dumpbin /exports HookDLL.dll
    HOOKPROC hHookProc = (HOOKPROC)GetProcAddress(hDLL, "?CallWndProc@@YA_JH_K_J@Z");

    if (!hHookProc)
    {
        cout << "Can't find CallWndProc";
        return 1;
    }

    HHOOK hHook = SetWindowsHookEx(WH_CALLWNDPROC, hHookProc, hDLL, 0);

    if (hHook)
    {
        cout << "Success";
    }
    else
    {
        cout << "Error: " << GetLastError();
        return 1;
    }

    cout << endl;

    cout << "Press any key to unhook and exit" << endl;

    system("PAUSE");

    UnhookWindowsHookEx(hHook);
    FreeLibrary(hDLL);
    SendMessageTimeout(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 1000, NULL);

    cout << "Unhooked" << endl;
    return 0;
}