#include "stdafx.h"
#include "functions.h"
#include "explorer_vars.h"
#include "taskbar_inspector.h"
#include "options_ex.h"
#include "pointer_redirection.h"
#include "minhook_preloaded.h"

// superglobals
extern HINSTANCE hDllInst;
extern HWND hTweakerWnd;
extern int nOptionsEx[OPTS_EX_COUNT];
extern DWORD dwTaskbarThreadId;
extern HWND hTaskbarWnd, hTaskBandWnd, hTaskSwWnd, hTaskListWnd, hThumbnailWnd;
extern LONG_PTR lpTaskbarLongPtr, lpTaskBandLongPtr, lpTaskSwLongPtr, lpTaskListLongPtr, lpThumbnailLongPtr;
extern MODULEINFO ExplorerModuleInfo;

static HWND hGetClientRectTaskListWnd;
static LONG_PTR *closing_task_item;

// Window and task item functions
static VOID CALLBACK SwitchToWindowTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
static BOOL IsValidTaskItem(LONG_PTR *task_item);

// Taskbar functions
static void TaskbarMoveTaskInTaskList(LONG_PTR lpMMTaskListLongPtr,
	LONG_PTR *task_group, LONG_PTR *task_item_from, LONG_PTR *task_item_to);
static BOOL LLMoveThumbInGroup(LONG_PTR lpMMThumbnailLongPtr, int index_from, int index_to);
static BOOL WINAPI GetClientRectHook(void *pRetAddr, HWND hWnd, LPRECT lpRect);

// General functions

