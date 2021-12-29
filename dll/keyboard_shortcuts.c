#include "stdafx.h"
#include "keyboard_shortcuts.h"
#include "portable_settings.h"

// superglobals
extern HINSTANCE hDllInst;

static HWND CreateHotKeyWnd(WNDPROC pHotKeyWndProc);
static void DestroyHotKeyWnd(HWND hWnd);
static BOOL RegisterKeyboardShortcut(WCHAR *p, HWND hWnd, int nId);
static WCHAR *StrToDword(WCHAR *pszStr, DWORD *pdw);

static HWND hHotKeyWnd;
static int *pHotKeyValues;
static UINT nHotKeyCount;

BOOL LoadKeybdShortcuts(WNDPROC pHotKeyWndProc)
{
	PS_SECTION section;
	PS_FIND find;
	UINT nKeyCount;
	WCHAR szValueName[256];
	UINT uValueNameSize;
	int nValue;
	int *pReAlloc;
	LSTATUS error;

	error = PSOpenSection(L"Keyboard Shortcuts", FALSE, &section);
	if(error == ERROR_SUCCESS)
	{
		error = PSFindInit(&section, &find);
		if(error == ERROR_SUCCESS)
		{
			error = PSFindGetCount(&section, &find, &nKeyCount);
			if(error == ERROR_SUCCESS && nKeyCount > 0)
			{
				hHotKeyWnd = CreateHotKeyWnd(pHotKeyWndProc);
				if(hHotKeyWnd)
				{
					pHotKeyValues = (int *)HeapAlloc(GetProcessHeap(), 0, nKeyCount * sizeof(int));
					if(pHotKeyValues)
					{
						do
						{
							uValueNameSize = 256;

							error = PSFindNextInt(&section, &find, szValueName, &uValueNameSize, &nValue);
							if(error == ERROR_SUCCESS)
							{
								if(KEYBD_SHORTCUT_IS_VALID_VALUE(nValue))
								{
									if(RegisterKeyboardShortcut(szValueName, hHotKeyWnd, nHotKeyCount))
									{
										pHotKeyValues[nHotKeyCount] = nValue;
										nHotKeyCount++;
									}
								}
							}
							else if(error == ERROR_MORE_DATA)
								error = PSFindSkip(&section, &find);
						}
						while(error == ERROR_SUCCESS);

						if(error == ERROR_NO_MORE_ITEMS)
						{
							error = ERROR_SUCCESS;

							if(nHotKeyCount < nKeyCount)
							{
								pReAlloc = (int *)HeapReAlloc(GetProcessHeap(), 0, pHotKeyValues, nHotKeyCount * sizeof(int));
								if(pReAlloc)
									pHotKeyValues = pReAlloc;
							}
						}
					}
					else
						error = ERROR_NOT_ENOUGH_MEMORY;
				}
				else
					error = ERROR_INVALID_WINDOW_HANDLE;
			}

			PSFindClose(&section, &find);
		}

		PSCloseSection(&section);
	}

	if(error != ERROR_SUCCESS)
	{
		FreeKeybdShortcuts();
		return FALSE;
	}

	return TRUE;
}

void FreeKeybdShortcuts()
{
	if(hHotKeyWnd)
	{
		if(pHotKeyValues)
		{
			for(UINT i = 0; i < nHotKeyCount; i++)
				UnregisterHotKey(hHotKeyWnd, i);

			nHotKeyCount = 0;

			HeapFree(GetProcessHeap(), 0, pHotKeyValues);
			pHotKeyValues = NULL;
		}

		DestroyHotKeyWnd(hHotKeyWnd);
		hHotKeyWnd = NULL;
	}
}

BOOL GetKeybdShortcutValue(int nId, int *pnValue)
{
	if((UINT)nId >= nHotKeyCount)
		return FALSE;

	*pnValue = pHotKeyValues[nId];
	return TRUE;
}

