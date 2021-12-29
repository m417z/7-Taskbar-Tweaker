#include "stdafx.h"
#include "portable_settings.h"

// Macros
#define PS_FUNCTION_ARRAY_INIT(function) \
                              { Reg_ ## function, Ini_ ## function }

// Registry
static LSTATUS Reg_Init(const WCHAR *pStorageData);
static LSTATUS Reg_OpenSection(const WCHAR *pSectionName, BOOL bWrite, PS_SECTION *pSection);
static LSTATUS Reg_CloseSection(PS_SECTION *pSection);
static LSTATUS Reg_RemoveSection(const WCHAR *pSectionName);
static LSTATUS Reg_GetString(PS_SECTION *pSection, const WCHAR *pValueName, WCHAR *pReturnedString, UINT *pStringSize);
static LSTATUS Reg_SetString(PS_SECTION *pSection, const WCHAR *pValueName, const WCHAR *pString);
static LSTATUS Reg_GetInt(PS_SECTION *pSection, const WCHAR *pValueName, int *pReturnedInt);
static LSTATUS Reg_SetInt(PS_SECTION *pSection, const WCHAR *pValueName, int nInt);
static LSTATUS Reg_Remove(PS_SECTION *pSection, const WCHAR *pValueName);
static LSTATUS Reg_FindInit(PS_SECTION *pSection, PS_FIND *pFind);
static LSTATUS Reg_FindGetCount(PS_SECTION *pSection, PS_FIND *pFind, UINT *pnCount);
static LSTATUS Reg_FindNextString(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, WCHAR *pReturnedString, UINT *pStringSize);
static LSTATUS Reg_FindNextInt(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, int *pReturnedInt);
static LSTATUS Reg_FindSkip(PS_SECTION *pSection, PS_FIND *pFind);
static LSTATUS Reg_FindClose(PS_SECTION *pSection, PS_FIND *pFind);

// Ini
static LSTATUS Ini_Init(const WCHAR *pStorageData);
static LSTATUS Ini_OpenSection(const WCHAR *pSectionName, BOOL bWrite, PS_SECTION *pSection);
static LSTATUS Ini_CloseSection(PS_SECTION *pSection);
static LSTATUS Ini_RemoveSection(const WCHAR *pSectionName);
static LSTATUS Ini_GetString(PS_SECTION *pSection, const WCHAR *pValueName, WCHAR *pReturnedString, UINT *pStringSize);
static LSTATUS Ini_SetString(PS_SECTION *pSection, const WCHAR *pValueName, const WCHAR *pString);
static LSTATUS Ini_GetInt(PS_SECTION *pSection, const WCHAR *pValueName, int *pReturnedInt);
static LSTATUS Ini_SetInt(PS_SECTION *pSection, const WCHAR *pValueName, int nInt);
static LSTATUS Ini_Remove(PS_SECTION *pSection, const WCHAR *pValueName);
static LSTATUS Ini_FindInit(PS_SECTION *pSection, PS_FIND *pFind);
static LSTATUS Ini_FindGetCount(PS_SECTION *pSection, PS_FIND *pFind, UINT *pnCount);
static LSTATUS Ini_FindNextString(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, WCHAR *pReturnedString, UINT *pStringSize);
static LSTATUS Ini_FindNextInt(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, int *pReturnedInt);
static LSTATUS Ini_FindSkip(PS_SECTION *pSection, PS_FIND *pFind);
static LSTATUS Ini_FindClose(PS_SECTION *pSection, PS_FIND *pFind);

// Ini helper functions
static WCHAR *Ini_Helper_ValueNameEncode(const WCHAR *pValueName);
static void Ini_Helper_ValueNameEncodedFree(WCHAR *pEncodedValueName);
static BOOL Ini_Helper_ValueNameDecode(WCHAR *pDecodedValueName, UINT uDecodedValueNameSize, const WCHAR *pEncodedValueName);

// Helper functions
static int StringToInt(WCHAR *pszStr);

static int g_nStorage;
static WCHAR g_szStorageData[MAX_PATH];

LSTATUS PSInit(int nStorage, const WCHAR *pStorageData)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(Init);
	LSTATUS error;

	if(nStorage < 0 || nStorage > PS_STORAGE_COUNT - 1)
		return ERROR_INVALID_PARAMETER;

	error = ((LSTATUS(*)(const WCHAR *))f[nStorage])(pStorageData);
	if(error != ERROR_SUCCESS)
		return error;

	g_nStorage = nStorage;
	return ERROR_SUCCESS;
}

