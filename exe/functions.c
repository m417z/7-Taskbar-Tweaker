#include "stdafx.h"
#include "functions.h"

BOOL CompareWindowsVersion(DWORD dwMajorVersion, DWORD dwMinorVersion)
{
	OSVERSIONINFOEX ver;
	DWORDLONG dwlConditionMask = 0;

	ZeroMemory(&ver, sizeof(OSVERSIONINFOEX));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	ver.dwMajorVersion = dwMajorVersion;
	ver.dwMinorVersion = dwMinorVersion;

	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_EQUAL);

	return VerifyVersionInfo(&ver, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask);
}

BOOL CompareWindowsBuildNumber(DWORD dwBuildNumber)
{
	OSVERSIONINFOEX ver;
	DWORDLONG dwlConditionMask = 0;

	ZeroMemory(&ver, sizeof(OSVERSIONINFOEX));
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	ver.dwBuildNumber = dwBuildNumber;

	VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, VER_EQUAL);

	return VerifyVersionInfo(&ver, VER_BUILDNUMBER, dwlConditionMask);
}

BOOL RegisterDialogClass(LPCTSTR lpszClassName, HINSTANCE hInstance)
{
	WNDCLASS wndcls;
	GetClassInfo(hInstance, MAKEINTRESOURCE(32770), &wndcls);

	// Set our own class name
	wndcls.lpszClassName = lpszClassName;

	// Just register the class
	return RegisterClass(&wndcls);
}

WCHAR *LoadStrFromRsrc(UINT uStrId)
{
	static WCHAR szBuffer[1024+1];
	DWORD dwExtraParam;
	WCHAR *pStr;

	dwExtraParam = uStrId & 0xFFF00000;
	if(dwExtraParam)
	{
		dwExtraParam >>= 4*5;
		dwExtraParam &= 0x00000FFF;

		uStrId &= 0x000FFFFF;
	}

	if(!LoadString(NULL, uStrId, (WCHAR *)&pStr, 0))
		pStr = L"(Could not load resource)";

	if(dwExtraParam)
	{
		wsprintf(szBuffer, L"%s (%u)", pStr, dwExtraParam);
		pStr = szBuffer;
	}

	return pStr;
}

UINT GetUniqueTempDir(TCHAR *pPrefixString, TCHAR *pDir)
{
	TCHAR *p;
	UINT uTempLen;
	WORD wUnique, wUniqueInit;

	uTempLen = GetTempPath(MAX_PATH, pDir);
	if(uTempLen == 0 || uTempLen > MAX_PATH-1)
		return 0;

	p = pDir + uTempLen;

	if(pPrefixString)
		uTempLen += lstrlen(pPrefixString);

	uTempLen += 4;

	if(uTempLen > MAX_PATH-1)
		return 0;

	if(pPrefixString)
		while(*pPrefixString)
			*p++ = *pPrefixString++;

	// Get a "random" unique number and try to create the directory
	wUnique = (WORD)GetTickCount();
	wUniqueInit = wUnique;

	do
	{
		wsprintf(p, L"%04X", wUnique);

		if(CreateDirectory(pDir, NULL)) // We created it
			return uTempLen;

		if(GetLastError() != ERROR_FILE_EXISTS)
			return 0;

		wUnique++;
	}
	while(wUnique != wUniqueInit);

	return 0;
}

