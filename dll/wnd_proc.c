#include "stdafx.h"
#include "wnd_proc.h"
#include "explorer_vars.h"
#include "MinHook/MinHook.h"
#include "library_load_errors.h"
#include "functions.h"
#include "options_ex.h"
#include "tweaker_messages.h"
#include "taskbar_refresh.h"
#include "mouse_button_control.h"
#include "keyboard_shortcuts.h"
#include "appid_lists.h"
#include "com_func_hook.h"
#include "mouse_hook.h"
#include "sndvol.h"
#include "taskbar_inspector.h"
#include "pointer_redirection.h"
#include "portable_settings.h"
#include "prevent_explorer_appid_changes.h"
#include "dpa_func_hook.h"
#include "minhook_preloaded.h"

typedef struct _notifyiconidentifier_internal {
	DWORD dwMagic;   // 0x34753423
	DWORD dwRequest; // 1 for (x,y) | 2 for (w,h)
	DWORD cbSize;    // 0x20
	DWORD hWndHigh;
	DWORD hWndLow;
	UINT uID;
	GUID guidItem;
} NOTIFYICONIDENTIFIER_INTERNAL;

typedef struct _new_button_lparam {
	HWND hButtonWnd;
	WCHAR szPathStr[MAX_PATH];
	WCHAR szAppIdStr[MAX_PATH];
	ITEMIDLIST *pAppItemIdList;
	HWND hThumbInsertBeforeWnd;
	HWND hThumbParentWnd;
	BOOL bSetPinnableAndLaunchable;
	BOOL bSetThumbFlag;
} NEW_BUTTON_LPARAM;

static LRESULT THISCALL_C InitTaskbarProc(THISCALL_C_THIS_ARG(LONG_PTR *this_ptr), HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void SubclassExplorerWindows(void);
static void UnsubclassExplorerWindows(void);
static DWORD InitFromExplorerThread(int pOptions[OPTS_COUNT]);
static LONG_PTR ExitFromExplorerThread(void);
static void UninitializeTweakerComponents(BOOL bDontRefreshTaskbar);
static int SetOptions(int pNewOptions[OPTS_COUNT], int pNewOptionsEx[OPTS_EX_COUNT]);
static LRESULT CALLBACK NewTaskbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK NewMMTaskbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK NewTaskBandProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK NewTaskSwProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK NewTaskListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK NewThumbnailProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK NewTrayNotifyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK NewTrayOverflowToolbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK NewTrayTemporaryToolbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK NewTrayToolbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT ProcessTrayToolbarMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK NewTrayClockProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK NewShowDesktopProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK NewW7StartButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
static LRESULT CALLBACK KeyboardShortcutsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL LaunchEmptySpaceFunction(int nFunction, LONG_PTR lpMMTaskbarLongPtr);
static BOOL LaunchCommandFromConfigString(int nFunction);
static HWND GetWindows10ImmersiveWorkerWindow(void);
static void ShortcutTaskbarActiveItemFunction(int nFunction);
static void SubclassSecondaryTaskListWindows(LONG_PTR lpSecondaryTaskListLongPtr);
static void UnsubclassSecondaryTaskListWindows(LONG_PTR lpSecondaryTaskListLongPtr);
static void GetSecondaryTaskListWindows(LONG_PTR lpSecondaryTaskListLongPtr,
	HWND *phSecondaryTaskbarWnd, HWND *phSecondaryTaskBandWnd, HWND *phSecondaryTaskListWnd, HWND *phSecondaryThumbnailWnd);
static HWND WINAPI GetCaptureHook(void *pRetAddr);
static SHORT WINAPI GetKeyStateInvertShiftHook(void *pRetAddr, int nVirtKey);
static BOOL WINAPI SetWindowPosHook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
static LONG WINAPI GetWindowLongWAlwaysTopmostHook(void *pRetAddr, HWND hWnd, int nIndex);
static BOOL WINAPI PostMessageWIgnoreTopmostHook(void *pRetAddr, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static BOOL WINAPI PostMessageWIgnoreShowDesktopHook(void *pRetAddr, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
static int WINAPI MulDivHook(int nNumber, int nNumerator, int nDenominator);
static HANDLE WINAPI SHChangeNotification_LockHook(HANDLE hChange, DWORD dwProcId, PIDLIST_ABSOLUTE **pppidl, LONG *plEvent);
static BOOL WINAPI SHChangeNotification_UnlockHook(HANDLE hLock);
static int WINAPI GetTimeFormat_W_or_Ex_Hook(LONG_PTR var1, DWORD dwFlags, SYSTEMTIME *lpTime, LPCWSTR lpFormat, LPWSTR lpTimeStr, int cchTime);
static HTHEME WINAPI OpenThemeDataHook(HWND hwnd, LPCWSTR pszClassList);
static HWND WINAPI ChildWindowFromPointHook(HWND hWndParent, POINT Point);
static HRESULT WINAPI DwmEnableBlurBehindWindowHook(void *pRetAddr, HWND hWnd, const DWM_BLURBEHIND *pBlurBehind);
static int WINAPI GetSystemMetricsHook(void *pRetAddr, int nIndex);
static LONG_PTR WINAPI SetWindowBandNoChangeTaskbarHook(HWND hWnd, LONG_PTR var2, LONG_PTR var3);
static BOOL WINAPI DPA_SortHook(HDPA hdpa, PFNDACOMPARE pfnCompare, LPARAM lParam);
static HRESULT WINAPI DrawThemeBackgroundTaskbarHook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect);
static HRESULT WINAPI DrawThemeBackgroundShowDesktopBtnHook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect);

// superglobals
HINSTANCE hDllInst;
extern LANGID language_id;
extern HANDLE hCleanEvent;
extern HWND hTweakerWnd;
extern UINT uTweakerMsg;
extern int nOptions[OPTS_COUNT];
extern int nOptionsEx[OPTS_EX_COUNT];
extern DWORD dwTaskbarThreadId;
extern HWND hTaskbarWnd, hTaskBandWnd, hTaskSwWnd, hTaskListWnd, hThumbnailWnd;
extern LONG_PTR lpTaskbarLongPtr, lpTaskBandLongPtr, lpTaskSwLongPtr, lpTaskListLongPtr, lpThumbnailLongPtr;
extern void **ppDrawThemeBackground;
extern MODULEINFO ExplorerModuleInfo;

// subclasses
static HWND hTrayNotifyWnd, hTrayOverflowToolbarWnd, hTrayTemporaryToolbarWnd, hTrayToolbarWnd, hTrayClockWnd, hShowDesktopWnd, hW7StartBtnWnd;
static volatile int wnd_proc_call_counter;

// hooks
static void **ppMulDiv;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prMulDiv);
static void **ppSHChangeNotification_Lock;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prSHChangeNotification_Lock);
static void **ppSHChangeNotification_Unlock;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prSHChangeNotification_Unlock);
static void **ppOpenThemeData;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prOpenThemeData);
static void **ppChildWindowFromPoint;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prChildWindowFromPoint);
static void **ppGetTimeFormat_W_or_Ex;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prGetTimeFormat_W_or_Ex);
static void **ppTaskbarSubWndProc;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prTaskbarSubWndProc);
//static void **ppDrawThemeBackground;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prDrawThemeBackgroundTaskbar);
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prDrawThemeBackgroundShowDesktopBtn);

static void *pSetWindowPosOriginal;
static void *pSetWindowBand, *pSetWindowBandOriginal;
static void *pDPA_SortOriginal;
static BOOL bEnableDPA_SortHook;

static BOOL bGetCaptureHooked, bGetKeyStateHooked;
static BOOL bDrawThemeBackgroundTaskbarHooked;
static int nMulDivHookLargeIconsCounter;
static int nInTrayNotifyWndWindowPosChanged;
static DWORD dwOldNumThumbnails;
static WORD wOldTrayHPadding;
static WORD wClockMilliseconds;
static BOOL bOldShowClockSeconds;
static HTHEME hTaskbarNonCompositionTheme;
static BYTE bPrevPtrDevSupported, bPrevPtrDevSupportedValid;
static BOOL bUnintializeStarted;

DWORD WndProcInit(int pOptions[OPTS_COUNT])
{
	HMODULE hExplorer;
	HANDLE hExplorerIsShellMutex;
	LONG_PTR lpTrayNotifyLongPtr;
	DWORD dwError;

	// Imports
	hExplorer = GetModuleHandle(NULL);

	GetModuleInformation(GetCurrentProcess(), hExplorer, &ExplorerModuleInfo, sizeof(MODULEINFO));

	if(nWinVersion >= WIN_VERSION_10_T1)
	{
		pSetWindowBand = (void *)GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowBand");
		if(nWinVersion >= WIN_VERSION_10_R2)
			ppMulDiv = FindImportPtr(hExplorer, "api-ms-win-core-largeinteger-l1-1-0.dll", "MulDiv");
		else
			ppMulDiv = FindImportPtr(hExplorer, "kernel32.dll", "MulDiv");
		ppSHChangeNotification_Lock = FindImportPtr(hExplorer, "shell32.dll", MAKEINTRESOURCEA(644));
		ppSHChangeNotification_Unlock = FindImportPtr(hExplorer, "shell32.dll", MAKEINTRESOURCEA(645));
	}
	else if(nWinVersion >= WIN_VERSION_81)
	{
		ppMulDiv = FindImportPtr(hExplorer, "api-ms-win-core-kernel32-legacy-l1-1-1.dll", "MulDiv");
	}
	else if(nWinVersion >= WIN_VERSION_8)
	{
		ppMulDiv = FindImportPtr(hExplorer, "api-ms-win-core-kernel32-legacy-l1-1-0.dll", "MulDiv");
	}

	if(nWinVersion == WIN_VERSION_7)
	{
		if(!(ppOpenThemeData = FindImportPtr(hExplorer, "uxtheme.dll", "OpenThemeData")))
			return LIB_ERR_FIND_IMPORT_1;

		if(!(ppChildWindowFromPoint = FindImportPtr(hExplorer, "user32.dll", "ChildWindowFromPoint")))
			return LIB_ERR_FIND_IMPORT_1;
	}

	if(nWinVersion <= WIN_VERSION_811)
	{
		if(!(ppDrawThemeBackground = FindImportPtr(hExplorer, "uxtheme.dll", "DrawThemeBackground")))
			return LIB_ERR_FIND_IMPORT_1;
	}

	if(nWinVersion <= WIN_VERSION_10_T2)
	{
		if(nWinVersion >= WIN_VERSION_8)
			ppGetTimeFormat_W_or_Ex = FindImportPtr(hExplorer, "api-ms-win-core-datetime-l1-1-1.dll", "GetTimeFormatEx");
		else
			ppGetTimeFormat_W_or_Ex = FindImportPtr(hExplorer, "kernel32.dll", "GetTimeFormatW");

		if(!ppGetTimeFormat_W_or_Ex)
			return LIB_ERR_FIND_IMPORT_2;
	}

	// Wait until explorer shell is created
	hExplorerIsShellMutex = OpenMutex(SYNCHRONIZE, FALSE, L"Local\\ExplorerIsShellMutex");
	if(hExplorerIsShellMutex)
	{
		switch(WaitForSingleObject(hExplorerIsShellMutex, INFINITE))
		{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			ReleaseMutex(hExplorerIsShellMutex);
			break;
		}

		CloseHandle(hExplorerIsShellMutex);
	}

	// Find our windows, and get their LongPtr's
	hTaskbarWnd = FindWindow(L"Shell_TrayWnd", NULL);
	if(!hTaskbarWnd)
		return LIB_ERR_WND_TASKBAR;

	lpTaskbarLongPtr = GetWindowLongPtr(hTaskbarWnd, 0);

	hTaskBandWnd = *EV_TASKBAR_TASKBAND_WND();
	if(!hTaskBandWnd)
		return LIB_ERR_WND_TASKBAND;

	lpTaskBandLongPtr = GetWindowLongPtr(hTaskBandWnd, 0);

	hTaskSwWnd = (HWND)GetProp(hTaskbarWnd, L"TaskbandHWND");
	if(!hTaskSwWnd)
		return LIB_ERR_WND_TASKSW;

	lpTaskSwLongPtr = GetWindowLongPtr(hTaskSwWnd, 0);

	hTrayNotifyWnd = *EV_TASKBAR_TRAY_NOTIFY_WND();
	if(!hTrayNotifyWnd)
		return LIB_ERR_WND_TRAYNOTIFY;

	lpTrayNotifyLongPtr = GetWindowLongPtr(hTrayNotifyWnd, 0);

	hTaskListWnd = FindWindowEx(hTaskSwWnd, NULL, L"MSTaskListWClass", NULL);
	if(!hTaskListWnd)
		return LIB_ERR_WND_TASKLIST;

	lpTaskListLongPtr = GetWindowLongPtr(hTaskListWnd, 0);

	lpThumbnailLongPtr = *EV_MM_TASKLIST_MM_THUMBNAIL_LONG_PTR(lpTaskListLongPtr);

	hThumbnailWnd = *EV_MM_THUMBNAIL_HWND(lpThumbnailLongPtr);
	if(!hThumbnailWnd)
		return LIB_ERR_WND_THUMB;

	//lpThumbnailLongPtr = GetWindowLongPtr(hThumbnailWnd, 0);

	hTrayOverflowToolbarWnd = *EV_TRAY_NOTIFY_OVERFLOW_TOOLBAR_WND(lpTrayNotifyLongPtr);
	if(!hTrayOverflowToolbarWnd)
		return LIB_ERR_WND_TRAYOVERFLOWTOOLBAR;

	hTrayTemporaryToolbarWnd = *EV_TRAY_NOTIFY_TEMPORARY_TOOLBAR_WND(lpTrayNotifyLongPtr);
	if(!hTrayTemporaryToolbarWnd)
		return LIB_ERR_WND_TRAYTEMPORARYTOOLBAR;

	hTrayToolbarWnd = *EV_TRAY_NOTIFY_TOOLBAR_WND(lpTrayNotifyLongPtr);
	if(!hTrayToolbarWnd)
		return LIB_ERR_WND_TRAYTOOLBAR;

	if(nWinVersion >= WIN_VERSION_10_R1)
	{
		LONG_PTR lp = *EV_TRAY_NOTIFY_CLOCK_LONG_PTR(lpTrayNotifyLongPtr);
		hTrayClockWnd = *EV_CLOCK_BUTTON_HWND(lp);
	}
	else
		hTrayClockWnd = *EV_TRAY_NOTIFY_CLOCK_WND(lpTrayNotifyLongPtr);

	if(!hTrayClockWnd)
		return LIB_ERR_WND_TRAYCLOCK;

	hShowDesktopWnd = *EV_TRAY_NOTIFY_SHOW_DESKTOP_WND(lpTrayNotifyLongPtr);
	if(!hShowDesktopWnd)
		return LIB_ERR_WND_SHOWDESKTOP;

	if(nWinVersion == WIN_VERSION_7)
	{
		hW7StartBtnWnd = *EV_TASKBAR_START_BTN_WND();
		if(!hW7StartBtnWnd)
			return LIB_ERR_WND_W7STARTBTN;
	}

	// Init other stuff
	ppTaskbarSubWndProc = &((*(void ***)lpTaskbarLongPtr)[2]);
	PointerRedirectionAdd(ppTaskbarSubWndProc, InitTaskbarProc, &prTaskbarSubWndProc);

	dwError = (DWORD)SendMessage(hTaskbarWnd, uTweakerMsg, (WPARAM)pOptions, MSG_DLL_INIT);
	if(dwError == 0)
	{
		PointerRedirectionRemove(ppTaskbarSubWndProc, &prTaskbarSubWndProc);
		return LIB_ERR_MSG_DLL_INIT;
	}

	dwError -= 1;
	if(dwError)
	{
		SendMessage(hTaskbarWnd, uTweakerMsg, 0, MSG_DLL_UNSUBCLASS);
		while(wnd_proc_call_counter > 0)
			Sleep(10);

		return dwError;
	}

	RefreshTaskbarHardcore_WaitTillDone();
	return 0;
}

static LRESULT THISCALL_C InitTaskbarProc(THISCALL_C_THIS_ARG(LONG_PTR *this_ptr), HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == uTweakerMsg && lParam == MSG_DLL_INIT)
	{
		PointerRedirectionRemove(ppTaskbarSubWndProc, &prTaskbarSubWndProc);

		SubclassExplorerWindows();

		return 1 + InitFromExplorerThread((int *)wParam);
	}

	return ((LRESULT(THISCALL_C *)(THISCALL_C_THIS_TYPE(LONG_PTR *), HWND, UINT, WPARAM, LPARAM))prTaskbarSubWndProc.pOriginalAddress)(THISCALL_C_THIS_VAL(this_ptr), hWnd, uMsg, wParam, lParam);
}

static void SubclassExplorerWindows(void)
{
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;

	SetWindowSubclass(hTaskbarWnd, NewTaskbarProc, 0, 0);
	SetWindowSubclass(hTaskBandWnd, NewTaskBandProc, 0, 0);
	SetWindowSubclass(hTaskSwWnd, NewTaskSwProc, 0, 0);
	SetWindowSubclass(hTaskListWnd, NewTaskListProc, 0, 0);
	SetWindowSubclass(hThumbnailWnd, NewThumbnailProc, 0, 0);
	SetWindowSubclass(hTrayNotifyWnd, NewTrayNotifyWndProc, 0, 0);
	SetWindowSubclass(hTrayOverflowToolbarWnd, NewTrayOverflowToolbarProc, 0, 0);
	SetWindowSubclass(hTrayTemporaryToolbarWnd, NewTrayTemporaryToolbarProc, 0, 0);
	SetWindowSubclass(hTrayToolbarWnd, NewTrayToolbarProc, 0, 0);
	if(nWinVersion <= WIN_VERSION_10_T2) // Since Windows 10 update 2, the clock is implemented differently
		SetWindowSubclass(hTrayClockWnd, NewTrayClockProc, 0, 0);
	SetWindowSubclass(hShowDesktopWnd, NewShowDesktopProc, 0, 0);
	if(nWinVersion == WIN_VERSION_7)
		SetWindowSubclass(hW7StartBtnWnd, NewW7StartButtonProc, 0, 0);

	lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		SubclassSecondaryTaskListWindows(lpSecondaryTaskListLongPtr);
		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}
}

static void UnsubclassExplorerWindows(void)
{
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;

	RemoveWindowSubclass(hTaskbarWnd, NewTaskbarProc, 0);
	RemoveWindowSubclass(hTaskBandWnd, NewTaskBandProc, 0);
	RemoveWindowSubclass(hTaskSwWnd, NewTaskSwProc, 0);
	RemoveWindowSubclass(hTaskListWnd, NewTaskListProc, 0);
	RemoveWindowSubclass(hThumbnailWnd, NewThumbnailProc, 0);
	RemoveWindowSubclass(hTrayNotifyWnd, NewTrayNotifyWndProc, 0);
	RemoveWindowSubclass(hTrayOverflowToolbarWnd, NewTrayOverflowToolbarProc, 0);
	RemoveWindowSubclass(hTrayTemporaryToolbarWnd, NewTrayTemporaryToolbarProc, 0);
	RemoveWindowSubclass(hTrayToolbarWnd, NewTrayToolbarProc, 0);
	if(nWinVersion <= WIN_VERSION_10_T2) // Since Windows 10 update 2, the clock is implemented differently
		RemoveWindowSubclass(hTrayClockWnd, NewTrayClockProc, 0);
	RemoveWindowSubclass(hShowDesktopWnd, NewShowDesktopProc, 0);
	if(nWinVersion == WIN_VERSION_7)
		RemoveWindowSubclass(hW7StartBtnWnd, NewW7StartButtonProc, 0);

	lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
	while(lpSecondaryTaskListLongPtr)
	{
		UnsubclassSecondaryTaskListWindows(lpSecondaryTaskListLongPtr);
		lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
	}
}

