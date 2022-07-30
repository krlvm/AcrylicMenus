#include "pch.h"
#include "MenuHandler.h"
#include "MenuManager.h"
#include "ThemeHelper.h"
#include "AcrylicHelper.h"
#include "WindowHelper.h"
#include "SystemHelper.h"
#include "AppearanceConfiguration.h"

#define MN_BUTTONDOWN     0x1ED
#define MN_BUTTONUP       0x1EF
#define WM_REDRAWBORDER   WM_APP + 13

#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#define DWMWA_BORDER_COLOR 34

#define DWMWCP_ROUNDSMALL 3

using namespace AcrylicMenus;

thread_local HWND g_hWndPrimary = NULL;

void CALLBACK MenuHandler::WinEventProc(
	HWINEVENTHOOK hWinEventHook,
	DWORD dwEvent,
	HWND hWnd,
	LONG idObject,
	LONG idChild,
	DWORD dwEventThread,
	DWORD dwmsEventTime
)
{
	if (!hWnd || !IsWindow(hWnd))
	{
		return;
	}

	switch (dwEvent)
	{
	case EVENT_OBJECT_CREATE:
		{
			if (SystemHelper::IsTransparencyEnabled() && ThemeHelper::IsPopupMenu(hWnd))
			{
				if (!g_hWndPrimary)
				{
					g_hWndPrimary = hWnd;
				}

				SetWindowSubclass(hWnd, SubclassProc, 0, 0);
				MenuManager::SetCurrentMenu(hWnd);
			}
			break;
		}
	case EVENT_OBJECT_SHOW:
		{
			if (g_hWndPrimary && SystemHelper::IsTransparencyEnabled() && ThemeHelper::IsPopupMenu(hWnd))
			{
				if (SystemHelper::g_bIsWindows11)
				{
					if (WIN11_SET_SMALL_CORNERS)
					{
						DWORD dwCornerPreference = DWMWCP_ROUNDSMALL;
						DwmSetWindowAttribute(hWnd, DWMWA_WINDOW_CORNER_PREFERENCE, &dwCornerPreference, sizeof(DWORD));
					}
					if (WIN11_SET_POPUP_BORDERS)
					{
						COLORREF dwColorBorder = MenuManager::g_bIsDarkMode ? WIN11_POPUP_BORDER_DARK : WIN11_POPUP_BORDER_LIGHT;
						DwmSetWindowAttribute(hWnd, DWMWA_BORDER_COLOR, &dwColorBorder, sizeof(COLORREF));
					}
				}
				else if (MENU_REDRAW_BORDER && (!MenuManager::g_bIsDarkMode || WIN10_MENU_REDRAW_BORDER_DARK))
				{
					// We can't draw on non-client area right after window has showed
					WindowHelper::SendMessageDelayed(hWnd, WM_REDRAWBORDER, 160);
				}
			}
			break;
		}
	case EVENT_OBJECT_DESTROY:
		{
			if (hWnd == g_hWndPrimary)
			{
				MenuManager::SetCurrentMenu(NULL);
				g_hWndPrimary = NULL;
			}
			break;
		}
	default:
		{
			break;
		}
	}
}

LRESULT CALLBACK MenuHandler::SubclassProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam,
	UINT_PTR uIdSubclass,
	DWORD_PTR dwRefData
)
{
	switch (uMsg)
	{
	case WM_REDRAWBORDER:
		{
			WindowHelper::RedrawMenuBorder(hWnd);
		}
		break;
	case MN_BUTTONUP:
		{
			// We need to prevent the system default menu fade out animation
			// and begin a re-implemented one
			// 
			// Windows does not show animation if the selection was done
			// with keyboard (i.e. Enter)

			int pvParam;
			SystemParametersInfoW(SPI_GETSELECTIONFADE, 0, &pvParam, 0);
			if (!pvParam)
			{
				// Fade out animation is disabled system-wide
				break;
			}

			HMENU hMenu = (HMENU)SendMessage(hWnd, MN_GETHMENU, 0, 0);
			int iPosition = (int)wParam;

			MENUITEMINFO mii;
			ZeroMemory(&mii, sizeof(MENUITEMINFO));
			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_SUBMENU | MIIM_STATE | MIIM_FTYPE;

			GetMenuItemInfo(hMenu, iPosition, TRUE, &mii);

			// Animation will be shown if a menu item is selected
			// 
			// We will ignore cases where user clicked on non-clickable item, i.e.:
			//  a) separator
			//  b) disabled item
			//  c) item that has submenu

			if (!(mii.fType & MFT_SEPARATOR) && !(mii.fState & MFS_DISABLED) && !mii.hSubMenu)
			{
				MENUBARINFO pmbi;
				ZeroMemory(&pmbi, sizeof(MENUBARINFO));   // memset(&info, 0, sizeof(MENUBARINFO));
				pmbi.cbSize = sizeof(MENUBARINFO);
				ZeroMemory(&pmbi.rcBar, 0, sizeof(RECT));

				GetMenuBarInfo(hWnd, OBJID_CLIENT, wParam + 1, &pmbi);

				WindowHelper::BeginMenuFadeOutAnimation(pmbi);

				SystemParametersInfoW(SPI_SETSELECTIONFADE, 0, FALSE, 0);
				LRESULT lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);
				SystemParametersInfoW(SPI_SETSELECTIONFADE, 0, (PVOID)TRUE, 0);

				return lResult;
			}
		}
		break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}