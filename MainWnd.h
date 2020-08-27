#pragma once

#include <windows.h>
#include "MagnifierWnd.h"

#define MAX_LOADSTRING 100

class CMainWnd
{
	HINSTANCE m_hInst;
	HWND m_hWindow;
	TCHAR m_szTitle[MAX_LOADSTRING];
	TCHAR m_szWindowClass[MAX_LOADSTRING];
	// Brushes
	HBRUSH m_brClient;
	HBRUSH m_brCaption;
	// Caption buttons bitmap
	HBITMAP m_hBmpClose;
	HBITMAP m_hBmpMinimize;

	std::unique_ptr<CMagnifierWnd> m_wndMagnifier;

	static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

	ATOM InternalRegisterClass(HINSTANCE hInstance);

	void DrawTitleButton(HDC dc, int ibtn, int state);
	void DrawTitleButton(HWND hwnd, int ibtn, int state);
public:
	CMainWnd(HINSTANCE hInstance);

	int Run(int nCmdShow);

};