static DWORD InitFromExplorerThread(int pOptions[OPTS_COUNT])
{
	DWORD dwError;

	// Don't do that as it's explorer's thread
	// We also shouldn't use language specific strings in this thread
	//if(language_id)
	//	SetThreadUILanguage(language_id);

	dwTaskbarThreadId = GetCurrentThreadId();

	if(MH_Initialize() == MH_OK)
	{
		BOOL bPreventAppidChanges = PreventExplorerAppidChanges_Init();

		if(MHP_Initialize())
		{
			if(LoadMouseCtrl())
			{
				if(LoadKeybdShortcuts(KeyboardShortcutsProc))
				{
					if(LoadAppidLists())
					{
						if(ComFuncHook_Init(SubclassSecondaryTaskListWindows))
						{
							if(DPAFuncHook_Init())
							{
								if(RefreshTaskbarHardcore_Init())
								{
									if(MH_ApplyQueued() == MH_OK)
									{
										SndVolInit();

										int nLoadedOptionsEx[OPTS_EX_COUNT];
										LoadOptionsEx(nLoadedOptionsEx);

										BOOL bRefreshHardcore =
											!IsAppidListEmpty(AILIST_GROUP) ||
											!IsAppidListEmpty(AILIST_GROUPPINNED) ||
											!IsAppidListEmpty(AILIST_COMBINE);

										BOOL bRecomputeLayout =
											!IsAppidListEmpty(AILIST_LABEL);

										int nRefreshTaskbar = SetOptions(pOptions, nLoadedOptionsEx);

										if(nRefreshTaskbar == 2 || bRefreshHardcore)
											RefreshTaskbarHardcore();
										else if(nRefreshTaskbar == 1 || bRecomputeLayout)
											MMTaskListRecomputeLayout();

										return 0;
									}
									else
										dwError = LIB_ERR_EXTHREAD_MINHOOK_APPLY;

									RefreshTaskbarHardcore_Exit();
								}
								else
									dwError = LIB_ERR_EXTHREAD_REFRESHTASKBAR;

								DPAFuncHook_Exit();
							}
							else
								dwError = LIB_ERR_EXTHREAD_DPAHOOK;

							ComFuncHook_Exit();
						}
						else
							dwError = LIB_ERR_EXTHREAD_COMFUNCHOOK;

						FreeAppidLists();
					}
					else
						dwError = LIB_ERR_EXTHREAD_APPIDLISTS;

					FreeKeybdShortcuts();
				}
				else
					dwError = LIB_ERR_EXTHREAD_KEYBDHOTKEYS;

				FreeMouseCtrl();
			}
			else
				dwError = LIB_ERR_EXTHREAD_MOUSECTRL;

			MHP_Uninitialize();
		}
		else
			dwError = LIB_ERR_EXTHREAD_MINHOOK_PRELOADED;

		if(bPreventAppidChanges)
			PreventExplorerAppidChanges_Exit();

		MH_Uninitialize();
	}
	else
		dwError = LIB_ERR_EXTHREAD_MINHOOK;

	return dwError;
}

void WndProcExit(void)
{
	LRESULT lExited = SendMessage(hTaskbarWnd, uTweakerMsg, (WPARAM)ExitFromExplorerThread, MSG_DLL_CALLFUNC);
	if(lExited)
	{
		RefreshTaskbarHardcore_WaitTillDone();
		SendMessage(hTaskbarWnd, uTweakerMsg, (WPARAM)RefreshTaskbarHardcore_Exit, MSG_DLL_CALLFUNC);
		SendMessage(hTaskbarWnd, uTweakerMsg, 0, MSG_DLL_UNSUBCLASS);
	}
}

void WndProcWaitTillDone(void)
{
	MHP_WaitTillDone();
	DPAFuncHook_WaitTillDone();
	ComFuncHook_WaitTillDone();
	PreventExplorerAppidChanges_WaitTillDone();

	while(wnd_proc_call_counter > 0)
		Sleep(10);
}

static LONG_PTR ExitFromExplorerThread(void)
{
	if(bUnintializeStarted)
		return 0;

	bUnintializeStarted = TRUE;
	UninitializeTweakerComponents(FALSE);
	return 1;
}

static void UninitializeTweakerComponents(BOOL bDontRefreshTaskbar)
{
	CloseInspectorDlg();

	BOOL bRefreshHardcore =
		!IsAppidListEmpty(AILIST_GROUP) ||
		!IsAppidListEmpty(AILIST_GROUPPINNED) ||
		!IsAppidListEmpty(AILIST_COMBINE);

	BOOL bRecomputeLayout =
		!IsAppidListEmpty(AILIST_LABEL);

	ClearAppidLists();

	int nDefOptions[OPTS_COUNT];
	int nDefOptionsEx[OPTS_EX_COUNT];

	ZeroMemory(nDefOptions, OPTS_BUFF);
	ZeroMemory(nDefOptionsEx, OPTS_EX_BUFF);

	int nRefreshTaskbar = SetOptions(nDefOptions, nDefOptionsEx);

	if(!bDontRefreshTaskbar)
	{
		if(nRefreshTaskbar == 2 || bRefreshHardcore)
			RefreshTaskbarHardcore();
		else if(nRefreshTaskbar == 1 || bRecomputeLayout)
			MMTaskListRecomputeLayout();
	}

	SndVolUninit();
	DPAFuncHook_Exit();
	ComFuncHook_Exit();
	FreeAppidLists();
	FreeKeybdShortcuts();
	FreeMouseCtrl();
	PreventExplorerAppidChanges_Exit();
	MHP_Uninitialize();
	MH_Uninitialize();
}

