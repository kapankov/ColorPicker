#include "MagnifierWnd.h"
#include <TCHAR.h>

constexpr TCHAR szWndClassName[] = TEXT("MagnifierWnd");

LRESULT CMagnifierWnd::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CMagnifierWnd* pWnd = reinterpret_cast<CMagnifierWnd*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg)
    {
    case WM_CREATE:
        pWnd = reinterpret_cast<CMagnifierWnd*>(reinterpret_cast<LPCREATESTRUCT>(lParam)->lpCreateParams);
        pWnd->m_hdc = GetDC(hwnd);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pWnd);
        break;
    case WM_PAINT:
        pWnd->OnPaint();
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

CMagnifierWnd::CMagnifierWnd(HINSTANCE hInstance, HWND hwndParent, const RECT& rect)
	: 
    m_Rect(rect),
    m_hwnd{nullptr},
    m_hdc(nullptr),
    m_ptLast{ 0, 0 }
{
    // fill monitors info
    EnumDisplayMonitors(NULL, NULL, DisplayMonitorCallback, (LPARAM)this);

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
    for (auto& mi : m_lstMonitors)
    {
        if (mi.szProfile.get()) mi.szProfile.reset();
        if (mi.dcMem)
        {
            if (mi.objOld)
                ::SelectObject(mi.dcMem, mi.objOld);
            ::DeleteObject(mi.hbmpScr);
            ::DeleteDC(mi.dcMem);
        }
        ::DeleteDC(mi.dcMonitor);
        if (mi.hTransform) cmsDeleteTransform(mi.hTransform);
        if (mi.hOutProfile) cmsCloseProfile(mi.hOutProfile);
        if (mi.hInProfile) cmsCloseProfile(mi.hInProfile);
    }
    m_lstMonitors.clear();
    if (m_hdc) ReleaseDC(m_hwnd, m_hdc);
    ::DestroyWindow(m_hwnd);
}

void CMagnifierWnd::UpdateView(const POINT& pt)
{
    m_ptLast = pt;
    RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CMagnifierWnd::OnPaint()
{
    for (auto& mi : m_lstMonitors)
    {
        if (PtInRect(&mi.rcMonitor, m_ptLast))
        {
            int w = m_Rect.right - m_Rect.left;
            int h = m_Rect.bottom - m_Rect.top;
            const int zoom = 4;

            // позиционировать курсор по центру
            int x = m_ptLast.x - mi.rcMonitor.left - w / (zoom<<1);
            int y = m_ptLast.y - mi.rcMonitor.top - h / (zoom<<1);
            if (x < 0) x = 0;
            if (y < 0) y = 0;
            if (x + w / zoom > (mi.rcMonitor.right - mi.rcMonitor.left)) x = (mi.rcMonitor.right - mi.rcMonitor.left) - w / zoom;
            if (y + h / zoom > (mi.rcMonitor.bottom - mi.rcMonitor.top)) y = (mi.rcMonitor.bottom - mi.rcMonitor.top) - h / zoom;
            ::StretchBlt(mi.dcMem, 0, 0, w, h, mi.dcMonitor, x, y, w/zoom, h/zoom, SRCCOPY);
            //TODO: draw sample

            // draw result
            if (m_hdc)
                BitBlt(m_hdc, 0, 0, w, h, mi.dcMem, 0, 0, SRCCOPY);
                    
            break;
        }
    }
}

BOOL CMagnifierWnd::DisplayMonitorCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    UNREFERENCED_PARAMETER(hdcMonitor);

    CMagnifierWnd* pWnd = reinterpret_cast<CMagnifierWnd*>(dwData);
    MONITORINFOEX miex = { sizeof(MONITORINFOEX) };
    if (GetMonitorInfo(hMonitor, &miex))
    {
        MONINFO mi;
        CopyRect(&mi.rcMonitor, lprcMonitor);
        mi.dcMonitor = CreateDC(NULL, miex.szDevice, NULL, NULL);
        if (mi.dcMonitor)
        {
            mi.dcMem = ::CreateCompatibleDC(mi.dcMonitor);
            if (mi.dcMem)
                ::SetStretchBltMode(mi.dcMem, COLORONCOLOR);
            mi.hbmpScr = ::CreateCompatibleBitmap(mi.dcMonitor, pWnd->m_Rect.right - pWnd->m_Rect.left, pWnd->m_Rect.bottom - pWnd->m_Rect.top);
            if (mi.hbmpScr)
                mi.objOld = ::SelectObject(mi.dcMem, mi.hbmpScr);
            else mi.objOld = nullptr;


            mi.szProfile = nullptr;
            DWORD dwSizeOfProfileName = 0;
            if (!::GetICMProfile(mi.dcMonitor, &dwSizeOfProfileName, mi.szProfile.get()) && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER && dwSizeOfProfileName)
            {
                mi.szProfile = std::make_unique<TCHAR[]>((dwSizeOfProfileName + 1) * sizeof(TCHAR));
                if (::GetICMProfile(mi.dcMonitor, &dwSizeOfProfileName, mi.szProfile.get()))
                {
                    std::unique_ptr<char[]> szProfile = std::make_unique<char[]>(dwSizeOfProfileName + 1);
#ifdef _UNICODE
                    WideCharToMultiByte(CP_ACP, 0, mi.szProfile.get(), -1, szProfile.get(), (dwSizeOfProfileName + 1), NULL, NULL);
#else
                    lstrcpyA(szProfile, mi.szProfile.get());
#endif
                    mi.hTransform = nullptr;
                    mi.hOutProfile = nullptr;
                    mi.hInProfile = cmsOpenProfileFromFile(szProfile.get(), "r");
                    if (mi.hInProfile)
                    {
                        mi.hOutProfile = cmsCreate_sRGBProfile();
                        if (mi.hOutProfile)
                            mi.hTransform = cmsCreateTransform(mi.hInProfile, TYPE_RGBA_8, mi.hOutProfile, TYPE_RGBA_8, INTENT_PERCEPTUAL, 0);
                    }
                }
            }
            pWnd->m_lstMonitors.emplace_back(std::move(mi));
        }
    }
    return TRUE;
}