#include "stdafx.h"
#include "mouse_button_control.h"
#include "portable_settings.h"
// uthash {
#include "uthash.h"

// undefine the defaults
#undef uthash_fatal
#undef uthash_malloc
#undef uthash_free

// re-define, specifying alternate functions
#define uthash_fatal(msg) __debugbreak();
#define uthash_malloc(sz) HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sz)
#define uthash_free(ptr,sz) HeapFree(GetProcessHeap(), 0, ptr)
// } // uthash

typedef struct _mousectrl_keyval {
	DWORD dwKey;
	int nValue;
	UT_hash_handle hh;
} MOUSECTRL_KEYVAL;

static BOOL MouseCtrlKeyStrToDword(int nValue, WCHAR *p, DWORD *pdw);
static BOOL AddKey(MOUSECTRL_KEYVAL **ppKeyVal, DWORD dwKey, int nValue);
static MOUSECTRL_KEYVAL *FindKey(MOUSECTRL_KEYVAL *pKeyVal, DWORD dwKey);
static BOOL RemoveKey(MOUSECTRL_KEYVAL **ppKeyVal, DWORD dwKey);
static void RemoveAllKeys(MOUSECTRL_KEYVAL **ppKeyVal);

static MOUSECTRL_KEYVAL *mousectrl_keyval;

BOOL LoadMouseCtrl()
{
	PS_SECTION section;
	PS_FIND find;
	WCHAR szValueName[256];
	UINT uValueNameSize;
	DWORD dwKey;
	int nValue;
	LSTATUS error;

	error = PSOpenSection(L"Mouse Button Control", FALSE, &section);
	if(error == ERROR_SUCCESS)
	{
		error = PSFindInit(&section, &find);
		if(error == ERROR_SUCCESS)
		{
			do
			{
				uValueNameSize = 256;

				error = PSFindNextInt(&section, &find, szValueName, &uValueNameSize, &nValue);
				if(error == ERROR_SUCCESS)
				{
					if(MouseCtrlKeyStrToDword(nValue, szValueName, &dwKey) && !AddKey(&mousectrl_keyval, dwKey, nValue))
						error = ERROR_OUTOFMEMORY;
				}
				else if(error == ERROR_MORE_DATA)
					error = PSFindSkip(&section, &find);
			}
			while(error == ERROR_SUCCESS);

			if(error == ERROR_NO_MORE_ITEMS)
				error = ERROR_SUCCESS;

			PSFindClose(&section, &find);
		}

		PSCloseSection(&section);
	}

	if(error != ERROR_SUCCESS)
	{
		RemoveAllKeys(&mousectrl_keyval);
		return FALSE;
	}

	return TRUE;
}

void FreeMouseCtrl()
{
	RemoveAllKeys(&mousectrl_keyval);
}

BOOL GetMouseCtrlValue(BYTE bTarget, UINT uMsg, WPARAM wParam, int *pnValue)
{
	WORD wKeys, wButton;
	BYTE bButton, bMod;
	DWORD dwKey;
	MOUSECTRL_KEYVAL *pItem;

	bMod = 0;

	if(uMsg >= WM_NCMOUSEMOVE && uMsg <= WM_NCXBUTTONDBLCLK)
	{
		uMsg += -WM_NCMOUSEMOVE + WM_MOUSEMOVE;

		if(GetKeyState(VK_CONTROL) < 0)
			bMod |= MOUSECTRL_MOD_CONTROL;

		if(GetKeyState(VK_SHIFT) < 0)
			bMod |= MOUSECTRL_MOD_SHIFT;
	}
	else
	{
		wKeys = GET_KEYSTATE_WPARAM(wParam);

		if(wKeys & MK_CONTROL)
			bMod |= MOUSECTRL_MOD_CONTROL;

		if(wKeys & MK_SHIFT)
			bMod |= MOUSECTRL_MOD_SHIFT;
	}

	switch(uMsg)
	{
	case WM_LBUTTONDBLCLK:
		bMod |= MOUSECTRL_MOD_DBLCLICK;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		bButton = MOUSECTRL_BUTTON_L;
		break;

	case WM_RBUTTONDBLCLK:
		bMod |= MOUSECTRL_MOD_DBLCLICK;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		bButton = MOUSECTRL_BUTTON_R;
		break;

	case WM_MBUTTONDBLCLK:
		bMod |= MOUSECTRL_MOD_DBLCLICK;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		bButton = MOUSECTRL_BUTTON_M;
		break;

	case WM_XBUTTONDBLCLK:
		bMod |= MOUSECTRL_MOD_DBLCLICK;
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
		wButton = GET_XBUTTON_WPARAM(wParam);

		if(wButton == XBUTTON1)
			bButton = MOUSECTRL_BUTTON_X1;
		else if(wButton == XBUTTON2)
			bButton = MOUSECTRL_BUTTON_X2;
		else
			return FALSE;
		break;

	default:
		return FALSE;
	}

	dwKey = MOUSECTRL_MAKEKEY(bTarget, bButton, bMod);

	pItem = FindKey(mousectrl_keyval, dwKey);
	/*if(!pItem && (bMod & (MOUSECTRL_MOD_CONTROL | MOUSECTRL_MOD_SHIFT)))
	{
		bMod &= ~(MOUSECTRL_MOD_CONTROL | MOUSECTRL_MOD_SHIFT);

		dwKey = MOUSECTRL_MAKEKEY(bTarget, bButton, bMod);

		pItem = FindKey(mousectrl_keyval, dwKey);
	}*/

	if(!pItem)
		return FALSE;

	*pnValue = pItem->nValue;
	return TRUE;
}

