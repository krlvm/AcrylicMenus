#include "pch.h"
#include "amapi.h"
#include "SystemHelper.h"
#include "MenuHandler.h"

#pragma data_seg("shared")
HWINEVENTHOOK g_hHook = NULL;
#pragma data_seg()
#pragma comment(linker,"/SECTION:shared,RWS")

using namespace AcrylicMenus;

extern HMODULE g_hModule;

extern "C"
{
	BOOL WINAPI RegisterHook(DWORD dwProcessId)
	{
		if (IsHookInstalled())
		{
			SetLastError(ERROR_ALREADY_EXISTS);
			return FALSE;
		}

		g_hHook = SetWinEventHook(
			EVENT_OBJECT_CREATE, EVENT_OBJECT_SHOW,
			g_hModule,
			MenuHandler::WinEventProc,
			dwProcessId, 0,
			WINEVENT_INCONTEXT
		);

		AcrylicMenus::Startup();

		return !!g_hHook;
	}

	BOOL WINAPI UnregisterHook()
	{
		if (!IsHookInstalled())
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
		}

		BOOL bResult = UnhookWinEvent(g_hHook);
		if (bResult)
		{
			g_hHook = NULL;
			AcrylicMenus::Shutdown();
		}

		return bResult;
	}

	BOOL WINAPI IsHookInstalled()
	{
		return g_hHook != NULL;
	}
}

void AcrylicMenus::Startup()
{
	Detours::Begin();
	//
	MenuHooks::Win32HookStartup();
	//
	Detours::Commit();
}

void AcrylicMenus::Shutdown()
{
	Detours::Begin();
	//
	MenuHooks::Win32HookShutdown();
	//
	Detours::Commit();
}