#pragma once
#include "pch.h"

namespace AcrylicMenus
{
namespace ThemeHelper
{
	// Windows 11 22H2+
	enum MENUPARTSEX
	{
		MENU_POPUPBACKGROUND_INTERNAL = 26,
		MENU_POPUPITEM_INTERNAL = 27
	};

	typedef HRESULT(WINAPI*pfnGetThemeClass)(HTHEME hTheme, LPCTSTR pszClassName, int cchClassName);
	static pfnGetThemeClass GetThemeClass
		= (pfnGetThemeClass)GetProcAddress(GetModuleHandle(L"Uxtheme"), MAKEINTRESOURCEA(74));

	typedef BOOL(WINAPI*pfnIsThemeClassDefined)(HTHEME hTheme, LPCTSTR pszAppName, LPCTSTR pszClassName, BOOL bMatchClass);
	static pfnIsThemeClassDefined IsThemeClassDefined
		= (pfnIsThemeClassDefined)GetProcAddress(GetModuleHandle(L"Uxtheme"), MAKEINTRESOURCEA(50));

	typedef BOOL(WINAPI*pfnIsTopLevelWindow)(HWND hWnd);
	static pfnIsTopLevelWindow IsTopLevelWindow
		= (pfnIsTopLevelWindow)GetProcAddress(GetModuleHandle(L"User32"), "IsTopLevelWindow");

	static void GetMenuFillColor(LPCWSTR pszClassList, COLORREF *pColor)
	{
		HTHEME hTheme = OpenThemeData(NULL, pszClassList);
		GetThemeColor(hTheme, MENU_POPUPBACKGROUND, 0, TMT_FILLCOLOR, pColor);
		CloseThemeData(hTheme);
	}

	static bool IsWindowUseDarkMode(HWND hwnd)
	{
		BOOL bResult { FALSE };
		DwmGetWindowAttribute(hwnd, 20, &bResult, sizeof(bResult));
		return bResult;
	}

	static inline bool IsAncestorUseDarkMode(GUITHREADINFO gti)
	{
		HWND hwndMenuRoot = GetAncestor(gti.hwndMenuOwner, GA_ROOT);
		return IsWindowUseDarkMode(hwndMenuRoot);
	}

	static bool IsMenuUseDarkMode(HWND hwnd)
	{
		GUITHREADINFO gti = { sizeof(GUITHREADINFO), };
		DWORD tid = GetWindowThreadProcessId(hwnd, NULL);

		if (tid && GetGUIThreadInfo(tid, &gti))
		{
			return IsWindowUseDarkMode(gti.hwndMenuOwner);   // || IsAncestorUseDarkMode(gti);
		}

		return false;
	}

	static inline bool VerifyThemeData(HTHEME hTheme, LPCTSTR pszThemeClassName)
	{
		TCHAR pszClassName[MAX_PATH + 1];
		GetThemeClass(hTheme, pszClassName, MAX_PATH);
		return !_wcsicmp(pszClassName, pszThemeClassName);
	}

	static inline bool VerifyWindowClass(HWND hWnd, LPCTSTR pszClassName, BOOL bRequireTopLevel = FALSE)
	{
		TCHAR pszClass[MAX_PATH + 1] = {};
		GetClassName(hWnd, pszClass, MAX_PATH);
		return (!_tcscmp(pszClass, pszClassName) && (bRequireTopLevel ? IsTopLevelWindow(hWnd) : TRUE));
	}

	static inline bool IsPopupMenu(HWND hWnd)
	{
		if (GetClassLong(hWnd, GCW_ATOM) == 32768)
		{
			return true;
		}

		TCHAR pszClass[MAX_PATH + 1];
		GetClassName(hWnd, pszClass, MAX_PATH);

		return IsTopLevelWindow(hWnd) && _tcscmp(pszClass, TEXT("#32768")) == 0;
	}

	static inline void Clear(HDC hdc, LPCRECT lpRect)
	{
		PatBlt(
		    hdc,
		    lpRect->left,
		    lpRect->top,
		    lpRect->right - lpRect->left,
		    lpRect->bottom - lpRect->top,
		    BLACKNESS
		);
	}

	static inline HBITMAP CreateDIB(HDC hdc, LONG nWidth, LONG nHeight, PVOID* pvBits)
	{
		BITMAPINFO bitmapInfo = {};
		bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
		bitmapInfo.bmiHeader.biBitCount = 32;
		bitmapInfo.bmiHeader.biCompression = BI_RGB;
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biWidth = nWidth;
		bitmapInfo.bmiHeader.biHeight = -nHeight;

		return CreateDIBSection(hdc, &bitmapInfo, DIB_RGB_COLORS, pvBits, nullptr, 0);
	}

