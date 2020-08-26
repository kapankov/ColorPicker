#pragma once

#include <windows.h>
#pragma warning(push)
#pragma warning(disable:5033)
#include "lcms2.h"
#pragma warning(pop)

class CScreenPixel
{
	HDC m_hdcScreen;
	HDC m_hdcMem;
	HBITMAP m_hBmp;
	HGDIOBJ m_hBmpOld;
	BITMAPINFO m_bmpi;
	BYTE m_bmpBits[4];

	// lcms
	cmsHPROFILE hRgbProfile;
	cmsHPROFILE hLabProfile;
	cmsHTRANSFORM hTransform;
public:
	CScreenPixel();
	~CScreenPixel();
	COLORREF GetPixel(const POINT& p);
	void Rgb2Lab(const COLORREF& cr, double* lab);
	static void GetHsvl(const COLORREF& cr, double* hslv);
	static void GetLuminance(const COLORREF& cr, double* l);
};