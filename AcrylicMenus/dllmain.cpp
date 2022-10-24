#include "pch.h"
#include "amapi.h"
#include "SystemHelper.h"
#include "SettingsHelper.h"

using namespace AcrylicMenus;

HMODULE g_hModule = nullptr;
bool SystemHelper::g_bIsWindows11 = false;
bool SettingsHelper::g_redrawDarkThemeBorders10 = false;

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  dwReason,
    LPVOID lpReserved
)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			g_hModule = hModule;
			DisableThreadLibraryCalls(hModule);
			SystemHelper::g_bIsWindows11 = SystemHelper::DetectOSBuildNumber() >= 22000;
			SettingsHelper::g_redrawDarkThemeBorders10 = SettingsHelper::IsFeatureEnabled(L"RedrawDarkThemeBorders");
			AcrylicMenus::Startup();
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			AcrylicMenus::Shutdown();
			break;
		}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}