static int SetOptions(int pNewOptions[OPTS_COUNT], int pNewOptionsEx[OPTS_EX_COUNT])
{
	int nOldOptions[OPTS_COUNT];
	int nOldOptionsEx[OPTS_EX_COUNT];
	int nTaskbarPos;
	int nInitialMinWidth;
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;
	BOOL bOldMouseHook, bNewMouseHook;
	int nRefreshTaskbar;
	long nNewMinTaskbarWidth, nNewMinSecondaryTaskbarWidth;
	BOOL bResizedTaskbar;
	RECT rc;

	// Save our (soon to be) old options
	CopyMemory(nOldOptions, nOptions, OPTS_BUFF);
	CopyMemory(nOldOptionsEx, nOptionsEx, OPTS_EX_BUFF);

	// Should we refresh the taskbar when done?
	if(
		(nOldOptions[OPT_GROUPING] != pNewOptions[OPT_GROUPING]) ||
		(nOldOptions[OPT_GROUPING_NOPINNED] != pNewOptions[OPT_GROUPING_NOPINNED]) ||
		(nOldOptions[OPT_COMBINING] != pNewOptions[OPT_COMBINING])
	)
	{
		nRefreshTaskbar = 2; // Hardcore refresh
	}
	else if(
		(nOldOptions[OPT_PINNED_REMOVEGAP] != pNewOptions[OPT_PINNED_REMOVEGAP]) ||
		(nOldOptionsEx[OPT_EX_MULTIROW_EQUAL_WIDTH] != pNewOptionsEx[OPT_EX_MULTIROW_EQUAL_WIDTH]) ||
		((nOldOptionsEx[OPT_EX_W10_LARGE_ICONS] == 0) != (pNewOptionsEx[OPT_EX_W10_LARGE_ICONS] == 0) && nWinVersion >= WIN_VERSION_10_T1) ||
		(nOldOptionsEx[OPT_EX_SHOW_LABELS] != pNewOptionsEx[OPT_EX_SHOW_LABELS])
	)
	{
		nRefreshTaskbar = 1; // Recompute layout
	}
	else
		nRefreshTaskbar = 0;

	// Taskbar position and initial min width
	nTaskbarPos = *EV_TASKBAR_POS();
	nInitialMinWidth = GetTaskbarMinWidth();

	// Clean up for turned off options
	if(GetCapture() != NULL)
		ReleaseCapture();

	if(nOldOptions[OPT_GROUPING_RIGHTDRAG] == 1 && pNewOptions[OPT_GROUPING_RIGHTDRAG] == 0)
	{
		if(bGetCaptureHooked)
		{
			MHP_HookGetCapture(NULL);
			bGetCaptureHooked = FALSE;
		}
	}

	if(nOldOptions[OPT_WHEEL_MINTHUMB] == 1 && pNewOptions[OPT_WHEEL_MINTHUMB] == 0)
		ComFuncSetThumbNoDismiss(FALSE);

	if(
		(
			(nOldOptions[OPT_WHEEL_VOLTASKBAR] == 1) ||
			(nOldOptions[OPT_WHEEL_VOLNOTIFY] == 1)
		) &&
		(pNewOptions[OPT_WHEEL_VOLTASKBAR] == 0) &&
		(pNewOptions[OPT_WHEEL_VOLNOTIFY] == 0)
	)
		CleanupSndVol();

	// Apply stuff
	if((nOldOptions[OPT_HOVER] == 1) != (pNewOptions[OPT_HOVER] == 1))
	{
		DWORD *pdw = EV_MM_THUMBNAIL_NUM_THUMBNAILS(lpThumbnailLongPtr);

		if(pNewOptions[OPT_HOVER] == 1)
		{
			dwOldNumThumbnails = *pdw;
			*pdw = 0;
		}
		else
			*pdw = dwOldNumThumbnails;

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
		while(lpSecondaryTaskListLongPtr)
		{
			LONG_PTR lpSecondaryThumbnailLongPtr = *EV_MM_TASKLIST_MM_THUMBNAIL_LONG_PTR(lpSecondaryTaskListLongPtr);

			pdw = EV_MM_THUMBNAIL_NUM_THUMBNAILS(lpSecondaryThumbnailLongPtr);

			*pdw = (pNewOptions[OPT_HOVER] == 1) ? 0 : dwOldNumThumbnails;

			lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
		}
	}

	bOldMouseHook =
		(nOldOptions[OPT_WHEEL_CYCLE] == 1) ||
		(nOldOptions[OPT_WHEEL_CYCLE_SKIPMIN] == 1) ||
		(nOldOptions[OPT_WHEEL_MINTASKBAR] == 1) ||
		(nOldOptions[OPT_WHEEL_MINTHUMB] == 1) ||
		(nOldOptions[OPT_WHEEL_VOLTASKBAR] == 1) ||
		(nOldOptions[OPT_WHEEL_VOLNOTIFY] == 1) ||
		nOldOptionsEx[OPT_EX_MULTIPAGE_WHEEL_SCROLL];

	bNewMouseHook =
		(pNewOptions[OPT_WHEEL_CYCLE] == 1) ||
		(pNewOptions[OPT_WHEEL_CYCLE_SKIPMIN] == 1) ||
		(pNewOptions[OPT_WHEEL_MINTASKBAR] == 1) ||
		(pNewOptions[OPT_WHEEL_MINTHUMB] == 1) ||
		(pNewOptions[OPT_WHEEL_VOLTASKBAR] == 1) ||
		(pNewOptions[OPT_WHEEL_VOLNOTIFY] == 1) ||
		pNewOptionsEx[OPT_EX_MULTIPAGE_WHEEL_SCROLL];

	if(bOldMouseHook != bNewMouseHook)
	{
		if(bNewMouseHook)
			MouseHook_Init();
		else
			MouseHook_Exit();
	}

	if(nOldOptions[OPT_OTHER_NOSTARTBTN] != pNewOptions[OPT_OTHER_NOSTARTBTN])
	{
		int nCmdShow = pNewOptions[OPT_OTHER_NOSTARTBTN] ? SW_HIDE : SW_SHOW;

		if(nWinVersion == WIN_VERSION_7)
		{
			ShowWindow(hW7StartBtnWnd, nCmdShow);
		}
		else if(nWinVersion >= WIN_VERSION_81)
		{
			HWND hStartBtnWnd;

			if(nWinVersion >= WIN_VERSION_10_T1)
			{
				LONG_PTR lp = *EV_TASKBAR_START_BTN_LONG_PTR();
				// lp can be NULL on shutdown
				hStartBtnWnd = lp ? *EV_START_BUTTON_HWND(lp) : NULL;
			}
			else
				hStartBtnWnd = *EV_TASKBAR_START_BTN_WND();

			if(hStartBtnWnd)
				ShowWindow(hStartBtnWnd, nCmdShow);

			lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
			while(lpSecondaryTaskListLongPtr)
			{
				LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
				LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);

				if(nWinVersion >= WIN_VERSION_10_T1)
				{
					LONG_PTR lp = *EV_SECONDARY_TASKBAR_START_BTN_LONG_PTR(lpSecondaryTaskbarLongPtr);
					// lp can be NULL on shutdown
					hStartBtnWnd = lp ? *EV_START_BUTTON_HWND(lp) : NULL;
				}
				else
					hStartBtnWnd = *EV_SECONDARY_TASKBAR_START_BTN_WND(lpSecondaryTaskbarLongPtr);

				if(hStartBtnWnd)
					ShowWindow(hStartBtnWnd, nCmdShow);

				lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
			}
		}
	}

	if(nOldOptionsEx[OPT_EX_TRAY_ICONS_PADDING] != pNewOptionsEx[OPT_EX_TRAY_ICONS_PADDING])
	{
		if(nWinVersion >= WIN_VERSION_10_T1)
		{
			// Handled in GetSystemMetricsHook
		}
		else
		{
			DWORD dwOldPadding = (DWORD)SendMessage(hTrayToolbarWnd, TB_GETPADDING, 0, 0);

			if(pNewOptionsEx[OPT_EX_TRAY_ICONS_PADDING])
			{
				SendMessage(hTrayToolbarWnd, TB_SETPADDING, 0,
					MAKELPARAM(pNewOptionsEx[OPT_EX_TRAY_ICONS_PADDING], HIWORD(dwOldPadding)));

				if(nOldOptionsEx[OPT_EX_TRAY_ICONS_PADDING] == 0)
					wOldTrayHPadding = LOWORD(dwOldPadding);
			}
			else
			{
				SendMessage(hTrayToolbarWnd, TB_SETPADDING, 0,
					MAKELPARAM(wOldTrayHPadding, HIWORD(dwOldPadding)));
			}

			SendMessage(hTrayToolbarWnd, WM_THEMECHANGED, 0, 0);
		}
	}

	if((nOldOptionsEx[OPT_EX_W7_SHOW_DESKTOP_CLASSIC_CORNER] != 0) != (pNewOptionsEx[OPT_EX_W7_SHOW_DESKTOP_CLASSIC_CORNER] != 0) &&
		nWinVersion == WIN_VERSION_7)
	{
		if(pNewOptionsEx[OPT_EX_W7_SHOW_DESKTOP_CLASSIC_CORNER])
			PointerRedirectionAdd(ppChildWindowFromPoint, ChildWindowFromPointHook, &prChildWindowFromPoint);
		else
			PointerRedirectionRemove(ppChildWindowFromPoint, &prChildWindowFromPoint);
	}

	if(
		(nOldOptionsEx[OPT_EX_DISABLE_TOPMOST] != 0 || (nOldOptions[OPT_OTHER_NOSTARTBTN] != 0 && nWinVersion >= WIN_VERSION_10_T1)) !=
		(pNewOptionsEx[OPT_EX_DISABLE_TOPMOST] != 0 || (pNewOptions[OPT_OTHER_NOSTARTBTN] != 0 && nWinVersion >= WIN_VERSION_10_T1))
	)
	{
		if(pNewOptionsEx[OPT_EX_DISABLE_TOPMOST] != 0 || (pNewOptions[OPT_OTHER_NOSTARTBTN] != 0 && nWinVersion >= WIN_VERSION_10_T1))
		{
			if(MH_CreateHook(SetWindowPos, SetWindowPosHook, &pSetWindowPosOriginal) == MH_OK)
			{
				if(MH_EnableHook(SetWindowPos) != MH_OK)
				{
					MH_RemoveHook(SetWindowPos);
					pSetWindowPosOriginal = NULL;
				}
			}
			else
				pSetWindowPosOriginal = NULL;
		}
		else if(pSetWindowPosOriginal)
		{
			MH_RemoveHook(SetWindowPos);
			pSetWindowPosOriginal = NULL;
		}
	}

	if(nOldOptionsEx[OPT_EX_DISABLE_TOPMOST] != pNewOptionsEx[OPT_EX_DISABLE_TOPMOST])
	{
		if(nOldOptionsEx[OPT_EX_DISABLE_TOPMOST] == 0)
		{
			DisableTaskbarTopmost(TRUE);

			MHP_HookGetWindowLongW(GetWindowLongWAlwaysTopmostHook);

			if(nWinVersion >= WIN_VERSION_10_T1 && pSetWindowBand)
			{
				if(MH_CreateHook(pSetWindowBand, SetWindowBandNoChangeTaskbarHook, &pSetWindowBandOriginal) == MH_OK)
				{
					if(MH_EnableHook(pSetWindowBand) != MH_OK)
					{
						MH_RemoveHook(pSetWindowBand);
						pSetWindowBandOriginal = NULL;
					}
				}
				else
					pSetWindowBandOriginal = NULL;
			}
		}
		else if(pNewOptionsEx[OPT_EX_DISABLE_TOPMOST] == 0)
		{
			DisableTaskbarTopmost(FALSE);

			MHP_HookGetWindowLongW(NULL);

			if(nWinVersion >= WIN_VERSION_10_T1 && pSetWindowBand && pSetWindowBandOriginal)
			{
				MH_RemoveHook(pSetWindowBand);
				pSetWindowBandOriginal = NULL;
			}
		}

		if(nOldOptionsEx[OPT_EX_DISABLE_TOPMOST] == 2)
		{
			*EV_TASKBAR_TOPMOST_EX_FLAG() = FALSE;
		}
		else if(pNewOptionsEx[OPT_EX_DISABLE_TOPMOST] == 2)
		{
			*EV_TASKBAR_TOPMOST_EX_FLAG() = TRUE;
		}
	}

	if((nOldOptionsEx[OPT_EX_CYCLE_SAME_VIRTUAL_DESKTOP] == 0) != (pNewOptionsEx[OPT_EX_CYCLE_SAME_VIRTUAL_DESKTOP] == 0) &&
		nWinVersion >= WIN_VERSION_10_T1 && nWinVersion <= WIN_VERSION_10_T2)
	{
		if(pNewOptionsEx[OPT_EX_CYCLE_SAME_VIRTUAL_DESKTOP])
		{
			if(MH_CreateHook(DPA_Sort, DPA_SortHook, &pDPA_SortOriginal) == MH_OK)
			{
				if(MH_EnableHook(DPA_Sort) != MH_OK)
				{
					MH_RemoveHook(DPA_Sort);
					pDPA_SortOriginal = NULL;
				}
			}
			else
				pDPA_SortOriginal = NULL;
		}
		else
		{
			if(pDPA_SortOriginal)
			{
				MH_RemoveHook(DPA_Sort);
				pDPA_SortOriginal = NULL;
			}
		}
	}

	if(
		((nOldOptionsEx[OPT_EX_NO_WIDTH_LIMIT] && nWinVersion >= WIN_VERSION_8) || (nOldOptionsEx[OPT_EX_W10_LARGE_ICONS] && nWinVersion >= WIN_VERSION_10_T1)) !=
		((pNewOptionsEx[OPT_EX_NO_WIDTH_LIMIT] && nWinVersion >= WIN_VERSION_8) || (pNewOptionsEx[OPT_EX_W10_LARGE_ICONS] && nWinVersion >= WIN_VERSION_10_T1))
	)
	{
		if(ppMulDiv)
		{
			if((pNewOptionsEx[OPT_EX_NO_WIDTH_LIMIT] && nWinVersion >= WIN_VERSION_8) || (pNewOptionsEx[OPT_EX_W10_LARGE_ICONS] && nWinVersion >= WIN_VERSION_10_T1))
				PointerRedirectionAdd(ppMulDiv, MulDivHook, &prMulDiv);
			else
				PointerRedirectionRemove(ppMulDiv, &prMulDiv);
		}
	}

	// See if we need to set a new width
	nNewMinTaskbarWidth = 0;
	nNewMinSecondaryTaskbarWidth = 0;
	if((nOldOptionsEx[OPT_EX_NO_WIDTH_LIMIT] != 0) != (pNewOptionsEx[OPT_EX_NO_WIDTH_LIMIT] != 0))
	{
		if(pNewOptionsEx[OPT_EX_NO_WIDTH_LIMIT])
		{
			if(PSGetSingleInt(NULL, L"last_taskbar_width", &nNewMinTaskbarWidth) != ERROR_SUCCESS)
				nNewMinTaskbarWidth = 0;

			if(PSGetSingleInt(NULL, L"last_secondary_taskbar_width", &nNewMinSecondaryTaskbarWidth) != ERROR_SUCCESS)
				nNewMinSecondaryTaskbarWidth = 0;
		}
	}

	// Copy new options!
	if(pNewOptions != nOptions)
		CopyMemory(nOptions, pNewOptions, OPTS_BUFF);

	if(pNewOptionsEx != nOptionsEx)
		CopyMemory(nOptionsEx, pNewOptionsEx, OPTS_EX_BUFF);

	if(nOldOptionsEx[OPT_EX_DISABLE_TASKBAR_TRANSPARENCY] != pNewOptionsEx[OPT_EX_DISABLE_TASKBAR_TRANSPARENCY])
	{
		if(pNewOptionsEx[OPT_EX_DISABLE_TASKBAR_TRANSPARENCY] == 2 && nWinVersion == WIN_VERSION_7)
			hTaskbarNonCompositionTheme = OpenThemeData(hTaskbarWnd, L"TaskBar2::TaskBar");

		if((nOldOptionsEx[OPT_EX_DISABLE_TASKBAR_TRANSPARENCY] != 0) != (pNewOptionsEx[OPT_EX_DISABLE_TASKBAR_TRANSPARENCY] != 0))
		{
			if(pNewOptionsEx[OPT_EX_DISABLE_TASKBAR_TRANSPARENCY])
			{
				EnableTaskbarBlurBehindWindow((nWinVersion >= WIN_VERSION_8) ? TRUE : FALSE);
				MHP_HookDwmEnableBlurBehindWindow(DwmEnableBlurBehindWindowHook);
			}
			else
			{
				MHP_HookDwmEnableBlurBehindWindow(NULL);
				EnableTaskbarBlurBehindWindow((nWinVersion >= WIN_VERSION_8) ? FALSE : TRUE);
			}
		}

		if(nOldOptionsEx[OPT_EX_DISABLE_TASKBAR_TRANSPARENCY] == 2 && nWinVersion == WIN_VERSION_7)
		{
			CloseThemeData(hTaskbarNonCompositionTheme);
			hTaskbarNonCompositionTheme = NULL;
		}

		if((nOldOptionsEx[OPT_EX_DISABLE_TASKBAR_TRANSPARENCY] == 2 || pNewOptionsEx[OPT_EX_DISABLE_TASKBAR_TRANSPARENCY] == 2) &&
			nWinVersion == WIN_VERSION_7)
		{
			SendMessage(hTaskbarWnd, WM_THEMECHANGED, 0, 0);
		}
	}

	// Do what needs to be done after setting the options

	// Reload icons if needed.
	if((nOldOptionsEx[OPT_EX_W10_LARGE_ICONS] == 0) != (pNewOptionsEx[OPT_EX_W10_LARGE_ICONS] == 0) && nWinVersion >= WIN_VERSION_10_T1)
	{
		if(ppSHChangeNotification_Lock && ppSHChangeNotification_Unlock)
		{
			PointerRedirectionAdd(ppSHChangeNotification_Lock, SHChangeNotification_LockHook, &prSHChangeNotification_Lock);
			PointerRedirectionAdd(ppSHChangeNotification_Unlock, SHChangeNotification_UnlockHook, &prSHChangeNotification_Unlock);

			SendMessage(hTaskSwWnd, 0x43A, 0xDEADBEEF, 0x12345678);

			PointerRedirectionRemove(ppSHChangeNotification_Lock, &prSHChangeNotification_Lock);
			PointerRedirectionRemove(ppSHChangeNotification_Unlock, &prSHChangeNotification_Unlock);
		}
	}

	if((nOldOptions[OPT_OTHER_EXTRAEMPTY] == 1) != (pNewOptions[OPT_OTHER_EXTRAEMPTY] == 1))
	{
		GetWindowRect(hTaskSwWnd, &rc);
		long cx = rc.right - rc.left;
		long cy = rc.bottom - rc.top;

		if(pNewOptions[OPT_OTHER_EXTRAEMPTY] == 0)
		{
			switch(nTaskbarPos)
			{
			case 1: // Is taskbar on top of the screen
			case 3: // Is taskbar on bottom of the screen
				cx += GetSystemMetrics(SM_CXICON);
				break;
			}
		}

		SetWindowPos(hTaskSwWnd, NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	if(
		(
			nOldOptions[OPT_OTHER_NOSTARTBTN] != pNewOptions[OPT_OTHER_NOSTARTBTN] ||
			nOldOptionsEx[OPT_EX_NO_START_BTN_SPACING] != pNewOptionsEx[OPT_EX_NO_START_BTN_SPACING]
		) &&
		nWinVersion == WIN_VERSION_7
	)
	{
		SendMessage(hW7StartBtnWnd, WM_THEMECHANGED, 0, 0);
	}

	if(nWinVersion <= WIN_VERSION_10_T2)
	{
		if(
			((nOldOptions[OPT_OTHER_CLOCKSHOWSEC] == 1) != (pNewOptions[OPT_OTHER_CLOCKSHOWSEC] == 1)) ||
			((nOldOptionsEx[OPT_EX_TRAY_CLOCK_FIX_WIDTH] != 0) != (pNewOptionsEx[OPT_EX_TRAY_CLOCK_FIX_WIDTH] != 0))
		)
		{
			LONG_PTR lpTrayClockLongPtr = GetWindowLongPtr(hTrayClockWnd, 0);

			if((nOldOptions[OPT_OTHER_CLOCKSHOWSEC] == 1) != (pNewOptions[OPT_OTHER_CLOCKSHOWSEC] == 1))
			{
				*EV_TRAY_CLOCK_TEXT(lpTrayClockLongPtr) = L'\0';
				SetTimer(hTrayClockWnd, 0, 0, NULL);
			}

			if(nWinVersion >= WIN_VERSION_8)
			{
				*EV_TRAY_CLOCK_CACHED_TEXT_SIZE(lpTrayClockLongPtr) = -1;
			}
		}
	}
	else
	{
		if((nOldOptions[OPT_OTHER_CLOCKSHOWSEC] == 1) != (pNewOptions[OPT_OTHER_CLOCKSHOWSEC] == 1))
		{
			LONG_PTR lpTrayClockLongPtr = GetWindowLongPtr(hTrayClockWnd, 0);
			BYTE *pb = EV_CLOCK_BUTTON_SHOW_SECONDS(lpTrayClockLongPtr);

			if(pNewOptions[OPT_OTHER_CLOCKSHOWSEC] == 1)
			{
				bOldShowClockSeconds = *pb;
				*pb = 1;
			}
			else
				*pb = bOldShowClockSeconds;

			*EV_CLOCK_BUTTON_HOURS_CACHE(lpTrayClockLongPtr) = -1;
			*EV_CLOCK_BUTTON_SIZES_CACHED(lpTrayClockLongPtr) = 0;

			HWND hClockButtonWnd = *EV_CLOCK_BUTTON_HWND(lpTrayClockLongPtr);
			InvalidateRect(hClockButtonWnd, NULL, FALSE);

			lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
			while(lpSecondaryTaskListLongPtr)
			{
				LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
				LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);
				LONG_PTR lpSecondaryTrayClockLongPtr = *EV_SECONDARY_TASKBAR_CLOCK_LONG_PTR(lpSecondaryTaskbarLongPtr);

				if(lpSecondaryTrayClockLongPtr)
				{
					BYTE *pbSecondary = EV_CLOCK_BUTTON_SHOW_SECONDS(lpSecondaryTrayClockLongPtr);
					*pbSecondary = *pb;

					*EV_CLOCK_BUTTON_HOURS_CACHE(lpSecondaryTrayClockLongPtr) = -1;
					*EV_CLOCK_BUTTON_SIZES_CACHED(lpSecondaryTrayClockLongPtr) = 0;

					HWND hSecondaryClockButtonWnd = *EV_CLOCK_BUTTON_HWND(lpSecondaryTrayClockLongPtr);
					InvalidateRect(hSecondaryClockButtonWnd, NULL, FALSE);
				}

				lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
			}
		}
	}

	bResizedTaskbar = FALSE;

	if(nNewMinTaskbarWidth)
	{
		switch(nTaskbarPos)
		{
		case 0: // Is taskbar on left of the screen
		case 2: // Is taskbar on right of the screen
			GetWindowRect(hTaskbarWnd, &rc);
			if(rc.right - rc.left == nInitialMinWidth)
			{
				if(nNewMinTaskbarWidth > 0 && nNewMinTaskbarWidth < rc.right - rc.left)
				{
					BOOL *pbIsUnlocked = EV_TASKBAR_UNLOCKED_FLAG();
					BOOL bWasLocked = FALSE;
					if(!*pbIsUnlocked)
					{
						*pbIsUnlocked = TRUE;
						bWasLocked = TRUE;
					}

					if(nTaskbarPos == 0) // Is taskbar on left of the screen
						rc.right = rc.left + nNewMinTaskbarWidth;
					else // Is taskbar on right of the screen
						rc.left = rc.right - nNewMinTaskbarWidth;

					SendMessage(hTaskbarWnd, WM_SIZING, WMSZ_LEFT, (LPARAM)&rc);
					SetWindowPos(hTaskbarWnd, NULL, 0, 0, nNewMinTaskbarWidth, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

					if(bWasLocked)
						*pbIsUnlocked = FALSE;

					bResizedTaskbar = TRUE;
				}
			}
			break;
		}
	}

	if(!bResizedTaskbar)
	{
		if(
			(nOldOptions[OPT_OTHER_CLOCKSHOWSEC] == 1) != (pNewOptions[OPT_OTHER_CLOCKSHOWSEC] == 1) ||
			(
				(nOldOptionsEx[OPT_EX_TRAY_CLOCK_FIX_WIDTH] != 0) != (pNewOptionsEx[OPT_EX_TRAY_CLOCK_FIX_WIDTH] != 0) &&
				nWinVersion <= WIN_VERSION_10_T2
			) ||
			(nOldOptions[OPT_OTHER_NOSHOWDESK] == 1) != (pNewOptions[OPT_OTHER_NOSHOWDESK] == 1) ||
			(nOldOptionsEx[OPT_EX_SHOW_DESKTOP_BUTTON_SIZE] != pNewOptionsEx[OPT_EX_SHOW_DESKTOP_BUTTON_SIZE]) ||
			(nOldOptionsEx[OPT_EX_TRAY_ICONS_PADDING] != pNewOptionsEx[OPT_EX_TRAY_ICONS_PADDING]) ||
			(
				(
					nOldOptions[OPT_OTHER_NOSTARTBTN] != pNewOptions[OPT_OTHER_NOSTARTBTN] ||
					nOldOptionsEx[OPT_EX_NO_START_BTN_SPACING] != pNewOptionsEx[OPT_EX_NO_START_BTN_SPACING]
				) &&
				(nWinVersion == WIN_VERSION_7 || (nWinVersion >= WIN_VERSION_81 && nWinVersion <= WIN_VERSION_811))
			) ||
			(
				(nOldOptionsEx[OPT_EX_DISABLE_TOPMOST] != pNewOptionsEx[OPT_EX_DISABLE_TOPMOST]) &&
				(nOldOptionsEx[OPT_EX_DISABLE_TOPMOST] == 2 || pNewOptionsEx[OPT_EX_DISABLE_TOPMOST] == 2)
			)
		)
		{
			GetClientRect(hTaskbarWnd, &rc);
			SendMessage(hTaskbarWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top));
		}
	}

	if(nNewMinSecondaryTaskbarWidth)
	{
		lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
		while(lpSecondaryTaskListLongPtr)
		{
			LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
			LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);

			HWND hSecondaryTaskbarWnd = *EV_SECONDARY_TASKBAR_HWND(lpSecondaryTaskbarLongPtr);
			int nSecondaryTaskbarPos = *EV_SECONDARY_TASKBAR_POS(lpSecondaryTaskbarLongPtr);

			switch(nSecondaryTaskbarPos)
			{
			case 0: // Is taskbar on left of the screen
			case 2: // Is taskbar on right of the screen
				GetWindowRect(hSecondaryTaskbarWnd, &rc);
				if(rc.right - rc.left == nInitialMinWidth)
				{
					if(nNewMinSecondaryTaskbarWidth > 0 && nNewMinSecondaryTaskbarWidth < rc.right - rc.left)
					{
						if(nSecondaryTaskbarPos == 0) // Is taskbar on left of the screen
							rc.right = rc.left + nNewMinSecondaryTaskbarWidth;
						else // Is taskbar on right of the screen
							rc.left = rc.right - nNewMinSecondaryTaskbarWidth;

						SendMessage(hSecondaryTaskbarWnd, WM_SIZING, WMSZ_LEFT, (LPARAM)&rc);
						SetWindowPos(hSecondaryTaskbarWnd, NULL, 0, 0, nNewMinSecondaryTaskbarWidth, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
						SendMessage(hSecondaryTaskbarWnd, WM_EXITSIZEMOVE, 0, 0);
					}
				}
				break;
			}

			lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
		}
	}

	if(
		((
			nOldOptions[OPT_OTHER_NOSTARTBTN] != pNewOptions[OPT_OTHER_NOSTARTBTN] ||
			nOldOptionsEx[OPT_EX_NO_START_BTN_SPACING] != pNewOptionsEx[OPT_EX_NO_START_BTN_SPACING]
		) && nWinVersion >= WIN_VERSION_81) ||
		((nOldOptionsEx[OPT_EX_W10_LARGE_ICONS] == 0) != (pNewOptionsEx[OPT_EX_W10_LARGE_ICONS] == 0) && nWinVersion >= WIN_VERSION_10_T1)
	)
	{
		if(nWinVersion >= WIN_VERSION_10_T1)
		{
			GetWindowRect(hTaskbarWnd, &rc);

			if(rc.left == 0 && rc.top == 0)
			{
				// Here we fix the following issue:
				// If the taskbar is on the left, Windows "optimizes" stuff by not
				// updating the positions of the search/multitasking/back buttons.
				// Therefore, we change them to zeros to force the update.

				LONG_PTR lp;

				lp = *EV_TASKBAR_SEARCH_LONG_PTR();
				if(lp)
				{
					HWND hSearchBtnWnd = *EV_TRAY_BUTTON_HWND(lp);
					SetWindowPos(hSearchBtnWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
				}

				if(nWinVersion >= WIN_VERSION_10_19H1)
				{
					lp = *EV_TASKBAR_CORTANA_LONG_PTR();
					if(lp)
					{
						HWND hCortanaBtnWnd = *EV_TRAY_BUTTON_HWND(lp);
						SetWindowPos(hCortanaBtnWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
					}
				}

				lp = *EV_TASKBAR_MULTITASKING_LONG_PTR();
				if(lp)
				{
					HWND hMultitaskBtnWnd = *EV_TRAY_BUTTON_HWND(lp);
					SetWindowPos(hMultitaskBtnWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
				}

				lp = *EV_TASKBAR_BACK_LONG_PTR();
				if(lp)
				{
					HWND hBackBtnWnd = *EV_TRAY_BUTTON_HWND(lp);
					SetWindowPos(hBackBtnWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}

			GetClientRect(hTaskbarWnd, &rc);

			WINDOWPOS windowpos;
			windowpos.hwnd = hTaskbarWnd;
			windowpos.hwndInsertAfter = NULL;
			windowpos.x = 0;
			windowpos.y = 0;
			windowpos.cx = rc.right - rc.left;
			windowpos.cy = rc.bottom - rc.top;
			windowpos.flags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE;

			SendMessage(hTaskbarWnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)&windowpos);
		}

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
		while(lpSecondaryTaskListLongPtr)
		{
			LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
			LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);

			HWND hSecondaryTaskbarWnd = *EV_SECONDARY_TASKBAR_HWND(lpSecondaryTaskbarLongPtr);

			GetClientRect(hSecondaryTaskbarWnd, &rc);

			WINDOWPOS windowpos;
			windowpos.hwnd = hSecondaryTaskbarWnd;
			windowpos.hwndInsertAfter = NULL;
			windowpos.x = 0;
			windowpos.y = 0;
			windowpos.cx = rc.right - rc.left;
			windowpos.cy = rc.bottom - rc.top;
			windowpos.flags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE;

			SendMessage(hSecondaryTaskbarWnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)&windowpos);

			lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
		}
	}
	else if(
		(nOldOptions[OPT_OTHER_CLOCKSHOWSEC] == 1) != (pNewOptions[OPT_OTHER_CLOCKSHOWSEC] == 1) &&
		nWinVersion >= WIN_VERSION_10_R1
	)
	{
		lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
		while(lpSecondaryTaskListLongPtr)
		{
			LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
			LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);

			HWND hSecondaryTaskbarWnd = *EV_SECONDARY_TASKBAR_HWND(lpSecondaryTaskbarLongPtr);

			GetClientRect(hSecondaryTaskbarWnd, &rc);

			WINDOWPOS windowpos;
			windowpos.hwnd = hSecondaryTaskbarWnd;
			windowpos.hwndInsertAfter = NULL;
			windowpos.x = 0;
			windowpos.y = 0;
			windowpos.cx = rc.right - rc.left;
			windowpos.cy = rc.bottom - rc.top;
			windowpos.flags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE;

			SendMessage(hSecondaryTaskbarWnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)&windowpos);

			lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
		}
	}

	return nRefreshTaskbar;
}

