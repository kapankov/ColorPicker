#include "Hook.h"

namespace {
    HWND g_hWindow = nullptr;
    HHOOK g_mouseHook = nullptr;
    HHOOK g_keyboardHook = nullptr;
}

LRESULT LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    switch (wParam) {
    case WM_MOUSEMOVE:
        if (LPMSLLHOOKSTRUCT lpMM = reinterpret_cast<LPMSLLHOOKSTRUCT>(lParam))
            PostMessage(g_hWindow, WM_HOOKMOUSEPOS, lpMM->pt.x, lpMM->pt.y);
        //SendMessage(g_hWindow, WM_HOOKMOUSEPOS, lpMM->pt.x, lpMM->pt.y);
        break;
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if ((wParam == WM_KEYUP) || (wParam == WM_KEYDOWN)) 
        SendMessage(g_hWindow, WM_HOOKKEYEVENT, wParam, lParam);
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

BOOL SetHook(HWND hWnd)
{
    if (g_hWindow)
        UnHook();
    if (hWnd)
    {
        g_hWindow = hWnd;
        g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(nullptr), 0);
        g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(nullptr), 0);
    }
    return (g_hWindow != nullptr) && (g_mouseHook != nullptr) && (g_keyboardHook != nullptr);
}

BOOL UnHook()
{
    BOOL result = TRUE;
    result &= UnhookWindowsHookEx(g_mouseHook);
    result &= UnhookWindowsHookEx(g_keyboardHook);
    g_mouseHook = g_keyboardHook = nullptr;
    g_hWindow = nullptr;
    return  result;
}