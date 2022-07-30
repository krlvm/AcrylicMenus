#pragma once
#include "pch.h"
#include "MenuHooks.h"

namespace AcrylicMenus
{
	namespace MenuHandler
	{
		extern void CALLBACK WinEventProc(
			HWINEVENTHOOK hWinEventHook,
			DWORD dwEvent,
			HWND hWnd,
			LONG idObject,
			LONG idChild,
			DWORD dwEventThread,
			DWORD dwmsEventTime
		);
		LRESULT CALLBACK SubclassProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam,
			UINT_PTR uIdSubclass,
			DWORD_PTR dwRefData
		);
	}
};