static BOOL MouseCtrlKeyStrToDword(int nValue, WCHAR *p, DWORD *pdw)
{
	BYTE bTarget, bButton, bMod;
	WCHAR *pString;

	// target
	pString = p;

	do
	{
		if((*p < L'A' || *p > L'Z') &&
			(*p < L'a' || *p > L'z') &&
			(*p < L'0' || *p > L'9') &&
			*p != L'_')
			return FALSE;

		p++;
	}
	while(*p != L'|');

	*p = L'\0';
	p++;

	if(lstrcmpi(pString, L"taskbaritem") == 0)
	{
		if(!MOUSECTRL_IS_VALID_TASKBARITEM(nValue))
			return FALSE;

		bTarget = MOUSECTRL_TARGET_TASKBARITEM;
	}
	else if(lstrcmpi(pString, L"emptyspace") == 0)
	{
		if(!MOUSECTRL_IS_VALID_EMPTYSPACE(nValue))
			return FALSE;

		bTarget = MOUSECTRL_TARGET_EMPTYSPACE;
	}
	else if(lstrcmpi(pString, L"volumeicon") == 0)
	{
		if(!MOUSECTRL_IS_VALID_VOLUMEICON(nValue))
			return FALSE;

		bTarget = MOUSECTRL_TARGET_VOLUMEICON;
	}
	else if(lstrcmpi(pString, L"showdesktop") == 0)
	{
		if(!MOUSECTRL_IS_VALID_SHOWDESKTOP(nValue))
			return FALSE;

		bTarget = MOUSECTRL_TARGET_SHOWDESKTOP;
	}
	else
		return FALSE;

	// button, modifiers
	bButton = 0;
	bMod = 0;

	do
	{
		pString = p;

		do
		{
			if((*p < L'A' || *p > L'Z') &&
				(*p < L'a' || *p > L'z') &&
				(*p < L'0' || *p > L'9') &&
				*p != L'_')
				return FALSE;

			p++;
		}
		while(*p != L'+' && *p != L'\0');

		if(*p != L'\0')
		{
			*p = L'\0';
			p++;
		}
		else
			p = NULL;

		if(lstrcmpi(pString, L"ctrl") == 0)
		{
			if(bMod & MOUSECTRL_MOD_CONTROL)
				return FALSE;

			bMod |= MOUSECTRL_MOD_CONTROL;
		}
		else if(lstrcmpi(pString, L"shift") == 0)
		{
			if(bMod & MOUSECTRL_MOD_SHIFT)
				return FALSE;

			bMod |= MOUSECTRL_MOD_SHIFT;
		}
		else
		{
			if(bButton)
				return FALSE;

			switch(*pString)
			{
			case L'L':
			case L'l':
				bButton = MOUSECTRL_BUTTON_L;
				break;

			case L'R':
			case L'r':
				bButton = MOUSECTRL_BUTTON_R;
				break;

			case L'M':
			case L'm':
				bButton = MOUSECTRL_BUTTON_M;
				break;

			case L'X':
			case L'x':
				pString++;

				if(*pString == L'1')
					bButton = MOUSECTRL_BUTTON_X1;
				else if(*pString == L'2')
					bButton = MOUSECTRL_BUTTON_X2;
				else
					return FALSE;
				break;

			default:
				return FALSE;
			}

			pString++;

			if(lstrcmpi(pString, L"dblclick") == 0)
				bMod |= MOUSECTRL_MOD_DBLCLICK;
			else if(lstrcmpi(pString, L"click") != 0)
				return FALSE;
		}
	}
	while(p);

	if(!bButton)
		return FALSE;

	*pdw = MOUSECTRL_MAKEKEY(bTarget, bButton, bMod);

	return TRUE;
}

static BOOL AddKey(MOUSECTRL_KEYVAL **ppKeyVal, DWORD dwKey, int nValue)
{
	MOUSECTRL_KEYVAL *pItem;

	HASH_FIND(hh, *ppKeyVal, &dwKey, sizeof(DWORD), pItem);
	if(!pItem)
	{
		pItem = (MOUSECTRL_KEYVAL *)HeapAlloc(GetProcessHeap(), 0, sizeof(MOUSECTRL_KEYVAL));
		if(!pItem)
			return FALSE;

		pItem->dwKey = dwKey;
		pItem->nValue = nValue;

		HASH_ADD(hh, *ppKeyVal, dwKey, sizeof(DWORD), pItem);
	}
	else
		pItem->nValue = nValue;

	return TRUE;
}

static MOUSECTRL_KEYVAL *FindKey(MOUSECTRL_KEYVAL *pKeyVal, DWORD dwKey)
{
	MOUSECTRL_KEYVAL *pItem;

	HASH_FIND(hh, pKeyVal, &dwKey, sizeof(DWORD), pItem);

	return pItem;
}

static BOOL RemoveKey(MOUSECTRL_KEYVAL **ppKeyVal, DWORD dwKey)
{
	MOUSECTRL_KEYVAL *pItem;

	HASH_FIND(hh, *ppKeyVal, &dwKey, sizeof(DWORD), pItem);
	if(!pItem)
		return FALSE;

	HASH_DEL(*ppKeyVal, pItem);
	HeapFree(GetProcessHeap(), 0, pItem);

	return TRUE;
}

static void RemoveAllKeys(MOUSECTRL_KEYVAL **ppKeyVal)
{
	MOUSECTRL_KEYVAL *pItem, *pTemp;

	HASH_ITER(hh, *ppKeyVal, pItem, pTemp)
	{
		HASH_DEL(*ppKeyVal, pItem);
		HeapFree(GetProcessHeap(), 0, pItem);
	}
}
