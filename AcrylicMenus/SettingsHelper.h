#pragma once
#include "pch.h"

#pragma warning(disable : 4996)

namespace AcrylicMenus
{
namespace SettingsHelper
{
    extern bool g_redrawDarkThemeBorders10;

	static bool IsFeatureEnabled(LPCWSTR lpRegistryValue)
	{
        HKEY hKey;
        RegOpenKeyEx(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\AcrylicMenus", 0, KEY_READ, &hKey
        );

        if (!hKey) return FALSE;

        DWORD dwResult = 0;
        DWORD dwSize = sizeof(DWORD);
        LSTATUS nError = RegQueryValueExW(
            hKey,
            lpRegistryValue,
            0,
            NULL,
            (LPBYTE)&dwResult,
            &dwSize
        );

        RegCloseKey(hKey);

        return ERROR_SUCCESS == nError ? dwResult : false;
	}
}
}