LSTATUS PSOpenSection(const WCHAR *pSectionName, BOOL bWrite, PS_SECTION *pSection)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(OpenSection);
	return ((LSTATUS(*)(const WCHAR *, BOOL, PS_SECTION *))f[g_nStorage])(pSectionName, bWrite, pSection);
}

LSTATUS PSCloseSection(PS_SECTION *pSection)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(CloseSection);
	return ((LSTATUS(*)(PS_SECTION *))f[g_nStorage])(pSection);
}

LSTATUS PSRemoveSection(const WCHAR *pSectionName)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(RemoveSection);
	return ((LSTATUS(*)(const WCHAR *))f[g_nStorage])(pSectionName);
}

LSTATUS PSGetString(PS_SECTION *pSection, const WCHAR *pValueName, WCHAR *pReturnedString, UINT *pStringSize)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(GetString);
	return ((LSTATUS(*)(PS_SECTION *, const WCHAR *, WCHAR *, UINT *))f[g_nStorage])
		(pSection, pValueName, pReturnedString, pStringSize);
}

LSTATUS PSSetString(PS_SECTION *pSection, const WCHAR *pValueName, const WCHAR *pString)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(SetString);
	return ((LSTATUS(*)(PS_SECTION *, const WCHAR *, const WCHAR *))f[g_nStorage])(pSection, pValueName, pString);
}

LSTATUS PSGetInt(PS_SECTION *pSection, const WCHAR *pValueName, int *pReturnedInt)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(GetInt);
	return ((LSTATUS(*)(PS_SECTION *, const WCHAR *, int *))f[g_nStorage])(pSection, pValueName, pReturnedInt);
}

LSTATUS PSSetInt(PS_SECTION *pSection, const WCHAR *pValueName, int nInt)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(SetInt);
	return ((LSTATUS(*)(PS_SECTION *, const WCHAR *, int))f[g_nStorage])(pSection, pValueName, nInt);
}

LSTATUS PSRemove(PS_SECTION *pSection, const WCHAR *pValueName)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(Remove);
	return ((LSTATUS(*)(PS_SECTION *, const WCHAR *))f[g_nStorage])(pSection, pValueName);
}

LSTATUS PSFindInit(PS_SECTION *pSection, PS_FIND *pFind)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(FindInit);
	return ((LSTATUS(*)(PS_SECTION *, PS_FIND *))f[g_nStorage])(pSection, pFind);
}

LSTATUS PSFindGetCount(PS_SECTION *pSection, PS_FIND *pFind, UINT *pnCount)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(FindGetCount);
	return ((LSTATUS(*)(PS_SECTION *, PS_FIND *, UINT *))f[g_nStorage])(pSection, pFind, pnCount);
}

LSTATUS PSFindNextString(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, WCHAR *pReturnedString, UINT *pStringSize)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(FindNextString);
	return ((LSTATUS(*)(PS_SECTION *, PS_FIND *, WCHAR *, UINT *, WCHAR *, UINT *))f[g_nStorage])
		(pSection, pFind, pReturnedValueName, pValueNameSize, pReturnedString, pStringSize);
}

LSTATUS PSFindNextInt(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, int *pReturnedInt)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(FindNextInt);
	return ((LSTATUS(*)(PS_SECTION *, PS_FIND *, WCHAR *, UINT *, int *))f[g_nStorage])
		(pSection, pFind, pReturnedValueName, pValueNameSize, pReturnedInt);
}

LSTATUS PSFindSkip(PS_SECTION *pSection, PS_FIND *pFind)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(FindSkip);
	return ((LSTATUS(*)(PS_SECTION *, PS_FIND *))f[g_nStorage])(pSection, pFind);
}

LSTATUS PSFindClose(PS_SECTION *pSection, PS_FIND *pFind)
{
	void *f[] = PS_FUNCTION_ARRAY_INIT(FindClose);
	return ((LSTATUS(*)(PS_SECTION *, PS_FIND *))f[g_nStorage])(pSection, pFind);
}

LSTATUS PSGetSingleString(const WCHAR *pSectionName, const WCHAR *pValueName, WCHAR *pReturnedString, UINT *pStringSize)
{
	PS_SECTION section;
	LSTATUS error;

	error = PSOpenSection(pSectionName, FALSE, &section);
	if(error == ERROR_SUCCESS)
	{
		error = PSGetString(&section, pValueName, pReturnedString, pStringSize);
		PSCloseSection(&section);
	}

	return error;
}