VS_FIXEDFILEINFO *GetModuleVersionInfo(HMODULE hModule, UINT *puPtrLen)
{
	HRSRC hResource;
	HGLOBAL hGlobal;
	void *pData;
	void *pFixedFileInfo;
	UINT uPtrLen;

	pFixedFileInfo = NULL;
	uPtrLen = 0;

	hResource = FindResource(hModule, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
	if(hResource != NULL)
	{
		hGlobal = LoadResource(hModule, hResource);
		if(hGlobal != NULL)
		{
			pData = LockResource(hGlobal);
			if(pData != NULL)
			{
				if(!VerQueryValue(pData, L"\\", &pFixedFileInfo, &uPtrLen) || uPtrLen == 0)
				{
					pFixedFileInfo = NULL;
					uPtrLen = 0;
				}
			}
		}
	}

	if(puPtrLen)
		*puPtrLen = uPtrLen;

	return (VS_FIXEDFILEINFO *)pFixedFileInfo;
}

void **FindImportPtr(HMODULE hFindInModule, const char *pModuleName, const char *pImportName)
{
	IMAGE_DOS_HEADER *pDosHeader;
	IMAGE_NT_HEADERS *pNtHeader;
	ULONG_PTR ImageBase;
	IMAGE_IMPORT_DESCRIPTOR *pImportDescriptor;
	ULONG_PTR *pOriginalFirstThunk;
	ULONG_PTR *pFirstThunk;
	ULONG_PTR ImageImportByName;

	// Init
	pDosHeader = (IMAGE_DOS_HEADER *)hFindInModule;
	pNtHeader = (IMAGE_NT_HEADERS *)((char *)pDosHeader + pDosHeader->e_lfanew);

	if(!pNtHeader->OptionalHeader.DataDirectory[1].VirtualAddress)
		return NULL;

	ImageBase = (ULONG_PTR)hFindInModule;
	pImportDescriptor = (IMAGE_IMPORT_DESCRIPTOR *)(ImageBase + pNtHeader->OptionalHeader.DataDirectory[1].VirtualAddress);

	// Search!
	while(pImportDescriptor->OriginalFirstThunk)
	{
		if(lstrcmpiA((char *)(ImageBase + pImportDescriptor->Name), pModuleName) == 0)
		{
			pOriginalFirstThunk = (ULONG_PTR *)(ImageBase + pImportDescriptor->OriginalFirstThunk);
			ImageImportByName = *pOriginalFirstThunk;

			pFirstThunk = (ULONG_PTR *)(ImageBase + pImportDescriptor->FirstThunk);

			while(ImageImportByName)
			{
				if(!(ImageImportByName & IMAGE_ORDINAL_FLAG))
				{
					if((ULONG_PTR)pImportName & ~0xFFFF)
					{
						ImageImportByName += sizeof(WORD);

						if(lstrcmpA((char *)(ImageBase + ImageImportByName), pImportName) == 0)
							return (void **)pFirstThunk;
					}
				}
				else
				{
					if(((ULONG_PTR)pImportName & ~0xFFFF) == 0)
						if((ImageImportByName & 0xFFFF) == (ULONG_PTR)pImportName)
							return (void **)pFirstThunk;
				}

				pOriginalFirstThunk++;
				ImageImportByName = *pOriginalFirstThunk;

				pFirstThunk++;
			}
		}

		pImportDescriptor++;
	}

	return NULL;
}

void PatchPtr(void **ppAddress, void *pPtr)
{
	DWORD dwOldProtect, dwOtherProtect;

	VirtualProtect(ppAddress, sizeof(void *), PAGE_EXECUTE_READWRITE, &dwOldProtect);
	*ppAddress = pPtr;
	VirtualProtect(ppAddress, sizeof(void *), dwOldProtect, &dwOtherProtect);
}

void PatchMemory(void *pDest, void *pSrc, size_t nSize)
{
	DWORD dwOldProtect, dwOtherProtect;

	VirtualProtect(pDest, nSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	CopyMemory(pDest, pSrc, nSize);
	VirtualProtect(pDest, nSize, dwOldProtect, &dwOtherProtect);
}

BOOL StringBeginsWith(const WCHAR *pString, const WCHAR *pBeginStr)
{
	do
	{
		if(*pBeginStr == L'\0')
			return TRUE;
	}
	while(*pString++ == *pBeginStr++);

	return FALSE;
}

void StripAmpersand(WCHAR *pDst, const WCHAR *pSrc)
{
	const WCHAR *p;

	p = pSrc;

	do
	{
		if(*p == L'&')
			p++;

		*pDst++ = *p++; // next char or an escaped ampersand
	}
	while(p[-1] != L'\0');
}

const WCHAR *LoadStrFromRsrc(UINT uStrId)
{
	const WCHAR *pStr;

	if(!LoadString(hDllInst, uStrId, (WCHAR *)&pStr, 0))
		pStr = L"(Could not load resource)";

	return pStr;
}

LRESULT SendMessageBlock(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;

	if(!SendMessageTimeout(hWnd, Msg, wParam, lParam, SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000 * 60, (DWORD_PTR *)&lResult))
		lResult = 0;

	return lResult;
}

LRESULT TweakerSendErrorMsg(const WCHAR *pText)
{
	COPYDATASTRUCT cds;
	DWORD dwProcess;

	cds.dwData = 0xDEADBEEF;
	cds.cbData = (lstrlen(pText) + 1) * sizeof(WCHAR);
	cds.lpData = (void *)pText;

	GetWindowThreadProcessId(hTweakerWnd, &dwProcess);
	AllowSetForegroundWindow(dwProcess);

	return SendMessageBlock(hTweakerWnd, WM_COPYDATA, (WPARAM)hTaskbarWnd, (LPARAM)&cds);
}

// Undocumented WinAPI functions

HWND HungWindowFromGhostWindow_(HWND hwndGhost)
{
	static FARPROC pFunc;
	HMODULE hModule;

	if(!pFunc)
	{
		hModule = GetModuleHandle(L"user32.dll");
		if(!hModule)
			return NULL;

		pFunc = GetProcAddress(hModule, "HungWindowFromGhostWindow");
		if(!pFunc)
			return NULL;
	}

	return ((HWND(WINAPI *)(HWND))pFunc)(hwndGhost);
}

HWND GhostWindowFromHungWindow_(HWND hwndHung)
{
	static FARPROC pFunc;
	HMODULE hModule;

	if(!pFunc)
	{
		hModule = GetModuleHandle(L"user32.dll");
		if(!hModule)
			return NULL;

		pFunc = GetProcAddress(hModule, "GhostWindowFromHungWindow");
		if(!pFunc)
			return NULL;
	}

	return ((HWND(WINAPI *)(HWND))pFunc)(hwndHung);
}

BOOL MirrorIcon_(HICON *phIconSmall, HICON *phIconLarge)
{
	static FARPROC pFunc;
	HMODULE hModule;

	if(!pFunc)
	{
		hModule = GetModuleHandle(L"comctl32.dll");
		if(!hModule)
			return FALSE;

		pFunc = GetProcAddress(hModule, MAKEINTRESOURCEA(414));
		if(!pFunc)
			return FALSE;
	}

	return ((BOOL(WINAPI *)(HICON *, HICON *))pFunc)(phIconSmall, phIconLarge);
}

// Window and task item functions

BOOL IsGhostWindowClass(HWND hWnd)
{
	static ATOM GhostAtom = 0xFFFF;
	ATOM WndAtom;
	WNDCLASS WndClass;

	if(GhostAtom == 0xFFFF)
		GhostAtom = (ATOM)GetClassInfo(NULL, L"Ghost", &WndClass);

	WndAtom = (ATOM)GetClassLong(hWnd, GCW_ATOM);

	return WndAtom && (WndAtom == GhostAtom);
}

HWND TryGetWndFromGhostIfHung(HWND hWnd)
{
	HWND hHungWnd;

	if(IsHungAppWindow(hWnd))
	{
		hHungWnd = HungWindowFromGhostWindow_(hWnd);
		if(hHungWnd)
			return hHungWnd;
	}

	return hWnd;
}

HWND TryGetGhostFromWndIfHung(HWND hWnd)
{
	HWND hGhostWnd;

	if(IsHungAppWindow(hWnd))
	{
		hGhostWnd = GhostWindowFromHungWindow_(hWnd);
		if(hGhostWnd)
			return hGhostWnd;
	}

	return hWnd;
}

BOOL WndGetAppId(HWND hWnd, WCHAR pAppId[MAX_PATH])
{
	IPropertyStore *pps;
	PROPVARIANT pv;
	int nLen;
	HRESULT hr;

	hr = SHGetPropertyStoreForWindow(hWnd, &IID_IPropertyStore, (void **)&pps);
	if(SUCCEEDED(hr))
	{
		hr = pps->lpVtbl->GetValue(pps, &PKEY_AppUserModel_ID, &pv);
		if(SUCCEEDED(hr))
		{
			hr = E_FAIL;
			if(pv.vt == VT_LPWSTR)
			{
				nLen = lstrlen(pv.pwszVal);
				if(nLen < MAX_PATH)
				{
					CopyMemory(pAppId, pv.pwszVal, (nLen + 1) * sizeof(WCHAR));
					hr = S_OK;
				}
			}

			PropVariantClear(&pv);
		}

		pps->lpVtbl->Release(pps);
	}

	return SUCCEEDED(hr);
}

BOOL WndSetAppId(HWND hWnd, const WCHAR *pAppId)
{
	IPropertyStore *pps;
	PROPVARIANT pv;
	HRESULT hr;

	hr = SHGetPropertyStoreForWindow(hWnd, &IID_IPropertyStore, (void **)&pps);
	if(SUCCEEDED(hr))
	{
		if(pAppId)
		{
			pv.vt = VT_LPWSTR;
			hr = SHStrDup(pAppId, &pv.pwszVal);
		}
		else
			PropVariantInit(&pv);

		if(SUCCEEDED(hr))
		{
			hr = pps->lpVtbl->SetValue(pps, &PKEY_AppUserModel_ID, &pv);
			if(SUCCEEDED(hr))
				hr = pps->lpVtbl->Commit(pps);

			PropVariantClear(&pv);
		}

		pps->lpVtbl->Release(pps);
	}

	return SUCCEEDED(hr);
}

void PostWindowScroll(HWND hWnd, int nBar, int nPages)
{
	SCROLLINFO si;

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;

	if(!GetScrollInfo(hWnd, nBar, &si))
		return;

	if(nPages > 0)
	{
		while(nPages && si.nPos < si.nMax)
		{
			if(nBar == SB_VERT)
				PostMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
			else
				PostMessage(hWnd, WM_HSCROLL, SB_PAGERIGHT, 0);

			nPages--;
			si.nPos += si.nPage;
		}
	}
	else
	{
		while(nPages && si.nPos > si.nMin)
		{
			if(nBar == SB_VERT)
				PostMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0);
			else
				PostMessage(hWnd, WM_HSCROLL, SB_PAGELEFT, 0);

			nPages++;
			si.nPos -= si.nPage;
		}
	}
}

BOOL IsWindows811ImmersiveTaskItem(LONG_PTR *task_item)
{
	if(nWinVersion == WIN_VERSION_811)
	{
		// DEF3264: CImmersiveTaskItem::GetImmersiveApp
		return *(LONG_PTR **)((BYTE *)task_item + DEF3264(0x90, 0xD0)) != NULL;
	}

	return FALSE;
}

BOOL IsMinimizedTaskItem(LONG_PTR *task_item)
{
	if(IsWindows811ImmersiveTaskItem(task_item))
	{
		BOOL bIsMinimized = TRUE;

		// DEF3264: CImmersiveTaskItem::GetImmersiveApp
		LONG_PTR *immersive_app = *(LONG_PTR **)((BYTE *)task_item + DEF3264(0x90, 0xD0));

		LONG_PTR this_ptr = (LONG_PTR)immersive_app;
		LONG_PTR *plp = *(LONG_PTR **)this_ptr;

		// AddRef
		((LONG_PTR(__stdcall *)(LONG_PTR))plp[1])(this_ptr);

		// twinui_appcore.CImmersiveApp::IsVisible
		BOOL bIsVisible;
		HRESULT hr = ((HRESULT(__stdcall *)(LONG_PTR, BOOL *))plp[9])(this_ptr, &bIsVisible);
		if(SUCCEEDED(hr) && bIsVisible)
			bIsMinimized = FALSE;

		// Release
		((LONG_PTR(__stdcall *)(LONG_PTR))plp[2])(this_ptr);

		return bIsMinimized;
	}

	return IsIconic(GetTaskItemWnd(task_item));
}

BOOL CanMinimizeTaskItem(LONG_PTR *task_item)
{
	if(IsWindows811ImmersiveTaskItem(task_item))
		return !IsMinimizedTaskItem(task_item);

	return CanMinimizeWindow(GetTaskItemWnd(task_item));
}

BOOL CanMinimizeWindow(HWND hWnd)
{
	if(nOptionsEx[OPT_EX_NOCHECK_MINIMIZE])
		return TRUE;

	if(IsIconic(hWnd) || !IsWindowEnabled(hWnd))
		return FALSE;

	long lWndStyle = GetWindowLong(hWnd, GWL_STYLE);
	if(!(lWndStyle & WS_MINIMIZEBOX))
		return FALSE;

	if((lWndStyle & (WS_CAPTION | WS_SYSMENU)) != (WS_CAPTION | WS_SYSMENU))
		return TRUE;

	HMENU hSystemMenu = GetSystemMenu(hWnd, FALSE);
	if(!hSystemMenu)
		return FALSE;

	UINT uMenuState = GetMenuState(hSystemMenu, SC_MINIMIZE, MF_BYCOMMAND);
	if(uMenuState == -1)
		return TRUE;

	return ((uMenuState & MF_DISABLED) == FALSE);
}

BOOL IsMaximizedTaskItem(LONG_PTR *task_item)
{
	if(IsWindows811ImmersiveTaskItem(task_item))
		return !IsMinimizedTaskItem(task_item);

	return IsZoomed(GetTaskItemWnd(task_item));
}

BOOL CanMaximizeTaskItem(LONG_PTR *task_item)
{
	if(IsWindows811ImmersiveTaskItem(task_item))
		return FALSE;

	return CanMaximizeWindow(GetTaskItemWnd(task_item));
}

BOOL CanMaximizeWindow(HWND hWnd)
{
	if(nOptionsEx[OPT_EX_NOCHECK_MAXIMIZE])
		return TRUE;

	if(!IsWindowEnabled(hWnd))
		return FALSE;

	long lWndStyle = GetWindowLong(hWnd, GWL_STYLE);
	if(!(lWndStyle & WS_MAXIMIZEBOX))
		return FALSE;

/*	// This method doesn't work as expected, just return TRUE
	if((lWndStyle & (WS_CAPTION | WS_SYSMENU)) != (WS_CAPTION | WS_SYSMENU))
		return TRUE;

	HMENU hSystemMenu = GetSystemMenu(hWnd, FALSE);
	if(!hSystemMenu)
		return FALSE;

	UINT uMenuState = GetMenuState(hSystemMenu, SC_MAXIMIZE, MF_BYCOMMAND);
	if(uMenuState == -1)
		return TRUE;

	return ((uMenuState & MF_DISABLED) == FALSE);
*/
	return TRUE;
}

BOOL CanRestoreTaskItem(LONG_PTR *task_item)
{
	if(IsWindows811ImmersiveTaskItem(task_item))
		return FALSE;

	return CanRestoreWindow(GetTaskItemWnd(task_item));
}

BOOL CanRestoreWindow(HWND hWnd)
{
	if(nOptionsEx[OPT_EX_NOCHECK_MAXIMIZE])
		return TRUE;

	if(!IsWindowEnabled(hWnd))
		return FALSE;

	long lWndStyle = GetWindowLong(hWnd, GWL_STYLE);
	if(!(lWndStyle & WS_MAXIMIZEBOX))
		return FALSE;

/*	// This method doesn't work as expected, just return TRUE
	if((lWndStyle & (WS_CAPTION | WS_SYSMENU)) != (WS_CAPTION | WS_SYSMENU))
		return TRUE;

	HMENU hSystemMenu = GetSystemMenu(hWnd, FALSE);
	if(!hSystemMenu)
		return FALSE;

	UINT uMenuState = GetMenuState(hSystemMenu, SC_RESTORE, MF_BYCOMMAND);
	if(uMenuState == -1)
		return TRUE;

	return ((uMenuState & MF_DISABLED) == FALSE);
*/
	return TRUE;
}

BOOL CanCloseTaskItem(LONG_PTR *task_item)
{
	if(IsWindows811ImmersiveTaskItem(task_item))
		return TRUE;

	return CanCloseWindow(GetTaskItemWnd(task_item));
}

BOOL CanCloseWindow(HWND hWnd)
{
	if(nOptionsEx[OPT_EX_NOCHECK_CLOSE])
		return TRUE;

	if(!IsWindowEnabled(hWnd))
		return FALSE;

	long lWndStyle = GetWindowLong(hWnd, GWL_STYLE);
	if((lWndStyle & (WS_CAPTION | WS_SYSMENU)) != (WS_CAPTION | WS_SYSMENU))
		return TRUE;

	HMENU hSystemMenu = GetSystemMenu(hWnd, FALSE);
	if(!hSystemMenu)
		return FALSE;

	UINT uMenuState = GetMenuState(hSystemMenu, SC_CLOSE, MF_BYCOMMAND);
	if(uMenuState == -1)
		return TRUE;

	return ((uMenuState & MF_DISABLED) == FALSE);
}

void SwitchToTaskItem(LONG_PTR *task_item)
{
	LONG_PTR this_ptr = (LONG_PTR)(lpTaskSwLongPtr + DO4_3264(0x20, 0x40, ,, ,, 0x24, 0x48));
	LONG_PTR *plp = *(LONG_PTR **)this_ptr;

	if(nWinVersion >= WIN_VERSION_811)
	{
		// CTaskBand::SwitchTo(this, task_item, true_means_bring_to_front_false_means_toggle_minimize_restore)
		FUNC_CTaskBand_SwitchTo(plp)(this_ptr, task_item, TRUE);
	}
	else
	{
		// CTaskBand::SwitchTo(this, task_group, task_item, true_means_bring_to_front_false_means_toggle_minimize_restore)
		//((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR *, BOOL))plp[7])(this_ptr, /*???*/, task_item, TRUE);

		SwitchToWindow(GetTaskItemWnd(task_item));
	}
}

void SwitchToWindow(HWND hWnd)
{
	BOOL bRestore = FALSE;
	HWND hActiveWnd = hWnd;
	HWND hTempWnd = GetLastActivePopup(GetAncestor(hWnd, GA_ROOTOWNER));

	if(hTempWnd && hTempWnd != hWnd &&
		IsWindowVisible(hTempWnd) && IsWindowEnabled(hTempWnd))
	{
		HWND hOwnerWnd = GetWindow(hTempWnd, GW_OWNER);

		while(hOwnerWnd && hOwnerWnd != hWnd)
			hOwnerWnd = GetWindow(hOwnerWnd, GW_OWNER);

		if(hOwnerWnd == hWnd)
		{
			bRestore = IsIconic(hWnd);
			hActiveWnd = hTempWnd;
		}
	}

	if(IsIconic(hActiveWnd) && !IsWindowEnabled(hActiveWnd))
	{
		ShowWindowAsync(hWnd, SW_RESTORE);
	}
	else
	{
		SwitchToThisWindow(hActiveWnd, TRUE);
		if(bRestore)
			ShowWindowAsync(hWnd, SW_RESTORE);
	}
}

void MinimizeTaskItem(LONG_PTR *task_item)
{
	if(CanMinimizeTaskItem(task_item))
	{
		LONG_PTR this_ptr = (LONG_PTR)(lpTaskSwLongPtr + DO4_3264(0x20, 0x40, ,, ,, 0x24, 0x48));
		LONG_PTR *plp = *(LONG_PTR **)this_ptr;

		if(nWinVersion >= WIN_VERSION_811)
		{
			// CTaskBand::SwitchTo(this, task_item, true_means_bring_to_front_false_means_toggle_minimize_restore)
			FUNC_CTaskBand_SwitchTo(plp)(this_ptr, task_item, FALSE);
		}
		else
		{
			// CTaskBand::SwitchTo(this, task_group, task_item, true_means_bring_to_front_false_means_toggle_minimize_restore)
			//((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR *, BOOL))plp[7])(this_ptr, /*???*/, task_item, FALSE);

			HWND hWnd = GetTaskItemWnd(task_item);

			DWORD dwProcessId;
			GetWindowThreadProcessId(hWnd, &dwProcessId);
			AllowSetForegroundWindow(dwProcessId);
			PostMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
	}
}

void MinimizeThumbTaskItem(LONG_PTR *task_item)
{
	HWND hWnd = GetTaskItemWnd(task_item);

	// DEF3264: CTaskItem::GetFlags
	DWORD dwFlags = (DWORD)task_item[DO4(2, 5, , 4)];
	if(dwFlags & 2)
	{
		// if it's a fake window for a thumbnail
		DWORD dwProcessId;
		GetWindowThreadProcessId(hWnd, &dwProcessId);
		AllowSetForegroundWindow(dwProcessId);
		PostMessage(hWnd, WM_SYSCOMMAND, SC_RESTORE, -1);
	}
	else if(CanMinimizeWindow(hWnd))
	{
		DWORD dwProcessId;
		GetWindowThreadProcessId(hWnd, &dwProcessId);
		AllowSetForegroundWindow(dwProcessId);
		PostMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
	}
}

void CloseTaskItem(LONG_PTR *task_item, BOOL bSwitchOnTimeout)
{
	if(!CanCloseTaskItem(task_item))
	{
		SwitchToTaskItem(task_item);
		return;
	}

	BOOL bAskExplorerToClose = IsWindows811ImmersiveTaskItem(task_item);

	if(!bAskExplorerToClose && nWinVersion >= WIN_VERSION_10_T1)
	{
		// Windows 10:
		// Don't switch on timeout for immersive windows, since it can cause
		// the app to re-launch after closing.
		// That probably happens because if the app is closing but the window
		// still exists, switching to it launches the app again.
		// Note: Most UWP apps don't have a closing prompt, but some do. The
		// issue also exists in Windows:
		// https://github.com/microsoft/ProjectReunion/issues/59

		LONG_PTR this_ptr = (LONG_PTR)task_item;
		LONG_PTR *plp = *(LONG_PTR **)this_ptr;

		// CWindowTaskItem::IsImmersive(this)
		BOOL bIsImmersive = FUNC_CWindowTaskItem_IsImmersive(plp)(this_ptr) != 0;
		if(bIsImmersive)
		{
			bAskExplorerToClose = TRUE;
		}
	}

	if(bAskExplorerToClose)
	{
		LONG_PTR this_ptr = (LONG_PTR)(lpTaskSwLongPtr + DO4_3264(0x20, 0x40, ,, ,, 0x24, 0x48));
		LONG_PTR *plp = *(LONG_PTR **)this_ptr;

		// CTaskBand::CloseItem(this, task_item)
		FUNC_CTaskBand_CloseItem(plp)(this_ptr, task_item);

		return;
	}

	// We could use CTaskBand::CloseItem for non-immersive windows too, but:
	// 1. CTaskBand::CloseItem is available only since Windows 8.1.1
	// 2. Our implementation is better, as CTaskBand::CloseItem brings the window to front before closing

	HWND hWnd = GetTaskItemWnd(task_item);

	DWORD dwProcessId;
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	AllowSetForegroundWindow(dwProcessId);
	PostMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);

	if(bSwitchOnTimeout && !closing_task_item)
	{
		closing_task_item = task_item;
		SetTimer(NULL, 0, 50, SwitchToWindowTimerProc);
	}
}

static VOID CALLBACK SwitchToWindowTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hWnd, idEvent);

	if(closing_task_item)
	{
		if(IsValidTaskItem(closing_task_item))
		{
			HWND hClosingWnd = GetTaskItemWnd(closing_task_item);
			if(IsWindow(hClosingWnd) && IsWindowVisible(hClosingWnd))
			{
				SwitchToTaskItem(closing_task_item);
			}
		}

		closing_task_item = NULL;
	}
}

