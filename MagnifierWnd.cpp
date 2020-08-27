#include "MagnifierWnd.h"
#include <TCHAR.h>

constexpr TCHAR szWndClassName[] = TEXT("MagnifierWnd");

LRESULT CMagnifierWnd::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    CMagnifierWnd* pWnd = reinterpret_cast<CMagnifierWnd*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg)
    {
    case WM_CREATE:
        pWnd = reinterpret_cast<CMagnifierWnd*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pWnd);
        break;
    case WM_PAINT:
        if (HDC hdc = BeginPaint(hwnd, &ps))
        {
            pWnd->OnPaint(hdc);
            EndPaint(hwnd, &ps);
        }
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

CMagnifierWnd::CMagnifierWnd(HINSTANCE hInstance, HWND hwndParent, const RECT& rect)
	: m_hwnd{nullptr},
    m_pt{ 0, 0 },
    m_dc{nullptr},
    m_rc{0, 0, 0, 0}
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = 0;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = 0;    // no menu
    wcex.lpszClassName = szWndClassName;
    wcex.hIconSm = 0;

    if (RegisterClassExW(&wcex))
    {
        m_hwnd = CreateWindow(szWndClassName, TEXT(""), WS_CHILD | WS_VISIBLE,
            rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
            hwndParent, nullptr, hInstance, this);

    }
}

CMagnifierWnd::~CMagnifierWnd()
{
    DestroyWindow(m_hwnd);
}

void CMagnifierWnd::UpdateView(const POINT& pt, HDC hdcMon, const RECT& rc)
{
    m_pt = pt;
    m_dc = hdcMon;
    m_rc = rc;
    UpdateWindow(m_hwnd);
}

void CMagnifierWnd::OnPaint(HDC dc)
{
    if (HBRUSH brClient = CreateSolidBrush(RGB(0xff, 0x00, 0x00)))
    {
        // draw caption
        RECT rect;
        // draw client
        GetClientRect(m_hwnd, &rect);
        FillRect(dc, &rect, brClient);

        DeleteObject(brClient);
    }
}