LSTATUS PSSetSingleString(const WCHAR *pSectionName, const WCHAR *pValueName, const WCHAR *pString)
{
	PS_SECTION section;
	LSTATUS error;

	error = PSOpenSection(pSectionName, TRUE, &section);
	if(error == ERROR_SUCCESS)
	{
		error = PSSetString(&section, pValueName, pString);
		PSCloseSection(&section);
	}

	return error;
}

LSTATUS PSGetSingleInt(const WCHAR *pSectionName, const WCHAR *pValueName, int *pReturnedInt)
{
	PS_SECTION section;
	LSTATUS error;

	error = PSOpenSection(pSectionName, FALSE, &section);
	if(error == ERROR_SUCCESS)
	{
		error = PSGetInt(&section, pValueName, pReturnedInt);
		PSCloseSection(&section);
	}

	return error;
}

LSTATUS PSSetSingleInt(const WCHAR *pSectionName, const WCHAR *pValueName, int nInt)
{
	PS_SECTION section;
	LSTATUS error;

	error = PSOpenSection(pSectionName, TRUE, &section);
	if(error == ERROR_SUCCESS)
	{
		error = PSSetInt(&section, pValueName, nInt);
		PSCloseSection(&section);
	}

	return error;
}

LSTATUS PSRemoveSingle(const WCHAR *pSectionName, const WCHAR *pValueName)
{
	PS_SECTION section;
	LSTATUS error;

	error = PSOpenSection(pSectionName, TRUE, &section);
	if(error == ERROR_SUCCESS)
	{
		error = PSRemove(&section, pValueName);
		PSCloseSection(&section);
	}

	return error;
}

// Registry

static LSTATUS Reg_Init(const WCHAR *pStorageData)
{
	if(lstrlen(pStorageData) > MAX_PATH - 1 - (sizeof("Software\\") - 1))
		return ERROR_INVALID_PARAMETER;

	lstrcpy(g_szStorageData, L"Software\\");
	lstrcat(g_szStorageData, pStorageData);

	return ERROR_SUCCESS;
}

static LSTATUS Reg_OpenSection(const WCHAR *pSectionName, BOOL bWrite, PS_SECTION *pSection)
{
	WCHAR szRegSubKey[MAX_PATH + 255];
	HKEY hSubKey;
	LSTATUS error;

	if(pSectionName)
	{
		if(lstrlen(pSectionName) > 255)
			return ERROR_INVALID_PARAMETER;

		lstrcpy(szRegSubKey, g_szStorageData);
		lstrcat(szRegSubKey, L"\\");
		lstrcat(szRegSubKey, pSectionName);
	}

	error = RegCreateKeyEx(HKEY_CURRENT_USER, pSectionName ? szRegSubKey : g_szStorageData,
		0, NULL, 0, KEY_READ | (bWrite ? KEY_WRITE : 0), NULL, &hSubKey, NULL);
	if(error != ERROR_SUCCESS)
		return error;

	pSection->hSubKey = hSubKey;

	return ERROR_SUCCESS;
}

static LSTATUS Reg_CloseSection(PS_SECTION *pSection)
{
	return RegCloseKey(pSection->hSubKey);
}

static LSTATUS Reg_RemoveSection(const WCHAR *pSectionName)
{
	WCHAR szRegSubKey[MAX_PATH + 255];

	if(pSectionName)
	{
		if(lstrlen(pSectionName) > 255)
			return ERROR_INVALID_PARAMETER;

		lstrcpy(szRegSubKey, g_szStorageData);
		lstrcat(szRegSubKey, L"\\");
		lstrcat(szRegSubKey, pSectionName);
	}

	return RegDeleteTree(HKEY_CURRENT_USER, pSectionName ? szRegSubKey : g_szStorageData);
}