static BOOL IsValidTaskItem(LONG_PTR *task_item)
{
	LONG_PTR *plp = (LONG_PTR *)*EV_TASK_SW_TASK_GROUPS_HDPA();
	if(!plp || (int)plp[0] == 0)
		return FALSE;

	int task_groups_count = (int)plp[0];
	LONG_PTR **task_groups = (LONG_PTR **)plp[1];

	for(int i = 0; i < task_groups_count; i++)
	{
		plp = (LONG_PTR *)task_groups[i][4];
		if(!plp || (int)plp[0] == 0)
			continue;

		int task_items_count = (int)plp[0];
		LONG_PTR **task_items = (LONG_PTR **)plp[1];

		for(int j = 0; j < task_items_count; j++)
		{
			if(task_item == task_items[j])
				return TRUE;
		}
	}

	return FALSE;
}

BOOL TerminateProcessOfTaskItem(LONG_PTR *task_item)
{
	if(IsWindows811ImmersiveTaskItem(task_item))
	{
		// Killing the process of an immersive app doesn't make the taskbar button go away
		// Also, killing it might not be a good idea because it might share a process with other apps, who knows how this stuff is implemented
		CloseTaskItem(task_item, FALSE);
		return TRUE;
	}

	HWND hWnd = GetTaskItemWnd(task_item);

	if(IsGhostWindowClass(hWnd))
	{
		HWND hHungWnd = HungWindowFromGhostWindow_(hWnd);
		if(hHungWnd)
			hWnd = hHungWnd;
	}

	DWORD dwProcessId = 0;
	if(!GetWindowThreadProcessId(hWnd, &dwProcessId) || dwProcessId == 0)
		return FALSE;

	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
	if(!hProcess)
		return FALSE;

	BOOL bSucceeded = TerminateProcess(hProcess, 0xDEAD777); // Dead by 7TT ^^

	CloseHandle(hProcess);

	return bSucceeded;
}

// Taskbar functions

BOOL IsAppIdARandomGroup(WCHAR *pAppId)
{
	return StringBeginsWith(pAppId, L"random_group_");
}

BOOL TaskbarMoveButtonInGroup(MOVE_BUTTON_IN_GROUP *p_move_button)
{
	LONG_PTR *button_group;
	int index_from;
	int index_to;
	LONG_PTR *plp;
	int buttons_count;
	LONG_PTR **buttons;
	LONG_PTR *task_group;
	LONG_PTR *task_item_from, *task_item_to;

	button_group = p_move_button->button_group;
	index_from = p_move_button->index_from;
	index_to = p_move_button->index_to;

	task_group = (LONG_PTR *)button_group[DO2(3, 4)];

	plp = (LONG_PTR *)button_group[DO2(5, 7)];

	buttons_count = (int)plp[0];
	buttons = (LONG_PTR **)plp[1];

	task_item_from = (LONG_PTR *)buttons[index_from][DO2(3, 4)];
	task_item_to = (LONG_PTR *)buttons[index_to][DO2(3, 4)];

	return TaskbarMoveTaskInGroup(task_group, task_item_from, task_item_to);
}

BOOL TaskbarMoveThumbInGroup(LONG_PTR lpMMThumbnailLongPtr, int index_from, int index_to)
{
	LONG_PTR *plp;
	int thumbs_count;
	LONG_PTR **thumbs;
	LONG_PTR *task_group;
	LONG_PTR *task_item_from, *task_item_to;

	task_group = *EV_MM_THUMBNAIL_TASK_GROUP(lpMMThumbnailLongPtr);

	plp = (LONG_PTR *)*EV_MM_THUMBNAIL_THUMBNAILS_HDPA(lpMMThumbnailLongPtr);

	thumbs_count = (int)plp[0];
	thumbs = (LONG_PTR **)plp[1];

	task_item_from = (LONG_PTR *)thumbs[index_from][DO2(2, 3)];
	task_item_to = (LONG_PTR *)thumbs[index_to][DO2(2, 3)];

	return TaskbarMoveTaskInGroup(task_group, task_item_from, task_item_to);
}

