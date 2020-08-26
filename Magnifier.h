#pragma once

#include <windows.h>

class CMagnifier
{
	HWND m_hwnd;
	static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	CMagnifier(HINSTANCE hInstance, HWND hwndParent, const RECT& rect);
	~CMagnifier();

};