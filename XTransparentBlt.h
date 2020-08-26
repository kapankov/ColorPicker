#pragma once

#include <windows.h>

// Special version of TransparentBlt
bool XTransparentBlt(HDC hdcDest, int iDx, int iDy, int iDw, int iDh,
    HDC hdcSrc, int iSx, int iSy, int iSw, int iSh,
    UINT clrTransparent);