static LRESULT CALLBACK NewTaskbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	COPYDATASTRUCT *p_copydata;
	NOTIFYICONIDENTIFIER_INTERNAL *p_icon_ident;
	BOOL bProcessed;
	LRESULT result;

	wnd_proc_call_counter++;

	switch(uMsg)
	{
	case WM_COPYDATA:
		p_copydata = (COPYDATASTRUCT *)lParam;

		if((HWND)wParam == hTweakerWnd && p_copydata->dwData == 0xDEADBEEF && p_copydata->cbData == OPTS_BUFF)
		{
			switch(SetOptions((int *)p_copydata->lpData, nOptionsEx))
			{
			case 2:
				RefreshTaskbarHardcore();
				break;

			case 1:
				MMTaskListRecomputeLayout();
				break;
			}

			result = TRUE;
		}
		else
		{
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

			if(nOptions[OPT_WHEEL_VOLTASKBAR] == 1 || nOptions[OPT_WHEEL_VOLNOTIFY] == 1)
			{
				// Is this the sndvol Shell_NotifyIconGetRect function message?
				if(result == 0 && p_copydata->dwData == 0x03 && p_copydata->cbData == sizeof(NOTIFYICONIDENTIFIER_INTERNAL))
				{
					p_icon_ident = (NOTIFYICONIDENTIFIER_INTERNAL *)p_copydata->lpData;
					if(
						p_icon_ident->dwMagic == 0x34753423 &&
						(p_icon_ident->dwRequest == 0x01 || p_icon_ident->dwRequest == 0x02) &&
						p_icon_ident->cbSize == 0x20 &&
						memcmp(&p_icon_ident->guidItem, "\x73\xAE\x20\x78\xE3\x23\x29\x42\x82\xC1\xE4\x1C\xB6\x7D\x5B\x9C", sizeof(GUID)) == 0
					)
					{
						RECT rc;

						GetWindowRect(hTrayNotifyWnd, &rc);

						if(p_icon_ident->dwRequest == 0x01)
							result = MAKEWORD(rc.left, rc.top);
						else
							result = MAKEWORD(rc.right - rc.left, rc.bottom - rc.top);
					}
				}
			}
		}
		break;

	case WM_SIZING:
		bProcessed = FALSE;

		if(nOptionsEx[OPT_EX_NO_WIDTH_LIMIT] && nWinVersion == WIN_VERSION_7)
		{
			switch(*EV_TASKBAR_POS())
			{
			case 0: // Is taskbar on left of the screen
			case 2: // Is taskbar on right of the screen
				if(*EV_TASKBAR_UNLOCKED_FLAG()) // Is taskbar unlocked?
				{
					BOOL *pboolAutoposFlag = EV_TASKBAR_AUTOPOS_FLAG(); // Is taskbar NOT getting manually positioned
					BYTE *pbyteAutoposFlag = EV_TASKBAR_AUTOPOS_FLAG_BYTE();
					if(!(nWinVersion <= WIN_VERSION_811 ? *pboolAutoposFlag : *pbyteAutoposFlag))
					{
						if(nWinVersion <= WIN_VERSION_811)
							*pboolAutoposFlag = TRUE;
						else
							*pbyteAutoposFlag = TRUE;

						result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
						bProcessed = TRUE;

						if(nWinVersion <= WIN_VERSION_811)
							*pboolAutoposFlag = FALSE;
						else
							*pbyteAutoposFlag = FALSE;
					}
				}
				break;
			}
		}

		if(!bProcessed)
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_WINDOWPOSCHANGED:
		if(!(((WINDOWPOS *)lParam)->flags & SWP_NOSIZE) && nOptionsEx[OPT_EX_NO_WIDTH_LIMIT])
		{
			switch(*EV_TASKBAR_POS())
			{
			case 0: // Is taskbar on left of the screen
			case 2: // Is taskbar on right of the screen
				if((nWinVersion == WIN_VERSION_7 && nOptions[OPT_OTHER_NOSTARTBTN]) ||
					((WINDOWPOS *)lParam)->cx < GetTaskbarMinWidth())
				{
					PSSetSingleInt(NULL, L"last_taskbar_width", ((WINDOWPOS *)lParam)->cx);
				}
				else
				{
					PSRemoveSingle(NULL, L"last_taskbar_width");
				}
			}
		}

		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_WTSSESSION_CHANGE:
		if(nOptionsEx[OPT_EX_DISABLE_TOPMOST] == 2)
		{
			MHP_HookPostMessageW(PostMessageWIgnoreTopmostHook);

			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

			MHP_HookPostMessageW(NULL);
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_WININICHANGE:
		if(lstrcmp((const WCHAR *)lParam, L"TraySettings") == 0)
		{
			// Icons might be reloaded, make sure the large icons are loaded.
			nMulDivHookLargeIconsCounter++;
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
			nMulDivHookLargeIconsCounter--;
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_DESTROY:
		if(!bUnintializeStarted)
		{
			bUnintializeStarted = TRUE;

			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

			UninitializeTweakerComponents(TRUE);
			RefreshTaskbarHardcore_Exit();
			UnsubclassExplorerWindows();
			SetEvent(hCleanEvent);
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	default:
		if(uMsg == uTweakerMsg)
		{
			int nLoadedOptionsEx[OPTS_EX_COUNT];

			result = 0;

			switch(lParam)
			{
			case MSG_DLL_INSPECTOR:
				result = ShowInspectorDlg();
				break;

			case MSG_DLL_INSPECTOR_FROM_TRAY:
				if(GetTickCount() > GetInspectorCloseTime() + GetDoubleClickTime())
				{
					result = ShowInspectorDlg();
				}
				break;

			case MSG_DLL_SETLANG:
				SetThreadUILanguage((LANGID)wParam);
				language_id = (LANGID)wParam;
				break;

			case MSG_DLL_SETHWND:
				hTweakerWnd = (HWND)wParam;
				break;

			case MSG_DLL_RELOAD_OPTIONSEX:
				if(LoadOptionsEx(nLoadedOptionsEx))
				{
					switch(SetOptions(nOptions, nLoadedOptionsEx))
					{
					case 2:
						RefreshTaskbarHardcore();
						break;

					case 1:
						MMTaskListRecomputeLayout();
						break;
					}
				}
				break;

			case MSG_DLL_RELOAD_MOUSE_BUTTON_CONTROL:
				FreeMouseCtrl();
				LoadMouseCtrl();
				break;

			case MSG_DLL_RELOAD_KEYBOARD_SHORTCUTS:
				FreeKeybdShortcuts();
				LoadKeybdShortcuts(KeyboardShortcutsProc);
				break;

			case MSG_DLL_UNSUBCLASS:
				UnsubclassExplorerWindows();
				break;

			case MSG_DLL_CALLFUNC:
				result = ((LONG_PTR(*)())wParam)();
				break;

			case MSG_DLL_CALLFUNC_PARAM:
				result = ((LONG_PTR(*)(void *))((CALLFUCN_PARAM *)wParam)->pFunction)(((CALLFUCN_PARAM *)wParam)->pParam);
				break;

			case MSG_DLL_MOUSE_HOOK_WND_IDENT:
				if(wParam)
				{
					MOUSE_HOOK_WND_IDENT_PARAM *pParam = (MOUSE_HOOK_WND_IDENT_PARAM *)wParam;
					pParam->nMaybeTransWndIdent = IdentifyTaskbarWindow(pParam->hMaybeTransWnd);
					pParam->hNonTransWnd = WindowFromPoint(*pParam->ppt);

					result = 1;
				}
				break;
			}
		}
		else
			result = NewMMTaskbarProc(hWnd, uMsg, wParam, lParam, uIdSubclass, dwRefData);
		break;
	}

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK NewMMTaskbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	static int nDoubleClickOption = 0;
	LONG_PTR lpMMTaskbarLongPtr;
	LONG_PTR lp;
	int nOption;
	INPUT input;
	RECT rc;
	POINT pt;
	BOOL bMutedAndNotZero;
	BOOL bProcessed;
	LRESULT result;

	wnd_proc_call_counter++;

	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCXBUTTONDOWN:
		nDoubleClickOption = 0;
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONUP:
	case WM_XBUTTONDBLCLK:
	case WM_NCLBUTTONUP:
	case WM_NCLBUTTONDBLCLK:
	case WM_NCRBUTTONUP:
	case WM_NCRBUTTONDBLCLK:
	case WM_NCMBUTTONUP:
	case WM_NCMBUTTONDBLCLK:
	case WM_NCXBUTTONUP:
	case WM_NCXBUTTONDBLCLK:
		lpMMTaskbarLongPtr = GetWindowLongPtr(hWnd, 0);
		bProcessed = FALSE;

		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);

		if(uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
			MapWindowPoints(hWnd, NULL, &pt, 1);

		if(hWnd != hTaskbarWnd)
		{
			// Secondary taskbar (multimonitor environment)
			lp = EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_LONG_PTR_VALUE(lpMMTaskbarLongPtr);
			lp = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lp);

			GetWindowRect(*EV_SECONDARY_TASK_BAND_HWND(lp), &rc);
		}
		else
		{
			GetWindowRect(hTaskSwWnd, &rc);

			if(nOptions[OPT_OTHER_EXTRAEMPTY] == 1)
			{
				switch(*EV_TASKBAR_POS())
				{
				case 1: // Is taskbar on top of the screen
				case 3: // Is taskbar on bottom of the screen
					if(GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
						rc.left -= GetSystemMetrics(SM_CXICON);
					else
						rc.right += GetSystemMetrics(SM_CXICON);
					break;
				}
			}
		}

		if(!PtInRect(&rc, pt))
		{
			// Missed the clickable area. For Windows 10, check whether it's the
			// empty space of the hidden start button.

			nOption = 0;

			if((uMsg == WM_LBUTTONUP || uMsg == WM_NCLBUTTONUP || uMsg == WM_RBUTTONUP || uMsg == WM_NCRBUTTONUP) &&
				nOptions[OPT_OTHER_NOSTARTBTN] && nWinVersion >= WIN_VERSION_10_T1)
			{
				int nTaskbarPos;

				if(hWnd != hTaskbarWnd)
				{
					// Secondary taskbar (multimonitor environment)
					nTaskbarPos = *EV_SECONDARY_TASKBAR_POS(lpMMTaskbarLongPtr);
				}
				else
				{
					nTaskbarPos = *EV_TASKBAR_POS();
				}

				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);

				if(uMsg == WM_NCLBUTTONUP || uMsg == WM_NCRBUTTONUP)
					MapWindowPoints(NULL, hWnd, &pt, 1);

				int nSpacing = nOptionsEx[OPT_EX_NO_START_BTN_SPACING];
				BOOL bInsideStartBtnArea;

				switch(nTaskbarPos)
				{
				case 0: // Is taskbar on left of the screen
				case 2: // Is taskbar on right of the screen
					bInsideStartBtnArea = pt.y < nSpacing;
					break;

				case 1: // Is taskbar on top of the screen
				case 3: // Is taskbar on bottom of the screen
					bInsideStartBtnArea = pt.x < nSpacing;
					break;
				}

				if(bInsideStartBtnArea)
				{
					if(uMsg == WM_LBUTTONUP || uMsg == WM_NCLBUTTONUP)
						Win10ShowStartMenu(lpMMTaskbarLongPtr);
					else // if(uMsg == WM_RBUTTONUP || uMsg == WM_NCRBUTTONUP)
						Win10ShowWinXPowerMenu(lpMMTaskbarLongPtr);

					result = 0;
					bProcessed = TRUE;
				}
			}
		}
		else
		{
			if(!GetMouseCtrlValue(MOUSECTRL_TARGET_EMPTYSPACE, uMsg, wParam, &nOption))
			{
				if(uMsg == WM_LBUTTONDBLCLK || uMsg == WM_NCLBUTTONDBLCLK)
					nOption = nOptions[OPT_EMPTYDBLCLICK];
				else if(uMsg == WM_MBUTTONUP || uMsg == WM_NCMBUTTONUP)
					nOption = nOptions[OPT_EMPTYMCLICK];
				else
					nOption = 0;
			}

			if(nOption)
			{
				switch(uMsg)
				{
				case WM_LBUTTONUP:
				case WM_RBUTTONUP:
				case WM_MBUTTONUP:
				case WM_XBUTTONUP:
				case WM_NCLBUTTONUP:
				case WM_NCRBUTTONUP:
				case WM_NCMBUTTONUP:
				case WM_NCXBUTTONUP:
					if(nDoubleClickOption)
					{
						nDoubleClickOption = 0;

						switch(uMsg)
						{
						case WM_XBUTTONUP:
						case WM_NCXBUTTONUP:
							result = TRUE;
							break;

						default:
							result = 0;
							break;
						}

						bProcessed = TRUE;
					}
					break;

				case WM_LBUTTONDBLCLK:
				case WM_RBUTTONDBLCLK:
				case WM_MBUTTONDBLCLK:
				case WM_XBUTTONDBLCLK:
				case WM_NCLBUTTONDBLCLK:
				case WM_NCRBUTTONDBLCLK:
				case WM_NCMBUTTONDBLCLK:
				case WM_NCXBUTTONDBLCLK:
					nDoubleClickOption = nOption;
					break;
				}
			}
		}

		if(!bProcessed)
		{
			if(nOption && LaunchEmptySpaceFunction(nOption, lpMMTaskbarLongPtr))
			{
				switch(uMsg)
				{
				//case WM_XBUTTONDOWN:
				case WM_XBUTTONUP:
				case WM_XBUTTONDBLCLK:
				//case WM_NCXBUTTONDOWN:
				case WM_NCXBUTTONUP:
				case WM_NCXBUTTONDBLCLK:
					result = TRUE;
					break;

				default:
					result = 0;
					break;
				}
			}
			else
				result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}
		break;

	case WM_MOUSEWHEEL:
		lpMMTaskbarLongPtr = GetWindowLongPtr(hWnd, 0);
		result = 1;

		if(
			(
				(nOptions[OPT_WHEEL_CYCLE] == 1 || nOptions[OPT_WHEEL_VOLTASKBAR] == 1 || nOptions[OPT_WHEEL_VOLNOTIFY] == 1) &&
				GetCapture() == NULL
			) ||
			nOptionsEx[OPT_EX_MULTIPAGE_WHEEL_SCROLL])
		{
			LONG_PTR lpMMTaskListLongPtr, lpMMTaskSwOrTaskBandLongPtr;
			HWND hMMTaskListWnd, hMMTaskSwOrTaskBandWnd;

			if(hWnd == hTaskbarWnd)
			{
				lpMMTaskListLongPtr = lpTaskListLongPtr;
				hMMTaskListWnd = hTaskListWnd;
				lpMMTaskSwOrTaskBandLongPtr = lpTaskSwLongPtr;
				hMMTaskSwOrTaskBandWnd = hTaskSwWnd;
			}
			else
			{
				lpMMTaskListLongPtr = EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_LONG_PTR_VALUE(lpMMTaskbarLongPtr);
				hMMTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);

				lpMMTaskSwOrTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpMMTaskListLongPtr);
				hMMTaskSwOrTaskBandWnd = *EV_SECONDARY_TASK_BAND_HWND(lpMMTaskSwOrTaskBandLongPtr);
			}

			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);

			if(nOptions[OPT_WHEEL_CYCLE] == 1 && GetCapture() == NULL)
			{
				GetWindowRect(hMMTaskListWnd, &rc);
				if(PtInRect(&rc, pt))
				{
					int nRotates = -GET_WHEEL_DELTA_WPARAM(wParam) / 120;

					if(nOptionsEx[OPT_EX_SCROLL_REVERSE_CYCLE])
						nRotates = -nRotates;

					LONG_PTR *temp_task_item = TaskbarScroll(lpMMTaskListLongPtr,
						nRotates, nOptions[OPT_WHEEL_CYCLE_SKIPMIN] == 1, nOptionsEx[OPT_EX_SCROLL_NO_WRAP] == 0, NULL);
					if(temp_task_item)
					{
						// Allows to steal focus
						ZeroMemory(&input, sizeof(INPUT));
						SendInput(1, &input, sizeof(INPUT));

						SwitchToTaskItem(temp_task_item);
					}

					result = 0;
				}
			}

			if(result && nOptionsEx[OPT_EX_MULTIPAGE_WHEEL_SCROLL])
			{
				GetWindowRect(hMMTaskSwOrTaskBandWnd, &rc);
				if(PtInRect(&rc, pt))
				{
					long lStyle = GetWindowLong(hMMTaskSwOrTaskBandWnd, GWL_STYLE);
					if(lStyle & (WS_VSCROLL | WS_HSCROLL))
					{
						int nBar;
						int nPages = -GET_WHEEL_DELTA_WPARAM(wParam) / 120;

						if(lStyle & WS_VSCROLL)
							nBar = SB_VERT;
						else
							nBar = SB_HORZ;

						PostWindowScroll(hMMTaskSwOrTaskBandWnd, nBar, nPages);
					}

					result = 0;
				}
			}

			if(result && (nOptions[OPT_WHEEL_VOLTASKBAR] == 1 || nOptions[OPT_WHEEL_VOLNOTIFY] == 1) && GetCapture() == NULL)
			{
				if(nOptions[OPT_WHEEL_VOLTASKBAR] == 1)
				{
					GetWindowRect(hWnd, &rc);
				}
				else // if(nOptions[OPT_WHEEL_VOLNOTIFY] == 1)
				{
					if(hWnd == hTaskbarWnd)
					{
						GetWindowRect(hTrayNotifyWnd, &rc);
					}
					else if(nWinVersion >= WIN_VERSION_10_R1)
					{
						LONG_PTR lpSecondaryTrayClockLongPtr = *EV_SECONDARY_TASKBAR_CLOCK_LONG_PTR(lpMMTaskbarLongPtr);
						HWND hSecondaryClockButtonWnd = *EV_CLOCK_BUTTON_HWND(lpSecondaryTrayClockLongPtr);
						GetWindowRect(hSecondaryClockButtonWnd, &rc);
					}
					else // no notification area on secondary taskbars
					{
						SetRectEmpty(&rc);
					}
				}

				if(PtInRect(&rc, pt))
				{
					if(!IsSndVolOpen())
					{
						// Allows to steal focus
						ZeroMemory(&input, sizeof(INPUT));
						SendInput(1, &input, sizeof(INPUT));

						if(OpenScrollSndVol(wParam, lParam))
							SetSndVolTimer();
					}
					else
					{
						if(IsVolMutedAndNotZero(&bMutedAndNotZero) && !bMutedAndNotZero)
							ScrollSndVol(wParam, lParam);

						ResetSndVolTimer();
					}

					result = 0;
				}
			}
		}

		if(result)
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_NCHITTEST:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

		// This fixes a bug with Windows 7 returning HTCAPTION even when taskbar is locked
		if(nWinVersion == WIN_VERSION_7 && nOptionsEx[OPT_EX_W7_TASKLIST_HTCLIENT])
		{
			// If result is HTCAPTION and taskbar is locked
			if(result == HTCAPTION && !*EV_TASKBAR_UNLOCKED_FLAG())
			{
				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);

				GetWindowRect(hTaskSwWnd, &rc);

				if(nOptions[OPT_OTHER_EXTRAEMPTY] == 1)
				{
					switch(*EV_TASKBAR_POS())
					{
					case 1: // Is taskbar on top of the screen
					case 3: // Is taskbar on bottom of the screen
						if(GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
							rc.left -= GetSystemMetrics(SM_CXICON);
						else
							rc.right += GetSystemMetrics(SM_CXICON);
						break;
					}
				}

				if(PtInRect(&rc, pt))
					result = HTCLIENT;
			}
		}
		break;

	case WM_PRINTCLIENT:
	case WM_PAINT:
		if(nWinVersion == WIN_VERSION_7 &&
			nOptionsEx[OPT_EX_DISABLE_TASKBAR_TRANSPARENCY] == 2 &&
			!bDrawThemeBackgroundTaskbarHooked)
		{
			bDrawThemeBackgroundTaskbarHooked = TRUE;
			PointerRedirectionAdd(ppDrawThemeBackground, DrawThemeBackgroundTaskbarHook, &prDrawThemeBackgroundTaskbar);
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
			PointerRedirectionRemove(ppDrawThemeBackground, &prDrawThemeBackgroundTaskbar);
			bDrawThemeBackgroundTaskbarHooked = FALSE;
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_MOUSEMOVE:
		if(ComFuncGetLastActiveTaskItem())
		{
			if(
				(GetKeyState(VK_LBUTTON) < 0) ||
				(GetKeyState(VK_RBUTTON) < 0) ||
				(GetKeyState(VK_MBUTTON) < 0) ||
				(GetKeyState(VK_XBUTTON1) < 0) ||
				(GetKeyState(VK_XBUTTON2) < 0)
			)
			{
				// one of the mouse buttons is down, don't reset
			}
			else
				ComFuncResetLastActiveTaskItem();
		}

		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_WINDOWPOSCHANGED:
		if(!(((WINDOWPOS *)lParam)->flags & SWP_NOSIZE) && nOptionsEx[OPT_EX_NO_WIDTH_LIMIT])
		{
			LONG_PTR lpSecondaryTaskbarLongPtr = GetWindowLongPtr(hWnd, 0);

			switch(*EV_SECONDARY_TASKBAR_POS(lpSecondaryTaskbarLongPtr))
			{
			case 0: // Is taskbar on left of the screen
			case 2: // Is taskbar on right of the screen
				if(((WINDOWPOS *)lParam)->cx < GetTaskbarMinWidth())
				{
					PSSetSingleInt(NULL, L"last_secondary_taskbar_width", ((WINDOWPOS *)lParam)->cx);
				}
				else
				{
					PSRemoveSingle(NULL, L"last_secondary_taskbar_width");
				}
			}
		}

		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	default:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK NewTaskBandProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	LRESULT result;

	wnd_proc_call_counter++;

	switch(uMsg)
	{
	case WM_WINDOWPOSCHANGING:
		if(nOptions[OPT_OTHER_NOSTARTBTN] && nWinVersion >= WIN_VERSION_81 && !(((WINDOWPOS *)lParam)->flags & SWP_NOSIZE))
		{
			if(nWinVersion <= WIN_VERSION_811)
			{
				int nDesiredSpacing = nOptionsEx[OPT_EX_NO_START_BTN_SPACING];
				if(nDesiredSpacing == 0)
				{
					// Magic 13 value taken from Windows 8
					nDesiredSpacing = 13;
				}

				int nSpacing;
				int nTaskbarPos;

				if(hWnd == hTaskBandWnd)
				{
					MARGINS margins;

					SendMessage(hWnd, RB_GETBANDMARGINS, 0, (LPARAM)&margins);

					if(margins.cxLeftWidth < nDesiredSpacing)
						nSpacing = nDesiredSpacing - margins.cxLeftWidth;
					else
						nSpacing = 0;

					nTaskbarPos = *EV_TASKBAR_POS();
				}
				else // Secondary taskbar (multimonitor environment)
				{
					LONG_PTR lp;

					nSpacing = nDesiredSpacing;

					lp = GetWindowLongPtr(hWnd, 0);
					lp = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lp);
					nTaskbarPos = *EV_SECONDARY_TASKBAR_POS(lp);
				}

				switch(nTaskbarPos)
				{
				// Horizontal
				case 1: // Is taskbar on top of the screen
				case 3: // Is taskbar on bottom of the screen
					if(nSpacing < ((WINDOWPOS *)lParam)->x)
					{
						((WINDOWPOS *)lParam)->cx += (((WINDOWPOS *)lParam)->x - nSpacing);
						((WINDOWPOS *)lParam)->x = nSpacing;
					}
					break;

				// Vertical
				case 0:
				case 2:
					if(nSpacing < ((WINDOWPOS *)lParam)->y)
					{
						((WINDOWPOS *)lParam)->cy += (((WINDOWPOS *)lParam)->y - nSpacing);
						((WINDOWPOS *)lParam)->y = nSpacing;
					}
					break;
				}
			}
			else // nWinVersion >= WIN_VERSION_10_T1
			{
				int nTaskbarPos;
				HWND hStartBtnWnd;

				if(hWnd == hTaskBandWnd)
				{
					nTaskbarPos = *EV_TASKBAR_POS();

					LONG_PTR lp = *EV_TASKBAR_START_BTN_LONG_PTR();
					hStartBtnWnd = *EV_START_BUTTON_HWND(lp);
				}
				else // Secondary taskbar (multimonitor environment)
				{
					LONG_PTR lpSecondaryTaskBandLongPtr = GetWindowLongPtr(hWnd, 0);
					LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);

					nTaskbarPos = *EV_SECONDARY_TASKBAR_POS(lpSecondaryTaskbarLongPtr);

					LONG_PTR lp = *EV_SECONDARY_TASKBAR_START_BTN_LONG_PTR(lpSecondaryTaskbarLongPtr);
					hStartBtnWnd = *EV_START_BUTTON_HWND(lp);

					// Make sure that we have the correct start button size.
					HWND hSecondaryTaskbarWnd = *EV_SECONDARY_TASKBAR_HWND(lpSecondaryTaskbarLongPtr);
					SendMessage(hSecondaryTaskbarWnd, 0x565, 1, 0); // CSecondaryTray::_UpdatePearlSize
				}

				RECT rcStartBtn;
				GetWindowRect(hStartBtnWnd, &rcStartBtn);

				int nSpacing = nOptionsEx[OPT_EX_NO_START_BTN_SPACING];

				switch(nTaskbarPos)
				{
				// Horizontal
				case 1: // Is taskbar on top of the screen
				case 3: // Is taskbar on bottom of the screen
					((WINDOWPOS *)lParam)->x += -(rcStartBtn.right - rcStartBtn.left) + nSpacing;
					((WINDOWPOS *)lParam)->cx -= -(rcStartBtn.right - rcStartBtn.left) + nSpacing;
					break;

				// Vertical
				case 0:
				case 2:
					((WINDOWPOS *)lParam)->y += -(rcStartBtn.bottom - rcStartBtn.top) + nSpacing;
					((WINDOWPOS *)lParam)->cy -= -(rcStartBtn.bottom - rcStartBtn.top) + nSpacing;
					break;
				}
			}
		}

		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	default:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK NewTaskSwProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	NEW_BUTTON_LPARAM *p_new_button_lparam;
	HWND hButtonWnd;
	UINT uSysFrostedWindow;
	LRESULT result;

	wnd_proc_call_counter++;

	switch(uMsg)
	{
	case 0x043A: // Calls CTaskBand::_HandleChangeNotify
	case 0x0446: // Calls CTaskBand::_EnumPinnedItems, related to loading immersive pinned item icons
	case 0x0452: // Calls CTaskBand::_HandleSyncDisplayChange, icons might be reloaded here on DPI change, make sure the large icons are loaded
	case 0x0467: // Related to loading immersive app icons
		nMulDivHookLargeIconsCounter++;
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		nMulDivHookLargeIconsCounter--;
		break;

	case 0x043E: // Button size (Win 7)
		result = 1;

		if(nOptions[OPT_OTHER_NOSTARTBTN] && nWinVersion == WIN_VERSION_7)
		{
			if(lParam == 0 && wParam == 0)
				result = 0;
		}

		if(result)
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case 0x044A: // Button create
		p_new_button_lparam = (NEW_BUTTON_LPARAM *)lParam;

		if(RefreshTaskbarHardcore_ButtonCreating(p_new_button_lparam->hButtonWnd))
		{
			hButtonWnd = p_new_button_lparam->hButtonWnd;

			ComFuncSetCreatedThumb(p_new_button_lparam->hButtonWnd, p_new_button_lparam->hThumbParentWnd);

			if(nOptions[OPT_GROUPING_RIGHTDRAG] == 1 && ComFuncIsAttachPending(hButtonWnd))
			{
				// Animation causes troubles when moving between groups, so we disable it
				SendMessage(hTaskListWnd, WM_SETREDRAW, FALSE, 0);
				result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
				SendMessage(hTaskListWnd, WM_SETREDRAW, TRUE, 0);
			}
			else
				result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

			ComFuncSetCreatedThumb(NULL, NULL);

			if(nOptions[OPT_GROUPING_RIGHTDRAG] == 1 && (ComFuncMoveDetachedToCursor() || ComFuncMoveAttachedToCursor()))
			{
				// Processed
			}
			else if(nOptions[OPT_GROUPING_OPENNEAR] == 1)
				ComFuncMoveNearMatching(hButtonWnd);

			RefreshTaskbarHardcore_ButtonCreated(hButtonWnd);
		}
		else
			result = 0;
		break;

	case 0x0454: // Button destroy
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

		RefreshTaskbarHardcore_ButtonDestroyed((HWND)wParam);
		break;

	case WM_WINDOWPOSCHANGING:
		if(nOptions[OPT_OTHER_EXTRAEMPTY] == 1 && !(((WINDOWPOS *)lParam)->flags & SWP_NOSIZE))
		{
			switch(*EV_TASKBAR_POS())
			{
			case 1: // Is taskbar on top of the screen
			case 3: // Is taskbar on bottom of the screen
				((WINDOWPOS *)lParam)->cx -= GetSystemMetrics(SM_CXICON);
				if(((WINDOWPOS *)lParam)->cx < 0)
					((WINDOWPOS *)lParam)->cx = 0;
				break;
			}
		}

		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	default:
		uSysFrostedWindow = *EV_TASK_SW_SYS_FROSTED_WINDOW_MSG();
		if(uSysFrostedWindow && uMsg == uSysFrostedWindow)
		{
			result = 1;

			if(nOptionsEx[OPT_EX_FIX_HANG_REPOSITION])
			{
				if(lParam && !IsWindow((HWND)lParam))
					result = 0;
			}

			if(result)
				result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK NewTaskListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	static int nDoubleClickOption = 0;
	static BOOL bControllingContextMenu = FALSE;
	static BOOL bImmidiateTooltip = FALSE;
	static DWORD dwLastWheelMinimizeTickCount, dwLastWheelRestoreTickCount;
	LONG_PTR lpMMTaskListLongPtr;
	int nOption;
	DWORD dwOldUserPrefSetBits, dwOldUserPrefRemoveBits;
	LONG_PTR *prev_button_group_active;
	LONG_PTR lp;
	int nTaskbarPos;
	BOOL bMinimize;
	LONG_PTR *button_group;
	int button_group_type;
	INPUT input;
	LONG_PTR *temp_task_item;
	BOOL bProcessed;
	LRESULT result;

	wnd_proc_call_counter++;

	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONDBLCLK:
		lpMMTaskListLongPtr = GetWindowLongPtr(hWnd, 0);
		bProcessed = FALSE;

		nDoubleClickOption = 0;

		if(!GetMouseCtrlValue(MOUSECTRL_TARGET_TASKBARITEM, uMsg, wParam, &nOption))
		{
			nOption = 0;
			switch(uMsg)
			{
			case WM_LBUTTONDBLCLK:
				if(nOptions[OPT_PINNED_DBLCLICK] == 1)
				{
					button_group = TaskbarGetTrackedButtonGroup(lpMMTaskListLongPtr);
					if(button_group)
					{
						button_group_type = (int)button_group[DO2(6, 0 /* omitted from public code */)];
						if(button_group_type == 2)
						{
							DefSubclassProc(hWnd, WM_LBUTTONDOWN, wParam, lParam);
							result = DefSubclassProc(hWnd, WM_LBUTTONUP, wParam, lParam);
							bProcessed = TRUE;
						}
					}
				}
				break;

			case WM_RBUTTONDOWN:
				if(nOptions[OPT_GROUPING_RIGHTDRAG] == 1)
				{
					if(!bGetCaptureHooked)
					{
						MHP_HookGetCapture(GetCaptureHook);
						bGetCaptureHooked = TRUE;
					}

					SetCapture(hWnd);

					ComFuncTaskListRightDragInit(lpMMTaskListLongPtr);

					result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
					bProcessed = TRUE;
				}
				else if(nOptions[OPT_RCLICK] == 1)
				{
					result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
					bProcessed = TRUE;
				}
				break;

			case WM_MBUTTONDOWN:
				nOption = nOptions[OPT_MCLICK];
				break;
			}
		}

		if(!bProcessed)
		{
			if(nOption)
			{
				switch(nOption)
				{
				case 1:
				case 2:
				case 3:
				case 7:
				case 8:
				case 13:
				case 14:
					result = DefSubclassProc(hWnd, WM_LBUTTONDOWN, wParam & ~(MK_SHIFT | MK_CONTROL), lParam);
					break;

				case 4:
					result = DefSubclassProc(hWnd, WM_MBUTTONDOWN, wParam & ~(MK_SHIFT | MK_CONTROL), lParam);
					break;

				case 5:
				case 6:
					result = DefSubclassProc(hWnd, WM_RBUTTONDOWN, wParam & ~(MK_SHIFT | MK_CONTROL), lParam);
					break;

				case 9:
				case 10:
				case 11:
				case 12:
				case 15:
					result = DefSubclassProc(hWnd, WM_LBUTTONDOWN, wParam & ~(MK_SHIFT | MK_CONTROL), lParam);
					break;
				}

				switch(uMsg)
				{
				case WM_LBUTTONDBLCLK:
				case WM_RBUTTONDBLCLK:
				case WM_MBUTTONDBLCLK:
				case WM_XBUTTONDBLCLK:
					nDoubleClickOption = nOption;
					break;
				}

				switch(uMsg)
				{
				case WM_XBUTTONDOWN:
				case WM_XBUTTONDBLCLK:
					if(result == 0)
						result = TRUE;
					break;
				}
			}
			else
				result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}
		break;

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
		lpMMTaskListLongPtr = GetWindowLongPtr(hWnd, 0);
		bProcessed = FALSE;

		if(uMsg == WM_LBUTTONUP)
			ComFuncTaskListBeforeLButtonUp(lpMMTaskListLongPtr, &dwOldUserPrefSetBits, &dwOldUserPrefRemoveBits, &prev_button_group_active);

		switch(uMsg)
		{
		case WM_RBUTTONUP:
			if(nOptions[OPT_GROUPING_RIGHTDRAG] == 1)
			{
				if(ComFuncTaskListRightDragProcessed())
				{
					result = 0;
					bProcessed = TRUE;
				}
			}

			if(bGetCaptureHooked)
				ReleaseCapture();
			break;
		}

		if(!bProcessed)
		{
			if(nDoubleClickOption)
			{
				nOption = nDoubleClickOption;
				nDoubleClickOption = 0;
			}
			else if(!GetMouseCtrlValue(MOUSECTRL_TARGET_TASKBARITEM, uMsg, wParam, &nOption))
			{
				nOption = 0;
				switch(uMsg)
				{
				case WM_LBUTTONUP:
					// button_group the mouse is pressing on
					button_group = *EV_MM_TASKLIST_PRESSED_BUTTON_GROUP(lpMMTaskListLongPtr);
					if(button_group)
					{
						button_group_type = (int)button_group[DO2(6, 0 /* omitted from public code */)];
						if(button_group_type == 2) // Pinned
						{
							if(nOptions[OPT_PINNED_DBLCLICK] == 1 && !(wParam & MK_SHIFT))
							{
								result = DefSubclassProc(hWnd, WM_LBUTTONUP, wParam, MAKELPARAM(-1, -1));
								bProcessed = TRUE;
							}
						}
						else
						{
							if(WillExtendedUIGlom(lpMMTaskListLongPtr, button_group) && !(wParam & MK_SHIFT))
							{
								BOOL bToggleCtrl = nOptions[OPT_COMBINED_LCLICK] == 1 ||
									(nOptions[OPT_COMBINED_LCLICK] == 2 && button_group != TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr));

								if(bToggleCtrl || (nOptionsEx[OPT_EX_CYCLE_SAME_VIRTUAL_DESKTOP] && nWinVersion >= WIN_VERSION_10_T1 && nWinVersion <= WIN_VERSION_10_T2))
								{
									WPARAM wNewParam = wParam;

									if(bToggleCtrl)
										wNewParam ^= MK_CONTROL;

									if((wNewParam & MK_CONTROL) && (nOptionsEx[OPT_EX_CYCLE_SAME_VIRTUAL_DESKTOP] && nWinVersion >= WIN_VERSION_10_T1 && nWinVersion <= WIN_VERSION_10_T2))
									{
										bEnableDPA_SortHook = TRUE;
										result = DefSubclassProc(hWnd, WM_LBUTTONUP, wNewParam, lParam);
										bEnableDPA_SortHook = FALSE;
									}
									else
										result = DefSubclassProc(hWnd, WM_LBUTTONUP, wNewParam, lParam);

									bProcessed = TRUE;
								}
							}
						}
					}
					break;

				case WM_RBUTTONUP:
					if(nOptions[OPT_RCLICK] == 1)
					{
						result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
						bProcessed = TRUE;
					}
					else if(nOptions[OPT_GROUPING_RIGHTDRAG] == 1)
					{
						result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
						bProcessed = TRUE;
					}
					break;

				case WM_MBUTTONUP:
					nOption = nOptions[OPT_MCLICK];
					break;
				}
			}
		}

		if(!bProcessed)
		{
			if(nOption)
			{
				switch(nOption)
				{
				case 1:
				case 2:
				case 3:
				case 7:
				case 8:
				case 13:
				case 14:
					ComFuncSwitchToHookEnable(nOption, lpMMTaskListLongPtr);

					if(uMsg != WM_LBUTTONUP)
						ComFuncTaskListBeforeLButtonUp(lpMMTaskListLongPtr, &dwOldUserPrefSetBits, &dwOldUserPrefRemoveBits, &prev_button_group_active);

					if(nOption == 1 && uMsg == WM_LBUTTONUP)
					{
						// button_group the mouse is pressing on
						button_group = *EV_MM_TASKLIST_PRESSED_BUTTON_GROUP(lpMMTaskListLongPtr);
						if(button_group)
						{
							button_group_type = (int)button_group[DO2(6, 0 /* omitted from public code */)];
							if(button_group_type == 2) // Pinned
							{
								if(nOptions[OPT_PINNED_DBLCLICK] == 1)
								{
									result = DefSubclassProc(hWnd, WM_LBUTTONUP, wParam & ~(MK_SHIFT | MK_CONTROL), MAKELPARAM(-1, -1));
									bProcessed = TRUE;
								}
							}
							else
							{
								if(WillExtendedUIGlom(lpMMTaskListLongPtr, button_group))
								{
									if(nOptions[OPT_COMBINED_LCLICK] == 1)
									{
										result = DefSubclassProc(hWnd, WM_LBUTTONUP, (wParam | MK_CONTROL) & ~MK_SHIFT, lParam);
										bProcessed = TRUE;
									}
									else if(nOptions[OPT_COMBINED_LCLICK] == 2)
									{
										if(button_group != TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr))
										{
											result = DefSubclassProc(hWnd, WM_LBUTTONUP, (wParam | MK_CONTROL) & ~MK_SHIFT, lParam);
											bProcessed = TRUE;
										}
									}
								}
							}
						}
					}

					if(!bProcessed)
						result = DefSubclassProc(hWnd, WM_LBUTTONUP, wParam & ~(MK_SHIFT | MK_CONTROL), lParam);

					if(uMsg != WM_LBUTTONUP)
						ComFuncTaskListAfterLButtonUp(lpMMTaskListLongPtr, dwOldUserPrefSetBits, dwOldUserPrefRemoveBits, prev_button_group_active);

					ComFuncSwitchToHookDisable();
					break;

				case 4:
					result = DefSubclassProc(hWnd, WM_MBUTTONUP, wParam & ~(MK_SHIFT | MK_CONTROL), lParam);
					break;

				case 5:
				case 6:
					bControllingContextMenu = TRUE;

					if(((nOption == 5) ? GetKeyState(VK_SHIFT) < 0 : GetKeyState(VK_SHIFT) >= 0) && !bGetKeyStateHooked)
					{
						bGetKeyStateHooked = TRUE;
						MHP_HookGetKeyState(GetKeyStateInvertShiftHook);

						result = DefSubclassProc(hWnd, WM_RBUTTONUP, wParam & ~(MK_SHIFT | MK_CONTROL), lParam);

						MHP_HookGetKeyState(NULL);
						bGetKeyStateHooked = FALSE;
					}
					else
						result = DefSubclassProc(hWnd, WM_RBUTTONUP, wParam & ~(MK_SHIFT | MK_CONTROL), lParam);

					bControllingContextMenu = FALSE;
					break;

				case 9:
				case 10:
				case 11:
				case 12:
				case 15:
					// Check drag operation flag
					if(*EV_MM_TASKLIST_DRAG_FLAG(lpMMTaskListLongPtr) == 0)
					{
						// button_group the mouse is pressing on
						button_group = *EV_MM_TASKLIST_PRESSED_BUTTON_GROUP(lpMMTaskListLongPtr);
					}
					else
					{
						button_group = NULL;
					}

					if(uMsg != WM_LBUTTONUP)
						ComFuncTaskListBeforeLButtonUp(lpMMTaskListLongPtr, &dwOldUserPrefSetBits, &dwOldUserPrefRemoveBits, &prev_button_group_active);

					result = DefSubclassProc(hWnd, WM_LBUTTONUP, wParam & ~(MK_SHIFT | MK_CONTROL), MAKELPARAM(-1, -1));

					if(uMsg != WM_LBUTTONUP)
						ComFuncTaskListAfterLButtonUp(lpMMTaskListLongPtr, dwOldUserPrefSetBits, dwOldUserPrefRemoveBits, prev_button_group_active);

					if(button_group)
					{
						button_group_type = (int)button_group[DO2(6, 0 /* omitted from public code */)];
						if(button_group_type == 1 || button_group_type == 3)
						{
							switch(nOption)
							{
							case 9:
								ButtonGroupExecMenuCommand(button_group, SC_MINIMIZE);
								break;

							case 10:
								ButtonGroupExecMenuCommand(button_group, SC_CLOSE);
								break;

							case 11:
								ButtonGroupExecMenuCommand(button_group, SC_MAXIMIZE);
								break;

							case 12:
								ButtonGroupExecMenuCommand(button_group, SC_RESTORE);
								break;

							case 15:
								SortButtonGroupItems(button_group);
								break;
							}
						}
					}
					break;
				}

				if(uMsg == WM_XBUTTONUP && result == 0)
					result = TRUE;
			}
			else
				result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}

		if(uMsg == WM_LBUTTONUP)
			ComFuncTaskListAfterLButtonUp(lpMMTaskListLongPtr, dwOldUserPrefSetBits, dwOldUserPrefRemoveBits, prev_button_group_active);
		break;

	case WM_CONTEXTMENU:
		if(!bControllingContextMenu && nOptions[OPT_RCLICK] == 1 && !bGetKeyStateHooked)
		{
			bGetKeyStateHooked = TRUE;
			MHP_HookGetKeyState(GetKeyStateInvertShiftHook);

			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

			MHP_HookGetKeyState(NULL);
			bGetKeyStateHooked = FALSE;
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_CAPTURECHANGED:
		lpMMTaskListLongPtr = GetWindowLongPtr(hWnd, 0);

		ComFuncTaskListCaptureChanged(lpMMTaskListLongPtr);

		if(bGetCaptureHooked)
		{
			MHP_HookGetCapture(NULL);
			bGetCaptureHooked = FALSE;
		}

		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_MOUSEMOVE:
		lpMMTaskListLongPtr = GetWindowLongPtr(hWnd, 0);

		ComFuncTaskListBeforeMouseMove();

		// OPT_HOVER: Tooltip or nothing
		if(nOptions[OPT_HOVER] == 2 || nOptions[OPT_HOVER] == 3)
		{
			// Should disable thumbs on some weird situations
			*EV_MM_TASKLIST_THUMB_DISABLING_FLAG(lpMMTaskListLongPtr) = TRUE;
		}

		if(nOptionsEx[OPT_EX_DISABLE_ITEMS_DRAG] == 1 && GetCapture() == hWnd)
		{
			DWORD dwOldDragFlag = *EV_MM_TASKLIST_DRAG_FLAG(lpMMTaskListLongPtr);
			*EV_MM_TASKLIST_DRAG_FLAG(lpMMTaskListLongPtr) = -1;

			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

			*EV_MM_TASKLIST_DRAG_FLAG(lpMMTaskListLongPtr) = dwOldDragFlag;
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

		// OPT_HOVER: Tooltip or nothing
		if(nOptions[OPT_HOVER] == 2 || nOptions[OPT_HOVER] == 3)
		{
			if(*EV_MM_TASKLIST_TOOLTIP_TIMER_ID(lpMMTaskListLongPtr) == 2)
			{
				if(nOptions[OPT_HOVER] == 2 && bImmidiateTooltip) // Tooltip
				{
					ShowWindow(*EV_MM_TASKLIST_TOOLTIP_WND(lpMMTaskListLongPtr), SW_HIDE);
					DefSubclassProc(hWnd, WM_TIMER, 2, 0);
				}
				else
				{
					KillTimer(hWnd, 2);
					*EV_MM_TASKLIST_TOOLTIP_TIMER_ID(lpMMTaskListLongPtr) = 0;
				}
			}
		}

		ComFuncTaskListAfterMouseMove();
		break;

	case WM_MOUSEHOVER:
		if(nOptions[OPT_HOVER] == 2) // Tooltip
		{
			lpMMTaskListLongPtr = GetWindowLongPtr(hWnd, 0);

			result = DefWindowProc(hWnd, uMsg, wParam, lParam);

			*EV_MM_TASKLIST_TOOLTIP_TIMER_ID(lpMMTaskListLongPtr) = 2;
			DefSubclassProc(hWnd, WM_TIMER, 2, 0);

			bImmidiateTooltip = TRUE;
		}
		else if(nOptions[OPT_HOVER] == 3) // Nothing
			result = DefWindowProc(hWnd, uMsg, wParam, lParam);
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_MOUSELEAVE:
		ComFuncTaskListMouseLeave();

		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

		bImmidiateTooltip = FALSE;
		break;

	case WM_MOUSEWHEEL:
		lpMMTaskListLongPtr = GetWindowLongPtr(hWnd, 0);

		if(GetCapture() == NULL)
		{
			result = 1;

			if(nOptions[OPT_WHEEL_MINTASKBAR] == 1)
			{
				temp_task_item = TaskbarGetTrackedTaskItem(lpMMTaskListLongPtr);
				if(temp_task_item)
				{
					if(hWnd != hTaskListWnd)
					{
						// Secondary taskbar (multimonitor environment)
						lp = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpMMTaskListLongPtr);
						lp = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lp);
						nTaskbarPos = *EV_SECONDARY_TASKBAR_POS(lp);
					}
					else
					{
						nTaskbarPos = *EV_TASKBAR_POS();
					}

					bMinimize = (GET_WHEEL_DELTA_WPARAM(wParam) < 0);

					if(nTaskbarPos == 1) // Is taskbar on top of the screen
						bMinimize = !bMinimize;

					if(nOptionsEx[OPT_EX_SCROLL_REVERSE_MINIMIZE])
						bMinimize = !bMinimize;

					// Add a limit, since if many "restore" commands fire one after another
					// with a short delay and the window is maximized, it may become un-maximized.
					if(GetTickCount() - (bMinimize ? dwLastWheelMinimizeTickCount : dwLastWheelRestoreTickCount) > 100)
					{
						// Allows to steal focus
						ZeroMemory(&input, sizeof(INPUT));
						SendInput(1, &input, sizeof(INPUT));

						if(bMinimize)
						{
							if(nOptionsEx[OPT_EX_SCROLL_MAXIMIZE_RESTORE] && CanRestoreTaskItem(temp_task_item) && IsMaximizedTaskItem(temp_task_item))
							{
								SwitchToTaskItem(temp_task_item);
								PostMessage(GetTaskItemWnd(temp_task_item), WM_SYSCOMMAND, SC_RESTORE, 0);
							}
							else
								MinimizeThumbTaskItem(temp_task_item);

							dwLastWheelMinimizeTickCount = GetTickCount();
						}
						else
						{
							if(nOptionsEx[OPT_EX_SCROLL_MAXIMIZE_RESTORE] && CanMaximizeTaskItem(temp_task_item) && !IsMinimizedTaskItem(temp_task_item) && !IsMaximizedTaskItem(temp_task_item))
							{
								SwitchToTaskItem(temp_task_item);
								PostMessage(GetTaskItemWnd(temp_task_item), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
							}
							else
								SwitchToTaskItem(temp_task_item);

							dwLastWheelRestoreTickCount = GetTickCount();
						}
					}

					result = 0;
				}
			}

			if(result)
				result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		}
		else if(ComFuncTaskListMouseWheel(lpMMTaskListLongPtr, GET_WHEEL_DELTA_WPARAM(wParam)))
		{
			result = 0;
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_TIMER:
		switch(wParam)
		{
		case 2: // Show a tooltip
			break;

		case 5: // Close thumbnails
			ComFuncSetThumbNoDismiss(FALSE);
			break;
		}

		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_PAINT:
		nMulDivHookLargeIconsCounter++;
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		nMulDivHookLargeIconsCounter--;
		break;

	case WM_NCDESTROY:
		lpMMTaskListLongPtr = GetWindowLongPtr(hWnd, 0);

		// Handle a secondary taskbar destroy (multimonitor environment)
		if(hWnd != hTaskListWnd)
			UnsubclassSecondaryTaskListWindows(lpMMTaskListLongPtr);

		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	default:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK NewThumbnailProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	static int dragged_thumb_index = -1;
	static BOOL bDragDone;
	static DWORD dwLastWheelMinimizeTickCount, dwLastWheelRestoreTickCount;
	LONG_PTR lpMMThumbnailLongPtr;
	int tracked_thumb_index;
	int active_thumb_index;
	LONG_PTR lp, *plp;
	int nTaskbarPos;
	BOOL bMinimize;
	INPUT input;
	LONG_PTR *temp_task_item;
	LONG_PTR *container_task_item;
	int *pnTemp;
	BOOL bProcessed;
	LRESULT result;

	wnd_proc_call_counter++;

	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

		if(nOptions[OPT_THUMB_REORDER] == 1)
		{
			lpMMThumbnailLongPtr = GetWindowLongPtr(hWnd, 0);

			tracked_thumb_index = *EV_MM_THUMBNAIL_TRACKED_THUMB_INDEX(lpMMThumbnailLongPtr);
			if(tracked_thumb_index >= 0)
			{
				bDragDone = FALSE;
				dragged_thumb_index = tracked_thumb_index;
				SetCapture(hWnd);
			}
		}
		break;

	case WM_MOUSEMOVE:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

		if(nOptions[OPT_THUMB_REORDER] == 1 && dragged_thumb_index >= 0 && GetCapture() == hWnd)
		{
			lpMMThumbnailLongPtr = GetWindowLongPtr(hWnd, 0);

			plp = (LONG_PTR *)*EV_MM_THUMBNAIL_THUMBNAILS_HDPA(lpMMThumbnailLongPtr);
			if(dragged_thumb_index < (int)plp[0]) // thumbs_count
			{
				tracked_thumb_index = *EV_MM_THUMBNAIL_TRACKED_THUMB_INDEX(lpMMThumbnailLongPtr);
				if(tracked_thumb_index >= 0 && tracked_thumb_index != dragged_thumb_index)
				{
					if(TaskbarMoveThumbInGroup(lpMMThumbnailLongPtr, dragged_thumb_index, tracked_thumb_index))
					{
						// Mouse down thumb
						pnTemp = EV_MM_THUMBNAIL_PRESSED_THUMB_INDEX(lpMMThumbnailLongPtr);
						if(*pnTemp == dragged_thumb_index)
							*pnTemp = tracked_thumb_index;

						InvalidateRect(hWnd, NULL, FALSE);

						dragged_thumb_index = tracked_thumb_index;

						bDragDone = TRUE;
					}
				}
			}
		}
		break;

	case WM_LBUTTONUP:
		bProcessed = FALSE;

		if(nOptions[OPT_THUMB_REORDER] == 1 && dragged_thumb_index >= 0 && GetCapture() == hWnd)
		{
			ReleaseCapture();
			if(bDragDone)
			{
				result = DefWindowProc(hWnd, uMsg, wParam, lParam);
				bProcessed = TRUE;
			}
		}

		if(!bProcessed && nOptions[OPT_THUMB_ACTIVEMIN] == 1)
		{
			lpMMThumbnailLongPtr = GetWindowLongPtr(hWnd, 0);

			tracked_thumb_index = *EV_MM_THUMBNAIL_TRACKED_THUMB_INDEX(lpMMThumbnailLongPtr);
			active_thumb_index = *EV_MM_THUMBNAIL_ACTIVE_THUMB_INDEX(lpMMThumbnailLongPtr);

			if(tracked_thumb_index == active_thumb_index)
			{
				ComFuncSwitchToHookEnable(2, 0);
				result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
				ComFuncSwitchToHookDisable();

				bProcessed = TRUE;
			}
		}

		if(!bProcessed)
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_CAPTURECHANGED:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

		if(nOptions[OPT_THUMB_REORDER] == 1 && dragged_thumb_index >= 0)
			dragged_thumb_index = -1;
		break;

	case WM_MOUSEWHEEL:
		if(nOptions[OPT_WHEEL_MINTHUMB] == 1)
		{
			lpMMThumbnailLongPtr = GetWindowLongPtr(hWnd, 0);

			temp_task_item = ThumbnailGetTrackedTaskItem(lpMMThumbnailLongPtr, &container_task_item);
			if(temp_task_item)
			{
				if(!container_task_item)
					container_task_item = temp_task_item;

				ShowLivePreview(lpMMThumbnailLongPtr, NULL);
				ComFuncSetThumbNoDismiss(TRUE);

				if(hWnd != hThumbnailWnd)
				{
					// Secondary taskbar (multimonitor environment)
					lp = EV_MM_THUMBNAIL_MM_TASKLIST_LONG_PTR_VALUE(lpMMThumbnailLongPtr);
					lp = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lp);
					lp = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lp);
					nTaskbarPos = *EV_SECONDARY_TASKBAR_POS(lp);
				}
				else
				{
					nTaskbarPos = *EV_TASKBAR_POS();
				}

				bMinimize = (GET_WHEEL_DELTA_WPARAM(wParam) < 0);

				if(nTaskbarPos == 1) // Is taskbar on top of the screen
					bMinimize = !bMinimize;

				if(nOptionsEx[OPT_EX_SCROLL_REVERSE_MINIMIZE])
					bMinimize = !bMinimize;

				// Add a limit, since if many "restore" commands fire one after another
				// with a short delay and the window is maximized, it may become un-maximized.
				if(GetTickCount() - (bMinimize ? dwLastWheelMinimizeTickCount : dwLastWheelRestoreTickCount) > 100)
				{
					// Allows to steal focus
					ZeroMemory(&input, sizeof(INPUT));
					SendInput(1, &input, sizeof(INPUT));

					if(bMinimize)
					{
						if(nOptionsEx[OPT_EX_SCROLL_MAXIMIZE_RESTORE] && CanRestoreTaskItem(container_task_item) && IsMaximizedTaskItem(container_task_item))
						{
							SwitchToTaskItem(temp_task_item);
							PostMessage(GetTaskItemWnd(temp_task_item), WM_SYSCOMMAND, SC_RESTORE, 0);
						}
						else
							MinimizeThumbTaskItem(temp_task_item);

						dwLastWheelMinimizeTickCount = GetTickCount();
					}
					else
					{
						if(nOptionsEx[OPT_EX_SCROLL_MAXIMIZE_RESTORE] && CanMaximizeTaskItem(container_task_item) && !IsMinimizedTaskItem(container_task_item) && !IsMaximizedTaskItem(container_task_item))
						{
							SwitchToTaskItem(temp_task_item);
							PostMessage(GetTaskItemWnd(temp_task_item), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
						}
						else
							SwitchToTaskItem(temp_task_item);

						dwLastWheelRestoreTickCount = GetTickCount();
					}

					// Remove active thumb
					*EV_MM_THUMBNAIL_ACTIVE_THUMB_INDEX(lpMMThumbnailLongPtr) = -10;
					InvalidateRect(hWnd, NULL, FALSE);
				}
			}

			result = 0;
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_TIMER:
		result = 1;

		if(wParam == 2006) // Aero peek window of hovered thumb
		{
			if(nOptions[OPT_THUMB_REORDER] == 1 && dragged_thumb_index >= 0 && GetCapture() == hWnd)
			{
				KillTimer(hWnd, wParam);
				result = 0;
			}

			ComFuncSetThumbNoDismiss(FALSE);
		}

		if(result)
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		else
			result = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_CANCELMODE:
		ComFuncSetThumbNoDismiss(FALSE);

		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_KILLFOCUS:
		if(ComFuncGetThumbNoDismiss())
		{
			lpMMThumbnailLongPtr = GetWindowLongPtr(hWnd, 0);

			// Make the thumbnail disappear correctly upon focus loss
			*EV_MM_THUMBNAIL_STICKY_FLAG(lpMMThumbnailLongPtr) = FALSE;

			result = DefWindowProc(hWnd, uMsg, wParam, lParam); // Don't hide thumbs on focus loss
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_PAINT:
		lpMMThumbnailLongPtr = GetWindowLongPtr(hWnd, 0);
		ComFuncThumbnailWndBeforePaint(lpMMThumbnailLongPtr);

		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	default:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK NewTrayNotifyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	LRESULT result;

	wnd_proc_call_counter++;

	switch(uMsg)
	{
	case WM_WINDOWPOSCHANGED:
		nInTrayNotifyWndWindowPosChanged++;
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		nInTrayNotifyWndWindowPosChanged--;
		break;

	default:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK NewTrayOverflowToolbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	LRESULT result;

	wnd_proc_call_counter++;

	result = ProcessTrayToolbarMsg(hWnd, uMsg, wParam, lParam);

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK NewTrayTemporaryToolbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	LRESULT result;

	wnd_proc_call_counter++;

	result = ProcessTrayToolbarMsg(hWnd, uMsg, wParam, lParam);

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK NewTrayToolbarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	LRESULT result;

	wnd_proc_call_counter++;

	switch(uMsg)
	{
	case WM_TIMER:
		if(wParam == 0 &&
			(nOptions[OPT_WHEEL_VOLTASKBAR] == 1 || nOptions[OPT_WHEEL_VOLNOTIFY] == 1) &&
			nOptionsEx[OPT_EX_SNDVOL_TOOLTIP])
		{
			OnSndVolTooltipTimer();
		}

		result = ProcessTrayToolbarMsg(hWnd, uMsg, wParam, lParam);
		break;

	case TB_GETBUTTONSIZE:
		result = ProcessTrayToolbarMsg(hWnd, uMsg, wParam, lParam);

		if(nOptionsEx[OPT_EX_TRAY_ICONS_PADDING] && nWinVersion >= WIN_VERSION_10_T1)
		{
			LONG_PTR lpTrayNotifyLongPtr = GetWindowLongPtr(hTrayNotifyWnd, 0);
			bPrevPtrDevSupported = *EV_TRAY_NOTIFY_PTRDEV_SUPPORTED(lpTrayNotifyLongPtr);
			bPrevPtrDevSupportedValid = *EV_TRAY_NOTIFY_PTRDEV_SUPPORTED_VALID(lpTrayNotifyLongPtr);

			*EV_TRAY_NOTIFY_PTRDEV_SUPPORTED(lpTrayNotifyLongPtr) = 1;
			*EV_TRAY_NOTIFY_PTRDEV_SUPPORTED_VALID(lpTrayNotifyLongPtr) = 1;

			MHP_HookGetSystemMetrics(GetSystemMetricsHook);
		}
		break;

	case TB_SETPADDING:
		if(nOptionsEx[OPT_EX_TRAY_ICONS_PADDING] && nWinVersion >= WIN_VERSION_10_T1)
		{
			// Fixup the horizontal padding. Can only be a multiple of 2 so truncate the least significant bit.
			lParam &= ~0xFFFF;
			lParam |= ((nOptionsEx[OPT_EX_TRAY_ICONS_PADDING] / 2 * 2) & 0xFFFF);
		}

		result = ProcessTrayToolbarMsg(hWnd, uMsg, wParam, lParam);
		break;

	default:
		result = ProcessTrayToolbarMsg(hWnd, uMsg, wParam, lParam);
		break;
	}

	wnd_proc_call_counter--;

	return result;
}

static LRESULT ProcessTrayToolbarMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int nDoubleClickOption = 0;
	LONG_PTR lpTrayNotifyLongPtr;
	POINT pt;
	RECT rc;
	DWORD dw;
	int nOption;
	LRESULT result;
	BOOL bProcessed;

	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCXBUTTONDOWN:
		nDoubleClickOption = 0;
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONUP:
	case WM_XBUTTONDBLCLK:
	case WM_NCLBUTTONUP:
	case WM_NCLBUTTONDBLCLK:
	case WM_NCRBUTTONUP:
	case WM_NCRBUTTONDBLCLK:
	case WM_NCMBUTTONUP:
	case WM_NCMBUTTONDBLCLK:
	case WM_NCXBUTTONUP:
	case WM_NCXBUTTONDBLCLK:
	case WM_MOUSEMOVE:
		bProcessed = FALSE;

		// Disables tray icons reordering
		if(uMsg == WM_LBUTTONUP || uMsg == WM_MOUSEMOVE)
		{
			lpTrayNotifyLongPtr = GetWindowLongPtr(hTrayNotifyWnd, 0);

			if(nOptionsEx[OPT_EX_DISABLE_TRAY_ICONS_DRAG] == 1 && GetCapture() == hWnd)
			{
				dw = *EV_TRAY_NOTIFY_DRAG_FLAG(lpTrayNotifyLongPtr);
				if(dw == 1)
				{
					*EV_TRAY_NOTIFY_DRAG_FLAG(lpTrayNotifyLongPtr) = 3;

					result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
					bProcessed = TRUE;

					if(uMsg == WM_MOUSEMOVE)
						*EV_TRAY_NOTIFY_DRAG_FLAG(lpTrayNotifyLongPtr) = dw;
					else
						*EV_TRAY_NOTIFY_DRAG_FLAG(lpTrayNotifyLongPtr) = 0;
				}
			}
		}

		if(bProcessed || uMsg == WM_MOUSEMOVE)
		{
			nOption = 0;
		}
		else if(!GetMouseCtrlValue(MOUSECTRL_TARGET_VOLUMEICON, uMsg, wParam, &nOption))
		{
			if(uMsg == WM_MBUTTONUP &&
				(nOptions[OPT_WHEEL_VOLTASKBAR] == 1 || nOptions[OPT_WHEEL_VOLNOTIFY] == 1))
			{
				nOption = 5; // toggle mute system volume
			}
			else
				nOption = 0;
		}

		if(nOption)
		{
			switch(uMsg)
			{
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			case WM_XBUTTONUP:
			case WM_NCLBUTTONUP:
			case WM_NCRBUTTONUP:
			case WM_NCMBUTTONUP:
			case WM_NCXBUTTONUP:
				if(nDoubleClickOption)
				{
					nDoubleClickOption = 0;
					nOption = 0;
				}
				break;

			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDBLCLK:
			case WM_XBUTTONDBLCLK:
			case WM_NCLBUTTONDBLCLK:
			case WM_NCRBUTTONDBLCLK:
			case WM_NCMBUTTONDBLCLK:
			case WM_NCXBUTTONDBLCLK:
				nDoubleClickOption = nOption;
				break;
			}
		}

		if(nOption)
		{
			if(GetSndVolTrayIconRect(&rc))
			{
				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);

				MapWindowPoints(hWnd, NULL, &pt, 1);

				if(PtInRect(&rc, pt))
				{
					switch(uMsg)
					{
					//case WM_XBUTTONDOWN:
					case WM_XBUTTONUP:
					case WM_XBUTTONDBLCLK:
					//case WM_NCXBUTTONDOWN:
					case WM_NCXBUTTONUP:
					case WM_NCXBUTTONDBLCLK:
						result = TRUE;
						break;

					case WM_LBUTTONUP:
						// The following trick prevents the button from looking "stuck".
						DefSubclassProc(hWnd, WM_RBUTTONDOWN, wParam, lParam);
						DefSubclassProc(hWnd, WM_CANCELMODE, 0, 0);
						result = 0;
						break;

					case WM_RBUTTONUP:
						DefSubclassProc(hWnd, WM_CANCELMODE, 0, 0);
						result = 0;
						break;

					default:
						result = 0;
						break;
					}

					bProcessed = TRUE;

					LaunchEmptySpaceFunction(nOption, lpTaskbarLongPtr);
				}
			}
		}

		if(!bProcessed)
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	default:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return result;
}

static LRESULT CALLBACK NewTrayClockProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	LONG_PTR lpTrayClockLongPtr;
	BOOL bProcess;
	RECT rc;
	LRESULT result;

	wnd_proc_call_counter++;

	switch(uMsg)
	{
	case WM_PAINT:
	case WM_TIMER:
		lpTrayClockLongPtr = GetWindowLongPtr(hWnd, 0);
		bProcess = FALSE;

		if(nOptions[OPT_OTHER_CLOCKSHOWSEC] == 1)
		{
			switch(uMsg)
			{
			case WM_PAINT:
				if(*EV_TRAY_CLOCK_TIMER_ENABLED_FLAG(lpTrayClockLongPtr))
					bProcess = TRUE;
				break;

			case WM_TIMER:
				if(wParam == 0)
					bProcess = TRUE;
				break;
			}
		}

		if(bProcess)
		{
			wClockMilliseconds = (WORD)-1;

			*EV_TRAY_CLOCK_TEXT(lpTrayClockLongPtr) = L'\0';

			PointerRedirectionAdd(ppGetTimeFormat_W_or_Ex, GetTimeFormat_W_or_Ex_Hook, &prGetTimeFormat_W_or_Ex);

			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

			PointerRedirectionRemove(ppGetTimeFormat_W_or_Ex, &prGetTimeFormat_W_or_Ex);

			if(wClockMilliseconds != (WORD)-1)
				if(KillTimer(hWnd, 0))
					SetTimer(hWnd, 0, 1000 - wClockMilliseconds, NULL);
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_USER + 100: // Calc size
		if(nOptionsEx[OPT_EX_TRAY_CLOCK_FIX_WIDTH])
		{
			GetClientRect(hWnd, &rc);
			lParam = 0;
		}

		if(nOptions[OPT_OTHER_CLOCKSHOWSEC] == 1)
		{
			PointerRedirectionAdd(ppGetTimeFormat_W_or_Ex, GetTimeFormat_W_or_Ex_Hook, &prGetTimeFormat_W_or_Ex);

			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

			PointerRedirectionRemove(ppGetTimeFormat_W_or_Ex, &prGetTimeFormat_W_or_Ex);
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	default:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK NewShowDesktopProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	static int nDoubleClickOption = 0;
	int nOption;
	LRESULT result;
	BOOL bProcessed;

	wnd_proc_call_counter++;

	switch(uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCXBUTTONDOWN:
		nDoubleClickOption = 0;
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_XBUTTONUP:
	case WM_XBUTTONDBLCLK:
	case WM_NCLBUTTONUP:
	case WM_NCLBUTTONDBLCLK:
	case WM_NCRBUTTONUP:
	case WM_NCRBUTTONDBLCLK:
	case WM_NCMBUTTONUP:
	case WM_NCMBUTTONDBLCLK:
	case WM_NCXBUTTONUP:
	case WM_NCXBUTTONDBLCLK:
		bProcessed = FALSE;

		if(!GetMouseCtrlValue(MOUSECTRL_TARGET_SHOWDESKTOP, uMsg, wParam, &nOption))
		{
			nOption = 0;
		}

		if(nOption)
		{
			switch(uMsg)
			{
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			case WM_XBUTTONUP:
			case WM_NCLBUTTONUP:
			case WM_NCRBUTTONUP:
			case WM_NCMBUTTONUP:
			case WM_NCXBUTTONUP:
				if(nDoubleClickOption)
				{
					nDoubleClickOption = 0;
					nOption = 0;
				}
				break;

			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDBLCLK:
			case WM_XBUTTONDBLCLK:
			case WM_NCLBUTTONDBLCLK:
			case WM_NCRBUTTONDBLCLK:
			case WM_NCMBUTTONDBLCLK:
			case WM_NCXBUTTONDBLCLK:
				nDoubleClickOption = nOption;
				break;
			}
		}

		if(nOption)
		{
			switch(uMsg)
			{
			//case WM_XBUTTONDOWN:
			case WM_XBUTTONUP:
			case WM_XBUTTONDBLCLK:
			//case WM_NCXBUTTONDOWN:
			case WM_NCXBUTTONUP:
			case WM_NCXBUTTONDBLCLK:
				result = TRUE;
				break;

			case WM_LBUTTONUP:
				MHP_HookPostMessageW(PostMessageWIgnoreShowDesktopHook);

				result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

				MHP_HookPostMessageW(NULL);
				break;

			default:
				result = 0;
				break;
			}

			bProcessed = TRUE;

			LaunchEmptySpaceFunction(nOption, lpTaskbarLongPtr);
		}

		if(!bProcessed)
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_USER + 100: // Calc size
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		if(result > 0)
		{
			if(nOptions[OPT_OTHER_NOSHOWDESK] == 1)
				result = 0;
			else if(nOptionsEx[OPT_EX_SHOW_DESKTOP_BUTTON_SIZE])
				result = nOptionsEx[OPT_EX_SHOW_DESKTOP_BUTTON_SIZE];
		}
		break;

	case WM_PAINT:
		if(nWinVersion == WIN_VERSION_7 && nOptionsEx[OPT_EX_DISABLE_TASKBAR_TRANSPARENCY] == 2)
		{
			PointerRedirectionAdd(ppDrawThemeBackground, DrawThemeBackgroundShowDesktopBtnHook, &prDrawThemeBackgroundShowDesktopBtn);
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
			PointerRedirectionRemove(ppDrawThemeBackground, &prDrawThemeBackgroundShowDesktopBtn);
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	default:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK NewW7StartButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	LRESULT result;

	wnd_proc_call_counter++;

	switch(uMsg)
	{
	case WM_THEMECHANGED:
	case WM_DWMCOMPOSITIONCHANGED:
		if(nOptions[OPT_OTHER_NOSTARTBTN])
		{
			PointerRedirectionAdd(ppOpenThemeData, OpenThemeDataHook, &prOpenThemeData);

			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

			PointerRedirectionRemove(ppOpenThemeData, &prOpenThemeData);
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_PRINTCLIENT:
		if(nOptions[OPT_OTHER_NOSTARTBTN])
			result = 0;
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case BCM_GETIDEALSIZE:
		result = FALSE;

		if(nOptions[OPT_OTHER_NOSTARTBTN])
		{
			LONG_PTR lpW7StartMenu;
			SIZE *pszW7StartBtnSize;

			lpW7StartMenu = (LONG_PTR)EV_TASKBAR_W7_START_BTN_CLASS();

			pszW7StartBtnSize = (SIZE *)(lpW7StartMenu + DEF3264(0x20, 0x38));

			if((SIZE *)lParam == pszW7StartBtnSize &&
				pszW7StartBtnSize->cx == 0 && pszW7StartBtnSize->cy == 0)
			{
				int nDesiredSpacing = nOptionsEx[OPT_EX_NO_START_BTN_SPACING];
				if(nDesiredSpacing == 0)
				{
					// Magic 13 value taken from Windows 8
					nDesiredSpacing = 13;
				}

				int nSpacing;
				MARGINS margins;

				SendMessage(hTaskBandWnd, RB_GETBANDMARGINS, 0, (LPARAM)&margins);

				if(margins.cxLeftWidth < nDesiredSpacing)
					nSpacing = nDesiredSpacing - margins.cxLeftWidth;
				else
					nSpacing = 0;

				pszW7StartBtnSize->cx = nSpacing;
				pszW7StartBtnSize->cy = nSpacing;

				result = TRUE;
			}
		}

		if(result == FALSE)
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	case WM_MOUSELEAVE:
		if(nOptionsEx[OPT_EX_DISABLE_TOPMOST])
		{
			// If we got here, the pointed window has nothing to do with the taskbar. Turn off start button focus.
			DWORD dwSetting = *EV_TASKBAR_SETTINGS();
			*EV_TASKBAR_SETTINGS() |= 2;
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
			*EV_TASKBAR_SETTINGS() = dwSetting;
		}
		else
			result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;

	default:
		result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	wnd_proc_call_counter--;

	return result;
}

static LRESULT CALLBACK KeyboardShortcutsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_HOTKEY)
	{
		int nValue;
		if(GetKeybdShortcutValue((int)wParam, &nValue))
		{
			if(LaunchEmptySpaceFunction(nValue, lpTaskbarLongPtr))
				return TRUE;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static BOOL LaunchEmptySpaceFunction(int nFunction, LONG_PTR lpMMTaskbarLongPtr)
{
	switch(nFunction)
	{
	case 1:
		SendMessage(hTaskbarWnd, WM_COMMAND, MAKELONG(407, 0), 0); // Show desktop
		break;

	case 2:
		if(nWinVersion <= WIN_VERSION_7)
		{
			HWND hAltTabKeyHookWnd = FindWindow(L"AltTab_KeyHookWnd", NULL);
			if(hAltTabKeyHookWnd)
			{
				ATOM hAltTabAtom = GlobalFindAtom(L"_IDH_ALTTAB_STICKY_NEXT");
				if(hAltTabAtom)
				{
					PostMessage(hAltTabKeyHookWnd, WM_HOTKEY, hAltTabAtom, MAKELPARAM(MOD_ALT | MOD_CONTROL, VK_TAB));

					hAltTabAtom = GlobalFindAtom(L"_IDH_ALTTAB_PREV");
					if(hAltTabAtom)
						PostMessage(hAltTabKeyHookWnd, WM_HOTKEY, hAltTabAtom, MAKELPARAM(MOD_ALT | MOD_SHIFT, VK_TAB));
				}
			}
		}
		else if(nWinVersion <= WIN_VERSION_811)
		{
			SendMessage(hTaskbarWnd, WM_COMMAND, MAKELONG(411, 0), 0); // CTray::_ActivateWindowSwitcher
		}
		else
		{
			HWND hImmersiveWorkerWnd = GetWindows10ImmersiveWorkerWindow();
			if(hImmersiveWorkerWnd)
			{
				WPARAM wHotkeyIdentifier = DO2(0, 0 /* omitted from public code */);
				PostMessage(hImmersiveWorkerWnd, WM_HOTKEY, wHotkeyIdentifier, MAKELPARAM(MOD_ALT | MOD_CONTROL, VK_TAB));
			}
		}
		break;

	case 3:
		SendMessage(hTaskbarWnd, WM_COMMAND, MAKELONG(420, 0), 0); // Task Manager
		break;

	case 4:
		ShowInspectorDlg();
		break;

	case 5:
		ToggleVolMuted();
		break;

	case 6:
		TaskbarToggleAutoHide();
		break;

	case 7:
		if(nWinVersion <= WIN_VERSION_811)
		{
			SendMessage(hTaskbarWnd, WM_COMMAND, MAKELONG(411, 0), 0); // CTray::_ActivateWindowSwitcher
		}
		else
		{
			HWND hImmersiveWorkerWnd = GetWindows10ImmersiveWorkerWindow();
			if(hImmersiveWorkerWnd)
				PostMessage(hImmersiveWorkerWnd, WM_HOTKEY, 11, MAKELPARAM(MOD_WIN, VK_TAB));
		}
		break;

	case 8:
		if(nWinVersion <= WIN_VERSION_811)
		{
			SendMessage(hTaskbarWnd, WM_COMMAND, MAKELONG(305, 0), 0); // Start menu/screen
		}
		else
		{
			Win10ShowStartMenu(lpMMTaskbarLongPtr);
		}
		break;

	case 101:
	case 102:
	case 103:
	case 104:
	case 105:
	case 106:
	case 107:
	case 108:
	case 109:
	case 110:
		ShortcutTaskbarActiveItemFunction(nFunction);
		break;

	default:
		return LaunchCommandFromConfigString(nFunction);
	}

	return TRUE;
}

static BOOL LaunchCommandFromConfigString(int nFunction)
{
	if(nFunction < 0)
		return FALSE;

	WCHAR szValueName[sizeof("cmdline_4294967295")];
	wsprintf(szValueName, L"cmdline_%d", nFunction);

	WCHAR szCommandLine[1025];
	UINT nCommandLineSize = _countof(szCommandLine);
	switch(PSGetSingleString(L"Strings", szValueName, szCommandLine, &nCommandLineSize))
	{
	case ERROR_SUCCESS:
		if(nCommandLineSize == 0)
			return FALSE;
		break;

	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
		lstrcpy(szCommandLine, L"notepad.exe");
		nCommandLineSize = sizeof("notepad.exe") - 1;
		if(PSSetSingleString(L"Strings", szValueName, szCommandLine) != ERROR_SUCCESS)
			return FALSE;
		break;

	default:
		return FALSE;
	}

	// Partial relative path support: replace dot with the tweaker's path.
	// Complete relative path support is not possible, as we're dealing with
	// a command line, so we can get something like "notepad.exe .\1.txt".
	if(
		(szCommandLine[0] == L'.' && szCommandLine[1] == L'\\') ||
		(szCommandLine[0] == L'"' && szCommandLine[1] == L'.' && szCommandLine[2] == L'\\')
	)
	{
		WCHAR szFolderPath[MAX_PATH];
		int i = GetModuleFileName(hDllInst, szFolderPath, MAX_PATH);
		while(i-- && szFolderPath[i] != L'\\');
		szFolderPath[i] = L'\0';

		// folder_len + cmdline_len - the_dot + the_null_terminator
		if(i + nCommandLineSize - 1 + 1 > _countof(szCommandLine))
			return FALSE;

		if(szCommandLine[0] == L'.')
		{
			MoveMemory(szCommandLine + i, szCommandLine + 1, (nCommandLineSize - 1 + 1) * sizeof(WCHAR));
			CopyMemory(szCommandLine, szFolderPath, i * sizeof(WCHAR));
		}
		else // if(szCommandLine[0] == L'"')
		{
			MoveMemory(szCommandLine + 1 + i, szCommandLine + 2, (nCommandLineSize - 2 + 1) * sizeof(WCHAR));
			CopyMemory(szCommandLine + 1, szFolderPath, i * sizeof(WCHAR));
		}
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if(!CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
		return FALSE;

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return TRUE;
}

static HWND GetWindows10ImmersiveWorkerWindow(void)
{
	HWND hApplicationManagerDesktopShellWindow = FindWindow(L"ApplicationManager_DesktopShellWindow", NULL);
	if(!hApplicationManagerDesktopShellWindow)
		return NULL;

	LONG_PTR lpApplicationManagerDesktopShellWindow = GetWindowLongPtr(hApplicationManagerDesktopShellWindow, 0);
	if(!lpApplicationManagerDesktopShellWindow)
		return NULL;

	LONG_PTR lpImmersiveShellHookService = *(LONG_PTR *)(lpApplicationManagerDesktopShellWindow + 0 /* omitted from public code */);
	if(!lpImmersiveShellHookService)
		return NULL;

	LONG_PTR lpImmersiveWindowMessageService = *(LONG_PTR *)(lpImmersiveShellHookService + 0 /* omitted from public code */);
	if(!lpImmersiveWindowMessageService)
		return NULL;

	return *(HWND *)(lpImmersiveWindowMessageService + 0 /* omitted from public code */);
}

static void ShortcutTaskbarActiveItemFunction(int nFunction)
{
	POINT pt;
	GetCursorPos(&pt);
	HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

	LONG_PTR lpMMTaskListLongPtr = MMTaskListLongPtrFromMonitor(hMonitor);

	LONG_PTR *task_item = TaskbarGetActiveTaskItem(lpMMTaskListLongPtr);
	BOOL bActivateAgain = FALSE;
	if(!task_item)
	{
		task_item = ComFuncGetLastActiveTaskItem();
		bActivateAgain = TRUE;
	}

	switch(nFunction)
	{
	case 101:
		task_item = TaskbarScroll(lpMMTaskListLongPtr, -1, FALSE, nOptionsEx[OPT_EX_SCROLL_NO_WRAP] == 0, task_item);
		if(task_item)
			SwitchToTaskItem(task_item);
		break;

	case 102:
		task_item = TaskbarScroll(lpMMTaskListLongPtr, 1, FALSE, nOptionsEx[OPT_EX_SCROLL_NO_WRAP] == 0, task_item);
		if(task_item)
			SwitchToTaskItem(task_item);
		break;

	case 103:
		task_item = TaskbarScroll(lpMMTaskListLongPtr, -1, TRUE, nOptionsEx[OPT_EX_SCROLL_NO_WRAP] == 0, task_item);
		if(task_item)
			SwitchToTaskItem(task_item);
		break;

	case 104:
		task_item = TaskbarScroll(lpMMTaskListLongPtr, 1, TRUE, nOptionsEx[OPT_EX_SCROLL_NO_WRAP] == 0, task_item);
		if(task_item)
			SwitchToTaskItem(task_item);
		break;

	case 105:
		if(task_item)
		{
			if(CanRestoreTaskItem(task_item))
				PostMessage(GetTaskItemWnd(task_item), WM_SYSCOMMAND, SC_RESTORE, 0);

			if(bActivateAgain)
				SwitchToTaskItem(task_item);
		}
		break;

	case 106:
		if(task_item)
			MinimizeTaskItem(task_item);
		break;

	case 107:
		if(task_item)
		{
			if(CanMaximizeTaskItem(task_item))
				PostMessage(GetTaskItemWnd(task_item), WM_SYSCOMMAND, SC_MAXIMIZE, 0);

			if(bActivateAgain)
				SwitchToTaskItem(task_item);
		}
		break;

	case 108:
		if(task_item)
			CloseTaskItem(task_item, TRUE);
		break;

	case 109:
		if(task_item)
		{
			TaskbarMoveGroupByTaskItem(lpMMTaskListLongPtr, task_item, -1);

			if(bActivateAgain)
				SwitchToTaskItem(task_item);
		}
		break;

	case 110:
		if(task_item)
		{
			TaskbarMoveGroupByTaskItem(lpMMTaskListLongPtr, task_item, 1);

			if(bActivateAgain)
				SwitchToTaskItem(task_item);
		}
		break;
	}
}

static void SubclassSecondaryTaskListWindows(LONG_PTR lpSecondaryTaskListLongPtr)
{
	HWND hSecondaryTaskbarWnd, hSecondaryTaskBandWnd, hSecondaryTaskListWnd, hSecondaryThumbnailWnd;
	LONG_PTR lpSecondaryThumbnailLongPtr;
	DWORD *pdw;

	GetSecondaryTaskListWindows(lpSecondaryTaskListLongPtr,
		&hSecondaryTaskbarWnd, &hSecondaryTaskBandWnd, &hSecondaryTaskListWnd, &hSecondaryThumbnailWnd);

	SetWindowSubclass(hSecondaryTaskbarWnd, NewMMTaskbarProc, 0, 0);
	SetWindowSubclass(hSecondaryTaskBandWnd, NewTaskBandProc, 0, 0);
	SetWindowSubclass(hSecondaryTaskListWnd, NewTaskListProc, 0, 0);
	SetWindowSubclass(hSecondaryThumbnailWnd, NewThumbnailProc, 0, 0);

	if(nOptions[OPT_HOVER] == 1)
	{
		lpSecondaryThumbnailLongPtr = *EV_MM_TASKLIST_MM_THUMBNAIL_LONG_PTR(lpSecondaryTaskListLongPtr);

		pdw = EV_MM_THUMBNAIL_NUM_THUMBNAILS(lpSecondaryThumbnailLongPtr);

		dwOldNumThumbnails = *pdw;
		*pdw = 0;
	}

	if(nOptions[OPT_OTHER_NOSTARTBTN] && nWinVersion >= WIN_VERSION_81)
	{
		LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
		LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);

		HWND hStartBtnWnd;

		if(nWinVersion >= WIN_VERSION_10_T1)
		{
			LONG_PTR lp = *EV_SECONDARY_TASKBAR_START_BTN_LONG_PTR(lpSecondaryTaskbarLongPtr);
			hStartBtnWnd = *EV_START_BUTTON_HWND(lp);
		}
		else
			hStartBtnWnd = *EV_SECONDARY_TASKBAR_START_BTN_WND(lpSecondaryTaskbarLongPtr);

		ShowWindow(hStartBtnWnd, SW_HIDE);
	}

	if(nOptions[OPT_OTHER_CLOCKSHOWSEC] == 1 && nWinVersion >= WIN_VERSION_10_R1)
	{
		LONG_PTR lpTrayClockLongPtr = GetWindowLongPtr(hTrayClockWnd, 0);
		BYTE *pb = EV_CLOCK_BUTTON_SHOW_SECONDS(lpTrayClockLongPtr);

		LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
		LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);
		LONG_PTR lpSecondaryTrayClockLongPtr = *EV_SECONDARY_TASKBAR_CLOCK_LONG_PTR(lpSecondaryTaskbarLongPtr);

		if(lpSecondaryTrayClockLongPtr)
		{
			BYTE *pbSecondary = EV_CLOCK_BUTTON_SHOW_SECONDS(lpSecondaryTrayClockLongPtr);
			*pbSecondary = *pb;

			*EV_CLOCK_BUTTON_HOURS_CACHE(lpSecondaryTrayClockLongPtr) = -1;
			*EV_CLOCK_BUTTON_SIZES_CACHED(lpSecondaryTrayClockLongPtr) = 0;

			HWND hSecondaryClockButtonWnd = *EV_CLOCK_BUTTON_HWND(lpSecondaryTrayClockLongPtr);
			InvalidateRect(hSecondaryClockButtonWnd, NULL, FALSE);
		}
	}
}

static void UnsubclassSecondaryTaskListWindows(LONG_PTR lpSecondaryTaskListLongPtr)
{
	HWND hSecondaryTaskbarWnd, hSecondaryTaskBandWnd, hSecondaryTaskListWnd, hSecondaryThumbnailWnd;

	GetSecondaryTaskListWindows(lpSecondaryTaskListLongPtr,
		&hSecondaryTaskbarWnd, &hSecondaryTaskBandWnd, &hSecondaryTaskListWnd, &hSecondaryThumbnailWnd);

	RemoveWindowSubclass(hSecondaryTaskbarWnd, NewMMTaskbarProc, 0);
	RemoveWindowSubclass(hSecondaryTaskBandWnd, NewTaskBandProc, 0);
	RemoveWindowSubclass(hSecondaryTaskListWnd, NewTaskListProc, 0);
	RemoveWindowSubclass(hSecondaryThumbnailWnd, NewThumbnailProc, 0);
}

static void GetSecondaryTaskListWindows(LONG_PTR lpSecondaryTaskListLongPtr,
	HWND *phSecondaryTaskbarWnd, HWND *phSecondaryTaskBandWnd, HWND *phSecondaryTaskListWnd, HWND *phSecondaryThumbnailWnd)
{
	LONG_PTR lpSecondaryTaskBandLongPtr = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
	LONG_PTR lpSecondaryTaskbarLongPtr = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lpSecondaryTaskBandLongPtr);
	LONG_PTR lpSecondaryThumbnailLongPtr = *EV_MM_TASKLIST_MM_THUMBNAIL_LONG_PTR(lpSecondaryTaskListLongPtr);

	*phSecondaryTaskbarWnd = *EV_SECONDARY_TASKBAR_HWND(lpSecondaryTaskbarLongPtr);
	*phSecondaryTaskBandWnd = *EV_SECONDARY_TASK_BAND_HWND(lpSecondaryTaskBandLongPtr);
	*phSecondaryTaskListWnd = *EV_MM_TASKLIST_HWND(lpSecondaryTaskListLongPtr);
	*phSecondaryThumbnailWnd = *EV_MM_THUMBNAIL_HWND(lpSecondaryThumbnailLongPtr);
}

static HWND WINAPI GetCaptureHook(void *pRetAddr)
{
	if(
		(ULONG_PTR)pRetAddr >= (ULONG_PTR)ExplorerModuleInfo.lpBaseOfDll &&
		(ULONG_PTR)pRetAddr < (ULONG_PTR)ExplorerModuleInfo.lpBaseOfDll + ExplorerModuleInfo.SizeOfImage &&
		GetCurrentThreadId() == dwTaskbarThreadId
	)
		return NULL;

	return OriginalGetCapture();
}

static SHORT WINAPI GetKeyStateInvertShiftHook(void *pRetAddr, int nVirtKey)
{
	SHORT nRet = OriginalGetKeyState(nVirtKey);

	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(nVirtKey == VK_SHIFT)
			nRet ^= 0x8000;
	}

	return nRet;
}

static BOOL WINAPI SetWindowPosHook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(nOptionsEx[OPT_EX_DISABLE_TOPMOST])
		{
			if(hWndInsertAfter == HWND_TOPMOST)
			{
				if(IsTaskbarWindow(hWnd) ||
					(nWinVersion == WIN_VERSION_7 && hWnd == hW7StartBtnWnd))
				{
					hWndInsertAfter = HWND_TOP;
				}
			}
		}

		if(nOptions[OPT_OTHER_NOSTARTBTN] && nWinVersion >= WIN_VERSION_10_T1)
		{
			HWND hAncestorWnd = GetAncestor(hWnd, GA_ROOT);
			if(hAncestorWnd != hWnd && IsTaskbarWindow(hAncestorWnd))
			{
				int nTaskbarPos;
				HWND hStartBtnWnd;
				HWND hSearchBtnWnd = NULL, hCortanaBtnWnd = NULL, hMultitaskBtnWnd = NULL, hBackBtnWnd = NULL, hSearchBarWnd = NULL;

				if(hAncestorWnd == hTaskbarWnd)
				{
					nTaskbarPos = *EV_TASKBAR_POS();

					LONG_PTR lp = *EV_TASKBAR_START_BTN_LONG_PTR();
					hStartBtnWnd = *EV_START_BUTTON_HWND(lp);

					lp = *EV_TASKBAR_SEARCH_LONG_PTR();
					if(lp)
						hSearchBtnWnd = *EV_TRAY_BUTTON_HWND(lp);

					if(nWinVersion >= WIN_VERSION_10_19H1)
					{
						lp = *EV_TASKBAR_CORTANA_LONG_PTR();
						if(lp)
							hCortanaBtnWnd = *EV_TRAY_BUTTON_HWND(lp);
					}

					lp = *EV_TASKBAR_MULTITASKING_LONG_PTR();
					if(lp)
						hMultitaskBtnWnd = *EV_TRAY_BUTTON_HWND(lp);

					lp = *EV_TASKBAR_BACK_LONG_PTR();
					if(lp)
						hBackBtnWnd = *EV_TRAY_BUTTON_HWND(lp);

					HWND *hExtraWnds = *EV_TASKBAR_EXTRA_BTN_HWNDS();
					if(hExtraWnds)
						hSearchBarWnd = hExtraWnds[0];
				}
				else
				{
					LONG_PTR lpSecondaryTaskbarLongPtr = GetWindowLongPtr(hAncestorWnd, 0);

					nTaskbarPos = *EV_SECONDARY_TASKBAR_POS(lpSecondaryTaskbarLongPtr);

					LONG_PTR lp = *EV_SECONDARY_TASKBAR_START_BTN_LONG_PTR(lpSecondaryTaskbarLongPtr);
					hStartBtnWnd = *EV_START_BUTTON_HWND(lp);

					lp = *EV_SECONDARY_TASKBAR_SEARCH_LONG_PTR(lpSecondaryTaskbarLongPtr);
					if(lp)
						hSearchBtnWnd = *EV_TRAY_BUTTON_HWND(lp);

					if(nWinVersion >= WIN_VERSION_10_19H1)
					{
						lp = *EV_SECONDARY_TASKBAR_CORTANA_LONG_PTR(lpSecondaryTaskbarLongPtr);
						if(lp)
							hCortanaBtnWnd = *EV_TRAY_BUTTON_HWND(lp);
					}

					lp = *EV_SECONDARY_TASKBAR_MULTITASKING_LONG_PTR(lpSecondaryTaskbarLongPtr);
					if(lp)
						hMultitaskBtnWnd = *EV_TRAY_BUTTON_HWND(lp);
				}

				if(hWnd == hSearchBtnWnd || hWnd == hCortanaBtnWnd || hWnd == hMultitaskBtnWnd || hWnd == hSearchBarWnd || hWnd == hBackBtnWnd)
				{
					RECT rcStartBtn;
					GetWindowRect(hStartBtnWnd, &rcStartBtn);

					int nSpacing = nOptionsEx[OPT_EX_NO_START_BTN_SPACING];

					switch(nTaskbarPos)
					{
					// Horizontal
					case 1: // Is taskbar on top of the screen
					case 3: // Is taskbar on bottom of the screen
						X -= rcStartBtn.right - rcStartBtn.left;
						X += nSpacing;
						break;

					// Vertical
					case 0:
					case 2:
						Y -= rcStartBtn.bottom - rcStartBtn.top;
						Y += nSpacing;
						break;
					}
				}
			}
		}
	}

	return ((BOOL(WINAPI *)(HWND, HWND, int, int, int, int, UINT))pSetWindowPosOriginal)(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

static LONG WINAPI GetWindowLongWAlwaysTopmostHook(void *pRetAddr, HWND hWnd, int nIndex)
{
	LONG lRet = OriginalGetWindowLongW(hWnd, nIndex);

	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(nIndex == GWL_EXSTYLE && IsTaskbarWindow(hWnd))
		{
			lRet |= WS_EX_TOPMOST;
		}
	}

	return lRet;
}

static BOOL WINAPI PostMessageWIgnoreTopmostHook(void *pRetAddr, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		// Ignore requests to reset the topmost flag
		if(hWnd == hTaskbarWnd && Msg == WM_USER + 385) // 0x581
			return TRUE;
	}

	return OriginalPostMessageW(hWnd, Msg, wParam, lParam);
}

static BOOL WINAPI PostMessageWIgnoreShowDesktopHook(void *pRetAddr, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		// Ignore requests to show desktop
		if(hWnd == hTaskbarWnd && Msg == WM_USER + 377) // 0x579
			return TRUE;
	}

	return OriginalPostMessageW(hWnd, Msg, wParam, lParam);
}

static int WINAPI MulDivHook(int nNumber, int nNumerator, int nDenominator)
{
	// assert(nWinVersion >= WIN_VERSION_8)

	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(nOptionsEx[OPT_EX_W10_LARGE_ICONS] && nWinVersion >= WIN_VERSION_10_T1 &&
			nInTrayNotifyWndWindowPosChanged == 0 && // prevents the touch keyboard button from getting bigger
			(ComFuncIsInGetIdealSpan() || nMulDivHookLargeIconsCounter > 0)
		)
		{
			if(nNumber == 24 && nDenominator == 96)
			{
				if(!(TaskbarGetPreference(lpTaskListLongPtr) & 0x10)) // if not using small icons
					nNumber = 32;
			}
		}

		if(nOptionsEx[OPT_EX_NO_WIDTH_LIMIT])
		{
			if(nNumber == 62 && nDenominator == 96)
			{
				if(TaskbarGetPreference(lpTaskListLongPtr) & 0x10) // if using small icons
					nNumber = 32;
				else
					nNumber = 48;
			}
		}
	}

	return ((int(WINAPI *)(int, int, int))prMulDiv.pOriginalAddress)(nNumber, nNumerator, nDenominator);
}

static HANDLE WINAPI SHChangeNotification_LockHook(HANDLE hChange, DWORD dwProcId, PIDLIST_ABSOLUTE **pppidl, LONG *plEvent)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(hChange == (HANDLE)(UINT_PTR)0xDEADBEEF && dwProcId == 0x12345678)
		{
			*pppidl = NULL;
			*plEvent = SHCNE_ASSOCCHANGED;
			return (HANDLE)(UINT_PTR)0xBEEFDEAD;
		}
	}

	return ((HANDLE(WINAPI *)(HANDLE, DWORD, PIDLIST_ABSOLUTE **, LONG *))prSHChangeNotification_Lock.pOriginalAddress)(hChange, dwProcId, pppidl, plEvent);
}

