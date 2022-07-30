#include <iostream>
#include <Windows.h>
#include "..\AcrylicMenus\amapi.h"
#ifdef _WIN64
#ifdef _DEBUG
#pragma comment(lib, "..\\x64\\Debug\\AcrylicMenus.lib")
#else
#pragma comment(lib, "..\\x64\\Release\\AcrylicMenus.lib")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "..\\Debug\\AcrylicMenus.lib")
#else
#pragma comment(lib, "..\\Release\\AcrylicMenus.lib")
#endif
#endif

#define INJECT_EXPLORER_ONLY TRUE

const LPCWSTR szWindowClass = L"ACRYLICMENUS";
const UINT WM_TASKBARCREATED = RegisterWindowMessage(L"TaskbarCreated");

bool InjectHook()
{
	if (IsHookInstalled())
	{
		UnregisterHook();
	}

	if (!INJECT_EXPLORER_ONLY)
	{
		return RegisterHook(0);
	}

	HWND hwnd = GetShellWindow();
	if (hwnd)
	{
		DWORD dwProcessId;
		GetWindowThreadProcessId(hwnd, &dwProcessId);
		return RegisterHook(dwProcessId);
	}
	SetLastError(ERROR_NOT_FOUND);
	return false;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_TASKBARCREATED)
	{
		InjectHook();
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	HANDLE hMutex = CreateMutex(NULL, TRUE, szWindowClass);
	if (!hMutex || ERROR_ALREADY_EXISTS == GetLastError())
	{
		return ERROR_ALREADY_EXISTS;
	}

	if (!InjectHook())
	{
		return GetLastError();
	}

	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = szWindowClass;
	if (!RegisterClassEx(&wcex))
	{
		return 2;
	}
	HWND hwnd = CreateWindowEx(0, szWindowClass, nullptr, 0, 0, 0, 0, 0, nullptr, NULL, NULL, NULL);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);

	return (int)msg.wParam;
}