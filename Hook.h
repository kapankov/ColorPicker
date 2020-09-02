#pragma once

#include <Windows.h>

#define WM_HOOKMOUSEPOS (WM_USER + 0x0101)
#define WM_HOOKKEYEVENT (WM_USER + 0x0102)

BOOL SetHook(HWND hWnd);
BOOL UnHook();