#pragma once
#include <Windows.h>

class AcrylicHelper
{

private:
	struct ACCENTPOLICY
	{
		int nAccentState;
		int nFlags;
		int nColor;
		int nAnimationId;
	};

	struct WINCOMPATTRDATA
	{
		int nAttribute;
		PVOID pData;
		ULONG ulDataSize;
	};

	typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);
	static pSetWindowCompositionAttribute SetWindowCompositionAttribute;
	static HINSTANCE hModule;

public:

	static void ApplyAcrylic(HWND hwnd, int nOpacity, int nTintColor)
	{
		if (!hModule)
		{
			hModule = LoadLibrary(TEXT("user32.dll"));
			SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hModule, "SetWindowCompositionAttribute");
		}

		ACCENTPOLICY policy = { 4, 0x20 | 0x40 | 0x80 | 0x100, (nOpacity << 24) | (nTintColor & 0xFFFFFF), 0 };
		WINCOMPATTRDATA data = { 19, &policy, sizeof(ACCENTPOLICY) };
		SetWindowCompositionAttribute(hwnd, &data);
	}

};
HINSTANCE AcrylicHelper::hModule;
AcrylicHelper::pSetWindowCompositionAttribute AcrylicHelper::SetWindowCompositionAttribute;