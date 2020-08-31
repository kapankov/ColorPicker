/*!
\file ColorPicker.cpp
\brief This desktop application allows you to measure the color under the 
cursor and display its values in different color models, as well as 
perform some useful actions with these measurements.
\note I don't like the fact that Windows lacks a trivial and understandable way
to change the style of the window title and buttons, and working with the 
nonclient scope is too confusing and unreliable in terms of compatibility with 
the next versions of Windows, so I completely abandoned it and imitate the 
behavior of the title and buttons through drawing in the client area.
And also, winAPI is used here (no MFC, ATL and other similar libraries and 
frameworks).
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
#include "SingleInstance.h"
#include "MainWnd.h"

// Application entry point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    TCHAR szMutexName[MAX_LOADSTRING];
    LoadString(hInstance, IDC_COLORPICKER, szMutexName, MAX_LOADSTRING);

    // Single instance support
    HANDLE hMutex = InitInstance(szMutexName, TRUE);
    if (!hMutex) return 0; // there is already a running instance

    // Create and run main window
    std::unique_ptr<CMainWnd> wndMain = std::make_unique<CMainWnd>(hInstance);
    int res = wndMain->Run(nCmdShow);
    // Close single instance mutex
    CloseHandle(hMutex);
    return res;
}
