#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <VersionHelpers.h>

#include <Uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <dwmapi.h>

#include <intrin.h>
#include <stdio.h>
#include <tchar.h>
#include <list>
#include <vector>
#include <algorithm>
#include <initializer_list>
#include <unordered_map>
#include <map>
#include <memory>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdiplus.lib")

#endif //PCH_H
