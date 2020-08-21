/*!
\file ColorPicker.cpp
\brief This desktop application allows you to measure the color under the 
cursor and display its values in different color models, as well as 
perform some useful actions with these measurements.
\note I don't like the fact that Windows lacks a trivial and understandable way
to change the style of the window title and buttons, and working with the 
nonclient scope is too confusing and unreliable in terms of compatibility with 
the next versions of Windows, so I completely abandoned it and imitate the 
ehavior of the title and buttons through drawing in the client area.
And also, winAPI is used here (no MFC, ATL and other similar libraries and 
frameworks).
I used the boilerplate application code, courtesy of Visual Studio.
\authors Konstantin A. Pankov, explorus@mail.ru
\copyright MIT License
\version 1.0
\date 16/08/2020
\warning In developing. Not a stable tested code.

The MIT License

Copyright(c) 2018 Konstantin Pankov, explorus@mail.ru

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "framework.h"
#include "ColorPicker.h"
#include "ScreenPixel.h"

// Sizes
constexpr LONG wndWidth = 240;
constexpr LONG wndHeight = 600;
constexpr LONG btnWidth = 24;
constexpr LONG btnHeight = 24;
constexpr LONG brdWidth = 1;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND hWindow;                                   // main window
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
// Brushes
HBRUSH brClient;
HBRUSH brCaption;
// Caption buttons bitmap
HBITMAP hBmpClose;
HBITMAP hBmpMinimize;
// Caption rects
constexpr RECT rcDrag = { 0, brdWidth, wndWidth - btnWidth * 2 - brdWidth - 2, btnHeight };
constexpr RECT rcMinimize = { wndWidth - btnWidth * 2 - brdWidth - 1, brdWidth, wndWidth - btnWidth - 1, btnHeight };
constexpr RECT rcClose = { wndWidth - btnWidth - brdWidth, brdWidth, wndWidth - brdWidth, btnHeight };

CScreenPixel g_ScreenPixel;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Mouse hook
#define WM_HOOKMOUSEPOS (WM_USER + 0x0001)
HHOOK   mouseHook;
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

// Special version of TransparentBlt
bool XTransparentBlt(HDC hdcDest, int iDx, int iDy, int iDw, int iDh,
    HDC hdcSrc, int iSx, int iSy, int iSw, int iSh,
    UINT clrTransparent);

// Application entry point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_COLORPICKER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COLORPICKER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COLORPICKER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = 0;    // no menu
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = 0;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   RECT r = { 0 };
   GetClientRect(GetDesktopWindow(), &r);

   hWindow = CreateWindowW(szWindowClass, szTitle, 0,
      (r.right-r.left-wndWidth)/2, (r.bottom-r.top-wndHeight)/2, wndWidth, wndHeight,
       nullptr, nullptr, hInstance, nullptr);

   if (!hWindow)
   {
      return FALSE;
   }

   // dirty hack: drop all styles
   SetWindowLong(hWindow, GWL_STYLE, 0);

   ShowWindow(hWindow, nCmdShow);
   UpdateWindow(hWindow);

   return TRUE;
}

// DrawTitleButton. Draws buttons in the window title
// hdc: window dc. Returned via regular GetDC
// ibtn: 0 - Close, 1 - Minimize
// state: 0 - Normal, 1 - Hover, -1 - Pushed
void DrawTitleButton(HDC dc, int ibtn, int state)
{
    BITMAP bm;
    if (HDC hdcMem = CreateCompatibleDC(dc))
    {
        LPCRECT pRect = (ibtn == 0) ? &rcClose : &rcMinimize;

        if (state == 1) // hover
        {
            if (HBRUSH brush = CreateSolidBrush(RGB(63, 63, 65)))
            {
                FillRect(dc, pRect, brush);
                DeleteObject(brush);
            }
        }
        else if (state == -1) // pushed
        {
            if (HBRUSH brush = CreateSolidBrush(RGB(0, 122, 204)))
            {
                FillRect(dc, pRect, brush);
                DeleteObject(brush);
            }
        }
        else // normal
        {
            if (HBRUSH brush = CreateSolidBrush(RGB(45, 45, 48)))
            {
                FillRect(dc, pRect, brush);
                DeleteObject(brush);
            }
        }
        HGDIOBJ hbmOld = SelectObject(hdcMem, ibtn == 0 ? hBmpClose : hBmpMinimize);
        GetObject(hBmpClose, sizeof(bm), &bm);
        XTransparentBlt(dc, 
            pRect->left+4, pRect->top+4, bm.bmWidth, bm.bmHeight,
            hdcMem, 
            0, 0, bm.bmWidth, bm.bmHeight, 0);
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
    }
}

void DrawTitleButton(HWND hwnd, int ibtn, int state)
{
    if (HDC dc = GetDC(hwnd))
    {
        DrawTitleButton(dc, ibtn, state);
        ReleaseDC(hwnd, dc);
    }
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int iCloseState = 0; // 0-normal, 1-hover, -1-pushed
    static int iMinimizeState = 0;
    static HFONT hMainFont;
    LRESULT hit;
    static POINT ptMouseDownPos = {-1, -1};
    static TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, nullptr, 0 };
    switch (message)
    {
    case WM_CREATE:
        if (HDC dc = GetWindowDC(hWnd))
        {
            hMainFont = CreateFont(-MulDiv(9, GetDeviceCaps(dc, LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Segoe UI");
            ReleaseDC(hWnd, dc);
        }
        else hMainFont = nullptr;
        hBmpClose = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CLOSE));
        hBmpMinimize = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MINIMIZE));
        brClient = CreateSolidBrush(RGB(0x66, 0x66, 0x66));
        brCaption = CreateSolidBrush(RGB(45, 45, 48));
        // title button tooltips
        {
            HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
                WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                hWnd, NULL, hInst, NULL);

            SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            // Set up "tool" information.
            // Don't use sizeof(TOOLINFO), because there some problem with Common-Controls and manifest
            TOOLINFO ti = { TTTOOLINFO_V1_SIZE };
            ti.uFlags = TTF_SUBCLASS;
            ti.hwnd = hWnd;
            ti.hinst = hInst;
            ti.lpszText = const_cast<LPTSTR>(TEXT("Close"));
            ti.rect = rcClose;
            SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

            ti.lpszText = const_cast<LPTSTR>(TEXT("Minimize"));
            ti.rect = rcMinimize;
            SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);


        }
        mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(nullptr), 0);
        break;
    // window dragging
    case WM_NCHITTEST: 
        hit = DefWindowProc(hWnd, message, wParam, lParam);
        if (hit == HTCLIENT)
        {
            POINT ptPos{ LOWORD(lParam), HIWORD(lParam) };
            ScreenToClient(hWnd, &ptPos);
            if (PtInRect(&rcDrag, ptPos)) hit = HTCAPTION;
        }
        return hit;
    case WM_LBUTTONDOWN:
        ptMouseDownPos.x = LOWORD(lParam);
        ptMouseDownPos.y = HIWORD(lParam);
        if (PtInRect(&rcClose, ptMouseDownPos))
        {
            DrawTitleButton(hWnd, 0, iCloseState = -1);
            SetCapture(hWnd);
        }
        else if (PtInRect(&rcMinimize, ptMouseDownPos))
        {
            DrawTitleButton(hWnd, 1, iMinimizeState = -1);
            SetCapture(hWnd);
        }
        break;
    case WM_LBUTTONUP:
        ReleaseCapture();
        // если mouse down был на кнопке close
        if (PtInRect(&rcClose, ptMouseDownPos))
        {
            // если mouse up произошел на кнопке close
            if (PtInRect(&rcClose, POINT{ LOWORD(lParam), HIWORD(lParam) }))
                DestroyWindow(hWnd);
            else
            {
                // иначе, отпустим кнопку close
                DrawTitleButton(hWnd, 0, iCloseState = 0);
            }

        }
        // если mouse down был на кнопке minimize
        else if (PtInRect(&rcMinimize, ptMouseDownPos))
        {
            // если mouse up произошел на кнопке minimize
            if (PtInRect(&rcMinimize, POINT{ LOWORD(lParam), HIWORD(lParam) }))
                ShowWindow(hWnd, SW_MINIMIZE);
            else
            {
                // иначе, отпустим кнопку minimize
                DrawTitleButton(hWnd, 1, iMinimizeState = 0);
            }
        }
        ptMouseDownPos.x = -1;
        ptMouseDownPos.y = -1;
        break;
    case WM_MOUSEMOVE:
        if (tme.hwndTrack == nullptr)
        {
            tme.hwndTrack = hWnd;
            TrackMouseEvent(&tme);
        }
        if (iCloseState != -1 && iMinimizeState != -1)
        {
            if (PtInRect(&rcClose, POINT{ LOWORD(lParam), HIWORD(lParam) }))
            {
                if (iCloseState != 1)
                    DrawTitleButton(hWnd, 0, iCloseState = 1);
            }
            else
            {
                if (iCloseState != 0)
                    DrawTitleButton(hWnd, 0, iCloseState = 0);
            }
            if (PtInRect(&rcMinimize, POINT{ LOWORD(lParam), HIWORD(lParam) }))
            {
                if (iMinimizeState != 1)
                    DrawTitleButton(hWnd, 1, iMinimizeState = 1);
            }
            else
            {
                if (iMinimizeState != 0)
                    DrawTitleButton(hWnd, 1, iMinimizeState = 0);
            }
        }
        break;
    case WM_MOUSELEAVE:
        tme.hwndTrack = nullptr;
        //  reset title buttons state
        if (iCloseState != 0)
            DrawTitleButton(hWnd, 0, iCloseState = 0);
        if (iMinimizeState != 0)
            DrawTitleButton(hWnd, 1, iMinimizeState = 0);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
   case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // draw caption
            RECT rect;
            GetWindowRect(hWnd, &rect);
            OffsetRect(&rect, -rect.left, -rect.top);
            FillRect(hdc, &rect, brCaption);
            // draw close button
            DrawTitleButton(hdc, 0, iCloseState);
            // draw minimize button
            DrawTitleButton(hdc, 1, iMinimizeState);
            // draw caption text
            HGDIOBJ fntOld = SelectObject(hdc, hMainFont);
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            TEXTMETRIC tm;
            GetTextMetrics(hdc, &tm);
            TextOut(hdc, 4, (btnHeight - tm.tmHeight)/2+2, szTitle, static_cast<int>(wcslen(szTitle)));
            SelectObject(hdc, fntOld);

            // draw client
            GetClientRect(hWnd, &rect);
            InflateRect(&rect, -1, -1);
            rect.top += 24;
            FillRect(hdc, &rect, brClient);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        UnhookWindowsHookEx(mouseHook);
        DeleteObject(hBmpClose);
        DeleteObject(hBmpMinimize);
        DeleteObject(hMainFont);
        DeleteObject(brClient);
        DeleteObject(brCaption);
        PostQuitMessage(0);
        break;
    case WM_HOOKMOUSEPOS:
        if (HDC dc = GetDC(hWnd))
        {
            double lab[3] = { 0, 0, 0 };
            COLORREF rgb = g_ScreenPixel.GetPixel(POINT{ (LONG)wParam, (LONG)lParam });
            g_ScreenPixel.Rgb2Lab(rgb, lab);
            TCHAR szTxt[MINCHAR] = _TEXT("");
            swprintf(szTxt, MINCHAR, _TEXT("Pos: x=%li, y=%li\nRed: %li, Green: %li, Blue: %li\nL: %f, a: %f, b: %f"), 
                (LONG)wParam, (LONG)lParam, 
                GetRValue(rgb), GetGValue(rgb), GetBValue(rgb),
                lab[0], lab[1], lab[2]);
            // Get text height
            RECT rc{ 4, btnHeight + 4,  wndWidth - 8, btnHeight + 4 };
            DrawText(dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING | DT_CALCRECT);
            // Clear
            FillRect(dc, &rc, brClient);
            // Draw text
            HGDIOBJ fntOld = SelectObject(dc, hMainFont);
            SetTextColor(dc, RGB(255, 255, 255));
            SetBkMode(dc, TRANSPARENT);
            DrawText(dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING);
            SelectObject(dc, fntOld);
            ReleaseDC(hWnd, dc);
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    switch (wParam) {
    case WM_MOUSEMOVE:
        if (LPMSLLHOOKSTRUCT lpMM = reinterpret_cast<LPMSLLHOOKSTRUCT>(lParam))
        {
            SendMessage(hWindow, WM_HOOKMOUSEPOS, lpMM->pt.x, lpMM->pt.y);
/*            TCHAR szMsg[MINCHAR] = _TEXT("");
            wsprintf(szMsg, _TEXT("x=%i, y=%i\n"), lpMM->pt.x, lpMM->pt.y);
            OutputDebugString(szMsg);*/
        }
        break;
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

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