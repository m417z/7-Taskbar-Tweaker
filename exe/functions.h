#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include "version.h"

#ifdef _WIN64
#define DEF3264(d32, d64) (d64)
#else
#define DEF3264(d32, d64) (d32)
#endif

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)	 ((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)	 ((int)(short)HIWORD(lParam))
#endif

BOOL CompareWindowsVersion(DWORD dwMajorVersion, DWORD dwMinorVersion);
BOOL CompareWindowsBuildNumber(DWORD dwBuildNumber);
BOOL RegisterDialogClass(LPCTSTR lpszClassName, HINSTANCE hInstance);
const WCHAR *LoadStrFromRsrc(UINT uStrId);
UINT GetUniqueTempDir(const TCHAR *pPrefixString, TCHAR *pDir);
BOOL RemoveDirectoryOnReboot(const TCHAR *pDir);
BOOL SimpleCreateProcess(const WCHAR *pApplicationName, WCHAR *pCommandLine, BOOL bWait);
BOOL CanAccessFolder(LPCTSTR szFolderName, DWORD dwGenericAccessRights);
BOOL GetHelpFilePath(LANGID langid, const WCHAR *szLauncherPath, WCHAR *szHelpFilePath);

#endif // _FUNCTIONS_H_
