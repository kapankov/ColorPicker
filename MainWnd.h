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

struct CaptionButtonInfo
{
	HBITMAP m_bitmap;
	int m_state;
	RECT m_rect;
};

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
	// Caption buttons
	std::vector<CaptionButtonInfo> m_buttons;
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