static BOOL WINAPI SHChangeNotification_UnlockHook(HANDLE hLock)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(hLock == (HANDLE)(UINT_PTR)0xBEEFDEAD)
		{
			return TRUE;
		}
	}

	return ((BOOL(WINAPI *)(HANDLE))prSHChangeNotification_Unlock.pOriginalAddress)(hLock);
}

static int WINAPI GetTimeFormat_W_or_Ex_Hook(LONG_PTR var1, DWORD dwFlags, SYSTEMTIME *lpTime, LPCWSTR lpFormat, LPWSTR lpTimeStr, int cchTime)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(wClockMilliseconds == (WORD)-1)
			wClockMilliseconds = lpTime->wMilliseconds;

		dwFlags &= ~TIME_NOSECONDS;
	}

	return ((int (WINAPI *)(LONG_PTR, DWORD, SYSTEMTIME *, LPCWSTR, LPWSTR, int))prGetTimeFormat_W_or_Ex.pOriginalAddress)(var1, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
}

static HTHEME WINAPI OpenThemeDataHook(HWND hwnd, LPCWSTR pszClassList)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(hwnd == hW7StartBtnWnd && lstrcmp(pszClassList, L"Button") == 0)
		{
			return NULL;
		}
	}

	return ((HWND(WINAPI *)(HWND, LPCWSTR))prOpenThemeData.pOriginalAddress)(hwnd, pszClassList);
}

