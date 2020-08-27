#include "MainWnd.h"
#include "resource.h"
#include "version.h"
#include "framework.h"
#include "XTransparentBlt.h"

constexpr LONG wndWidth = 240;
constexpr LONG wndHeight = 400;
constexpr LONG btnWidth = 24;
constexpr LONG btnHeight = 24;
constexpr LONG brdWidth = 1;
constexpr LONG ctrlMargin = 8;
// Caption rects
constexpr RECT rcDrag = { 0, brdWidth, wndWidth - btnWidth * 2 - brdWidth - 2, btnHeight };
constexpr RECT rcMinimize = { wndWidth - btnWidth * 2 - brdWidth - 1, brdWidth, wndWidth - btnWidth - 1, btnHeight };
constexpr RECT rcClose = { wndWidth - btnWidth - brdWidth, brdWidth, wndWidth - brdWidth, btnHeight };

// Mouse hook
#define WM_HOOKMOUSEPOS (WM_USER + 0x0001)
HWND g_hWindow = nullptr;

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

LRESULT CMainWnd::MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static int iCloseState = 0; // 0-normal, 1-hover, -1-pushed
    static int iMinimizeState = 0;
    
    LRESULT hit;
    static POINT ptMouseDownPos = { -1, -1 };
    static TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, nullptr, 0 };
    static HHOOK mouseHook;

    CMainWnd* pMainWnd = reinterpret_cast<CMainWnd*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_CREATE:
        pMainWnd = reinterpret_cast<CMainWnd*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pMainWnd);
        if (HDC dc = GetWindowDC(hwnd))
        {
            pMainWnd->m_hMainFont = CreateFont(-MulDiv(9, GetDeviceCaps(dc, LOGPIXELSY), 72), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Segoe UI");
            ReleaseDC(hwnd, dc);
        }
        pMainWnd->m_hBmpClose = LoadBitmap(pMainWnd->m_hInst, MAKEINTRESOURCE(IDB_CLOSE));
        pMainWnd->m_hBmpMinimize = LoadBitmap(pMainWnd->m_hInst, MAKEINTRESOURCE(IDB_MINIMIZE));
        pMainWnd->m_brClient = CreateSolidBrush(RGB(0x66, 0x66, 0x66));
        pMainWnd->m_brCaption = CreateSolidBrush(RGB(45, 45, 48));
        // title button tooltips
        {
            HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
                WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                hwnd, NULL, pMainWnd->m_hInst, NULL);

            SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            // Set up "tool" information.
            // Don't use sizeof(TOOLINFO), because there some problem with Common-Controls and manifest
            TOOLINFO ti = { TTTOOLINFO_V1_SIZE };
            ti.uFlags = TTF_SUBCLASS;
            ti.hwnd = hwnd;
            ti.hinst = pMainWnd->m_hInst;
            ti.lpszText = const_cast<LPTSTR>(TEXT("Close"));
            ti.rect = rcClose;
            SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);

            ti.lpszText = const_cast<LPTSTR>(TEXT("Minimize"));
            ti.rect = rcMinimize;
            SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
        }
        pMainWnd->m_wndMagnifier = std::make_unique<CMagnifierWnd>(pMainWnd->m_hInst, hwnd, RECT{ ctrlMargin, ctrlMargin + btnHeight, wndWidth - ctrlMargin, btnHeight + wndWidth - ctrlMargin });
        // Update color info
        pMainWnd->UpdateInfo(hwnd, POINT{ -1, -1 });
        // hook
        g_hWindow = hwnd;
        mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(nullptr), 0);
        break;
        // window dragging
    case WM_NCHITTEST:
        hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
        if (hit == HTCLIENT)
        {
            POINT ptPos{ LOWORD(lParam), HIWORD(lParam) };
            ScreenToClient(hwnd, &ptPos);
            if (PtInRect(&rcDrag, ptPos)) hit = HTCAPTION;
        }
        return hit;
    case WM_LBUTTONDOWN:
        ptMouseDownPos.x = LOWORD(lParam);
        ptMouseDownPos.y = HIWORD(lParam);
        if (PtInRect(&rcClose, ptMouseDownPos))
        {
            pMainWnd->DrawTitleButton(hwnd, 0, iCloseState = -1);
            SetCapture(hwnd);
        }
        else if (PtInRect(&rcMinimize, ptMouseDownPos))
        {
            pMainWnd->DrawTitleButton(hwnd, 1, iMinimizeState = -1);
            SetCapture(hwnd);
        }
        break;
    case WM_LBUTTONUP:
        ReleaseCapture();
        // если mouse down был на кнопке close
        if (PtInRect(&rcClose, ptMouseDownPos))
        {
            // если mouse up произошел на кнопке close
            if (PtInRect(&rcClose, POINT{ LOWORD(lParam), HIWORD(lParam) }))
                DestroyWindow(hwnd);
            else
            {
                // иначе, отпустим кнопку close
                pMainWnd->DrawTitleButton(hwnd, 0, iCloseState = 0);
            }

        }
        // если mouse down был на кнопке minimize
        else if (PtInRect(&rcMinimize, ptMouseDownPos))
        {
            // если mouse up произошел на кнопке minimize
            if (PtInRect(&rcMinimize, POINT{ LOWORD(lParam), HIWORD(lParam) }))
                ShowWindow(hwnd, SW_MINIMIZE);
            else
            {
                // иначе, отпустим кнопку minimize
                pMainWnd->DrawTitleButton(hwnd, 1, iMinimizeState = 0);
            }
        }
        ptMouseDownPos.x = -1;
        ptMouseDownPos.y = -1;
        break;
    case WM_MOUSEMOVE:
        if (tme.hwndTrack == nullptr)
        {
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
        }
        if (iCloseState != -1 && iMinimizeState != -1)
        {
            if (PtInRect(&rcClose, POINT{ LOWORD(lParam), HIWORD(lParam) }))
            {
                if (iCloseState != 1)
                    pMainWnd->DrawTitleButton(hwnd, 0, iCloseState = 1);
            }
            else
            {
                if (iCloseState != 0)
                    pMainWnd->DrawTitleButton(hwnd, 0, iCloseState = 0);
            }
            if (PtInRect(&rcMinimize, POINT{ LOWORD(lParam), HIWORD(lParam) }))
            {
                if (iMinimizeState != 1)
                    pMainWnd->DrawTitleButton(hwnd, 1, iMinimizeState = 1);
            }
            else
            {
                if (iMinimizeState != 0)
                    pMainWnd->DrawTitleButton(hwnd, 1, iMinimizeState = 0);
            }
        }
        break;
    case WM_MOUSELEAVE:
        tme.hwndTrack = nullptr;
        //  reset title buttons state
        if (iCloseState != 0)
            pMainWnd->DrawTitleButton(hwnd, 0, iCloseState = 0);
        if (iMinimizeState != 0)
            pMainWnd->DrawTitleButton(hwnd, 1, iMinimizeState = 0);
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
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    break;*/
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // draw caption
        RECT rect;
        GetWindowRect(hwnd, &rect);
        OffsetRect(&rect, -rect.left, -rect.top);
        FillRect(hdc, &rect, pMainWnd->m_brCaption);
        // draw close button
        pMainWnd->DrawTitleButton(hdc, 0, iCloseState);
        // draw minimize button
        pMainWnd->DrawTitleButton(hdc, 1, iMinimizeState);
        // draw caption text
        HGDIOBJ fntOld = SelectObject(hdc, pMainWnd->m_hMainFont);
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        TEXTMETRIC tm;
        GetTextMetrics(hdc, &tm);
        TextOut(hdc, 4, (btnHeight - tm.tmHeight) / 2 + 2, pMainWnd->m_szTitle, static_cast<int>(lstrlen(pMainWnd->m_szTitle)));
        SelectObject(hdc, fntOld);

        // draw client
        GetClientRect(hwnd, &rect);
        InflateRect(&rect, -1, -1);
        rect.top += 24;
        FillRect(hdc, &rect, pMainWnd->m_brClient);
        //TODO: put monitors info (amount, current monitor, current profile

        // Update color info
        pMainWnd->UpdateInfo(hwnd, POINT{ -1, -1 });

        EndPaint(hwnd, &ps);
    }
    break;
    case WM_DESTROY:
        UnhookWindowsHookEx(mouseHook);
        pMainWnd->m_wndMagnifier.reset();
        DeleteObject(pMainWnd->m_hBmpClose);
        DeleteObject(pMainWnd->m_hBmpMinimize);
        DeleteObject(pMainWnd->m_hMainFont);
        DeleteObject(pMainWnd->m_brClient);
        DeleteObject(pMainWnd->m_brCaption);
        PostQuitMessage(0);
        break;
    case WM_HOOKMOUSEPOS:
        pMainWnd->UpdateInfo(hwnd, POINT{ (LONG)wParam, (LONG)lParam });
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CMainWnd::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    switch (wParam) {
    case WM_MOUSEMOVE:
        if (LPMSLLHOOKSTRUCT lpMM = reinterpret_cast<LPMSLLHOOKSTRUCT>(lParam))
            SendMessage(g_hWindow, WM_HOOKMOUSEPOS, lpMM->pt.x, lpMM->pt.y);
        break;
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
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
        HGDIOBJ hbmOld = SelectObject(hdcMem, ibtn == 0 ? m_hBmpClose : m_hBmpMinimize);
        GetObject(m_hBmpClose, sizeof(bm), &bm);
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

void CMainWnd::UpdateInfo(HWND hWnd, POINT&& pt)
{
    constexpr TCHAR clrFormat[] = _TEXT("Pos: x=%li, y=%li\nRed: %li, Green: %li, Blue: %li\nL: %.2f, a: %.2f, b: %.2f\n"
        "Hue: %.0f,\n"
        "Saturation: %.2f\n"
        "Brightness: %.2f\n"
        "Lightness: %.2f\n"
        "Luminance: %.2f");
    if (pt.x == -1 || pt.y == -1)
        ::GetCursorPos(&pt);
    if (HDC dc = GetDC(hWnd))
    {
        double lab[3] = { };
        double hsvl[4] = { };
        double lum = 0;
        const COLORREF rgb = m_ScreenPixel.GetPixel(pt);
        m_ScreenPixel.Rgb2Lab(rgb, lab);
        m_ScreenPixel.GetHsvl(rgb, hsvl);
        m_ScreenPixel.GetLuminance(rgb, &lum);
        TCHAR szTxt[512] = _TEXT("");
        swprintf(szTxt, sizeof(szTxt) / sizeof(TCHAR), clrFormat,
            pt.x, pt.y,
            GetRValue(rgb), GetGValue(rgb), GetBValue(rgb),
            lab[0], lab[1], lab[2],
            hsvl[0], hsvl[1], hsvl[2], hsvl[3], lum);
        // Get text height
        LONG lTopOfText = wndWidth + btnHeight/* + ctrlMargin*/;
        RECT rc{ ctrlMargin, lTopOfText,  wndWidth - ctrlMargin * 2, lTopOfText + 1 };
        DrawText(dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING | DT_CALCRECT);
        // Clear
        FillRect(dc, &rc, m_brClient);
        // Draw text
        HGDIOBJ fntOld = SelectObject(dc, m_hMainFont);
        SetTextColor(dc, RGB(255, 255, 255));
        SetBkMode(dc, TRANSPARENT);
        DrawText(dc, szTxt, lstrlen(szTxt), &rc, DT_LEFT | DT_EXTERNALLEADING);
        SelectObject(dc, fntOld);
        ReleaseDC(hWnd, dc);

        m_wndMagnifier->UpdateView(pt);
    }
}

CMainWnd::CMainWnd(HINSTANCE hInstance)
    : m_hMainFont(nullptr)
{
    // Initialize strings
    LoadString(hInstance, IDS_APP_TITLE, m_szTitle, MAX_LOADSTRING);
#ifndef _UNICODE
    sprintf
#else
    swprintf
#endif
        (&m_szTitle[lstrlen(m_szTitle)], sizeof(m_szTitle) / sizeof(TCHAR) - lstrlen(m_szTitle), _TEXT(" ver. %li.%li.%li.%li"), MAJOR_VER, MINOR_VER, RELEASE_VER, BUILD_VER);
    LoadString(hInstance, IDC_COLORPICKER, m_szWindowClass, MAX_LOADSTRING);

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