BOOL RemoveDirectoryOnReboot(TCHAR *pDir)
{
	char *pVBScriptSrc = 
		"Sub Main()\r\n"
		"\r\n"
		"Set objFSO = CreateObject(\"Scripting.FileSystemObject\")\r\n"
		"Set objShell = CreateObject(\"Wscript.Shell\")\r\n"
		"\r\n"
		"objFile = Wscript.ScriptFullName\r\n"
		"objFolder = objFSO.GetParentFolderName(objFile)\r\n"
		"objParentFolder = objFSO.GetParentFolderName(objFolder)\r\n"
		"\r\n"
		"objShell.CurrentDirectory = objParentFolder\r\n"
		"objFSO.DeleteFolder(objFolder)\r\n"
		"\r\n"
		"End Sub\r\n"
		"\r\n"
		"On Error Resume Next\r\n"
		"Main\r\n";
	DWORD dwVBScriptLen = lstrlenA(pVBScriptSrc)*sizeof(char);

	WCHAR szFile[MAX_PATH];
	WCHAR szCommand[MAX_PATH + sizeof("\"%SystemRoot%\\System32\\WScript.exe\" /B \"\"") - 1];
	HANDLE hFile;
	DWORD dwNumberOfBytesWritten;
	BOOL bSuccess;

	HKEY hKey;
	WCHAR szValueName[16];
	WCHAR *p;
	WORD wUnique, wUniqueInit;
	LONG lRet;

	lstrcpy(szFile, pDir);
	lstrcat(szFile, L"\\del.vbs");

	hFile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	wsprintf(szCommand, L"\"%%SystemRoot%%\\System32\\WScript.exe\" /B \"%s\"", szFile);

	bSuccess = WriteFile(hFile, pVBScriptSrc, dwVBScriptLen, &dwNumberOfBytesWritten, NULL);
	CloseHandle(hFile);

	if(!bSuccess || dwNumberOfBytesWritten != dwVBScriptLen)
		return FALSE;

	lRet = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce", 
		0, NULL, 0, KEY_QUERY_VALUE|KEY_SET_VALUE, NULL, &hKey, NULL);
	if(lRet != ERROR_SUCCESS)
		return FALSE;

	lstrcpy(szValueName, L"del_vbs_");
	p = szValueName + (sizeof("del_vbs_")-1);

	wUnique = (WORD)GetTickCount();
	wUniqueInit = wUnique;

	do
	{
		wsprintf(p, L"%04X", wUnique);

		lRet = RegQueryValueEx(hKey, szValueName, NULL, NULL, NULL, NULL);
		switch(lRet)
		{
		case ERROR_SUCCESS:
			break;

		case ERROR_FILE_NOT_FOUND:
			lRet = RegSetValueEx(hKey, szValueName, 0, REG_SZ, (BYTE *)szCommand, (lstrlen(szCommand)+1)*sizeof(WCHAR));
			RegCloseKey(hKey);
			return (lRet == ERROR_SUCCESS);

		default:
			RegCloseKey(hKey);
			return FALSE;
		}

		wUnique++;
	}
	while(wUnique != wUniqueInit);

	RegCloseKey(hKey);

	return FALSE;
}

BOOL SimpleCreateProcess(WCHAR *pApplicationName, WCHAR *pCommandLine, BOOL bWait)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if(!CreateProcess(pApplicationName, pCommandLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
		return FALSE;

	if(bWait)
		WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return TRUE;
}

/*
Source:
http://blog.aaronballman.com/2011/08/how-to-check-access-rights/

Usage:
if(CanAccessFolder(TEXT("C:\\Users\\"), GENERIC_WRITE)) {}
if(CanAccessFolder(TEXT("C:\\"), GENERIC_READ | GENERIC_WRITE)) {}
*/

BOOL CanAccessFolder(LPCTSTR szFolderName, DWORD dwGenericAccessRights)
{
	BOOL bRet = FALSE;
	DWORD length = 0;

	if(!GetFileSecurity(szFolderName, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | 
		DACL_SECURITY_INFORMATION, NULL, 0, &length) && 
		ERROR_INSUFFICIENT_BUFFER == GetLastError())
	{
		PSECURITY_DESCRIPTOR security = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), 0, length);

		if(security && GetFileSecurity(szFolderName, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | 
			DACL_SECURITY_INFORMATION, security, length, &length))
		{
			HANDLE hToken = NULL;

			if(OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_QUERY | 
				TOKEN_DUPLICATE | STANDARD_RIGHTS_READ, &hToken ))
			{
				HANDLE hImpersonatedToken = NULL;

				if(DuplicateToken(hToken, SecurityImpersonation, &hImpersonatedToken))
				{
					GENERIC_MAPPING mapping = {0xFFFFFFFF};
					PRIVILEGE_SET privileges = {0};
					DWORD grantedAccess = 0, privilegesLength = sizeof(PRIVILEGE_SET);
					BOOL result = FALSE;

					mapping.GenericRead = FILE_GENERIC_READ;
					mapping.GenericWrite = FILE_GENERIC_WRITE;
					mapping.GenericExecute = FILE_GENERIC_EXECUTE;
					mapping.GenericAll = FILE_ALL_ACCESS;

					MapGenericMask(&dwGenericAccessRights, &mapping);

					if(AccessCheck(security, hImpersonatedToken, dwGenericAccessRights, 
						&mapping, &privileges, &privilegesLength, &grantedAccess, &result))
					{
						bRet = (result != FALSE);
					}

					CloseHandle(hImpersonatedToken);
				}

				CloseHandle(hToken);
			}

			HeapFree(GetProcessHeap(), 0, security);
		}
	}

	return bRet;
}
