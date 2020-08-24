#include "ScreenPixel.h"
#include <algorithm>
#include <cmath>

CScreenPixel::CScreenPixel()
{
	m_hdcScreen = ::GetDC(NULL);
	m_hdcMem = ::CreateCompatibleDC(NULL);
	m_hBmp = ::CreateCompatibleBitmap(m_hdcScreen, 1, 1);
	m_hBmpOld = ::SelectObject(m_hdcMem, m_hBmp);
	m_bmpi.bmiHeader.biSize = sizeof(BITMAPINFO);
	m_bmpi.bmiHeader.biWidth = 1;
	m_bmpi.bmiHeader.biHeight = 1;
	m_bmpi.bmiHeader.biPlanes = 1;
	m_bmpi.bmiHeader.biBitCount = 24;
	m_bmpi.bmiHeader.biCompression = BI_RGB;

	// lcms
	hRgbProfile = cmsCreate_sRGBProfile();
	hLabProfile = cmsCreateLab4Profile(NULL);
	hTransform = cmsCreateTransform(hRgbProfile, TYPE_RGB_8, hLabProfile, TYPE_Lab_DBL,
		INTENT_PERCEPTUAL /*INTENT_RELATIVE_COLORIMETRIC*/, 0);
}

CScreenPixel::~CScreenPixel()
{
	if (m_hBmpOld) ::SelectObject(m_hdcMem, m_hBmpOld);
	if (m_hBmp) ::DeleteObject(m_hBmp);
	if (m_hdcMem) ::DeleteDC(m_hdcMem);
	if (m_hdcScreen) ::ReleaseDC(NULL, m_hdcScreen);

	cmsDeleteTransform(hTransform);
	cmsCloseProfile(hLabProfile);
	cmsCloseProfile(hRgbProfile);
}

COLORREF CScreenPixel::GetPixel(const POINT& p)
{
	::BitBlt(m_hdcMem, 0, 0, 1, 1, m_hdcScreen, p.x, p.y, SRCCOPY);
	::GetDIBits(m_hdcMem, m_hBmp, 0, 1, m_bmpBits, &m_bmpi, DIB_RGB_COLORS);
	return  RGB(m_bmpBits[2], m_bmpBits[1], m_bmpBits[0]);
}

void CScreenPixel::Rgb2Lab(const COLORREF& cr, double* lab)
{
	if (hTransform)
		cmsDoTransform(hTransform, &cr, lab, 1);

}

void CScreenPixel::GetHsvl(const COLORREF& cr, double* hslv)
{
	double h = 0;
	double r = static_cast<double>(GetRValue(cr))/255.0;
	double g = static_cast<double>(GetGValue(cr))/255.0;
	double b = static_cast<double>(GetBValue(cr))/255.0;

	double max = max(max(r, g), b);
	double min = min(min(r, g), b);
	double d = max - min;
	if (d != 0)
	{
		if (r == max) h = (g - b) / d;
		else if (g == max) h = 2 + (b - r) / d;
		else h = 4 + (r - g) / d;
		h *= 60;
		if (h < 0) h += 360;
	}
	double s = max == 0 ? 0 : (d / max);
	double v = max;
	double l = (min + max) / 2;

	hslv[0] = h;
	hslv[1] = s;
	hslv[2] = v;
	hslv[3] = l;
}

void CScreenPixel::GetLuminance(const COLORREF& cr, double* l)
{
	double r = static_cast<double>(GetRValue(cr)) / 255.0;
	double g = static_cast<double>(GetGValue(cr)) / 255.0;
	double b = static_cast<double>(GetBValue(cr)) / 255.0;
	// BT.601 (NTSC 1953)
/*	constexpr double  Pr = .299;
	constexpr double  Pg = .587;
	constexpr double  Pb = .114;*/

	// should use these parameters
	// why? see https://habr.com/ru/post/304210/ (rus)
	// BT.709 (Rec.709)
	constexpr double  Pr = .2126;
	constexpr double  Pg = .7152;
	constexpr double  Pb = .0722;

	*l = sqrt(r * r * Pr + g * g * Pg + b * b * Pb);
}
