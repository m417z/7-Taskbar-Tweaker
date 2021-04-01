#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

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
WCHAR *LoadStrFromRsrc(UINT uStrId);
UINT GetUniqueTempDir(TCHAR *pPrefixString, TCHAR *pDir);
BOOL RemoveDirectoryOnReboot(TCHAR *pDir);
BOOL SimpleCreateProcess(WCHAR *pApplicationName, WCHAR *pCommandLine, BOOL bWait);
BOOL CanAccessFolder(LPCTSTR szFolderName, DWORD dwGenericAccessRights);

#endif // _FUNCTIONS_H_