static LSTATUS Reg_GetString(PS_SECTION *pSection, const WCHAR *pValueName, WCHAR *pReturnedString, UINT *pStringSize)
{
	DWORD dwType;
	WCHAR *pData;
	DWORD dwSize;
	DWORD dw;
	WCHAR szNumBuffer[sizeof("-2147483648")];
	UINT nStringSize;
	LSTATUS error;

	pData = pReturnedString;
	dwSize = (*pStringSize) * sizeof(WCHAR);

	if(dwSize < sizeof(DWORD))
	{
		pData = (WCHAR *)&dw;
		dwSize = sizeof(DWORD);
	}

	error = RegQueryValueEx(pSection->hSubKey, pValueName, NULL, &dwType, (BYTE *)pData, &dwSize);
	if(error == ERROR_SUCCESS)
	{
		switch(dwType)
		{
		case REG_SZ:
			if(dwSize > 0 && (dwSize % sizeof(WCHAR)) == 0)
			{
				nStringSize = dwSize / sizeof(WCHAR) - 1;

				if(pData[nStringSize] != '\0' || lstrlen(pData) != nStringSize)
					error = ERROR_FILE_NOT_FOUND;
			}
			else
				error = ERROR_FILE_NOT_FOUND;
			break;

		case REG_DWORD:
			if(dwSize == sizeof(DWORD))
			{
				nStringSize = wsprintf(szNumBuffer, L"%d", (int)*(DWORD *)pData);
				pData = szNumBuffer;
			}
			else
				error = ERROR_FILE_NOT_FOUND;
			break;

		default:
			error = ERROR_FILE_NOT_FOUND;
			break;
		}

		if(error == ERROR_SUCCESS && pData != pReturnedString)
		{
			if(nStringSize <= *pStringSize - 1)
				lstrcpy(pReturnedString, pData);
			else
				error = ERROR_MORE_DATA;
		}
	}

	if(error != ERROR_SUCCESS)
		return error;

	*pStringSize = nStringSize;

	return ERROR_SUCCESS;
}

static LSTATUS Reg_SetString(PS_SECTION *pSection, const WCHAR *pValueName, const WCHAR *pString)
{
	return RegSetValueEx(pSection->hSubKey, pValueName, 0, REG_SZ, (BYTE *)pString, (lstrlen(pString) + 1) * sizeof(WCHAR));
}

static LSTATUS Reg_GetInt(PS_SECTION *pSection, const WCHAR *pValueName, int *pReturnedInt)
{
	DWORD dwType;
	WCHAR szData[256];
	DWORD dwSize;
	UINT nStringSize;
	int nInt;
	LSTATUS error;

	dwSize = 256 * sizeof(WCHAR);

	error = RegQueryValueEx(pSection->hSubKey, pValueName, NULL, &dwType, (BYTE *)szData, &dwSize);
	if(error != ERROR_SUCCESS)
		return error;

	switch(dwType)
	{
	case REG_SZ:
		if(dwSize > 0 && (dwSize % sizeof(WCHAR)) == 0)
		{
			nStringSize = dwSize / sizeof(WCHAR) - 1;

			if(szData[nStringSize] == '\0' && lstrlen(szData) == nStringSize)
				nInt = StringToInt(szData);
			else
				error = ERROR_FILE_NOT_FOUND;
		}
		else
			error = ERROR_FILE_NOT_FOUND;
		break;

	case REG_DWORD:
		if(dwSize == sizeof(DWORD))
			nInt = (int)*(DWORD *)szData;
		else
			error = ERROR_FILE_NOT_FOUND;
		break;

	default:
		error = ERROR_FILE_NOT_FOUND;
		break;
	}

	if(error != ERROR_SUCCESS)
		return error;

	*pReturnedInt = nInt;

	return ERROR_SUCCESS;
}

static LSTATUS Reg_SetInt(PS_SECTION *pSection, const WCHAR *pValueName, int nInt)
{
	DWORD dwValue;

	dwValue = (DWORD)nInt;

	return RegSetValueEx(pSection->hSubKey, pValueName, 0, REG_DWORD, (BYTE *)&dwValue, sizeof(DWORD));
}

static LSTATUS Reg_Remove(PS_SECTION *pSection, const WCHAR *pValueName)
{
	return RegDeleteValue(pSection->hSubKey, pValueName);
}

static LSTATUS Reg_FindInit(PS_SECTION *pSection, PS_FIND *pFind)
{
	pFind->dwIndex = 0;

	return ERROR_SUCCESS;
}

static LSTATUS Reg_FindGetCount(PS_SECTION *pSection, PS_FIND *pFind, UINT *pnCount)
{
	DWORD dwValues;
	LSTATUS error;

	error = RegQueryInfoKey(pSection->hSubKey, NULL, NULL, NULL, NULL, NULL, NULL, &dwValues, NULL, NULL, NULL, NULL);
	if(error == ERROR_SUCCESS)
		*pnCount = dwValues;

	return error;
}