BOOL TaskbarMoveTaskInGroup(LONG_PTR *task_group, LONG_PTR *task_item_from, LONG_PTR *task_item_to)
{
	LONG_PTR *plp;
	int task_items_count;
	LONG_PTR **task_items;
	int index_from, index_to;
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;
	int i;

	plp = (LONG_PTR *)task_group[4];
	if(!plp)
		return FALSE;

	task_items_count = (int)plp[0];
	task_items = (LONG_PTR **)plp[1];

	for(i = 0; i < task_items_count; i++)
	{
		if(task_items[i] == task_item_from)
		{
			index_from = i;
			break;
		}
	}

	if(i == task_items_count)
		return FALSE;

	for(i = 0; i < task_items_count; i++)
	{
		if(task_items[i] == task_item_to)
		{
			index_to = i;
			break;
		}
	}

	if(i == task_items_count)
		return FALSE;

	InspectorBeforeTaskbarRefresh();

	plp = task_items[index_from];
	if(index_from < index_to)
		memmove(&task_items[index_from], &task_items[index_from + 1], (index_to - index_from) * sizeof(LONG_PTR *));
	else
		memmove(&task_items[index_to + 1], &task_items[index_to], (index_from - index_to) * sizeof(LONG_PTR *));
	task_items[index_to] = plp;

	TaskbarMoveTaskInTaskList(lpTaskListLongPtr, task_group, task_item_from, task_item_to);

	lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		TaskbarMoveTaskInTaskList(lpSecondaryTaskListLongPtr, task_group, task_item_from, task_item_to);
		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}

	InspectorAfterTaskbarRefresh();

	return TRUE;
}

static void TaskbarMoveTaskInTaskList(LONG_PTR lpMMTaskListLongPtr,
	LONG_PTR *task_group, LONG_PTR *task_item_from, LONG_PTR *task_item_to)
{
	LONG_PTR *plp;
	LONG_PTR *button_group;
	int button_group_type;
	HDPA hButtonsDpa;
	int buttons_count;
	LONG_PTR **buttons;
	HWND hMMTaskListWnd;
	LONG_PTR lpMMThumbnailLongPtr;
	int thumbs_count;
	LONG_PTR **thumbs;
	int index_from, index_to;
	int i;

	// Taskbar buttons
	button_group = ButtonGroupFromTaskGroup(lpMMTaskListLongPtr, task_group);
	if(button_group)
	{
		button_group_type = (int)button_group[DO2(6, 8)];
		if(button_group_type == 1 || button_group_type == 3)
		{
			plp = (LONG_PTR *)button_group[DO2(5, 7)];

			hButtonsDpa = (HDPA)plp;

			buttons_count = (int)plp[0];
			buttons = (LONG_PTR **)plp[1];

			for(i = 0; i < buttons_count; i++)
			{
				if((LONG_PTR *)buttons[i][DO2(3, 4)] == task_item_from)
				{
					index_from = i;
					break;
				}
			}

			if(i < buttons_count)
			{
				for(i = 0; i < buttons_count; i++)
				{
					if((LONG_PTR *)buttons[i][DO2(3, 4)] == task_item_to)
					{
						index_to = i;
						break;
					}
				}
			}

			if(i < buttons_count)
			{
				plp = (LONG_PTR *)DPA_DeletePtr(hButtonsDpa, index_from);
				if(plp)
				{
					DPA_InsertPtr(hButtonsDpa, index_to, plp);

					LONG_PTR *button_group_active = *EV_MM_TASKLIST_ACTIVE_BUTTON_GROUP(lpMMTaskListLongPtr);
					if(button_group == button_group_active)
					{
						int *p_button_index_active = EV_MM_TASKLIST_ACTIVE_BUTTON_INDEX(lpMMTaskListLongPtr);
						if(*p_button_index_active == index_from || *p_button_index_active == index_to)
						{
							if(*p_button_index_active == index_from)
								*p_button_index_active = index_to;
							else // if(*p_button_index_active == index_to)
								*p_button_index_active = index_from;
						}
					}

					hMMTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);
					InvalidateRect(hMMTaskListWnd, NULL, FALSE);
				}
			}
		}
	}

	// Thumbs
	lpMMThumbnailLongPtr = *EV_MM_TASKLIST_MM_THUMBNAIL_LONG_PTR(lpMMTaskListLongPtr);

	plp = (LONG_PTR *)*EV_MM_THUMBNAIL_THUMBNAILS_HDPA(lpMMThumbnailLongPtr);
	if(plp && *EV_MM_THUMBNAIL_TASK_GROUP(lpMMThumbnailLongPtr) == task_group)
	{
		thumbs_count = (int)plp[0];
		thumbs = (LONG_PTR **)plp[1];

		for(i = 0; i < thumbs_count; i++)
		{
			if((LONG_PTR *)thumbs[i][DO2(2, 3)] == task_item_from)
			{
				index_from = i;
				break;
			}
		}

		if(i < thumbs_count)
		{
			for(i = 0; i < thumbs_count; i++)
			{
				if((LONG_PTR *)thumbs[i][DO2(2, 3)] == task_item_to)
				{
					index_to = i;
					break;
				}
			}
		}

		if(i < thumbs_count)
			LLMoveThumbInGroup(lpMMThumbnailLongPtr, index_from, index_to);
	}
}

static BOOL LLMoveThumbInGroup(LONG_PTR lpMMThumbnailLongPtr, int index_from, int index_to)
{
	LONG_PTR *plp;
	int thumbs_count;
	LONG_PTR **thumbs;
	size_t nBufferSize = EV_TASK_THUMBNAIL_SIZE - 2 * sizeof(LONG_PTR);
	BYTE bBuffer[EV_TASK_THUMBNAIL_SIZE_BUFFER_ALL_WIN_VERSIONS - 2 * sizeof(LONG_PTR)];
	int *pnTemp;
	int i;

	plp = (LONG_PTR *)*EV_MM_THUMBNAIL_THUMBNAILS_HDPA(lpMMThumbnailLongPtr);
	if(!plp)
		return FALSE;

	thumbs_count = (int)plp[0];
	thumbs = (LONG_PTR **)plp[1];

	memcpy(bBuffer, thumbs[index_from], nBufferSize);
	if(index_from < index_to)
	{
		for(i = index_from; i < index_to; i++)
			memcpy(thumbs[i], thumbs[i + 1], nBufferSize);
	}
	else
	{
		for(i = index_from; i > index_to; i--)
			memcpy(thumbs[i], thumbs[i - 1], nBufferSize);
	}
	memcpy(thumbs[index_to], bBuffer, nBufferSize);

	// Active thumb
	pnTemp = EV_MM_THUMBNAIL_ACTIVE_THUMB_INDEX(lpMMThumbnailLongPtr);
	if(*pnTemp == index_from)
		*pnTemp = index_to;
	else if(*pnTemp == index_to)
		*pnTemp = index_from;

	// Redraw flag
	*EV_MM_THUMBNAIL_REDRAW_FLAGS(lpMMThumbnailLongPtr) |= 2;

	HWND hMMThumbnailWnd = *EV_MM_THUMBNAIL_HWND(lpMMThumbnailLongPtr);
	InvalidateRect(hMMThumbnailWnd, NULL, FALSE);

	return TRUE;
}

LONG_PTR *ButtonGroupFromTaskGroup(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *task_group)
{
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	int i;

	plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
	if(!plp)
		return NULL;

	button_groups_count = (int)plp[0];
	button_groups = (LONG_PTR **)plp[1];

	for(i = 0; i < button_groups_count; i++)
		if((LONG_PTR *)button_groups[i][DO2(3, 4)] == task_group)
			return button_groups[i];

	return NULL;
}

BOOL TaskbarMoveGroup(LONG_PTR lpMMTaskListLongPtr, int index_from, int index_to)
{
	LONG_PTR *plp;
	HDPA hButtonGroupsDpa;
	LONG_PTR *dpa_ptr;
	LONG_PTR *button_group, *task_group;
	int nVisualOrder, nCount;
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;

	hButtonGroupsDpa = *EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
	if(!hButtonGroupsDpa)
		return FALSE;

	plp = (LONG_PTR *)DPA_DeletePtr(hButtonGroupsDpa, index_from);
	if(!plp)
		return FALSE;

	DPA_InsertPtr(hButtonGroupsDpa, index_to, plp);
	TaskListRecomputeLayout(lpMMTaskListLongPtr);

	if((TaskbarGetPreference(lpMMTaskListLongPtr) & 0x100) == 0)
		return TRUE;

	dpa_ptr = ((LONG_PTR **)hButtonGroupsDpa)[1];

	if(index_from < index_to)
	{
		nVisualOrder = index_from;
		nCount = index_to - index_from + 1;
	}
	else
	{
		nVisualOrder = index_to;
		nCount = index_from - index_to + 1;
	}

	dpa_ptr += nVisualOrder;

	while(nCount--)
	{
		button_group = (LONG_PTR *)*dpa_ptr;
		task_group = (LONG_PTR *)button_group[DO2(3, 4)];

		*EV_TASKGROUP_VISUAL_ORDER(task_group) = nVisualOrder;

		nVisualOrder++;
		dpa_ptr++;
	}

	if(lpMMTaskListLongPtr != lpTaskListLongPtr)
	{
		hButtonGroupsDpa = *EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpTaskListLongPtr);
		if(hButtonGroupsDpa)
		{
			plp = (LONG_PTR *)DPA_DeletePtr(hButtonGroupsDpa, index_from);
			if(plp)
			{
				DPA_InsertPtr(hButtonGroupsDpa, index_to, plp);
				TaskListRecomputeLayout(lpTaskListLongPtr);
			}
		}
	}

	lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		if(lpMMTaskListLongPtr != lpSecondaryTaskListLongPtr)
		{
			hButtonGroupsDpa = *EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpSecondaryTaskListLongPtr);
			if(hButtonGroupsDpa)
			{
				plp = (LONG_PTR *)DPA_DeletePtr(hButtonGroupsDpa, index_from);
				if(plp)
				{
					DPA_InsertPtr(hButtonGroupsDpa, index_to, plp);
					TaskListRecomputeLayout(lpSecondaryTaskListLongPtr);
				}
			}
		}

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}

	return TRUE;
}

