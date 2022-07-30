#pragma once
#include "pch.h"

#pragma warning(disable : 4996)

namespace AcrylicMenus
{
namespace SystemHelper
{
	extern bool g_bIsWindows11;


	typedef BOOL(WINAPI* pfnRtlGetVersion)(OSVERSIONINFOEXW*);
	static DWORD DetectOSBuildNumber()
	{
		HMODULE hMod = LoadLibrary(TEXT("ntdll.dll"));
		if (!hMod)
		{
			return 0;
		}

		pfnRtlGetVersion RtlGetVersion = (pfnRtlGetVersion)GetProcAddress(hMod, "RtlGetVersion");

		OSVERSIONINFOEXW info;
		ZeroMemory(&info, sizeof(OSVERSIONINFOEXW));
		RtlGetVersion(&info);

		DWORD dwBuildNumber = info.dwBuildNumber;
		FreeLibrary(hMod);
		return dwBuildNumber;
	}

	static bool IsTransparencyEnabled()
	{
		DWORD dwResult = 0;
		DWORD dwSize = sizeof(DWORD);
		RegGetValue(
			HKEY_CURRENT_USER,
			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
			L"EnableTransparency",
			RRF_RT_REG_DWORD,
			nullptr,
			&dwResult, &dwSize
		);
		return dwResult == 1;
	}
}
}