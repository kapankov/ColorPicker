/*!
\file MainWindow.cpp
\brief this file contains the implementation of the main application window
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
#include "MainWnd.h"
#include "resource.h"
#include "version.h"
#include "framework.h"
#include "XTransparentBlt.h"
#include "SingleInstance.h"
#include "Hook.h"

// overriding char and unicode version of the function sprintf
#ifndef _UNICODE
#define lsprintf sprintf
#else
#define lsprintf swprintf
#endif

// constants
constexpr LONG wndWidth = 240;
constexpr LONG wndHeight = 400;
constexpr LONG btnWidth = 24;
constexpr LONG btnHeight = 24;
constexpr LONG brdWidth = 1;
constexpr LONG ctrlMargin = 8;
// Caption drag rect
constexpr RECT rcDrag = { 0, brdWidth, wndWidth - btnWidth * 2 - brdWidth - 2, btnHeight };

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

LRESULT CMainWnd::MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static POINT ptMouseDownPos = { -1, -1 };
    static TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, nullptr, 0 };
    static DWORD lastCtrlPressed = 0; // for keyboard hook

    CMainWnd* pMainWnd = reinterpret_cast<CMainWnd*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_CREATE:
        pMainWnd = reinterpret_cast<CMainWnd*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pMainWnd);
        if (pMainWnd)
            pMainWnd->OnCreate(hWnd);
        break;
    case WM_LBUTTONDOWN:
        ptMouseDownPos.x = LOWORD(lParam);
        ptMouseDownPos.y = HIWORD(lParam);
        if (PtInRect(&pMainWnd->m_buttons[0].m_rect, ptMouseDownPos))
            pMainWnd->DrawTitleButton(hWnd, 0, pMainWnd->m_buttons[0].m_state = -1);
        else if (PtInRect(&pMainWnd->m_buttons[1].m_rect, ptMouseDownPos))
            pMainWnd->DrawTitleButton(hWnd, 1, pMainWnd->m_buttons[1].m_state = -1);
        SetCapture(hWnd);
        break;
    case WM_LBUTTONUP:
        ReleaseCapture();
        // если mouse down был на кнопке close
        if (PtInRect(&pMainWnd->m_buttons[0].m_rect, ptMouseDownPos))
        {
            // если mouse up произошел на кнопке close
            if (PtInRect(&pMainWnd->m_buttons[0].m_rect, POINT{ LOWORD(lParam), HIWORD(lParam) }))
                DestroyWindow(hWnd);
            else
            {
                // иначе, отпустим кнопку close
                pMainWnd->DrawTitleButton(hWnd, 0, pMainWnd->m_buttons[0].m_state = 0);
            }
        }
        // если mouse down был на кнопке minimize
        else if (PtInRect(&pMainWnd->m_buttons[1].m_rect, ptMouseDownPos))
        {
            // если mouse up произошел на кнопке minimize
            if (PtInRect(&pMainWnd->m_buttons[1].m_rect, POINT{ LOWORD(lParam), HIWORD(lParam) }))
                ShowWindow(hWnd, SW_MINIMIZE);
            else
            {
                // иначе, отпустим кнопку minimize
                pMainWnd->DrawTitleButton(hWnd, 1, pMainWnd->m_buttons[1].m_state = 0);
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
        if (pMainWnd->m_buttons[0].m_state != -1 && pMainWnd->m_buttons[1].m_state != -1)
        {
            if (PtInRect(&pMainWnd->m_buttons[0].m_rect, POINT{ LOWORD(lParam), HIWORD(lParam) }))
            {
                if (pMainWnd->m_buttons[0].m_state != 1)
                    pMainWnd->DrawTitleButton(hWnd, 0, pMainWnd->m_buttons[0].m_state = 1);
            }
            else
            {
                if (pMainWnd->m_buttons[0].m_state != 0)
                    pMainWnd->DrawTitleButton(hWnd, 0, pMainWnd->m_buttons[0].m_state = 0);
            }
            if (PtInRect(&pMainWnd->m_buttons[1].m_rect, POINT{ LOWORD(lParam), HIWORD(lParam) }))
            {
                if (pMainWnd->m_buttons[1].m_state != 1)
                    pMainWnd->DrawTitleButton(hWnd, 1, pMainWnd->m_buttons[1].m_state = 1);
            }
            else
            {
                if (pMainWnd->m_buttons[1].m_state != 0)
                    pMainWnd->DrawTitleButton(hWnd, 1, pMainWnd->m_buttons[1].m_state = 0);
            }
        }
        if (GetCapture() == hWnd && PtInRect(&rcDrag, ptMouseDownPos))
        {
            // drag window
            POINT pt;
            GetCursorPos(&pt);
            MoveWindow(hWnd, pt.x - ptMouseDownPos.x, pt.y - ptMouseDownPos.y, wndWidth, wndHeight, FALSE);
        }
        break;
    case WM_MOUSELEAVE:
        tme.hwndTrack = nullptr;
        //  reset title buttons state
        if (pMainWnd->m_buttons[0].m_state != 0)
            pMainWnd->DrawTitleButton(hWnd, 0, pMainWnd->m_buttons[0].m_state = 0);
        if (pMainWnd->m_buttons[1].m_state != 0)
            pMainWnd->DrawTitleButton(hWnd, 1, pMainWnd->m_buttons[1].m_state = 0);
        break;
/*    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(pMainWnd->m_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
            break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }
    break;*/
    case WM_PAINT:
        if (pMainWnd)
            pMainWnd->OnPaint();
        break;
    case WM_DESTROY:
        if (pMainWnd)
            pMainWnd->OnDestroy();
        PostQuitMessage(0);
        break;
    // поддержка запуска одного экземпл€ра приложени€
    case UWM_ARE_YOU_ME: // отвечаем, что хотим активироватьс€ !!!
        return UWM_ARE_YOU_ME;
    case WM_HOOKMOUSEPOS:
        if (GetCapture() == NULL)
            pMainWnd->UpdateInfo();
        break;
    case WM_HOOKKEYEVENT:
        if (wParam == WM_KEYUP)
        {
            if ((((PKBDLLHOOKSTRUCT)lParam)->vkCode == VK_LCONTROL) || (((PKBDLLHOOKSTRUCT)lParam)->vkCode == VK_RCONTROL))
            {
                lastCtrlPressed = ((PKBDLLHOOKSTRUCT)lParam)->time;
            }
        }
        else if (wParam == WM_KEYDOWN)
        {
            if ((((PKBDLLHOOKSTRUCT)lParam)->vkCode == VK_LCONTROL) || (((PKBDLLHOOKSTRUCT)lParam)->vkCode == VK_RCONTROL))
            {
/*                if (lastCtrlPressed && ((((PKBDLLHOOKSTRUCT)lParam)->time - lastCtrlPressed) < 400))
                    OnSaveColor(0, wParam, lParam, f);*/
                lastCtrlPressed = 0;
            }

            POINT pt;
            GetCursorPos(&pt);
            switch (((PKBDLLHOOKSTRUCT)lParam)->vkCode)
            {
            case VK_LEFT:
                SetCursorPos(pt.x - 1, pt.y);
                pMainWnd->UpdateInfo();
                break;
            case VK_UP:
                SetCursorPos(pt.x, pt.y - 1);
                pMainWnd->UpdateInfo();
                break;
            case VK_RIGHT:
                SetCursorPos(pt.x + 1, pt.y);
                pMainWnd->UpdateInfo();
                break;
            case VK_DOWN:
                SetCursorPos(pt.x, pt.y + 1);
                pMainWnd->UpdateInfo();
                break;
            }
        }
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

