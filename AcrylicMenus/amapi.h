#pragma once
#include <windows.h>

#define AMAPI __declspec(dllimport)

#ifdef __cplusplus
extern "C" {
#endif

AMAPI BOOL WINAPI RegisterHook(DWORD dwProcessId);
AMAPI BOOL WINAPI UnregisterHook();
AMAPI BOOL WINAPI IsHookInstalled();

#ifdef __cplusplus
}
#endif

namespace AcrylicMenus
{
	void Startup();
	void Shutdown();
}