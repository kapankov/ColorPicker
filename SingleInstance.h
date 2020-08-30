#ifndef _SINGLEINSTANCE_H
#define _SINGLEINSTANCE_H

#include <windows.h>

#define UWM_ARE_YOU_ME (WM_APP + 100) 

HANDLE InitInstance(LPTSTR NameInstance, BOOL RunExistInst);

#endif