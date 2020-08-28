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
	HGDIOBJ hOldPen;
	HGDIOBJ hOldBrush;
} MONINFO, * LPMONINFO;

typedef std::vector<MONINFO> MONINFOLIST;

class CMagnifierWnd
{
	const int m_zoom = 4;
	RECT m_Rect;
	POINT m_ptLast;

	MONINFOLIST m_lstMonitors;

	HWND m_hwnd;
	HDC m_hdc;
	HPEN m_hSamplePen;
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK DisplayMonitorCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM  dwData);

	void OnCreate(HWND hWnd);
	void OnPaint();
public:
	CMagnifierWnd(HINSTANCE hInstance, HWND hwndParent, const RECT& rect);
	~CMagnifierWnd();

	void UpdateView(const POINT& pt);
};