ATOM CMainWnd::InternalRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MainWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COLORPICKER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = 0;    // no menu
    wcex.lpszClassName = m_szWindowClass;
    wcex.hIconSm = 0;

    return RegisterClassExW(&wcex);
}

void CMainWnd::DrawTitleButton(HDC dc, int ibtn, int state)
{
    BITMAP bm;
    if (HDC hdcMem = CreateCompatibleDC(dc))
    {
        LPCRECT pRect = &m_buttons[ibtn].m_rect;

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
        HGDIOBJ hbmOld = SelectObject(hdcMem, m_buttons[ibtn].m_bitmap);
        GetObject(m_buttons[ibtn].m_bitmap, sizeof(bm), &bm);
        XTransparentBlt(dc,
            pRect->left + 4, pRect->top + 4, bm.bmWidth, bm.bmHeight,
            hdcMem,
            0, 0, bm.bmWidth, bm.bmHeight, 0);
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
    }
}

void CMainWnd::DrawTitleButton(HWND hwnd, int ibtn, int state)
{
    if (HDC dc = GetDC(hwnd))
    {
        DrawTitleButton(dc, ibtn, state);
        ReleaseDC(hwnd, dc);
    }
}

void CMainWnd::UpdateInfo()
{
    POINT pt;
    ::GetCursorPos(&pt);
    double lab[3] = { };
    double hsvl[4] = { };
    double lum = 0;
    const COLORREF rgb = m_ScreenPixel.GetPixel(pt);
    m_ScreenPixel.Rgb2Lab(rgb, lab);
    m_ScreenPixel.GetHsvl(rgb, hsvl);
    m_ScreenPixel.GetLuminance(rgb, &lum);
    TCHAR szTxt[512] = _TEXT("");

    lsprintf(szTxt, sizeof(szTxt) / sizeof(TCHAR), TEXT("Pos: x=%li, y=%li"), pt.x, pt.y);

    LONG lTopOfText = wndWidth + btnHeight/* + ctrlMargin*/;
    RECT rc{ ctrlMargin, lTopOfText,  wndWidth - ctrlMargin * 2, wndHeight - ctrlMargin * 2 };
    // Clear
    FillRect(m_dc, &rc, m_brClient);
    // Get text height
    DrawText(m_dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING | DT_CALCRECT);
    // Draw Pos text
    DrawText(m_dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING);

    rc.top = rc.bottom++;
    lsprintf(szTxt, sizeof(szTxt) / sizeof(TCHAR), TEXT("R: %li\nG: %li\nB: %li"),
        GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    DrawText(m_dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING | DT_CALCRECT);
    // Draw RGB text
    DrawText(m_dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING);

    rc.left += (wndWidth - ctrlMargin * 2) / 3;
    lsprintf(szTxt, sizeof(szTxt) / sizeof(TCHAR), TEXT("L: %.2f\na: %.2f\nb: %.2f"),
        lab[0], lab[1], lab[2]);
    DrawText(m_dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING | DT_CALCRECT);
    // Draw Lab text
    DrawText(m_dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING);

    rc.left += (wndWidth - ctrlMargin * 2) / 3;
    lsprintf(szTxt, sizeof(szTxt) / sizeof(TCHAR), TEXT("H: %.0f\nS: %.2f\nB: %.2f"),
        hsvl[0], hsvl[1], hsvl[2]);
    DrawText(m_dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING | DT_CALCRECT);
    // Draw HSB text
    DrawText(m_dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING);

    rc.top = rc.bottom++;
    rc.left = ctrlMargin;
    lsprintf(szTxt, sizeof(szTxt) / sizeof(TCHAR), TEXT("Lightness: %.2f\nLuminance: %.2f\nWeb RGB: #%2X%2X%2X"), 
        hsvl[3], lum, GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
    DrawText(m_dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING | DT_CALCRECT);
    // Draw other text
    DrawText(m_dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING);


    m_wndMagnifier->UpdateView(pt);
}

void CMainWnd::OnCreate(HWND hWnd)
{
    m_dc = GetDC(hWnd);
    m_hMainFont = CreateFont(-MulDiv(9, GetDeviceCaps(m_dc, LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Segoe UI");
    m_fntOld = SelectObject(m_dc, m_hMainFont);
    SetTextColor(m_dc, RGB(255, 255, 255));
    SetBkMode(m_dc, TRANSPARENT);

    m_buttons[0].m_bitmap = LoadBitmap(m_hInst, MAKEINTRESOURCE(IDB_CLOSE));
    m_buttons[1].m_bitmap = LoadBitmap(m_hInst, MAKEINTRESOURCE(IDB_MINIMIZE));
    m_brClient = CreateSolidBrush(RGB(0x66, 0x66, 0x66));
    m_brCaption = CreateSolidBrush(RGB(45, 45, 48));
    // title button tooltips
    {
        m_hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
            WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            hWnd, NULL, m_hInst, NULL);

        SetWindowPos(m_hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        // Set up "tool" information.
        // Don't use sizeof(TOOLINFO), because there some problem with Common-Controls and manifest
        TOOLINFO ti = { TTTOOLINFO_V1_SIZE };
        ti.uFlags = TTF_SUBCLASS;
        ti.hwnd = hWnd;
        ti.hinst = m_hInst;
        ti.lpszText = const_cast<LPTSTR>(TEXT("Close"));
        ti.rect = m_buttons[0].m_rect;
        SendMessage(m_hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

        ti.lpszText = const_cast<LPTSTR>(TEXT("Minimize"));
        ti.rect = m_buttons[1].m_rect;
        SendMessage(m_hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
    }
    m_wndMagnifier = std::make_unique<CMagnifierWnd>(m_hInst, hWnd, RECT{ ctrlMargin, ctrlMargin + btnHeight, wndWidth - ctrlMargin, btnHeight + wndWidth - ctrlMargin });
    // Update color info
    UpdateInfo();
    // hook
    SetHook(hWnd);
}

void CMainWnd::OnDestroy()
{
    UnHook();
    m_wndMagnifier.reset();
    SelectObject(m_dc, m_fntOld);
    DeleteObject(m_buttons[1].m_bitmap);
    DeleteObject(m_buttons[0].m_bitmap);
    DeleteObject(m_hMainFont);
    DeleteObject(m_brClient);
    DeleteObject(m_brCaption);
    ReleaseDC(m_hWindow, m_dc);
    DestroyWindow(m_hwndTT);
}

void CMainWnd::OnPaint()
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hWindow, &ps);
    // draw caption
    RECT rect;
    GetWindowRect(m_hWindow, &rect);
    OffsetRect(&rect, -rect.left, -rect.top);
    FillRect(hdc, &rect, m_brCaption);
    // draw close button
    DrawTitleButton(hdc, 0, m_buttons[0].m_state);
    // draw minimize button
    DrawTitleButton(hdc, 1, m_buttons[1].m_state);
    // draw caption text
    HGDIOBJ fntOld = SelectObject(hdc, m_hMainFont);
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);
    TextOut(hdc, 4, (btnHeight - tm.tmHeight) / 2 + 2, m_szTitle, static_cast<int>(lstrlen(m_szTitle)));
    SelectObject(hdc, fntOld);

    // draw client
    GetClientRect(m_hWindow, &rect);
    InflateRect(&rect, -1, -1);
    rect.top += 24;
    FillRect(hdc, &rect, m_brClient);
    //TODO: put monitors info (amount, current monitor, current profile

    // Update color info
    UpdateInfo();

    EndPaint(m_hWindow, &ps);
}

CMainWnd::CMainWnd(HINSTANCE hInstance)
    : m_dc(nullptr),
    m_hMainFont(nullptr)
{
    // Initialize strings
    LoadString(hInstance, IDS_APP_TITLE, m_szTitle, MAX_LOADSTRING);
    lsprintf(&m_szTitle[lstrlen(m_szTitle)], sizeof(m_szTitle) / sizeof(TCHAR) - lstrlen(m_szTitle), _TEXT(" ver. %li.%li.%li.%li"), MAJOR_VER, MINOR_VER, RELEASE_VER, BUILD_VER);
    LoadString(hInstance, IDC_COLORPICKER, m_szWindowClass, MAX_LOADSTRING);

    m_buttons.reserve(3);
    for (int i=0;i<3;++i)
        m_buttons.emplace_back(CaptionButtonInfo{ nullptr, 0, {wndWidth - btnWidth * (i+1) - brdWidth - brdWidth, brdWidth, wndWidth - btnWidth * i - brdWidth, btnHeight} });

    InternalRegisterClass(hInstance);

    m_hInst = hInstance; // Store instance handle in our global variable

    RECT r = { 0 };
    GetClientRect(GetDesktopWindow(), &r);

    m_hWindow = CreateWindowW(m_szWindowClass, m_szTitle, 0,
        (r.right - r.left - wndWidth) / 2, (r.bottom - r.top - wndHeight) / 2, wndWidth, wndHeight,
        nullptr, nullptr, hInstance, this);
    
    if (!m_hWindow)
        throw 0;
}

int CMainWnd::Run(int nCmdShow)
{
    // dirty hack: drop all styles
    SetWindowLong(m_hWindow, GWL_STYLE, 0);

    ShowWindow(m_hWindow, nCmdShow);
    UpdateWindow(m_hWindow);

    HACCEL hAccelTable = LoadAccelerators(m_hInst, MAKEINTRESOURCE(IDC_COLORPICKER));

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

    return (int)msg.wParam;
}