static LSTATUS Reg_FindNextString(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, WCHAR *pReturnedString, UINT *pStringSize)
{
	DWORD dwValueNameSize;
	DWORD dwType;
	WCHAR *pData;
	DWORD dwSize;
	DWORD dw;
	WCHAR szNumBuffer[sizeof("-2147483648")];
	UINT nStringSize;
	LSTATUS error;

	dwValueNameSize = *pValueNameSize;

	pData = pReturnedString;
	dwSize = (*pStringSize) * sizeof(WCHAR);

	if(dwSize < sizeof(DWORD))
	{
		pData = (WCHAR *)&dw;
		dwSize = sizeof(DWORD);
	}

	error = RegEnumValue(pSection->hSubKey, pFind->dwIndex, pReturnedValueName, &dwValueNameSize,
		NULL, &dwType, (BYTE *)pData, &dwSize);
	if(error == ERROR_SUCCESS)
	{
		switch(dwType)
		{
		case REG_SZ:
			if(dwSize > 0 && (dwSize % sizeof(WCHAR)) == 0)
			{
				nStringSize = dwSize / sizeof(WCHAR) - 1;

				if(pData[nStringSize] != '\0' || lstrlen(pData) != nStringSize)
					error = ERROR_FILE_NOT_FOUND;
			}
			else
				error = ERROR_FILE_NOT_FOUND;
			break;

		case REG_DWORD:
			if(dwSize == sizeof(DWORD))
			{
				nStringSize = wsprintf(szNumBuffer, L"%d", (int)*(DWORD *)pData);
				pData = szNumBuffer;
			}
			else
				error = ERROR_FILE_NOT_FOUND;
			break;

		default:
			error = ERROR_FILE_NOT_FOUND;
			break;
		}

		if(error == ERROR_SUCCESS && pData != pReturnedString)
		{
			if(nStringSize <= *pStringSize - 1)
				lstrcpy(pReturnedString, pData);
			else
				error = ERROR_MORE_DATA;
		}
	}

	if(error != ERROR_SUCCESS)
	{
		if(error == ERROR_FILE_NOT_FOUND)
		{
			pFind->dwIndex++;
			return Reg_FindNextString(pSection, pFind, pReturnedValueName, pValueNameSize, pReturnedString, pStringSize);
		}
		else
		{
			if(error == ERROR_MORE_DATA && dwValueNameSize < *pValueNameSize)
				*pValueNameSize = dwValueNameSize;

			return error;
		}
	}

	*pValueNameSize = dwValueNameSize;
	*pStringSize = nStringSize;

	pFind->dwIndex++;

	return ERROR_SUCCESS;
}

static LSTATUS Reg_FindNextInt(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, int *pReturnedInt)
{
	DWORD dwValueNameSize;
	DWORD dwType;
	WCHAR szData[256];
	DWORD dwSize;
	UINT nStringSize;
	int nInt;
	LSTATUS error;

	dwValueNameSize = *pValueNameSize;

	dwSize = 256 * sizeof(WCHAR);

	error = RegEnumValue(pSection->hSubKey, pFind->dwIndex, pReturnedValueName, &dwValueNameSize,
		NULL, &dwType, (BYTE *)szData, &dwSize);
	if(error == ERROR_SUCCESS)
	{
		switch(dwType)
		{
		case REG_SZ:
			if(dwSize > 0 && (dwSize % sizeof(WCHAR)) == 0)
			{
				nStringSize = dwSize / sizeof(WCHAR) - 1;

				if(szData[nStringSize] == '\0' && lstrlen(szData) == nStringSize)
					nInt = StringToInt(szData);
				else
					error = ERROR_FILE_NOT_FOUND;
			}
			else
				error = ERROR_FILE_NOT_FOUND;
			break;

		case REG_DWORD:
			if(dwSize == sizeof(DWORD))
				nInt = (int)*(DWORD *)szData;
			else
				error = ERROR_FILE_NOT_FOUND;
			break;

		default:
			error = ERROR_FILE_NOT_FOUND;
			break;
		}
	}

	if(error != ERROR_SUCCESS)
	{
		if(error == ERROR_FILE_NOT_FOUND)
		{
			pFind->dwIndex++;
			return Reg_FindNextInt(pSection, pFind, pReturnedValueName, pValueNameSize, pReturnedInt);
		}
		else
		{
			if(error == ERROR_MORE_DATA && dwValueNameSize < *pValueNameSize)
				*pValueNameSize = dwValueNameSize;

			return error;
		}
	}

	*pValueNameSize = dwValueNameSize;
	*pReturnedInt = nInt;

	pFind->dwIndex++;

	return ERROR_SUCCESS;
}

