#pragma once

#include <windows.h>

class CScreenPixel
{
	HDC m_hdcScreen;
	HDC m_hdcMem;
	HBITMAP m_hBmp;
	HGDIOBJ m_hBmpOld;
	BITMAPINFO m_bmpi;
	BYTE m_bmpBits[4];
public:
	CScreenPixel();
	~CScreenPixel();
	COLORREF GetPixel(const POINT& p);
	void Rgb2Lab(const COLORREF& cr, double* lab);
};