BOOL TaskbarMoveGroupByTaskItem(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *task_item, int nMoveDelta)
{
	LONG_PTR *plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
	if(!plp)
		return FALSE;

	int button_groups_count = (int)plp[0];
	LONG_PTR **button_groups = (LONG_PTR **)plp[1];

	int index_from = -1;

	for(int i = 0; i < button_groups_count && index_from == -1; i++)
	{
		LONG_PTR *button_group = button_groups[i];

		LONG_PTR *task_group = (LONG_PTR *)button_group[DO2(3, 4)];
		plp = (LONG_PTR *)task_group[4];
		if(!plp)
			continue;

		int task_items_count = (int)plp[0];
		LONG_PTR **task_items = (LONG_PTR **)plp[1];

		for(int j = 0; j < task_items_count; j++)
		{
			if(task_items[j] == task_item)
			{
				index_from = i;
				break;
			}
		}
	}

	if(index_from == -1)
		return FALSE;

	int index_to = index_from + nMoveDelta;
	index_to %= button_groups_count;
	if(index_to < 0)
		index_to += button_groups_count;

	if(index_from == index_to)
		return TRUE;

	return TaskbarMoveGroup(lpMMTaskListLongPtr, index_from, index_to);
}

static BOOL TaskbarScrollRight(int button_groups_count, LONG_PTR **button_groups,
	int *p_button_group_index, int *p_button_index,
	int *p_buttons_count, LONG_PTR ***p_buttons)
{
	int button_group_index = *p_button_group_index;
	int button_index = *p_button_index;
	int buttons_count = *p_buttons_count;
	LONG_PTR **buttons = *p_buttons;
	LONG_PTR *plp;
	int button_group_type;

	if(button_group_index == -1 || ++button_index >= buttons_count)
	{
		do
		{
			button_group_index++;
			if(button_group_index >= button_groups_count)
				return FALSE;

			button_group_type = (int)button_groups[button_group_index][DO2(6, 8)];
		}
		while(button_group_type != 1 && button_group_type != 3);

		plp = (LONG_PTR *)button_groups[button_group_index][DO2(5, 7)];
		buttons_count = (int)plp[0];
		buttons = (LONG_PTR **)plp[1];

		button_index = 0;
	}

	*p_button_group_index = button_group_index;
	*p_button_index = button_index;
	*p_buttons_count = buttons_count;
	*p_buttons = buttons;

	return TRUE;
}

static BOOL TaskbarScrollLeft(int button_groups_count, LONG_PTR **button_groups,
	int *p_button_group_index, int *p_button_index,
	int *p_buttons_count, LONG_PTR ***p_buttons)
{
	int button_group_index = *p_button_group_index;
	int button_index = *p_button_index;
	int buttons_count = *p_buttons_count;
	LONG_PTR **buttons = *p_buttons;
	LONG_PTR *plp;
	int button_group_type;

	if(button_group_index == -1 || --button_index < 0)
	{
		if(button_group_index == -1)
			button_group_index = button_groups_count;

		do
		{
			button_group_index--;
			if(button_group_index < 0)
				return FALSE;

			button_group_type = (int)button_groups[button_group_index][DO2(6, 8)];
		}
		while(button_group_type != 1 && button_group_type != 3);

		plp = (LONG_PTR *)button_groups[button_group_index][DO2(5, 7)];
		buttons_count = (int)plp[0];
		buttons = (LONG_PTR **)plp[1];

		button_index = buttons_count - 1;
	}

	*p_button_group_index = button_group_index;
	*p_buttons_count = buttons_count;
	*p_buttons = buttons;
	*p_button_index = button_index;

	return TRUE;
}

static LONG_PTR *TaskbarScrollHelper(int button_groups_count, LONG_PTR **button_groups,
	int button_group_index_active, int button_index_active,
	int nRotates, BOOL bSkipMinimized, BOOL bWarpAround)
{
	int button_group_index, button_index;
	BOOL bRotateRight;
	int prev_button_group_index, prev_button_index;
	LONG_PTR *plp;
	int buttons_count;
	LONG_PTR **buttons;
	BOOL bScrollSucceeded;

	button_group_index = button_group_index_active;
	button_index = button_index_active;

	if(button_group_index != -1)
	{
		plp = (LONG_PTR *)button_groups[button_group_index][DO2(5, 7)];
		buttons_count = (int)plp[0];
		buttons = (LONG_PTR **)plp[1];
	}

	bRotateRight = TRUE;
	if(nRotates < 0)
	{
		bRotateRight = FALSE;
		nRotates = -nRotates;
	}

	prev_button_group_index = button_group_index;
	prev_button_index = button_index;

	while(nRotates--)
	{
		if(bRotateRight)
		{
			bScrollSucceeded = TaskbarScrollRight(button_groups_count, button_groups,
				&button_group_index, &button_index,
				&buttons_count, &buttons);
			while(bScrollSucceeded &&
				bSkipMinimized && IsMinimizedTaskItem((LONG_PTR *)buttons[button_index][DO2(3, 4)]))
			{
				bScrollSucceeded = TaskbarScrollRight(button_groups_count, button_groups,
					&button_group_index, &button_index,
					&buttons_count, &buttons);
			}
		}
		else
		{
			bScrollSucceeded = TaskbarScrollLeft(button_groups_count, button_groups,
				&button_group_index, &button_index,
				&buttons_count, &buttons);
			while(bScrollSucceeded &&
				bSkipMinimized && IsMinimizedTaskItem((LONG_PTR *)buttons[button_index][DO2(3, 4)]))
			{
				bScrollSucceeded = TaskbarScrollLeft(button_groups_count, button_groups,
					&button_group_index, &button_index,
					&buttons_count, &buttons);
			}
		}

		if(!bScrollSucceeded)
		{
			// If no results were found in the whole taskbar
			if(prev_button_group_index == -1)
			{
				return NULL;
			}

			if(bWarpAround)
			{
				// Continue from the beginning
				button_group_index = -1;
				button_index = -1;
				nRotates++;
			}
			else
			{
				// Use the last successful result and stop
				button_group_index = prev_button_group_index;
				button_index = prev_button_index;

				plp = (LONG_PTR *)button_groups[button_group_index][DO2(5, 7)];
				buttons_count = (int)plp[0];
				buttons = (LONG_PTR **)plp[1];

				break;
			}
		}

		prev_button_group_index = button_group_index;
		prev_button_index = button_index;
	}

	if(button_group_index == button_group_index_active && button_index == button_index_active)
		return NULL;

	return (LONG_PTR *)buttons[button_index][DO2(3, 4)];
}

LONG_PTR *TaskbarScroll(LONG_PTR lpMMTaskListLongPtr, int nRotates, BOOL bSkipMinimized, BOOL bWarpAround, LONG_PTR *src_task_item)
{
	LONG_PTR *button_group_active;
	int button_group_index_active, button_index_active;
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	int button_group_type;
	int buttons_count;
	LONG_PTR **buttons;
	int i, j;

	if(nRotates == 0)
		return NULL;

	plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
	if(!plp)
		return NULL;

	button_groups_count = (int)plp[0];
	button_groups = (LONG_PTR **)plp[1];

	if(src_task_item)
	{
		for(i = 0; i < button_groups_count; i++)
		{
			button_group_type = (int)button_groups[i][DO2(6, 8)];
			if(button_group_type == 1 || button_group_type == 3)
			{
				plp = (LONG_PTR *)button_groups[i][DO2(5, 7)];
				buttons_count = (int)plp[0];
				buttons = (LONG_PTR **)plp[1];

				for(j = 0; j < buttons_count; j++)
				{
					if((LONG_PTR *)buttons[j][DO2(3, 4)] == src_task_item)
					{
						button_group_index_active = i;
						button_index_active = j;
						break;
					}
				}

				if(j < buttons_count)
					break;
			}
		}

		if(i == button_groups_count)
		{
			button_group_index_active = -1;
			button_index_active = -1;
		}
	}
	else
	{
		button_group_active = *EV_MM_TASKLIST_ACTIVE_BUTTON_GROUP(lpMMTaskListLongPtr);
		button_index_active = *EV_MM_TASKLIST_ACTIVE_BUTTON_INDEX(lpMMTaskListLongPtr);

		if(button_group_active && button_index_active >= 0)
		{
			for(i = 0; i < button_groups_count; i++)
			{
				if(button_groups[i] == button_group_active)
				{
					button_group_index_active = i;
					break;
				}
			}

			if(i == button_groups_count)
				return NULL;
		}
		else
		{
			button_group_index_active = -1;
			button_index_active = -1;
		}
	}

	return TaskbarScrollHelper(button_groups_count, button_groups,
		button_group_index_active, button_index_active,
		nRotates, bSkipMinimized, bWarpAround);
}

LONG_PTR *TaskbarGetTrackedButton(LONG_PTR lpMMTaskListLongPtr)
{
	LONG_PTR *button_group_tracked = *EV_MM_TASKLIST_TRACKED_BUTTON_GROUP(lpMMTaskListLongPtr);
	int button_index_tracked = *EV_MM_TASKLIST_TRACKED_BUTTON_INDEX(lpMMTaskListLongPtr);

	if(!button_group_tracked || button_index_tracked < 0)
		return NULL;

	LONG_PTR *plp = (LONG_PTR *)button_group_tracked[DO2(5, 7)];
	LONG_PTR **buttons = (LONG_PTR **)plp[1];

	return buttons[button_index_tracked];
}

LONG_PTR *TaskbarGetTrackedTaskItem(LONG_PTR lpMMTaskListLongPtr)
{
	LONG_PTR *button = TaskbarGetTrackedButton(lpMMTaskListLongPtr);
	if(!button)
		return NULL;

	return (LONG_PTR *)button[DO2(3, 4)];
}

LONG_PTR *ThumbnailGetTrackedTaskItem(LONG_PTR lpMMThumbnailLongPtr, LONG_PTR **p_container_task_item)
{
	int tracked_thumb_index;
	LONG_PTR *plp;
	LONG_PTR *task_item;

	tracked_thumb_index = *EV_MM_THUMBNAIL_TRACKED_THUMB_INDEX(lpMMThumbnailLongPtr);
	if(tracked_thumb_index < 0)
		return NULL;

	plp = (LONG_PTR *)*EV_MM_THUMBNAIL_THUMBNAILS_HDPA(lpMMThumbnailLongPtr);
	plp = (LONG_PTR *)plp[1];
	plp = (LONG_PTR *)plp[tracked_thumb_index];
	task_item = (LONG_PTR *)plp[DO2(2, 3)];

	if(p_container_task_item)
	{
		*p_container_task_item = *EV_TASKITEM_CONTAINER_TASK_ITEM(task_item);
	}

	return task_item;
}

