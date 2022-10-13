#pragma once
#include "pch.h"
#include "SettingsHelper.h"
#include "AppearanceConfiguration.h"

namespace AcrylicMenus
{
namespace WindowHelper
{
	///
	/// Windows 10 has ugly white context menu borders
	/// As the borders are in the non-client area,
	/// we can't hook the painting event and need
	/// to redraw them manually
	/// 
	/// This is enabled only for light mode menus,
	/// because it is noticeable, due to low contrast
	/// between menu background color and original white border,
	/// but this is very noticeable in dark mode
	/// 
	/// On Windows 11, we change borders color
	/// natively using DwmSetWindowAttribute API
	///

	typedef struct _MENU_BORDER_ANIMATION_DATA
	{
		HDC wndDC;
		RECT wndRect;
	} MENU_BORDER_ANIMATION_DATA;

	DWORD WINAPI RedrawMenuBorderAnimationThreadProc(LPVOID lpParameter)
	{
		MENU_BORDER_ANIMATION_DATA* pData = (MENU_BORDER_ANIMATION_DATA*)lpParameter;

		HBRUSH hbr;
		for (int colorValue = 255; colorValue > 0; colorValue -= 25) // -> 5
		{
			hbr = CreateSolidBrush(RGB(colorValue, colorValue, colorValue));
			FillRect(pData->wndDC, &pData->wndRect, hbr);
			DeleteObject(hbr);
			Sleep(1);
		}

		return 0;
	}
	void RedrawMenuBorder(HWND hWnd)
	{
		RECT wndRect;
		GetWindowRect(hWnd, &wndRect);
		OffsetRect(&wndRect, -wndRect.left, -wndRect.top);

		HDC wndDC = GetWindowDC(hWnd);

		ExcludeClipRect(wndDC,
			wndRect.left + 1,
			wndRect.top + 1,
			wndRect.right - 1,
			wndRect.bottom - 1
		);

		if (MenuManager::g_bIsDarkMode && SettingsHelper::g_redrawDarkThemeBorders10Animation && MenuManager::g_menuOwnerDrawn)
		{
			MENU_BORDER_ANIMATION_DATA data = { wndDC, wndRect };
			HANDLE hThread = CreateThread(NULL, 0, RedrawMenuBorderAnimationThreadProc, &data, 0, NULL);
			if (hThread) CloseHandle(hThread);
		}
		else
		{
			FillRect(wndDC, &wndRect, (HBRUSH)GetStockObject(BLACK_BRUSH));
		}
	}

	///
	/// After user selects option, if menu fade out animation
	/// is enabled in system settings, system will draw
	/// the selected item, but its background color alpha value 
	/// is set to 0 and GDI does not support transparency, so
	/// it will be just black, which is not pleasant to see
	/// in light mode, so we temporary turn off the animation
	/// (see MenuHandler SubclassProc), then starting our
	/// custom animation and re-enabling the system preference.
	/// 
	/// We create a layered popup window, capture the item
	/// with BitBlt and draw it onto the window, then beginning
	/// the animation in a new thread - it will resemble
	/// the default fade out animation.
	/// 

	#pragma region Menu Fade Out Animation

	#ifdef _WIN64
		#define GetTickCountU  GetTickCount64
	#else
		#define GetTickCountU  GetTickCount
	#endif

	#define MENU_ANIMATION_FRAMES   350
	#define MENU_ANIMATION_INTERVAL 10

	DWORD WINAPI MenuFadeOutAnimationThreadProc(LPVOID lpParameter)
	{
		HWND hwnd = (HWND)lpParameter;
		BLENDFUNCTION pBlend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

		DWORD dwStart = GetTickCountU() - 55;

		Sleep(MENU_ANIMATION_INTERVAL);

		for (int i = GetTickCountU() - dwStart; i <= MENU_ANIMATION_FRAMES; i = GetTickCount64() - dwStart)
		{
			pBlend.SourceConstantAlpha = ((255 * MENU_ANIMATION_FRAMES) - (255 * i)) / MENU_ANIMATION_FRAMES;
			UpdateLayeredWindow(hwnd, NULL, NULL, NULL, NULL, NULL, 0, &pBlend, ULW_ALPHA);
			Sleep(MENU_ANIMATION_INTERVAL);
		}

		return PostMessageW(hwnd, WM_CLOSE, NULL, NULL);
	}

	void BeginMenuFadeOutAnimation(MENUBARINFO &pmbi)
	{
		HWND hwnd = CreateWindowExW(
			0x80800A8,                          // Spy++
			L"Static",
			L"Fade",
			WS_POPUP,
			pmbi.rcBar.left,
			pmbi.rcBar.top,
			pmbi.rcBar.right - pmbi.rcBar.left,
			pmbi.rcBar.bottom - pmbi.rcBar.top,
			NULL,
			NULL,
			NULL,
			NULL
		);

		RECT rcDst;
		GetWindowRect(hwnd, &rcDst);

		SIZE  szDst = { rcDst.right - rcDst.left, rcDst.bottom - rcDst.top };
		POINT ptDst = { rcDst.left, rcDst.top };
		POINT ptSrc = { 0, 0 };

		BLENDFUNCTION pBlend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

		HDC hdcScreen = GetDC(NULL);
		HDC hdcMemory = CreateCompatibleDC(hdcScreen);

		HBITMAP bmpMemory = CreateCompatibleBitmap(hdcScreen, szDst.cx, szDst.cy);
		SelectObject(hdcMemory, bmpMemory);

		BITMAP bitmap;
		GetObject(bmpMemory, sizeof(BITMAP), &bitmap);

		BitBlt(
			hdcMemory,
			0,
			0,
			szDst.cx,
			szDst.cy,
			hdcScreen,
			pmbi.rcBar.left,
			pmbi.rcBar.top,
			SRCCOPY
		);

		UpdateLayeredWindow(hwnd, hdcScreen, &ptDst, &szDst, hdcMemory, &ptSrc, 0, &pBlend, ULW_ALPHA);
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

		DeleteDC(hdcMemory);
		ReleaseDC(NULL, hdcScreen);

		HANDLE hThread = CreateThread(NULL, 0, MenuFadeOutAnimationThreadProc, hwnd, 0, NULL);
		if (hThread) CloseHandle(hThread);
	}

	#pragma endregion

	#pragma region Delayed Message

	struct DELAYEDMESSAGEINFO
	{
		HWND   hWnd;
		UINT   uMsg;
		DWORD  dwDelayMilliseconds;
	};
	DWORD WINAPI SendMessageDelayedThreadProc(LPVOID lpParameter)
	{
		DELAYEDMESSAGEINFO* info = (DELAYEDMESSAGEINFO*)lpParameter;
		if (info->dwDelayMilliseconds)
		{
			Sleep(info->dwDelayMilliseconds);
		}
		SendMessage(info->hWnd, info->uMsg, 0, 0);
		delete info;
		return 0;
	}
	void SendMessageDelayed(HWND hWnd, UINT uMsg, DWORD dwDelayMilliseconds)
	{
		DELAYEDMESSAGEINFO* info = new DELAYEDMESSAGEINFO;
		info->hWnd = hWnd;
		info->uMsg = uMsg;
		info->dwDelayMilliseconds = dwDelayMilliseconds;

		HANDLE hThread = CreateThread(NULL, 0, &SendMessageDelayedThreadProc, info, 0, NULL);
		if (hThread) CloseHandle(hThread);
	}

	#pragma endregion
}
}