static LSTATUS Reg_FindSkip(PS_SECTION *pSection, PS_FIND *pFind)
{
	DWORD dwValues;
	LSTATUS error;

	error = RegQueryInfoKey(pSection->hSubKey, NULL, NULL,
		NULL, NULL, NULL, NULL, &dwValues, NULL, NULL, NULL, NULL);
	if(error != ERROR_SUCCESS)
		return error;

	if(pFind->dwIndex >= dwValues)
		return ERROR_NO_MORE_ITEMS;

	pFind->dwIndex++;

	return ERROR_SUCCESS;
}

static LSTATUS Reg_FindClose(PS_SECTION *pSection, PS_FIND *pFind)
{
	return ERROR_SUCCESS;
}

// Ini

static LSTATUS Ini_Init(const WCHAR *pStorageData)
{
	if(lstrlen(pStorageData) > MAX_PATH - 1)
		return ERROR_INVALID_PARAMETER;

	lstrcpy(g_szStorageData, pStorageData);

	HANDLE hFile = CreateFile(pStorageData, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		// Write a UTF-16LE BOM to enable Unicode.
		DWORD dwNumberOfBytesWritten;
		WriteFile(hFile, "\xFF\xFE", 2, &dwNumberOfBytesWritten, NULL);
		CloseHandle(hFile);
	}

	return ERROR_SUCCESS;
}

static LSTATUS Ini_OpenSection(const WCHAR *pSectionName, BOOL bWrite, PS_SECTION *pSection)
{
	if(pSectionName)
	{
		if(lstrlen(pSectionName) > 255)
			return ERROR_INVALID_PARAMETER;

		lstrcpy(pSection->szSection, pSectionName);
	}
	else
		lstrcpy(pSection->szSection, L"Config");

	return ERROR_SUCCESS;
}

static LSTATUS Ini_CloseSection(PS_SECTION *pSection)
{
	return ERROR_SUCCESS;
}

static LSTATUS Ini_RemoveSection(const WCHAR *pSectionName)
{
	if(pSectionName)
	{
		SetLastError(0);

		WritePrivateProfileString(pSectionName, NULL, NULL, g_szStorageData);
	}
	else if(DeleteFile(g_szStorageData))
		return ERROR_SUCCESS;

	return GetLastError();
}

static LSTATUS Ini_GetString(PS_SECTION *pSection, const WCHAR *pValueName, WCHAR *pReturnedString, UINT *pStringSize)
{
	WCHAR *pEncodedValueName;
	UINT nReturnedStringSize;
	LSTATUS error;

	pEncodedValueName = Ini_Helper_ValueNameEncode(pValueName);
	if(!pEncodedValueName)
		return ERROR_OUTOFMEMORY;

	SetLastError(0);

	nReturnedStringSize = GetPrivateProfileString(pSection->szSection, pEncodedValueName,
		NULL, pReturnedString, *pStringSize, g_szStorageData);

	error = GetLastError();
	Ini_Helper_ValueNameEncodedFree(pEncodedValueName);
	if(error != ERROR_SUCCESS)
		return error;

	*pStringSize = nReturnedStringSize;

	return ERROR_SUCCESS;
}

static LSTATUS Ini_SetString(PS_SECTION *pSection, const WCHAR *pValueName, const WCHAR *pString)
{
	WCHAR *pEncodedValueName;
	LSTATUS error;

	pEncodedValueName = Ini_Helper_ValueNameEncode(pValueName);
	if(!pEncodedValueName)
		return ERROR_OUTOFMEMORY;

	SetLastError(0);

	WritePrivateProfileString(pSection->szSection, pEncodedValueName, pString, g_szStorageData);

	error = GetLastError();
	Ini_Helper_ValueNameEncodedFree(pEncodedValueName);
	return error;
}