LONG_PTR *TaskbarGetTrackedButtonGroup(LONG_PTR lpMMTaskListLongPtr)
{
	int button_index_tracked = *EV_MM_TASKLIST_TRACKED_BUTTON_INDEX(lpMMTaskListLongPtr);
	if(button_index_tracked == -1)
	{
		// Sometimes, button_index_tracked is -1, which means that no button is tracked,
		// but tracked_button_group is still not NULL. We want to get NULL in these cases.
		return NULL;
	}

	return *EV_MM_TASKLIST_TRACKED_BUTTON_GROUP(lpMMTaskListLongPtr);
}

LONG_PTR *TaskbarGetActiveButtonGroup(LONG_PTR lpMMTaskListLongPtr)
{
	int button_index_active = *EV_MM_TASKLIST_ACTIVE_BUTTON_INDEX(lpMMTaskListLongPtr);
	if(button_index_active == -1)
	{
		// This check is here for consistency with TaskbarGetTrackedButtonGroup.
		return NULL;
	}

	return *EV_MM_TASKLIST_ACTIVE_BUTTON_GROUP(lpMMTaskListLongPtr);
}

LONG_PTR *TaskbarGetActiveButton(LONG_PTR lpMMTaskListLongPtr)
{
	LONG_PTR *button_group_active = *EV_MM_TASKLIST_ACTIVE_BUTTON_GROUP(lpMMTaskListLongPtr);
	int button_index_active = *EV_MM_TASKLIST_ACTIVE_BUTTON_INDEX(lpMMTaskListLongPtr);

	if(!button_group_active || button_index_active < 0)
	{
		return NULL;
	}

	int button_group_type = (int)button_group_active[DO2(6, 8)];
	if(button_group_type == 1 || button_group_type == 3)
	{
		LONG_PTR *plp = (LONG_PTR *)button_group_active[DO2(5, 7)];
		int buttons_count = (int)plp[0];
		LONG_PTR **buttons = (LONG_PTR **)plp[1];

		return buttons[button_index_active];
	}

	return NULL;
}

LONG_PTR *TaskbarGetActiveTaskItem(LONG_PTR lpMMTaskListLongPtr)
{
	LONG_PTR *button = TaskbarGetActiveButton(lpMMTaskListLongPtr);
	if(!button)
		return NULL;

	return (LONG_PTR *)button[DO2(3, 4)];
}

void SortButtonGroupItems(LONG_PTR *button_group)
{
	LONG_PTR *plp;
	int button_group_type;
	int buttons_count;
	LONG_PTR **buttons;
	HWND hButtonWnd;
	MOVE_BUTTON_IN_GROUP move_button;
	WCHAR szBuffer[MAX_PATH], szCmpBuffer[MAX_PATH];
	int i, j;

	button_group_type = (int)button_group[DO2(6, 8)];
	if(button_group_type == 1 || button_group_type == 3)
	{
		plp = (LONG_PTR *)button_group[DO2(5, 7)];

		buttons_count = (int)plp[0];
		buttons = (LONG_PTR **)plp[1];

		if(buttons_count > 1)
		{
			InspectorBeforeTaskbarRefresh();

			move_button.button_group = button_group;

			for(i = 0; i < buttons_count - 1; i++)
			{
				hButtonWnd = GetButtonWnd(buttons[i]);
				if(!InternalGetWindowText(hButtonWnd, szBuffer, MAX_PATH))
					*szBuffer = '\0';

				move_button.index_from = i;
				move_button.index_to = i;

				for(j = i + 1; j < buttons_count; j++)
				{
					hButtonWnd = GetButtonWnd(buttons[j]);
					if(!InternalGetWindowText(hButtonWnd, szCmpBuffer, MAX_PATH))
						*szCmpBuffer = '\0';

					if(StrCmpLogicalW(szCmpBuffer, szBuffer) < 0)
					{
						lstrcpy(szBuffer, szCmpBuffer);
						move_button.index_from = j;
					}
				}

				if(move_button.index_from != move_button.index_to)
					TaskbarMoveButtonInGroup(&move_button);
			}

			InspectorAfterTaskbarRefresh();
		}
	}
}

BOOL ButtonGroupValidate(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group)
{
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	int i;

	if(button_group)
	{
		plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
		if(plp)
		{
			button_groups_count = (int)plp[0];
			button_groups = (LONG_PTR **)plp[1];

			for(i = 0; i < button_groups_count; i++)
				if(button_group == button_groups[i])
					return TRUE;
		}
	}

	return FALSE;
}

void TaskListRecomputeLayout(LONG_PTR lpMMTaskListLongPtr)
{
	LONG_PTR this_ptr;
	LONG_PTR *plp;

	if(nWinVersion >= WIN_VERSION_8)
	{
		hGetClientRectTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);
		MHP_HookGetClientRect(GetClientRectHook);
	}

	this_ptr = lpMMTaskListLongPtr + DEF3264(0x14, 0x28);
	plp = *(LONG_PTR **)this_ptr;

	// CTaskListWnd::AutoSize(this)
	FUNC_CTaskListWnd_AutoSize(plp)(this_ptr);
}

static BOOL WINAPI GetClientRectHook(void *pRetAddr, HWND hWnd, LPRECT lpRect)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(hWnd == hGetClientRectTaskListWnd)
		{
			MHP_HookGetClientRect(NULL);

			ZeroMemory(lpRect, sizeof(RECT));
			return TRUE;
		}
	}

	return OriginalGetClientRect(hWnd, lpRect);
}

void MMTaskListRecomputeLayout(void)
{
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;

	TaskListRecomputeLayout(lpTaskListLongPtr);

	lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		TaskListRecomputeLayout(lpSecondaryTaskListLongPtr);
		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}
}

DWORD TaskbarGetPreference(LONG_PTR lpMMTaskListLongPtr)
{
/*
	0x00: Never combine
	0x01: Combine when taskbar is full
	0x03: Always combine, hide labels
	0x08: Animate
	0x10: Small buttons

	Multiple displays, show taskbar buttons on:
	0x100: All taskbars
	0x200: Main taskbar and taskbar where window is open
	0x400: Taskbar where window is open
*/

	if(lpMMTaskListLongPtr == lpTaskListLongPtr)
	{
		return *EV_TASK_SW_PREFERENCES();
	}
	else
	{
		LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpMMTaskListLongPtr);
		return *EV_SECONDARY_TASK_BAND_PREFERENCES(lpSecondaryTaskBandLongPtr);
	}
}

void ShowLivePreview(LONG_PTR lpMMThumbnailLongPtr, HWND hWnd)
{
	HWND hMMThumbnailWnd;
	LONG_PTR this_ptr;
	LONG_PTR *plp;

	hMMThumbnailWnd = *(HWND *)EV_MM_THUMBNAIL_HWND(lpMMThumbnailLongPtr);

	KillTimer(hMMThumbnailWnd, 2006);

	this_ptr = *(LONG_PTR *)(lpMMThumbnailLongPtr + DO11_3264(0x20, 0x40, ,, ,, ,, 0x28, 0x48, ,, ,, ,, ,, ,, 0x2C, 0x50));
	plp = *(LONG_PTR **)this_ptr;

	// CTaskListWnd::ShowLivePreview(this, hWnd, uFlags)
	FUNC_CTaskListWnd_ShowLivePreview(plp)(this_ptr, hWnd, 1);
}

void TaskbarToggleAutoHide(void)
{
	DWORD dwSetting;
	COPYDATASTRUCT cds;
	DWORD pdwBuffer[11];

	dwSetting = *EV_TASKBAR_SETTINGS();
	dwSetting &= 1;

	cds.dwData = 0;
	cds.cbData = 11 * sizeof(DWORD);
	cds.lpData = pdwBuffer;

	ZeroMemory(pdwBuffer, 11 * sizeof(DWORD));
	pdwBuffer[10] = 0x0A;
	pdwBuffer[8] = dwSetting ? 0 : 1;

	SendMessage(hTaskbarWnd, WM_COPYDATA, 0, (LPARAM)&cds);

/*
	// A documented method

	APPBARDATA abd;

	// both ABM_GETSTATE and ABM_SETSTATE require cbSize to be set
	abd.cbSize = sizeof(APPBARDATA);
	// get state
	abd.lParam = (UINT)SHAppBarMessage(ABM_GETSTATE, &abd);
	// toggle auto hide state
	abd.lParam ^= ABS_AUTOHIDE;
	// ABM_SETSTATE requires hWnd to be set
	abd.hWnd = hTaskbarWnd;
	// set state
	SHAppBarMessage(ABM_SETSTATE, &abd);
*/
}

int GetSecondaryTaskListCount(void)
{
	int nSecondaryTaskbarsCount = 0;

	if(nWinVersion >= WIN_VERSION_8)
	{
		LONG_PTR lp = *EV_TASK_SW_MULTI_TASK_LIST_REF();

		// DEF3264: CTaskListWndMulti::ActivateTask
		nSecondaryTaskbarsCount = *(int *)(lp + DO8_3264(0x14, 0x28, ,, ,, ,, 0x1C, 0x38, ,, 0x14, 0x28, 0x18, 0x30));
	}

	return nSecondaryTaskbarsCount;
}

LONG_PTR SecondaryTaskListGetFirstLongPtr(SECONDARY_TASK_LIST_GET *p_secondary_task_list_get)
{
	LONG_PTR lp;
	int nSecondaryTaskbarsCount;
	LONG_PTR *dpa_ptr;
	LONG_PTR lpSecondaryTaskListLongPtr;

	if(nWinVersion >= WIN_VERSION_8)
	{
		lp = *EV_TASK_SW_MULTI_TASK_LIST_REF();

		// DEF3264: CTaskListWndMulti::ActivateTask
		nSecondaryTaskbarsCount = *(int *)(lp + DO8_3264(0x14, 0x28, ,, ,, ,, 0x1C, 0x38, ,, 0x14, 0x28, 0x18, 0x30));
		if(nSecondaryTaskbarsCount > 0)
		{
			dpa_ptr = (*(LONG_PTR ***)(lp + DO8_3264(0x10, 0x20, ,, ,, ,, ,, ,, ,, 0x14, 0x28)))[1];

			lpSecondaryTaskListLongPtr = *dpa_ptr;
			lpSecondaryTaskListLongPtr -= DEF3264(0x14, 0x28);

			p_secondary_task_list_get->count = nSecondaryTaskbarsCount - 1;
			p_secondary_task_list_get->dpa_ptr = dpa_ptr + 1;

			return lpSecondaryTaskListLongPtr;
		}
	}

	return 0;
}

