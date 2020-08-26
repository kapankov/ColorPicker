#pragma once

#include <windows.h>
#include <memory>
#include <vector>
#pragma warning(push)
#pragma warning(disable:5033)
#include "lcms2.h"
#pragma warning(pop)

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

class CMagnifier
{
	MONINFOLIST m_lstMonitors;
	HWND m_hwnd;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	CMagnifier(HINSTANCE hInstance, HWND hwndParent, const RECT& rect);
	~CMagnifier();

};