	static inline COLORREF GetBrushColor(HBRUSH hBrush)
	{
		LOGBRUSH lbr = {};
		GetObject(hBrush, sizeof(lbr), &lbr);
		if (lbr.lbStyle != BS_SOLID)
		{
			return CLR_NONE;
		}
		return lbr.lbColor;
	}

	static inline void SetPixel(PBYTE pvBits, BYTE b, BYTE g, BYTE r, BYTE a)
	{
		pvBits[0] = (b * (a + 1)) >> 8;
		pvBits[1] = (g * (a + 1)) >> 8;
		pvBits[2] = (r * (a + 1)) >> 8;
		pvBits[3] = a;
	}

	static void PrepareAlpha(HBITMAP hBitmap)
	{
		if (!hBitmap)
		{
			return;
		}

		if (GetObjectType(hBitmap) != OBJ_BITMAP)
		{
			return;
		}

		HDC hdc = CreateCompatibleDC(nullptr);
		BITMAPINFO bmi = { sizeof(bmi.bmiHeader) };

		if (hdc)
		{
			if (GetDIBits(hdc, hBitmap, 0, 0, nullptr, &bmi, DIB_RGB_COLORS) and bmi.bmiHeader.biBitCount == 32)
			{
				bmi.bmiHeader.biCompression = BI_RGB;

				BYTE* pvBits = new(std::nothrow) BYTE[bmi.bmiHeader.biSizeImage];

				if (pvBits)
				{
					if (GetDIBits(hdc, hBitmap, 0, bmi.bmiHeader.biHeight, (LPVOID)pvBits, &bmi, DIB_RGB_COLORS))
					{
						bool bHasAlpha = false;

						for (UINT i = 0; i < bmi.bmiHeader.biSizeImage; i += 4)
						{
							if (pvBits[i + 3] != 0)
							{
								bHasAlpha = true;
								break;
							}
						}

						if (!bHasAlpha)
						{
							for (UINT i = 0; i < bmi.bmiHeader.biSizeImage; i += 4)
							{
								pvBits[i] = (pvBits[i] * 256) >> 8;
								pvBits[i + 1] = (pvBits[i + 1] * 256) >> 8;
								pvBits[i + 2] = (pvBits[i + 2] * 256) >> 8;
								pvBits[i + 3] = 255;
							}
						}

						SetDIBits(hdc, hBitmap, 0, bmi.bmiHeader.biHeight, pvBits, &bmi, DIB_RGB_COLORS);
					}
					delete[] pvBits;
				}
			}
			DeleteDC(hdc);
		}
	}

	template <typename T>
	static BOOL DoBufferedPaint(
	    HDC hdc,
	    LPCRECT Rect,
	    T&& t,
	    BYTE dwOpacity = 255,
	    DWORD dwFlag = BPPF_ERASE,
	    BOOL bUpdateTarget = TRUE
	)
	{
		HDC hMemDC = nullptr;
		BLENDFUNCTION pBlend = {AC_SRC_OVER, 0, dwOpacity, AC_SRC_ALPHA};
		BP_PAINTPARAMS ppPaint = {sizeof(BP_PAINTPARAMS), dwFlag, nullptr, &pBlend };
		HPAINTBUFFER hPaintBuffer = BeginBufferedPaint(hdc, Rect, BPBF_TOPDOWNDIB, &ppPaint, &hMemDC);
		if (hPaintBuffer and hMemDC)
		{
			SetLayout(hMemDC, GetLayout(hdc));
			SelectObject(hMemDC, GetCurrentObject(hdc, OBJ_FONT));
			SelectObject(hMemDC, GetCurrentObject(hdc, OBJ_BRUSH));
			SelectObject(hMemDC, GetCurrentObject(hdc, OBJ_PEN));
			SetTextAlign(hMemDC, GetTextAlign(hdc));

			t(hMemDC, hPaintBuffer);

			EndBufferedPaint(hPaintBuffer, TRUE);

			return TRUE;
		}
		
		return FALSE;
	}

	template <typename T>
	static BOOL BufferedPaintWalkBits(
	    HPAINTBUFFER hPaintBuffer,
	    T&& t
	)
	{
		int cxRow = 0;
		RGBQUAD* pbBuffer = nullptr;
		RECT targetRect = {};
		if (SUCCEEDED(GetBufferedPaintTargetRect(hPaintBuffer, &targetRect)))
		{
			int cx = targetRect.right - targetRect.left;
			int cy = targetRect.bottom - targetRect.top;
			if (SUCCEEDED(GetBufferedPaintBits(hPaintBuffer, &pbBuffer, &cxRow)))
			{
				for (int y = 0; y < cy; y++)
				{
					for (int x = 0; x < cx; x++)
					{
						RGBQUAD *pRGBAInfo = &pbBuffer[y * cxRow + x];
						if (!t(y, x, pRGBAInfo))
						{
							break;
						}
					}
				}
				return TRUE;
			}
		}
		return FALSE;
	}
}
}