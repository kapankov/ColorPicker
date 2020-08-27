#pragma once

#include <windows.h>
#include <memory>
#include <vector>
#pragma warning(push)
#pragma warning(disable:5033)
#include "lcms2.h"
#pragma warning(pop)
#include "MagnifierWnd.h"

// структура описания монитора: dc, координаты, профиль (для преобразования в sRGB)
typedef struct _MONINFO
{
	HDC dcMonitor;
	RECT rcMonitor;
	std::unique_ptr<TCHAR[]> szProfile;
	cmsHPROFILE hInProfile;
	cmsHPROFILE hOutProfile;
	cmsHTRANSFORM hTransform;
} MONINFO, * LPMONINFO;

typedef std::vector<MONINFO> MONINFOLIST;

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

	MONINFOLIST m_lstMonitors;

	static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DisplayMonitorCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM  dwData);

	ATOM InternalRegisterClass(HINSTANCE hInstance);

	void DrawTitleButton(HDC dc, int ibtn, int state);
	void DrawTitleButton(HWND hwnd, int ibtn, int state);
public:
	CMainWnd(HINSTANCE hInstance);
	~CMainWnd();

	int Run(int nCmdShow);

};