static LSTATUS Ini_GetInt(PS_SECTION *pSection, const WCHAR *pValueName, int *pReturnedInt)
{
	WCHAR *pEncodedValueName;
	UINT nReturnedStringSize;
	WCHAR szData[256];
	LSTATUS error;

	pEncodedValueName = Ini_Helper_ValueNameEncode(pValueName);
	if(!pEncodedValueName)
		return ERROR_OUTOFMEMORY;

	SetLastError(0);

	nReturnedStringSize = GetPrivateProfileString(pSection->szSection, pEncodedValueName,
		NULL, szData, 256, g_szStorageData);

	error = GetLastError();
	Ini_Helper_ValueNameEncodedFree(pEncodedValueName);
	if(error != ERROR_SUCCESS)
		return error;

	*pReturnedInt = StringToInt(szData);

	return ERROR_SUCCESS;
}

static LSTATUS Ini_SetInt(PS_SECTION *pSection, const WCHAR *pValueName, int nInt)
{
	WCHAR *pEncodedValueName;
	WCHAR szBuffer[sizeof("-2147483648")];
	LSTATUS error;

	pEncodedValueName = Ini_Helper_ValueNameEncode(pValueName);
	if(!pEncodedValueName)
		return ERROR_OUTOFMEMORY;

	wsprintf(szBuffer, L"%d", nInt);

	SetLastError(0);

	WritePrivateProfileString(pSection->szSection, pEncodedValueName, szBuffer, g_szStorageData);

	error = GetLastError();
	Ini_Helper_ValueNameEncodedFree(pEncodedValueName);
	return error;
}

static LSTATUS Ini_Remove(PS_SECTION *pSection, const WCHAR *pValueName)
{
	WCHAR *pEncodedValueName;
	LSTATUS error;

	pEncodedValueName = Ini_Helper_ValueNameEncode(pValueName);
	if(!pEncodedValueName)
		return ERROR_OUTOFMEMORY;

	SetLastError(0);

	WritePrivateProfileString(pSection->szSection, pEncodedValueName, NULL, g_szStorageData);

	error = GetLastError();
	Ini_Helper_ValueNameEncodedFree(pEncodedValueName);
	return error;
}

static LSTATUS Ini_FindInit(PS_SECTION *pSection, PS_FIND *pFind)
{
	UINT uCharsAllocated;
	WCHAR *pValueNames, *pTemp;
	UINT uStringSize;
	LSTATUS error;

	uCharsAllocated = 1024;

	pValueNames = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, uCharsAllocated * sizeof(WCHAR));
	if(!pValueNames)
		return ERROR_OUTOFMEMORY;

	SetLastError(0);

	uStringSize = GetPrivateProfileString(pSection->szSection, NULL,
		NULL, pValueNames, uCharsAllocated, g_szStorageData);

	error = GetLastError();

	while(error == ERROR_MORE_DATA)
	{
		uCharsAllocated += 1024;

		pTemp = (WCHAR *)HeapReAlloc(GetProcessHeap(), 0, pValueNames, uCharsAllocated * sizeof(WCHAR));
		if(!pTemp)
		{
			error = ERROR_OUTOFMEMORY;
			break;
		}

		pValueNames = pTemp;

		SetLastError(0);

		uStringSize = GetPrivateProfileString(pSection->szSection, NULL,
			NULL, pValueNames, uCharsAllocated, g_szStorageData);

		error = GetLastError();
	}

	if(error == ERROR_FILE_NOT_FOUND)
	{
		*pValueNames = L'\0';
		error = ERROR_SUCCESS;
	}

	if(error != ERROR_SUCCESS)
	{
		HeapFree(GetProcessHeap(), 0, pValueNames);
		return error;
	}

	pFind->pValueNames = pValueNames;
	pFind->pCurrentValueName = pValueNames;

	return ERROR_SUCCESS;
}

static LSTATUS Ini_FindGetCount(PS_SECTION *pSection, PS_FIND *pFind, UINT *pnCount)
{
	const WCHAR *p;
	int nLen;
	UINT nCount;

	nCount = 0;

	p = pFind->pValueNames;
	nLen = lstrlen(p);

	while(nLen > 0)
	{
		nCount++;

		p += nLen + 1;
		nLen = lstrlen(p);
	}

	*pnCount = nCount;

	return ERROR_SUCCESS;
}

