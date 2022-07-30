#pragma once
#include "pch.h"

namespace AcrylicMenus
{
	namespace MenuManager
	{
		extern thread_local HWND g_hWnd;
		extern thread_local bool g_bIsDarkMode;
		extern thread_local DWORD g_dwBkColor;
		extern thread_local bool g_bBkColorRefresh;
		extern thread_local bool g_alphaFixedState;

		void SetCurrentMenu(HWND hWnd);
		void RefreshCurrentMenuBackground(bool bIsDarkMode, DWORD dwBkColor);
	}
}