static HWND WINAPI ChildWindowFromPointHook(HWND hWndParent, POINT Point)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		// We hook this only for the explorer module (IAT hook).
		// Seems like in Windows 7, there's only one location where this
		// function is used, so this general check should be safe to use.

		if(hWndParent != hTaskbarWnd)
			MapWindowPoints(hTaskbarWnd, hWndParent, &Point, 1);
	}

	return ((HWND(WINAPI *)(HWND, POINT))prChildWindowFromPoint.pOriginalAddress)(hWndParent, Point);
}

static HRESULT WINAPI DwmEnableBlurBehindWindowHook(void *pRetAddr, HWND hWnd, const DWM_BLURBEHIND *pBlurBehind)
{
	DWM_BLURBEHIND dwmNewBlurBehind;

	if(
		(ULONG_PTR)pRetAddr >= (ULONG_PTR)ExplorerModuleInfo.lpBaseOfDll &&
		(ULONG_PTR)pRetAddr < (ULONG_PTR)ExplorerModuleInfo.lpBaseOfDll + ExplorerModuleInfo.SizeOfImage &&
		GetCurrentThreadId() == dwTaskbarThreadId
	)
	{
		if((!!pBlurBehind->fEnable) == (nWinVersion >= WIN_VERSION_8) ? FALSE : TRUE)
		{
			if(IsTaskbarWindow(hWnd))
			{
				dwmNewBlurBehind = *pBlurBehind;
				dwmNewBlurBehind.fEnable = !dwmNewBlurBehind.fEnable;

				pBlurBehind = &dwmNewBlurBehind;
			}
		}
	}

	return OriginalDwmEnableBlurBehindWindow(hWnd, pBlurBehind);
}