static LSTATUS Ini_FindNextString(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, WCHAR *pReturnedString, UINT *pStringSize)
{
	WCHAR *pCurrentValueName;
	UINT nValueNameSize;
	UINT nReturnedStringSize;
	LSTATUS error;

	pCurrentValueName = pFind->pCurrentValueName;

	nValueNameSize = lstrlen(pCurrentValueName);
	if(nValueNameSize == 0)
		return ERROR_NO_MORE_ITEMS;

	if(!Ini_Helper_ValueNameDecode(pReturnedValueName, *pValueNameSize, pCurrentValueName))
		return ERROR_MORE_DATA;

	*pValueNameSize = lstrlen(pReturnedValueName);

	SetLastError(0);

	nReturnedStringSize = GetPrivateProfileString(pSection->szSection, pCurrentValueName,
		NULL, pReturnedString, *pStringSize, g_szStorageData);

	error = GetLastError();
	if(error != ERROR_SUCCESS)
		return error;

	*pStringSize = nReturnedStringSize;

	pFind->pCurrentValueName += nValueNameSize + 1;

	return ERROR_SUCCESS;
}

static LSTATUS Ini_FindNextInt(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, int *pReturnedInt)
{
	WCHAR szData[256];
	UINT uSize;
	LSTATUS error;

	uSize = 256;

	error = Ini_FindNextString(pSection, pFind, pReturnedValueName, pValueNameSize, szData, &uSize);
	if(error != ERROR_SUCCESS)
		return error;

	*pReturnedInt = StringToInt(szData);

	return ERROR_SUCCESS;
}

static LSTATUS Ini_FindSkip(PS_SECTION *pSection, PS_FIND *pFind)
{
	UINT nValueNameSize;

	nValueNameSize = lstrlen(pFind->pCurrentValueName);
	if(nValueNameSize == 0)
		return ERROR_NO_MORE_ITEMS;

	pFind->pCurrentValueName += nValueNameSize + 1;

	return ERROR_SUCCESS;
}

static LSTATUS Ini_FindClose(PS_SECTION *pSection, PS_FIND *pFind)
{
	HeapFree(GetProcessHeap(), 0, pFind->pValueNames);

	return ERROR_SUCCESS;
}

// Ini helper functions

static WCHAR *Ini_Helper_ValueNameEncode(const WCHAR *pValueName)
{
	UINT nValueNameSize = lstrlen(pValueName);
	WCHAR *pEncodedValueName = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, (nValueNameSize * 2 + 1) * sizeof(WCHAR));
	if(!pEncodedValueName)
		return NULL;

	const WCHAR *src = pValueName;
	WCHAR *dst = pEncodedValueName;
	while(*src != L'\0')
	{
		WCHAR ch = *src++;
		if(ch == L'=')
		{
			*dst++ = L'#';
			*dst++ = L'e';
		}
		else if(ch == L'#')
		{
			*dst++ = L'#';
			*dst++ = L'#';
		}
		else
		{
			*dst++ = ch;
		}
	}

	*dst++ = L'\0';

	return pEncodedValueName;
}

static void Ini_Helper_ValueNameEncodedFree(WCHAR *pEncodedValueName)
{
	HeapFree(GetProcessHeap(), 0, pEncodedValueName);
}

static BOOL Ini_Helper_ValueNameDecode(WCHAR *pDecodedValueName, UINT uDecodedValueNameSize, const WCHAR *pEncodedValueName)
{
	const WCHAR *src = pEncodedValueName;
	WCHAR *dst = pDecodedValueName;
	WCHAR *dstEnd = dst + uDecodedValueNameSize;
	while(*src != L'\0')
	{
		if(dst == dstEnd)
			return FALSE;

		WCHAR ch = *src++;
		if(ch == L'#')
		{
			ch = *src++;
			if(ch == L'e')
			{
				*dst++ = L'=';
			}
			else
			{
				*dst++ = ch;
			}
		}
		else
		{
			*dst++ = ch;
		}
	}

	if(dst == dstEnd)
		return FALSE;

	*dst++ = L'\0';
	return TRUE;
}

// Functions

static int StringToInt(WCHAR *pszStr)
{
	BOOL bMinus;
	int nInt;

	if(*pszStr == L'-')
	{
		bMinus = TRUE;
		pszStr++;
	}
	else
		bMinus = FALSE;

	nInt = 0;

	while(*pszStr >= L'0' && *pszStr <= L'9')
	{
		nInt *= 10;
		nInt += *pszStr - L'0';
		pszStr++;
	}

	if(bMinus)
		nInt = -nInt;

	return nInt;
}
