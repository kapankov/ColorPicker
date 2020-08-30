#include "SingleInstance.h"
#include <tchar.h>
#include <lmcons.h> // for UNLEN

#define UNIQUE_TO_SYSTEM  0
#define UNIQUE_TO_DESKTOP 1
#define UNIQUE_TO_SESSION 2
#define UNIQUE_TO_TRUSTEE 4

/*******************************************************************************
 * ������� : CreateExclusionName                                               *
 * ���������� : �������� ����������� ����� ��� ��������                        *
 * ��������� : kind - ��� ������������:                                        *
 *               UNIQUE_TO_SYSTEM                                              *
 *               UNIQUE_TO_DESKTOP                                             *
 *               UNIQUE_TO_SESSION                                             *
 *               UNIQUE_TO_TRUSTEE                                             *
 *             PostExclusionName - ������� ����� (����������������)            *
 * ��������� : ���������� ���������� ��� ��� ��������                          *
 *             GUID ��� UNIQUE ��������� ���������� ���� ��� ������ GUIDGEN    *
 *******************************************************************************/
#define UNIQUE _TEXT("{0CB926C6-163B-49E5-A4E2-9FD04851D2BB}") // ������ ���� ����������
LPTSTR CreateExclusionName(UINT kind, LPTSTR ExclusionName)
{
    lstrcpy(ExclusionName, UNIQUE);
    switch (kind) /* kind */
    {
    case UNIQUE_TO_SYSTEM: return ExclusionName;
    case UNIQUE_TO_DESKTOP: /* desktop */
    {
        DWORD len;
        HDESK desktop = GetThreadDesktop(GetCurrentThreadId());
        BOOL result = GetUserObjectInformation(desktop, UOI_NAME, NULL, 0, &len);
        DWORD err = GetLastError();
        if (!result && err == ERROR_INSUFFICIENT_BUFFER) /* NT/2000 */
        {
            LPBYTE data = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, len);
            GetUserObjectInformation(desktop, UOI_NAME, data, len, &len);
            lstrcat(ExclusionName, _TEXT("-"));
            lstrcat(ExclusionName, (LPCTSTR)data);
            HeapFree(GetProcessHeap(), 0, data);
        } /* NT/2000 */
        else /* Win9x */
            lstrcat(ExclusionName, _TEXT("-Win9x"));
        return ExclusionName;
    } /* desktop */
    case UNIQUE_TO_SESSION: /* session */
    {
        HANDLE token;
        DWORD len;
        BOOL result = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token);
        if (result) /* NT */
        {
            GetTokenInformation(token, TokenStatistics, NULL, 0, &len);
            LPBYTE data = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, len);
            GetTokenInformation(token, TokenStatistics, data, len, &len);
            LUID uid = ((PTOKEN_STATISTICS)data)->AuthenticationId;
            HeapFree(GetProcessHeap(), 0, data);
            TCHAR szUid[MINCHAR];
            wsprintf(szUid, _TEXT("-%#08x%#08x"), uid.HighPart, uid.LowPart);
            lstrcat(ExclusionName, szUid);
        } /* NT */
        return ExclusionName;
    } /* session */
    case UNIQUE_TO_TRUSTEE: /* trustee */
    {
        TCHAR  userName[UNLEN + 1];
        DWORD userNameLength = UNLEN + 1;
        TCHAR  domainName[UNLEN + 1];
        if (GetUserName(userName, &userNameLength)) /* get network name */
        {
            // The NetApi calls are very time consuming
            // This technique gets the domain name via an
            // environment variable
            DWORD domainNameLength = ExpandEnvironmentStrings(_TEXT("%USERDOMAIN%"),
                domainName,
                UNLEN + 1);
            TCHAR szNwName[MINCHAR];
            if (domainNameLength) wsprintf(szNwName, _TEXT("-%s"), domainName);
            lstrcat(szNwName, _TEXT("-"));
            lstrcat(szNwName, userName);
            lstrcat(ExclusionName, szNwName);
        } /* get network name */
        return ExclusionName;
    } /* trustee */
    default: return ExclusionName;
    } /* kind */
}

/*******************************************************************************
 * ������� : EnumWindowsProc                                                   *
 * ���������� : ����� ���� �� ������ ���������                                 *
 * ��������� : hWnd - ����� ���� ����������                                    *
 *             lParam - �������� ���������, ���� ������� ����� ����������� ����*
 * ��������� : ���������� FALSE � ������ ����������� �������� ����             *
 *******************************************************************************/
static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    DWORD result = 100;
    // ���������� ��� ���� � ����������
    if (!SendMessageTimeout(hWnd, UWM_ARE_YOU_ME, 0, 0,
        SMTO_BLOCK | SMTO_ABORTIFHUNG, 200, (PDWORD_PTR)&result)) return TRUE;
    if (result == UWM_ARE_YOU_ME) /* ����� */
    {
        HWND* target = (HWND*)lParam;
        *target = hWnd;
        return FALSE;
    } /* ����� */
    return TRUE; // ���������� �����
}
/*******************************************************************************
 * ������� : InitInstance                                                      *
 * ���������� : �������� ������������� ����������� ���������� ����������       *
 * ��������� : NameInstance - ��� ���������� (����������)                      *
 *             RunExistInst - ������������ ������ ��������� ?                  *
 * ��������� : ����� �������� - � ������ ������������� �������                 *
 *             NULL - � ������ ������������� ��� ����������,                   *
 *             ���������� ��������� ������                                     *
 *******************************************************************************/
HANDLE InitInstance(LPTSTR NameInstance, BOOL RunExistInst)
{
    HANDLE InstanceMutex;
    BOOL AlreadyRunning;
    // ��������� ���������� ��� ��������
    TCHAR ExclMutexName[MAXBYTE];
    lstrcpy(ExclMutexName, NameInstance);
    lstrcat(ExclMutexName, _TEXT("-"));
    TCHAR ExclusionName[MINCHAR];
    lstrcat(ExclMutexName, CreateExclusionName(UNIQUE_TO_TRUSTEE, ExclusionName));
    // ��������� ��� �������
    InstanceMutex = CreateMutex(NULL, TRUE, ExclMutexName);
    AlreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS ||
        GetLastError() == ERROR_ACCESS_DENIED);
    // ����� ���������� ERROR_ACCESS_DENIED, ���� ������� ��� ������
    // � ������ ���������������� ������, �.�. � �������� ���������
    // SECURITY_ATTRIBUTES ��� �������� �������� ���������� NULL
    if (AlreadyRunning) // ���� ���� ���������
    {
        if (RunExistInst)/* ������������ ���� ����������� ���������� */
        {
            HWND hOther = NULL;
            // ���������� ��� ������� ���� ��������� ����� ������� ���� �������
            // EnumWindowsProc, ���������� �� ��� ��� ���� ������� �� ������� �������
            // ��� �� ���������� ������� ������
            EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)&hOther);
            if (hOther) /* �������� ���� ������ ������ */
            {
                SetForegroundWindow(hOther);
                if (IsIconic(hOther)) /* ������������� ���� */
                    ShowWindow(hOther, SW_RESTORE);
            } /* �������� ���� ������ ������ */
        }
        return NULL; // ���������� ������
    } /* ������������ ���� ����������� ���������� */
    return InstanceMutex;
}
