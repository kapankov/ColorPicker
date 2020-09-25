/*!
\file MainWindow.h
\brief this file describes the declaration of the main application window
\authors Konstantin A. Pankov, explorus@mail.ru
\copyright MIT License
\version 1.0
\date 16/08/2020
\warning In developing. Not a stable tested code.

The MIT License

Copyright(c) 2018 Konstantin Pankov, explorus@mail.ru

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#pragma once

#include <windows.h>
#include <vector>
#include "ScreenPixel.h"
#include "MagnifierWnd.h"

#define MAX_LOADSTRING 100

struct TitleBarButtonInfo
{
	HBITMAP m_bitmap;
	int m_state; // 0 - normal, -1 - pushed, 1 - hover, -2 - fixed
	RECT m_rect;
};

class CMainWnd
{
	HINSTANCE m_hInst{ nullptr };
	HWND m_hWindow{ nullptr };
	TCHAR m_szTitle[MAX_LOADSTRING];
	TCHAR m_szWindowClass[MAX_LOADSTRING];
	HDC m_dc{ nullptr };
	// Brushes
	HBRUSH m_brClient{ nullptr };
	HBRUSH m_brTitleBar{ nullptr };
	// Title bar buttons
	std::vector<TitleBarButtonInfo> m_buttons;
	// Tooltips
	HWND m_hwndTT{ nullptr };
	TRACKMOUSEEVENT m_tme{ sizeof(TRACKMOUSEEVENT), TME_LEAVE, nullptr, 0 };
	POINT m_ptMouseDownPos{ -1, -1 };
	// Font
	HFONT m_hMainFont{ nullptr };
	HGDIOBJ m_fntOld{ nullptr };

	CScreenPixel m_ScreenPixel;

	std::unique_ptr<CMagnifierWnd> m_wndMagnifier;

	static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	ATOM InternalRegisterClass(HINSTANCE hInstance);

	void DrawTitleButton(HDC dc, size_t ibtn);
	void DrawTitleButton(HWND hwnd, size_t ibtn);

	void UpdateInfo();
	void OnCreate(HWND hWnd);
	void OnDestroy();
	void OnPaint();
	void OnMouseMove(WORD x, WORD y);
	void OnMouseLeave();
	void OnLButtonDown(WORD x, WORD y);
	void OnLButtonUp(WORD x, WORD y);
	void OnTitleButtonClick(int i);
public:
	CMainWnd(HINSTANCE hInstance);

	int Run(int nCmdShow);

};