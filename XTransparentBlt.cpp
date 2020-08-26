#include "XTransparentBlt.h"

bool XTransparentBlt(HDC hdcDest, int iDx, int iDy, int iDw, int iDh,
    HDC hdcSrc, int iSx, int iSy, int iSw, int iSh,
    UINT clrTransparent)
{
    RECT rect = { iSx,iSy,iSx + iSw,iSy + iSh };
    LPtoDP(hdcSrc, (POINT*)&rect, 2);
    int iMaskWidth = abs(rect.right - rect.left);
    int iMaskHeight = abs(rect.bottom - rect.top);
    HDC hdcMemDC = CreateCompatibleDC(hdcSrc);
    HBITMAP hMask = CreateBitmap(iMaskWidth, iMaskHeight, 1, 1, NULL);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMemDC, hMask);
    COLORREF oldBk = SetBkColor(hdcSrc, clrTransparent);
    StretchBlt(hdcMemDC, 0, 0, iMaskWidth, iMaskHeight,
        hdcSrc, iSx, iSy, iSw, iSh, SRCCOPY);
    SetBkColor(hdcSrc, oldBk);
    StretchBlt(hdcDest, iDx, iDy, iDw, iDh,
        hdcSrc, iSx, iSy, iSw, iSh, SRCINVERT);

    COLORREF oldFore = SetTextColor(hdcDest, RGB(0, 0, 0));
    COLORREF oldBack = SetBkColor(hdcDest, RGB(255, 255, 255));
    StretchBlt(hdcDest, iDx, iDy, iDw, iDh,
        hdcMemDC, 0, 0, iMaskWidth, iMaskHeight, SRCAND);
    SetTextColor(hdcDest, oldFore);
    SetBkColor(hdcDest, oldBack);
    StretchBlt(hdcDest, iDx, iDy, iDw, iDh,
        hdcSrc, iSx, iSy, iSw, iSh, SRCINVERT);
    SelectObject(hdcMemDC, hOld);
    DeleteObject(hMask);
    DeleteDC(hdcMemDC); // DeleteObject
    return true;
}