LONG_PTR SecondaryTaskListGetNextLongPtr(SECONDARY_TASK_LIST_GET *p_secondary_task_list_get)
{
	LONG_PTR lpSecondaryTaskListLongPtr;

	if(p_secondary_task_list_get->count > 0)
	{
		lpSecondaryTaskListLongPtr = *p_secondary_task_list_get->dpa_ptr;
		lpSecondaryTaskListLongPtr -= DEF3264(0x14, 0x28);

		p_secondary_task_list_get->count--;
		p_secondary_task_list_get->dpa_ptr++;

		return lpSecondaryTaskListLongPtr;
	}

	return 0;
}

BOOL WillExtendedUIGlom(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group)
{
	int button_group_type;
	DWORD dwTaskbarPrefs;

	button_group_type = (int)button_group[DO2(6, 8)];
	if(button_group_type == 3)
		return TRUE;

	dwTaskbarPrefs = TaskbarGetPreference(lpMMTaskListLongPtr);
	if(!(dwTaskbarPrefs & 2))
		return FALSE;

	if(nWinVersion >= WIN_VERSION_8)
	{
		LONG_PTR this_ptr;
		LONG_PTR *plp;
		int ret;

		this_ptr = (LONG_PTR)button_group;
		plp = *(LONG_PTR **)this_ptr;

		// CTaskBtnGroup::GetNumStacks(this)
		ret = FUNC_CTaskBtnGroup_GetNumStacks(plp)(this_ptr);
		if(ret > 1)
			return TRUE;

		return FALSE;
	}
	else
	{
		LONG_PTR *task_group;
		LONG_PTR this_ptr;
		LONG_PTR *plp;
		int a, b, ret;

		task_group = (LONG_PTR *)button_group[DO2(3, 4)];

		this_ptr = (LONG_PTR)task_group;
		plp = *(LONG_PTR **)this_ptr;

		// CTaskGroup::GetNumTabs(this, a, b)
		ret = FUNC_CTaskGroup_GetNumTabs(plp)(this_ptr, &a, &b);
		if(ret >= 0)
		{
			if(b > 1)
				return TRUE;
		}

		return FALSE;
	}
}

LONG_PTR MMTaskListLongPtrFromMonitor(HMONITOR hTargetMonitor)
{
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;
	LONG_PTR lp;
	HMONITOR hMonitor;
	POINT pt;

	pt.x = 0;
	pt.y = 0;

	hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
	if(hTargetMonitor == hMonitor)
		return lpTaskListLongPtr;

	lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		lp = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
		lp = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lp);

		hMonitor = *EV_SECONDARY_TASKBAR_MONITOR(lp);
		if(hTargetMonitor == hMonitor)
			return lpSecondaryTaskListLongPtr;

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}

	return lpTaskListLongPtr;
}

HWND GetTaskItemWnd(LONG_PTR *task_item)
{
	LONG_PTR this_ptr;
	LONG_PTR *plp;

	this_ptr = (LONG_PTR)task_item;
	plp = *(LONG_PTR **)this_ptr;

	// CWindowTaskItem::GetWindow(this)
	return FUNC_CWindowTaskItem_GetWindow(plp)(this_ptr);
}

HWND GetButtonWnd(LONG_PTR *button)
{
	return GetTaskItemWnd((LONG_PTR *)button[DO2(3, 4)]);
}

void OpenThumbnailPreview(LONG_PTR lpMMTaskListLongPtr)
{
	if(*EV_MM_TASKLIST_THUMB_TIMER_ID(lpMMTaskListLongPtr) == 0)
	{
		HWND hMMTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);
		SetTimer(hMMTaskListWnd, 4, 0, NULL);
		*EV_MM_TASKLIST_THUMB_TIMER_ID(lpMMTaskListLongPtr) = 4;
	}
}

void CreateNewInstance(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group)
{
	HWND hMMTaskListWnd;
	LONG_PTR *task_group;
	LONG_PTR this_ptr;
	LONG_PTR *plp;

	hMMTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);
	task_group = (LONG_PTR *)button_group[DO2(3, 4)];

	if(nWinVersion <= WIN_VERSION_7)
	{
		this_ptr = (LONG_PTR)(lpTaskSwLongPtr + DEF3264(0x20, 0x40));
		plp = *(LONG_PTR **)this_ptr;

		// CTaskBand::Launch(this, task_group)
		FUNC_CTaskBand_Launch_w7(plp)(this_ptr, task_group);
	}
	else
	{
		RECT rc;
		POINT pt;

		this_ptr = (LONG_PTR)button_group;
		plp = *(LONG_PTR **)this_ptr;

		// CTaskBtnGroup::GetLocation(this, task_item, p_rect)
		FUNC_CTaskBtnGroup_GetLocation(plp)(this_ptr, NULL, &rc);

		pt.x = rc.left;
		pt.y = rc.top;

		ClientToScreen(hMMTaskListWnd, &pt);

		this_ptr = (LONG_PTR)(lpTaskSwLongPtr + DO4_3264(0x20, 0x40, ,, ,, 0x24, 0x48));
		plp = *(LONG_PTR **)this_ptr;

		// CTaskBand::Launch(this, task_group, p_point, run_as_admin)
		FUNC_CTaskBand_Launch(plp)(this_ptr, task_group, &pt, 0);
	}

	this_ptr = (LONG_PTR)(lpMMTaskListLongPtr + DEF3264(0x1C, 0x38));
	plp = *(LONG_PTR **)this_ptr;

	// CTaskListWnd::StartAnimation(this, button_group, animation_id)
	FUNC_CTaskListWnd_StartAnimation(plp)(this_ptr, button_group, 4);
}

void DismissHoverUI(LONG_PTR lpMMTaskListLongPtr, BOOL bHideWithoutAnimation)
{
	LONG_PTR this_ptr;
	LONG_PTR *plp;

	this_ptr = (LONG_PTR)(lpMMTaskListLongPtr + DEF3264(0x14, 0x28));
	plp = *(LONG_PTR **)this_ptr;

	// CTaskListWnd::DismissHoverUI(this, hide_without_animation)
	FUNC_CTaskListWnd_DismissHoverUI(plp)(this_ptr, bHideWithoutAnimation);
}

int GetTaskbarMinWidth(void)
{
	if(nWinVersion == WIN_VERSION_7)
	{
		int nWidth = 2 * (GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXDLGFRAME));

		switch(*EV_TASKBAR_POS())
		{
		case ABE_LEFT:
		case ABE_RIGHT:
			nWidth += *EV_TASKBAR_W7_WIDTH_PADDING();
			break;
		}

		return nWidth;
	}

	// Hardcoded in CTray::GetMinSize

	HDC hdc = GetDC(NULL);
	if(!hdc)
	{
		return 62;
	}

	int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(NULL, hdc);
	return MulDiv(62, dpiX, 96);
}

int GetTaskbarMinHeight(void)
{
	if(nWinVersion == WIN_VERSION_7)
	{
		int nHeight = 2 * (GetSystemMetrics(SM_CYBORDER) + GetSystemMetrics(SM_CYDLGFRAME));

		switch(*EV_TASKBAR_POS())
		{
		case ABE_TOP:
		case ABE_BOTTOM:
			nHeight += *EV_TASKBAR_W7_HEIGHT_PADDING();
			break;
		}

		return nHeight;
	}

	// Hardcoded in CTray::GetMinSize

	HDC hdc = GetDC(NULL);
	if(!hdc)
	{
		return 40;
	}

	int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
	ReleaseDC(NULL, hdc);
	return MulDiv(40, dpiY, 96);
}

void DisableTaskbarTopmost(BOOL bDisable)
{
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;
	HWND hWndInsertAfter = bDisable ? HWND_NOTOPMOST : HWND_TOPMOST;

	SetWindowPos(hTaskbarWnd, hWndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
		LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);

		HWND hSecondaryTaskbarWnd = *EV_SECONDARY_TASKBAR_HWND(lpSecondaryTaskbarLongPtr);
		SetWindowPos(hSecondaryTaskbarWnd, hWndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}
}

void ButtonGroupExecMenuCommand(LONG_PTR *button_group, WPARAM wCommand)
{
	LONG_PTR *task_group = (LONG_PTR *)button_group[DO2(3, 4)];

	LONG_PTR this_ptr = (LONG_PTR)task_group;
	LONG_PTR *plp = *(LONG_PTR **)this_ptr;

	if(nWinVersion >= WIN_VERSION_10_T1)
	{
		LONG_PTR lpMMTaskListLongPtr = button_group[3];
		LONG_PTR lpTaskItemFilterLongPtr = *EV_MM_TASKLIST_TASK_ITEM_FILTER(lpMMTaskListLongPtr);

		// CTaskGroup::GroupMenuCommand(this, ITaskItemFilter, wCommand)
		FUNC_CTaskGroup_GroupMenuCommand(plp)(this_ptr, lpTaskItemFilterLongPtr, wCommand);
	}
	else if(nWinVersion >= WIN_VERSION_8)
	{
		HMONITOR hMonitor;

		LONG_PTR lpMMTaskListLongPtr = button_group[3];
		DWORD dwTaskbarPrefs = TaskbarGetPreference(lpMMTaskListLongPtr);
		if(dwTaskbarPrefs & 0x400 || ((dwTaskbarPrefs & 0x200) && lpMMTaskListLongPtr != lpTaskListLongPtr))
		{
			hMonitor = *EV_MM_TASKLIST_HMONITOR(lpMMTaskListLongPtr);
		}
		else
		{
			// All monitors
			hMonitor = NULL;
		}

		// CTaskGroup::GroupMenuCommand(this, hMonitor, wCommand)
		FUNC_CTaskGroup_GroupMenuCommand_w8(plp)(this_ptr, hMonitor, wCommand);
	}
	else // WIN_VERSION_7
	{
		// CTaskGroup::GroupMenuCommand(this, wCommand)
		FUNC_CTaskGroup_GroupMenuCommand_w7(plp)(this_ptr, wCommand);
	}
}

