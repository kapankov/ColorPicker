#pragma once

#include <windows.h>

class CMagnifierWnd
{
	POINT m_pt;
	HDC m_dc;
	RECT m_rc;

	HWND m_hwnd;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	CMagnifierWnd(HINSTANCE hInstance, HWND hwndParent, const RECT& rect);
	~CMagnifierWnd();

	void UpdateView(const POINT& pt, HDC hdcMon, const RECT& rc);

	void OnPaint(HDC dc);

};