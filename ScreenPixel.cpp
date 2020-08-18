#include "ScreenPixel.h"

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
}

CScreenPixel::~CScreenPixel()
{
	if (m_hBmpOld) ::SelectObject(m_hdcMem, m_hBmpOld);
	if (m_hBmp) ::DeleteObject(m_hBmp);
	if (m_hdcMem) ::DeleteDC(m_hdcMem);
	if (m_hdcScreen) ::ReleaseDC(NULL, m_hdcScreen);
}

COLORREF CScreenPixel::GetPixel(const POINT& p)
{
	::BitBlt(m_hdcMem, 0, 0, 1, 1, m_hdcScreen, p.x, p.y, SRCCOPY);
	::GetDIBits(m_hdcMem, m_hBmp, 0, 1, m_bmpBits, &m_bmpi, DIB_RGB_COLORS);
	return  RGB(m_bmpBits[2], m_bmpBits[1], m_bmpBits[0]);
}