static HWND CreateHotKeyWnd(WNDPROC pHotKeyWndProc)
{
	WNDCLASS wc;
	HWND hWnd;

	ZeroMemory(&wc, sizeof(WNDCLASS));

	wc.lpfnWndProc = pHotKeyWndProc;
	wc.hInstance = hDllInst;
	wc.lpszClassName = L"7ttHotKeyWnd";

	if(RegisterClass(&wc))
	{
		hWnd = CreateWindow(L"7ttHotKeyWnd", NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hDllInst, 0);
		if(hWnd)
			return hWnd;

		UnregisterClass(L"7ttHotKeyWnd", hDllInst);
	}

	return NULL;
}

static void DestroyHotKeyWnd(HWND hWnd)
{
	DestroyWindow(hWnd);
	UnregisterClass(L"7ttHotKeyWnd", hDllInst);
}

static BOOL RegisterKeyboardShortcut(WCHAR *p, HWND hWnd, int nId)
{
	WCHAR *pString;
	UINT uVk, uModifiers;

	p = StrToDword(p, (DWORD *)&uVk);

	if(uVk < 0x01 || uVk > 0xFE)
		return FALSE;

	uModifiers = 0;

	if(*p != L'\0')
	{
		if(*p != L'|')
			return FALSE;

		p++;

		while(*p != L'\0')
		{
			pString = p;

			do
			{
				if((*p < L'A' || *p > L'Z') && (*p < L'a' || *p > L'z'))
					return FALSE;

				p++;
			}
			while(*p != L'+' && *p != L'\0');

			if(*p == L'+')
			{
				*p = L'\0';
				p++;
			}

			if(lstrcmpi(pString, L"alt") == 0)
			{
				if(uModifiers & MOD_ALT)
					return FALSE;

				uModifiers |= MOD_ALT;
			}
			else if(lstrcmpi(pString, L"ctrl") == 0)
			{
				if(uModifiers & MOD_CONTROL)
					return FALSE;

				uModifiers |= MOD_CONTROL;
			}
			else if(lstrcmpi(pString, L"shift") == 0)
			{
				if(uModifiers & MOD_SHIFT)
					return FALSE;

				uModifiers |= MOD_SHIFT;
			}
			else if(lstrcmpi(pString, L"win") == 0)
			{
				if(uModifiers & MOD_WIN)
					return FALSE;

				uModifiers |= MOD_WIN;
			}
			else if(lstrcmpi(pString, L"norepeat") == 0)
			{
				if(uModifiers & MOD_NOREPEAT)
					return FALSE;

				uModifiers |= MOD_NOREPEAT;
			}
			else
				return FALSE;
		}
	}

	return RegisterHotKey(hWnd, nId, uModifiers, uVk);
}

static WCHAR *StrToDword(WCHAR *pszStr, DWORD *pdw)
{
	BOOL bMinus;
	DWORD dw, dw2;

	if(*pszStr == L'-')
	{
		bMinus = TRUE;
		pszStr++;
	}
	else
		bMinus = FALSE;

	dw = 0;

	if(pszStr[0] == L'0' && (pszStr[1] == L'x' || pszStr[1] == L'X'))
	{
		pszStr += 2;

		while(*pszStr != L'\0')
		{
			if(*pszStr >= L'0' && *pszStr <= L'9')
				dw2 = *pszStr - L'0';
			else if(*pszStr >= L'a' && *pszStr <= L'f')
				dw2 = *pszStr - 'a' + 0x0A;
			else if(*pszStr >= L'A' && *pszStr <= L'F')
				dw2 = *pszStr - 'A' + 0x0A;
			else
				break;

			dw <<= 0x04;
			dw |= dw2;
			pszStr++;
		}
	}
	else
	{
		while(*pszStr != L'\0')
		{
			if(*pszStr >= L'0' && *pszStr <= L'9')
				dw2 = *pszStr - L'0';
			else
				break;

			dw *= 10;
			dw += dw2;
			pszStr++;
		}
	}

	if(bMinus)
		*pdw = (DWORD)-(long)dw; // :)
	else
		*pdw = dw;

	return pszStr;
}
