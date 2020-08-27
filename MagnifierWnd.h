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
	// for OnPaint optimization
	HDC dcMem;
	HBITMAP hbmpScr;
	HGDIOBJ objOld;
} MONINFO, * LPMONINFO;

typedef std::vector<MONINFO> MONINFOLIST;

class CMagnifierWnd
{
	RECT m_Rect;
	POINT m_ptLast;

	MONINFOLIST m_lstMonitors;

	HWND m_hwnd;
	HDC m_hdc;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DisplayMonitorCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM  dwData);

public:
	CMagnifierWnd(HINSTANCE hInstance, HWND hwndParent, const RECT& rect);
	~CMagnifierWnd();

	void UpdateView(const POINT& pt);

	void OnPaint();

};