static int WINAPI GetSystemMetricsHook(void *pRetAddr, int nIndex)
{
	int nRet = OriginalGetSystemMetrics(nIndex);

	if(
		(ULONG_PTR)pRetAddr >= (ULONG_PTR)ExplorerModuleInfo.lpBaseOfDll &&
		(ULONG_PTR)pRetAddr < (ULONG_PTR)ExplorerModuleInfo.lpBaseOfDll + ExplorerModuleInfo.SizeOfImage &&
		GetCurrentThreadId() == dwTaskbarThreadId
	)
	{
		if(nIndex == SM_CXSMICON)
		{
			MHP_HookGetSystemMetrics(NULL);

			LONG_PTR lpTrayNotifyLongPtr = GetWindowLongPtr(hTrayNotifyWnd, 0);
			*EV_TRAY_NOTIFY_PTRDEV_SUPPORTED(lpTrayNotifyLongPtr) = bPrevPtrDevSupported;
			*EV_TRAY_NOTIFY_PTRDEV_SUPPORTED_VALID(lpTrayNotifyLongPtr) = bPrevPtrDevSupportedValid;

			return (nRet + nOptionsEx[OPT_EX_TRAY_ICONS_PADDING]) / 2;
		}
	}

	return nRet;
}

static LONG_PTR WINAPI SetWindowBandNoChangeTaskbarHook(HWND hWnd, LONG_PTR var2, LONG_PTR var3)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(var2 == 0 && var3 != 1 && IsTaskbarWindow(hWnd))
		{
			// var2 - ???
			// var3 - band id?
			// So we prevent from setting the band of taskbar windows to anything other than 1.

			// Bring the window to the foreground with the original SetWindowPos function, using the HWND_TOPMOST trick.
			if(pSetWindowPosOriginal)
			{
				((BOOL(WINAPI *)(HWND, HWND, int, int, int, int, UINT))pSetWindowPosOriginal)(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
				((BOOL(WINAPI *)(HWND, HWND, int, int, int, int, UINT))pSetWindowPosOriginal)(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
			}

			return 1;
		}
	}

	return ((LONG_PTR(WINAPI *)(HWND, LONG_PTR, LONG_PTR))pSetWindowBandOriginal)(hWnd, var2, var3);
}