BOOL IsMMTaskListLongPtr(LONG_PTR lp)
{
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;

	if(lp == lpTaskListLongPtr)
		return TRUE;

	lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		if(lp == lpSecondaryTaskListLongPtr)
			return TRUE;

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}

	return FALSE;
}

BOOL IsTaskbarWindow(HWND hWnd)
{
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;

	if(hWnd == hTaskbarWnd)
		return TRUE;

	lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
		LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);

		HWND hSecondaryTaskbarWnd = *EV_SECONDARY_TASKBAR_HWND(lpSecondaryTaskbarLongPtr);
		if(hWnd == hSecondaryTaskbarWnd)
			return TRUE;

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}

	return FALSE;
}

int IdentifyTaskbarWindow(HWND hWnd)
{
	if(hWnd == hThumbnailWnd)
		return TASKBAR_WINDOW_THUMBNAIL;

	if(hWnd == hTaskListWnd)
		return TASKBAR_WINDOW_TASKLIST;

	if(hWnd == hTaskSwWnd)
		return TASKBAR_WINDOW_TASKSW;

	if(hWnd == hTaskBandWnd)
		return TASKBAR_WINDOW_TASKBAND;

	HWND hTrayNotifyWnd = *EV_TASKBAR_TRAY_NOTIFY_WND();

	if(hWnd == hTrayNotifyWnd || IsChild(hTrayNotifyWnd, hWnd))
		return TASKBAR_WINDOW_NOTIFY;

	if(hWnd == hTaskbarWnd || IsChild(hTaskbarWnd, hWnd))
		return TASKBAR_WINDOW_TASKBAR;

	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		LONG_PTR lpSecondaryThumbnailLongPtr = *EV_MM_TASKLIST_MM_THUMBNAIL_LONG_PTR(lpSecondaryTaskListLongPtr);

		HWND hSecondaryThumbnailWnd = *(HWND *)EV_MM_THUMBNAIL_HWND(lpSecondaryThumbnailLongPtr);
		if(hWnd == hSecondaryThumbnailWnd)
			return TASKBAR_SECONDARY_THUMBNAIL;

		HWND hSecondaryTaskListWnd = *EV_MM_TASKLIST_HWND(lpSecondaryTaskListLongPtr);
		if(hWnd == hSecondaryTaskListWnd)
			return TASKBAR_SECONDARY_TASKLIST;

		LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);

		HWND hSecondaryTaskBandWnd = *EV_SECONDARY_TASK_BAND_HWND(lpSecondaryTaskBandLongPtr);
		if(hWnd == hSecondaryTaskBandWnd)
			return TASKBAR_SECONDARY_TASKBAND;

		LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);

		if(nWinVersion >= WIN_VERSION_10_R1)
		{
			// On a secondary taskbar, all there is that can be regarded
			// as the notification area is the clock.
			LONG_PTR lpSecondaryTrayClockLongPtr = *EV_SECONDARY_TASKBAR_CLOCK_LONG_PTR(lpSecondaryTaskbarLongPtr);

			// According to crash reports, lpSecondaryTrayClockLongPtr can be NULL.
			if(lpSecondaryTrayClockLongPtr)
			{
				HWND hSecondaryClockButtonWnd = *EV_CLOCK_BUTTON_HWND(lpSecondaryTrayClockLongPtr);
				if(hWnd == hSecondaryClockButtonWnd)
					return TASKBAR_WINDOW_NOTIFY;
			}
		}

		HWND hSecondaryTaskbarWnd = *EV_SECONDARY_TASKBAR_HWND(lpSecondaryTaskbarLongPtr);
		if(hWnd == hSecondaryTaskbarWnd || IsChild(hSecondaryTaskbarWnd, hWnd))
			return TASKBAR_SECONDARY_TASKBAR;

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}

	return TASKBAR_WINDOW_UNKNOWN;
}

void DisableTaskbarsAnimation(LONG_PTR **ppMainTaskListAnimationManager, ANIMATION_MANAGER_ITEM **plpSecondaryTaskListAnimationManagers)
{
	SendMessage(hTaskListWnd, WM_SETREDRAW, FALSE, 0);

	if(nWinVersion >= WIN_VERSION_10_T1)
	{
		LONG_PTR **ppAnimationManager = EV_MM_TASKLIST_ANIMATION_MANAGER(lpTaskListLongPtr);
		*ppMainTaskListAnimationManager = *ppAnimationManager;
		*ppAnimationManager = NULL;
	}
	else
	{
		*ppMainTaskListAnimationManager = NULL;
	}

	int nSecondaryTaskListCount = GetSecondaryTaskListCount();
	if(nSecondaryTaskListCount == 0)
	{
		*plpSecondaryTaskListAnimationManagers = NULL;
		return;
	}

	ANIMATION_MANAGER_ITEM *lpSecondaryTaskListAnimationManagers = NULL;

	if(nWinVersion >= WIN_VERSION_10_T1)
	{
		lpSecondaryTaskListAnimationManagers = HeapAlloc(GetProcessHeap(), 0, (nSecondaryTaskListCount + 1) * sizeof(ANIMATION_MANAGER_ITEM));
	}

	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	int i;
	for(i = 0; lpSecondaryTaskListLongPtr; i++)
	{
		HWND hSecondaryTaskListWnd = *EV_MM_TASKLIST_HWND(lpSecondaryTaskListLongPtr);

		SendMessage(hSecondaryTaskListWnd, WM_SETREDRAW, FALSE, 0);

		if(lpSecondaryTaskListAnimationManagers)
		{
			LONG_PTR **ppAnimationManager = EV_MM_TASKLIST_ANIMATION_MANAGER(lpSecondaryTaskListLongPtr);
			lpSecondaryTaskListAnimationManagers[i].lpSecondaryTaskListLongPtr = lpSecondaryTaskListLongPtr;
			lpSecondaryTaskListAnimationManagers[i].pAnimationManager = *ppAnimationManager;
			*ppAnimationManager = NULL;
		}

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}

	if(lpSecondaryTaskListAnimationManagers)
	{
		lpSecondaryTaskListAnimationManagers[i].lpSecondaryTaskListLongPtr = 0;
		lpSecondaryTaskListAnimationManagers[i].pAnimationManager = NULL;
	}

	*plpSecondaryTaskListAnimationManagers = lpSecondaryTaskListAnimationManagers;
}

void RestoreTaskbarsAnimation(LONG_PTR *pMainTaskListAnimationManager, ANIMATION_MANAGER_ITEM *lpSecondaryTaskListAnimationManagers)
{
	if(pMainTaskListAnimationManager)
	{
		*EV_MM_TASKLIST_ANIMATION_MANAGER(lpTaskListLongPtr) = pMainTaskListAnimationManager;
		pMainTaskListAnimationManager = NULL;
	}

	SendMessage(hTaskListWnd, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(hTaskListWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		HWND hSecondaryTaskListWnd = *EV_MM_TASKLIST_HWND(lpSecondaryTaskListLongPtr);

		if(lpSecondaryTaskListAnimationManagers)
		{
			for(int i = 0; lpSecondaryTaskListAnimationManagers[i].lpSecondaryTaskListLongPtr; i++)
			{
				if(lpSecondaryTaskListAnimationManagers[i].lpSecondaryTaskListLongPtr == lpSecondaryTaskListLongPtr)
				{
					*EV_MM_TASKLIST_ANIMATION_MANAGER(lpSecondaryTaskListLongPtr) = lpSecondaryTaskListAnimationManagers[i].pAnimationManager;
					break;
				}
			}
		}

		SendMessage(hSecondaryTaskListWnd, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hSecondaryTaskListWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}

	if(lpSecondaryTaskListAnimationManagers)
	{
		HeapFree(GetProcessHeap(), 0, lpSecondaryTaskListAnimationManagers);
		lpSecondaryTaskListAnimationManagers = NULL;
	}
}

void EnableTaskbars(BOOL bEnable)
{
	EnableWindow(hTaskListWnd, bEnable);

	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		HWND hSecondaryTaskListWnd = *EV_MM_TASKLIST_HWND(lpSecondaryTaskListLongPtr);

		EnableWindow(hSecondaryTaskListWnd, bEnable);

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}
}

void Win10ShowStartMenu(LONG_PTR lpMMTaskbarLongPtr)
{
	// assert(nWinVersion >= WIN_VERSION_10_T1)

	LONG_PTR lp;
	if(lpMMTaskbarLongPtr == lpTaskbarLongPtr)
	{
		lp = *EV_TASKBAR_START_BTN_LONG_PTR();
	}
	else
	{
		lp = *EV_SECONDARY_TASKBAR_START_BTN_LONG_PTR(lpMMTaskbarLongPtr);
	}

	HWND hStartBtnWnd = *EV_START_BUTTON_HWND(lp);
	PostMessage(hStartBtnWnd, WM_KEYDOWN, VK_RETURN, 0);
}

void Win10ShowWinXPowerMenu(LONG_PTR lpMMTaskbarLongPtr)
{
	// assert(nWinVersion >= WIN_VERSION_10_T1)

	LONG_PTR lp;
	if(lpMMTaskbarLongPtr == lpTaskbarLongPtr)
	{
		lp = *EV_TASKBAR_START_BTN_LONG_PTR();
	}
	else
	{
		lp = *EV_SECONDARY_TASKBAR_START_BTN_LONG_PTR(lpMMTaskbarLongPtr);
	}

	POINT pt;
	GetCursorPos(&pt);

	HWND hStartBtnWnd = *EV_START_BUTTON_HWND(lp);
	PostMessage(hStartBtnWnd, WM_CONTEXTMENU, (WPARAM)hStartBtnWnd, MAKELPARAM(pt.x, pt.y));
}
