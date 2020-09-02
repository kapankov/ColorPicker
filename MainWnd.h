#pragma once

#include <windows.h>
#include "ScreenPixel.h"
#include "MagnifierWnd.h"

#define MAX_LOADSTRING 100

class CMainWnd
{
	HINSTANCE m_hInst;
	HWND m_hWindow;
	TCHAR m_szTitle[MAX_LOADSTRING];
	TCHAR m_szWindowClass[MAX_LOADSTRING];
	HDC m_dc;
	// Brushes
	HBRUSH m_brClient;
	HBRUSH m_brCaption;
	// Caption buttons bitmap
	HBITMAP m_hBmpClose;
	HBITMAP m_hBmpMinimize;
	// Caption button states
	int m_iCloseState; // 0-normal, 1-hover, -1-pushed
	int m_iMinimizeState;
	// Tooltips
	HWND m_hwndTT;
	// Font
	HFONT m_hMainFont;
	HGDIOBJ m_fntOld;

	CScreenPixel m_ScreenPixel;

	std::unique_ptr<CMagnifierWnd> m_wndMagnifier;

	static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	ATOM InternalRegisterClass(HINSTANCE hInstance);

	void DrawTitleButton(HDC dc, int ibtn, int state);
	void DrawTitleButton(HWND hwnd, int ibtn, int state);

	void UpdateInfo();
	void OnCreate(HWND hWnd);
	void OnDestroy();
	void OnPaint();
public:
	CMainWnd(HINSTANCE hInstance);

	int Run(int nCmdShow);

};