static BOOL WINAPI DPA_SortHook(HDPA hdpa, PFNDACOMPARE pfnCompare, LPARAM lParam)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId && bEnableDPA_SortHook && lParam == 0)
	{
		for(int i = 0; i < DPA_GetPtrCount(hdpa);)
		{
			LONG_PTR *task_item = (LONG_PTR *)DPA_FastGetPtr(hdpa, i);

			LONG_PTR this_ptr = (LONG_PTR)task_item;
			LONG_PTR *plp = *(LONG_PTR **)this_ptr;

			// CTaskItem::IsVisibleOnCurrentVirtualDesktop(this)
			BOOL bVisibleOnCurrentVirtualDesktop = FUNC_CTaskItem_IsVisibleOnCurrentVirtualDesktop(plp)(this_ptr);

			if(!bVisibleOnCurrentVirtualDesktop)
			{
				DPA_DeletePtr(hdpa, i);

				// CTaskItem::Release(this)
				((ULONG(__stdcall *)(LONG_PTR))plp[2])(this_ptr);
			}
			else
			{
				i++;
			}
		}
	}

	return ((BOOL(WINAPI *)(HDPA, PFNDACOMPARE, LPARAM))pDPA_SortOriginal)(hdpa, pfnCompare, lParam);
}

static HRESULT WINAPI DrawThemeBackgroundTaskbarHook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(iPartId >= 1 && iPartId <= 4 && iStateId == 0 && hTaskbarNonCompositionTheme)
			hTheme = hTaskbarNonCompositionTheme;
	}

	return ((HRESULT(WINAPI *)(HTHEME, HDC, int, int, LPCRECT, LPCRECT))prDrawThemeBackgroundTaskbar.pOriginalAddress)
		(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

static HRESULT WINAPI DrawThemeBackgroundShowDesktopBtnHook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		if(iPartId == 1)
			DrawThemeParentBackground(hShowDesktopWnd, hdc, NULL);
	}

	return ((HRESULT(WINAPI *)(HTHEME, HDC, int, int, LPCRECT, LPCRECT))prDrawThemeBackgroundShowDesktopBtn.pOriginalAddress)
		(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}
