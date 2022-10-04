#include "pch.h"
#include "ThemeHelper.h"
#include "MenuHooks.h"
#include "MenuManager.h"
#include "SystemHelper.h"

using namespace AcrylicMenus;
using namespace AcrylicMenus::MenuHooks;
using namespace AcrylicMenus::MenuManager;
using namespace AcrylicMenus::ThemeHelper;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DetoursHook MenuHooks::DrawThemeBackgroundHook("Uxtheme", "DrawThemeBackground", MyDrawThemeBackground);

DetoursHook MenuHooks::DrawThemeTextExHook("Uxtheme", "DrawThemeTextEx", MyDrawThemeTextEx);
DetoursHook MenuHooks::DrawThemeTextHook("Uxtheme", "DrawThemeText", MyDrawThemeText);
DetoursHook MenuHooks::DrawTextWHook("User32", "DrawTextW", MyDrawTextW);
DetoursHook MenuHooks::DrawTextExWHook("User32", "DrawTextExW", MyDrawTextExW);

DetoursHook MenuHooks::SetMenuInfoHook("User32", "SetMenuInfo", MySetMenuInfo);
DetoursHook MenuHooks::SetMenuItemBitmapsHook("User32", "SetMenuItemBitmaps", MySetMenuItemBitmaps);
DetoursHook MenuHooks::InsertMenuItemWHook("User32", "InsertMenuItemW", MyInsertMenuItemW);
DetoursHook MenuHooks::SetMenuItemInfoWHook("User32", "SetMenuItemInfoW", MySetMenuItemInfoW);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Win32HookBatch(BOOL bBatchState)
{
	Detours::Batch(
		bBatchState,
		DrawThemeBackgroundHook,
		DrawThemeTextExHook,
		DrawThemeTextHook,
		DrawTextWHook,
		DrawTextExWHook,
		SetMenuInfoHook,
		SetMenuItemBitmapsHook,
		InsertMenuItemWHook,
		SetMenuItemInfoWHook
	);
}
void MenuHooks::Win32HookStartup()
{
	Win32HookBatch(TRUE);
}

void MenuHooks::Win32HookShutdown()
{
	Win32HookBatch(FALSE);
}

bool VerifyThemeBackgroundTransparency(HDC hdc, HTHEME hTheme, int iPartId, int iStateId)
{
	RECT rc = { 0, 0, 1, 1 };
	bool bResult = false;
	auto f = [&](HDC hMemDC, HPAINTBUFFER hPaintBuffer)
	{
		HRESULT hr = DrawThemeBackgroundHook.OldFunction<decltype(MyDrawThemeBackground)>(
			hTheme,
			hMemDC,
			iPartId,
			iStateId,
			&rc,
			nullptr
		);

		if (SUCCEEDED(hr))
		{
			auto verify = [&](int y, int x, RGBQUAD* pRGBAInfo)
			{
				if (pRGBAInfo->rgbReserved != 0xFF)
				{
					bResult = true;
					return false;
				}
				return true;
			};

			BufferedPaintWalkBits(hPaintBuffer, verify);
		}
	};

	MARGINS mr = {};
	if (SUCCEEDED(GetThemeMargins(hTheme, hdc, iPartId, iStateId, TMT_SIZINGMARGINS, nullptr, &mr)))
	{
		rc.right = max(mr.cxLeftWidth + mr.cxRightWidth, 1);
		rc.bottom = max(mr.cyTopHeight + mr.cyBottomHeight, 1);
	}
	DoBufferedPaint(hdc, &rc, f, 0xFF, BPPF_ERASE | BPPF_NOCLIP, FALSE, FALSE);

	return bResult;
};

