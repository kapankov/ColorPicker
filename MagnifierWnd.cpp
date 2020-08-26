#include "MagnifierWnd.h"
#include <TCHAR.h>

constexpr TCHAR szWndClassName[] = TEXT("MagnifierWnd");

BOOL CALLBACK DisplayMonitorCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM  dwData);

LRESULT CMagnifier::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
        if (HBRUSH brClient = CreateSolidBrush(RGB(0xff, 0x00, 0x00)))
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // draw caption
            RECT rect;
            // draw client
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, brClient);

            EndPaint(hwnd, &ps);
            DeleteObject(brClient);
        }
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

CMagnifier::CMagnifier(HINSTANCE hInstance, HWND hwndParent, const RECT& rect)
	: m_hwnd(nullptr)
{
    // fill monitors info
    EnumDisplayMonitors(NULL, NULL, DisplayMonitorCallback, (LPARAM)&m_lstMonitors);

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
            hwndParent, nullptr, hInstance, nullptr);

    }
}

CMagnifier::~CMagnifier()
{
    for(auto& mi: m_lstMonitors)
    {
        if (mi.szProfile.get()) mi.szProfile.reset();
        DeleteDC(mi.dcMonitor);
        if (mi.hTransform) cmsDeleteTransform(mi.hTransform);
        if (mi.hOutProfile) cmsCloseProfile(mi.hOutProfile);
        if (mi.hInProfile) cmsCloseProfile(mi.hInProfile);
    }
    m_lstMonitors.clear();
    DestroyWindow(m_hwnd);
}

BOOL CALLBACK DisplayMonitorCallback(HMONITOR hMonitor, HDC /*hdcMonitor*/, LPRECT lprcMonitor, LPARAM  dwData)
{
    MONITORINFOEX miex = { sizeof(MONITORINFOEX) };
    if (GetMonitorInfo(hMonitor, &miex))
    {
        MONINFO mi;
        CopyRect(&mi.rcMonitor, lprcMonitor);
        mi.dcMonitor = CreateDC(NULL, miex.szDevice, NULL, NULL);
        if (mi.dcMonitor)
        {
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
            reinterpret_cast<MONINFOLIST*>(dwData)->emplace_back(std::move(mi));
        }
    }
    return TRUE;
}