inline bool VerifyMenuTheme(HTHEME hTheme)
{
	return VerifyThemeData(hTheme, L"Menu");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DrawRoundedRect(HDC& hdc, const RECT& rcTarget, const HBRUSH& hBrush)
{
	HRGN hRgn = CreateRoundRectRgn(rcTarget.left, rcTarget.top, rcTarget.right + 1, rcTarget.bottom + 1, 8, 8);
	FillRgn(hdc, hRgn, hBrush);
	DeleteObject(hRgn);
}

HRESULT WINAPI MenuHooks::MyDrawThemeBackground(
	HTHEME  hTheme,
	HDC     hdc,
	int     iPartId,
	int     iStateId,
	LPCRECT pRect,
	LPCRECT pClipRect
)
{
	HRESULT hr = S_OK;
	
	if (g_hWnd && VerifyMenuTheme(hTheme))
	{
		RECT rc = *pRect;
		if (pClipRect)
		{
			IntersectRect(&rc, pRect, pClipRect);
		}

		if (
			iPartId == MENU_POPUPBACKGROUND ||
			(iPartId == MENU_POPUPITEM || iPartId == MENU_POPUPITEM_FOCUSABLE) ||
			iPartId == MENU_POPUPGUTTER ||
			iPartId == MENU_POPUPBORDERS
		)
		{
			if (
				iPartId == MENU_POPUPBACKGROUND ||
				(iPartId == MENU_POPUPITEM || iPartId == MENU_POPUPITEM_FOCUSABLE) ||
				(
					!IsThemeBackgroundPartiallyTransparent(hTheme, iPartId, iStateId) ||
					!VerifyThemeBackgroundTransparency(hdc, hTheme, iPartId, iStateId)
				)
			)
			{
				auto f = [&](HDC hMemDC, HPAINTBUFFER hPaintBuffer)
				{
					Clear(hdc, &rc);

					HBRUSH hBrush = CreateSolidBrush(g_bIsDarkMode ? POPUP_HOVER_COLOR_DARK : POPUP_HOVER_COLOR_LIGHT);
					FillRect(hMemDC, &rc, hBrush);
					DeleteObject(hBrush);

					//GdiFlush();
					BufferedPaintSetAlpha(hPaintBuffer, &rc, 0xFF);
				};

				BYTE bOpacity = ((iPartId == MENU_POPUPITEM || iPartId == MENU_POPUPITEM_FOCUSABLE) && iStateId == MPI_HOT) ?
					(g_bIsDarkMode ? POPUP_HOVER_OPACITY_DARK : POPUP_HOVER_OPACITY_LIGHT) :
					0;

				if (DoBufferedPaint(hdc, &rc, f, bOpacity, BPPF_ERASE | (iPartId == MENU_POPUPBORDERS ? BPPF_NONCLIENT : 0UL)))
				{
					return hr;
				}
			}
			else
			{
				if (iStateId != MPI_NORMAL)
				{
					MyDrawThemeBackground(
						hTheme,
						hdc,
						iPartId,
						MPI_NORMAL,
						pRect,
						pClipRect
					);
				}
				else
				{
					MyDrawThemeBackground(
						hTheme,
						hdc,
						iPartId,
						0,
						pRect,
						pClipRect
					);
				}
			}
		}
		else if (iPartId == MENU_POPUPITEMKBFOCUS)
		{
			HBRUSH hBrush = (HBRUSH) GetStockObject(g_bIsDarkMode ? WHITE_BRUSH : BLACK_BRUSH);

			bool bSuccess = true;
			RECT rcLine;

			auto f = [&](HDC hMemDC, HPAINTBUFFER hPaintBuffer)
			{
				FillRect(hMemDC, &rcLine, hBrush);
				BufferedPaintSetAlpha(hPaintBuffer, &rcLine, 0xFF);
			};

			Clear(hdc, &rc);
			
			// Top line
			rcLine = { rc.left, rc.top, rc.right, rc.top + 1 };
			bSuccess = bSuccess && DoBufferedPaint(hdc, &rcLine, f, 255, BPPF_ERASE);
			
			// Bottom line
			rcLine = { rc.left, rc.bottom - 1, rc.right, rc.bottom };
			bSuccess = bSuccess && DoBufferedPaint(hdc, &rcLine, f, 255, BPPF_ERASE);
			
			// Left line
			rcLine = { rc.left, rc.top, rc.left + 1, rc.bottom };
			bSuccess = bSuccess && DoBufferedPaint(hdc, &rcLine, f, 255, BPPF_ERASE);
			
			// Right line
			rcLine = { rc.right - 1, rc.top, rc.right, rc.bottom };
			bSuccess = bSuccess && DoBufferedPaint(hdc, &rcLine, f, 255, BPPF_ERASE);

			if (bSuccess)
			{
				return hr;
			}
		}
		else if (iPartId == MENU_POPUPSEPARATOR)
		{
			RECT rcSep = { rc.left, rc.top + 2, rc.right, rc.top + 3 };

			auto f = [&](HDC hMemDC, HPAINTBUFFER hPaintBuffer)
			{
				Clear(hdc, &rcSep);

				HBRUSH hBrush = CreateSolidBrush(g_bIsDarkMode ? POPUP_SEPARATOR_COLOR_DARK : POPUP_SEPARATOR_COLOR_LIGHT);
				FillRect(hMemDC, &rcSep, hBrush);
				DeleteObject(hBrush);

				//GdiFlush();
				BufferedPaintSetAlpha(hPaintBuffer, &rcSep, 0xFF);
			};
			if (DoBufferedPaint(hdc, &rcSep, f, g_bIsDarkMode ? POPUP_SEPARATOR_OPACITY_DARK : POPUP_SEPARATOR_OPACITY_LIGHT, BPPF_ERASE))
			{
				return hr;
			}
		}
	}

	hr = DrawThemeBackgroundHook.OldFunction<decltype(MyDrawThemeBackground)>(
		hTheme,
		hdc,
		iPartId,
		iStateId,
		pRect,
		pClipRect
	);

	return hr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT WINAPI MenuHooks::MyDrawThemeTextEx(
	HTHEME        hTheme,
	HDC           hdc,
	int           iPartId,
	int           iStateId,
	LPCTSTR       pszText,
	int           cchText,
	DWORD         dwTextFlags,
	LPRECT        pRect,
	const DTTOPTS* pOptions
)
{
	HRESULT hr = S_OK;

	if (
		g_hWnd &&
		!(dwTextFlags & DT_CALCRECT) &&
		pOptions &&
		(!(pOptions->dwFlags & DTT_CALCRECT) || !(pOptions->dwFlags & DTT_COMPOSITED)) &&
		VerifyMenuTheme(hTheme)
	)
	{
		if (!g_bBkColorRefresh)
		{
			DWORD dwColor;
			GetThemeColor(hTheme, MENU_POPUPITEM, MPI_NORMAL, TMT_TEXTCOLOR, &dwColor);
			RefreshCurrentMenuBackground(GetRValue(dwColor) > 128, NULL);
			g_bBkColorRefresh = true;
		}

		DTTOPTS Options = *pOptions;
		Options.dwFlags |= DTT_COMPOSITED;

		auto f = [&](HDC hMemDC, HPAINTBUFFER hPaintBuffer)
		{
			g_alphaFixedState = true;

			hr = DrawThemeTextExHook.OldFunction<decltype(MyDrawThemeTextEx)>(
				hTheme,
				hMemDC,
				iPartId,
				iStateId,
				pszText,
				cchText,
				dwTextFlags,
				pRect,
				&Options
			);

			//GdiFlush();
			g_alphaFixedState = false;
		};

		if (DoBufferedPaint(hdc, pRect, f))
		{
			return hr;
		}
	}

	g_alphaFixedState = true;

	hr = DrawThemeTextExHook.OldFunction<decltype(MyDrawThemeTextEx)>(
		hTheme,
		hdc,
		iPartId,
		iStateId,
		pszText,
		cchText,
		dwTextFlags,
		pRect,
		pOptions
	);

	g_alphaFixedState = false;
	return hr;
}

HRESULT WINAPI MenuHooks::MyDrawThemeText(
	HTHEME  hTheme,
	HDC     hdc,
	int     iPartId,
	int     iStateId,
	LPCTSTR pszText,
	int     cchText,
	DWORD   dwTextFlags,
	DWORD   dwTextFlags2,
	LPCRECT pRect
)
{
	if (g_hWnd && pRect && VerifyMenuTheme(hTheme)) {
		DTTOPTS dttOpts = { sizeof(DTTOPTS) };
		RECT rc = *pRect;
		return DrawThemeTextEx(
			hTheme,
			hdc,
			iPartId,
			iStateId,
			pszText,
			cchText,
			dwTextFlags,
			&rc,
			&dttOpts
		);
	}

	return DrawThemeTextHook.OldFunction<decltype(MyDrawThemeText)>(
		hTheme,
		hdc,
		iPartId,
		iStateId,
		pszText,
		cchText,
		dwTextFlags,
		dwTextFlags2,
		pRect
	);
}

int WINAPI MenuHooks::MyDrawTextW(
	HDC     hdc,
	LPCTSTR lpchText,
	int     cchText,
	LPRECT  lprc,
	UINT    format
)
{
	if (g_hWnd && !g_alphaFixedState && !(format & DT_CALCRECT) && GetBkMode(hdc) == TRANSPARENT)
	{
		DTTOPTS dttOpts = { sizeof(DTTOPTS) };
		dttOpts.dwFlags = DTT_TEXTCOLOR;
		dttOpts.crText = GetTextColor(hdc);

		HTHEME hTheme = OpenThemeData(NULL, L"Menu");
		if (hTheme)
		{
			DrawThemeTextEx(hTheme, hdc, 0, 0, lpchText, cchText, format, lprc, &dttOpts);
			CloseThemeData(hTheme);
		}

		return 0;
	}
	return DrawTextWHook.OldFunction<decltype(MyDrawTextW)>(
		hdc,
		lpchText,
		cchText,
		lprc,
		format
	);
}

int WINAPI MenuHooks::MyDrawTextExW(
	HDC              hdc,
	LPWSTR           lpchText,
	int              cchText,
	LPRECT           lprc,
	UINT             format,
	LPDRAWTEXTPARAMS lpdtp
)
{
	int nResult;

	if (g_hWnd && !g_alphaFixedState && !lpdtp && !(format & DT_CALCRECT) && GetBkMode(hdc) == TRANSPARENT)
	{
		DTTOPTS Options = { sizeof(DTTOPTS) };
		Options.dwFlags = DTT_TEXTCOLOR;
		Options.crText = GetTextColor(hdc);
		
		HTHEME hTheme = OpenThemeData(NULL, L"Menu");
		if (hTheme)
		{
			DrawThemeTextEx(hTheme, hdc, 0, 0, lpchText, cchText, format, lprc, &Options);
			CloseThemeData(hTheme);
		}

		return 0;
	}

	return nResult = DrawTextExWHook.OldFunction<decltype(MyDrawTextExW)>(
		hdc,
		lpchText,
		cchText,
		lprc,
		format,
		lpdtp
	);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI MenuHooks::MySetMenuInfo(
	HMENU hMenu,
	LPCMENUINFO lpMenuInfo
)
{
	// Makes Immersive Menu top and bottom frame transparent

	if (SystemHelper::IsTransparencyEnabled())
	{
		if ((lpMenuInfo->fMask & MIM_BACKGROUND) && lpMenuInfo->hbrBack)
		{
			COLORREF dwColor = GetBrushColor(lpMenuInfo->hbrBack);
			RefreshCurrentMenuBackground(GetRValue(dwColor) < 128, dwColor);

			PBYTE pvBits = nullptr;
			MENUINFO MenuInfo = *lpMenuInfo;
			HBITMAP hBitmap = CreateDIB(nullptr, 1, 1, (PVOID*)&pvBits);

			if (hBitmap && pvBits)
			{
				SetPixel(pvBits, 0, 0, 0, 0);

				HBRUSH hBrush = CreatePatternBrush(hBitmap);

				DeleteObject(hBitmap);

				if (hBrush)
				{
					MenuInfo.hbrBack = hBrush;
					DeleteObject(lpMenuInfo->hbrBack);

					// DO NOT delete the new brush

					return SetMenuInfoHook.OldFunction<decltype(MySetMenuInfo)>(
						hMenu,
						&MenuInfo
					);
				}
			}
		}
	}

	return SetMenuInfoHook.OldFunction<decltype(MySetMenuInfo)>(
		hMenu,
		lpMenuInfo
	);
}

BOOL WINAPI MenuHooks::MySetMenuItemBitmaps(
	HMENU   hMenu,
	UINT    uPosition,
	UINT    uFlags,
	HBITMAP hBitmapUnchecked,
	HBITMAP hBitmapChecked
)
{
	if (SystemHelper::IsTransparencyEnabled())
	{
		PrepareAlpha(hBitmapUnchecked);
		PrepareAlpha(hBitmapChecked);
	}

	return SetMenuItemBitmapsHook.OldFunction<decltype(MySetMenuItemBitmaps)>(
		hMenu,
		uPosition,
		uFlags,
		hBitmapUnchecked,
		hBitmapChecked
	);
}

BOOL WINAPI MenuHooks::MySetMenuItemInfoW(
	HMENU            hMenu,
	UINT             item,
	BOOL             fByPositon,
	LPCMENUITEMINFOW lpmii
)
{
	if (lpmii && (lpmii->fMask & MIIM_CHECKMARKS || lpmii->fMask & MIIM_BITMAP) && SystemHelper::IsTransparencyEnabled())
	{
		PrepareAlpha(lpmii->hbmpItem);
		PrepareAlpha(lpmii->hbmpUnchecked);
		PrepareAlpha(lpmii->hbmpChecked);
	}

	return SetMenuItemInfoWHook.OldFunction<decltype(MySetMenuItemInfoW)>(
		hMenu,
		item,
		fByPositon,
		lpmii
		);
}

BOOL WINAPI MenuHooks::MyInsertMenuItemW(
	HMENU            hMenu,
	UINT             item,
	BOOL             fByPosition,
	LPCMENUITEMINFOW lpmii
)
{
	if (lpmii && (lpmii->fMask & MIIM_CHECKMARKS || lpmii->fMask & MIIM_BITMAP) && SystemHelper::IsTransparencyEnabled())
	{
		PrepareAlpha(lpmii->hbmpItem);
		PrepareAlpha(lpmii->hbmpUnchecked);
		PrepareAlpha(lpmii->hbmpChecked);
	}

	return InsertMenuItemWHook.OldFunction<decltype(MyInsertMenuItemW)>(
		hMenu,
		item,
		fByPosition,
		lpmii
	);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////