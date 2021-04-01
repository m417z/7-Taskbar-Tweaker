#include "stdafx.h"
#include "exe.h"
#include "version.h"
#include "portable_settings.h"
#include "functions.h"
#include "options_load_save.h"
#include "options_dlg_handle.h"
#include "tweaker_messages.h"
#include "explorer_inject.h"
#include "library_load_errors.h"
#include "settings.h"
#include "settings_dlg.h"
#include "advanced_options_dlg.h"
#include "resource.h"
#include "settings.h"

int argc;
WCHAR **argv;
WCHAR szLauncherPath[MAX_PATH];
WCHAR szIniFile[MAX_PATH] = L"";

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	argv = CommandLineToArgvW(GetCommandLine(), &argc);
	if(!argv)
	{
		MessageBox(NULL, L"CommandLineToArgvW failed", NULL, MB_ICONEXCLAMATION);
		goto Exit;
	}

	// Application ID
	SetCurrentProcessExplicitAppUserModelID(L"7+ Taskbar Tweaker");

	// Init common controls
	INITCOMMONCONTROLSEX icce;
	ZeroMemory(&icce, sizeof(INITCOMMONCONTROLSEX));
	icce.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icce.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES | ICC_STANDARD_CLASSES;

	InitCommonControlsEx(&icce);

	// Init launcher
	int nArgIndex = FindCmdLineSwitch(L"-launcher");
	if(nArgIndex != 0 && nArgIndex+1 < argc && lstrlen(argv[nArgIndex+1]) < MAX_PATH)
		lstrcpy(szLauncherPath, argv[nArgIndex+1]);
	else
		GetModuleFileName(NULL, szLauncherPath, MAX_PATH);

	// Init registry/ini settings
	InitSettings();

	// Get language setting
	if(!LoadTweakerSettings())
		MessageBox(NULL, L"Could not load settings", NULL, MB_ICONEXCLAMATION);
	else if(twSettings.nLanguage)
		SetThreadUILanguage(twSettings.nLanguage);

	// Check OS version
	BOOL bIsWindows10 = CompareWindowsVersion(10, 0);
	if(
		!CompareWindowsVersion(6, 1) &&
		!CompareWindowsVersion(6, 2) &&
		!CompareWindowsVersion(6, 3) &&
		!CompareWindowsVersion(6, 4) &&
		!bIsWindows10
	)
	{
		MessageBox(NULL, LoadStrFromRsrc(IDS_ERROR_WINXONLY), NULL, MB_ICONHAND);
		goto Exit;
	}

	if(
		bIsWindows10 &&
		!CompareWindowsBuildNumber(10240) &&
		!CompareWindowsBuildNumber(10586) &&
		!CompareWindowsBuildNumber(14393) &&
		!CompareWindowsBuildNumber(15063) &&
		!CompareWindowsBuildNumber(16299) &&
		!CompareWindowsBuildNumber(17134) &&
		!CompareWindowsBuildNumber(17763) &&
		!CompareWindowsBuildNumber(18362) &&
		!CompareWindowsBuildNumber(18363) &&
		!CompareWindowsBuildNumber(19041) &&
		!CompareWindowsBuildNumber(19042)
	)
	{
		if(MessageBox(NULL, LoadStrFromRsrc(IDS_BUILD_WARNING_TEXT), LoadStrFromRsrc(IDS_BUILD_WARNING_TITLE), MB_ICONEXCLAMATION | MB_YESNO) != IDYES)
		{
			goto Exit;
		}
	}

	// Using the 32-bit version on a 64-bit OS?
#ifndef _WIN64
	BOOL bWow64Process;
	if(IsWow64Process(GetCurrentProcess(), &bWow64Process) && bWow64Process)
	{
		MessageBox(NULL, LoadStrFromRsrc(IDS_ERROR_64OS), NULL, MB_ICONHAND);
		goto Exit;
	}
#endif

	// Run!
	Run();

Exit:
	LocalFree(argv);
	CoUninitialize();

	ExitProcess(0);
}

BOOL InitSettings(void)
{
	int nArgIndex = FindCmdLineSwitch(L"-ini");
	if(nArgIndex == 0 || nArgIndex + 1 > argc - 1)
	{
		return PSInit(PS_REGISTRY, L"7 Taskbar Tweaker");
	}

	WCHAR *pIniFile = argv[nArgIndex + 1];
	if(PSInit(PS_INI, pIniFile) != ERROR_SUCCESS)
	{
		return FALSE;
	}

	WCHAR szAlternativeIniPath[MAX_PATH];

	WCHAR szAlternativeIniPathRelative[MAX_PATH];
	UINT nRelativePathSize = MAX_PATH;
	if(PSGetSingleString(NULL, L"alternative_ini_path", szAlternativeIniPathRelative, &nRelativePathSize) == ERROR_SUCCESS &&
		nRelativePathSize > 0)
	{
		WCHAR szLauncherDir[MAX_PATH];
		lstrcpy(szLauncherDir, szLauncherPath);
		*PathFindFileName(szLauncherDir) = L'\0';

		if(!PathCombine(szAlternativeIniPath, szLauncherDir, szAlternativeIniPathRelative) ||
			PSInit(PS_INI, szAlternativeIniPath) != ERROR_SUCCESS)
		{
			return FALSE;
		}

		pIniFile = szAlternativeIniPath;
	}

	lstrcpy(szIniFile, pIniFile);
	return TRUE;
}

BOOL Run(void)
{
	// Make sure this version and an old version of the tweaker aren't running simultaneously
	HANDLE hMutexOld = CreateMutex(NULL, FALSE, L"7 Taskbar Tweaker");
	if(!hMutexOld)
	{
		MessageBox(NULL, L"CreateMutex() failed", NULL, MB_ICONHAND);
		return FALSE;
	}

	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if(WaitForSingleObject(hMutexOld, 0) == WAIT_OBJECT_0)
		{
			ReleaseMutex(hMutexOld);
		}
		else
		{
			HWND hTweakerWnd = FindWindow(L"7+ Taskbar Tweaker", NULL);
			if(hTweakerWnd)
			{
				DWORD dwProcessId;

				GetWindowThreadProcessId(hTweakerWnd, &dwProcessId);
				AllowSetForegroundWindow(dwProcessId);

				PostMessage(hTweakerWnd, RegisterWindowMessage(L"7 Taskbar Tweaker"), VER_FILE_VERSION_LONG, MSG_EXE_SHOWWINDOW);
			}
			else
			{
				AllowSetForegroundWindow(ASFW_ANY);

				PostMessage(HWND_BROADCAST, RegisterWindowMessage(L"7 Taskbar Tweaker"), VER_FILE_VERSION_LONG, MSG_EXE_SHOWWINDOW);
			}

			CloseHandle(hMutexOld);
			return TRUE;
		}
	}

	HANDLE hDesktop = GetThreadDesktop(GetCurrentThreadId());
	if(!hDesktop)
	{
		MessageBox(NULL, L"GetThreadDesktop() failed", NULL, MB_ICONHAND);
		CloseHandle(hMutexOld);
		return FALSE;
	}

	WCHAR szMutexName[MAX_PATH];
	lstrcpy(szMutexName, L"7TT_");
	if(!GetUserObjectInformation(hDesktop, UOI_NAME, szMutexName + 4, sizeof(szMutexName)-(4 * sizeof(WCHAR)), NULL))
	{
		MessageBox(NULL, L"GetUserObjectInformation() failed", NULL, MB_ICONHAND);
		CloseHandle(hMutexOld);
		return FALSE;
	}

	HANDLE hMutex = CreateMutex(NULL, TRUE, szMutexName);
	if(!hMutex)
	{
		MessageBox(NULL, L"CreateMutex() failed", NULL, MB_ICONHAND);
		CloseHandle(hMutexOld);
		return FALSE;
	}

	if(GetLastError() != ERROR_ALREADY_EXISTS)
	{
		RegisterDialogClass(L"7+ Taskbar Tweaker", GetModuleHandle(NULL));

		DLG_PARAM DlgParam;
		DlgParam.bInitialized = FALSE;

		for(;;)
		{
			HWND hWnd = CreateDialogParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DlgProc, (LPARAM)&DlgParam);
			if(!hWnd)
				break;

			if(!DlgParam.bHideWnd)
				ShowWindow(hWnd, SW_SHOWNORMAL);

			for(int i = 0; i < DlgParam.nInitErrorsCount; i++)
				MessageBox(hWnd, LoadStrFromRsrc(DlgParam.dwInitErrors[i]), NULL, MB_ICONHAND);

			if(DlgParam.uInjectionErrorID)
				InjectionErrorMsgBox(hWnd, DlgParam.uInjectionErrorID);

			MSG msg;
			BOOL bRet;
			while((bRet = GetMessage(&msg, 0, 0, 0)) != FALSE)
			{
				if(bRet == -1)
					continue;

				if(!IsDialogMessage(hWnd, &msg) &&
					(!DlgParam.hAdvancedOptionsWnd || !IsDialogMessage(DlgParam.hAdvancedOptionsWnd, &msg)))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

			if(msg.wParam <= 0)
				break;
		}

		UnregisterClass(L"7+ Taskbar Tweaker", GetModuleHandle(NULL));

		ReleaseMutex(hMutex);
	}
	else
	{
		HWND hTweakerWnd = FindWindow(L"7+ Taskbar Tweaker", NULL);
		if(hTweakerWnd)
		{
			DWORD dwProcessId;

			GetWindowThreadProcessId(hTweakerWnd, &dwProcessId);
			AllowSetForegroundWindow(dwProcessId);

			PostMessage(hTweakerWnd, RegisterWindowMessage(L"7 Taskbar Tweaker"), VER_FILE_VERSION_LONG, MSG_EXE_SHOWWINDOW);
		}
		else
		{
			AllowSetForegroundWindow(ASFW_ANY);

			PostMessage(HWND_BROADCAST, RegisterWindowMessage(L"7 Taskbar Tweaker"), VER_FILE_VERSION_LONG, MSG_EXE_SHOWWINDOW);
		}
	}

	CloseHandle(hMutex);
	CloseHandle(hMutexOld);

	return TRUE;
}

LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DLG_PARAM *pDlgParam;

	if(uMsg == WM_INITDIALOG)
	{
		SetWindowLongPtr(hWnd, DWLP_USER, lParam);
		pDlgParam = (DLG_PARAM *)lParam;
	}
	else
	{
		pDlgParam = (DLG_PARAM *)GetWindowLongPtr(hWnd, DWLP_USER);
		if(!pDlgParam)
			return FALSE;
	}

	switch(uMsg)
	{
	case WM_INITDIALOG:
		return OnInitDialog(hWnd, uMsg, wParam, lParam, pDlgParam);

	case WM_NCHITTEST:
		return OnNcHitTest(hWnd, uMsg, wParam, lParam, pDlgParam);

	case WM_CTLCOLORDLG:
		return OnCtlColorDlg(hWnd, uMsg, wParam, lParam, pDlgParam);

	case WM_CTLCOLORSTATIC:
		return OnCtlColorStatic(hWnd, uMsg, wParam, lParam, pDlgParam);

	case WM_DPICHANGED:
		return OnDpiChanged(hWnd, uMsg, wParam, lParam, pDlgParam);

	case UWM_NOTIFYICON:
		return OnUNotifyIcon(hWnd, uMsg, wParam, lParam, pDlgParam);

	case UWM_SETTING_DLG:
		return OnUSettingsDlg(hWnd, uMsg, wParam, lParam, pDlgParam);

	case UWM_ADVANCED_OPTS_DLG:
		return OnUAdvancedOptsDlg(hWnd, uMsg, wParam, lParam, pDlgParam);

	case UWM_EJECTED_FROM_EXPLORER:
		return OnUEjectedFromExplorer(hWnd, uMsg, wParam, lParam, pDlgParam);

	case WM_COPYDATA:
		return OnCopyData(hWnd, uMsg, wParam, lParam, pDlgParam);

	case WM_COMMAND:
		return OnCommand(hWnd, uMsg, wParam, lParam, pDlgParam);

	case WM_SYSCOMMAND:
		return OnSysCommand(hWnd, uMsg, wParam, lParam, pDlgParam);

	case UWM_EXITBLOCKACQUIRE:
		return OnUExitBlockAcquire(hWnd, uMsg, wParam, lParam, pDlgParam);

	case UWM_EXITBLOCKRELEASE:
		return OnUExitBlockRelease(hWnd, uMsg, wParam, lParam, pDlgParam);

	case UWM_EXIT:
		return OnUExit(hWnd, uMsg, wParam, lParam, pDlgParam);

	case WM_DESTROY:
		return OnDestroy(hWnd, uMsg, wParam, lParam, pDlgParam);

	default:
		if(uMsg == pDlgParam->uTaskbarCreatedMsg)
			return OnRegisteredTaskbarCreated(hWnd, uMsg, wParam, lParam, pDlgParam);

		if(uMsg == pDlgParam->uTweakerMsg)
			return OnRegisteredTweakerMsg(hWnd, uMsg, wParam, lParam, pDlgParam);
		break;
	}

	return FALSE;
}

LRESULT OnInitDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	UINT uErrorID;

	pDlgParam->hAdvancedOptionsWnd = NULL;
	pDlgParam->nInitErrorsCount = 0;
	pDlgParam->uInjectionErrorID = 0;
	pDlgParam->bLangChanged = FALSE;
	pDlgParam->nExitBlockCount = 0;
	pDlgParam->bClosing = FALSE;

	if(pDlgParam->bInitialized)
	{
		// We're back after a setting change
		if(pDlgParam->hSmallIcon)
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)pDlgParam->hSmallIcon);

		if(pDlgParam->hLargeIcon)
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)pDlgParam->hLargeIcon);

		pDlgParam->hBgBrush = CreateBgBrush(hWnd);

		CustomizeSystemMenu(hWnd);

		InitNotifyIconData(hWnd, pDlgParam);
		Shell_NotifyIcon(NIM_ADD, &pDlgParam->nid);
		Shell_NotifyIcon(NIM_SETVERSION, &pDlgParam->nid);

		InitDlg(hWnd, pDlgParam->nOptions);

		if(ExplorerIsInjected())
			SendNewHwndMessage(ExplorerGetTaskbarWnd(), pDlgParam->uTweakerMsg, hWnd);
		else
			EnableOptions(hWnd, FALSE);

		return FALSE;
	}

	// Messages
	pDlgParam->uTaskbarCreatedMsg = RegisterWindowMessage(L"TaskbarCreated");
	pDlgParam->uTweakerMsg = RegisterWindowMessage(L"7 Taskbar Tweaker");

	// Interface
	if(!LoadCustomIcon(&pDlgParam->hSmallIcon, &pDlgParam->hLargeIcon))
	{
		pDlgParam->hSmallIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TRAY), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
		pDlgParam->hLargeIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
	}

	if(pDlgParam->hSmallIcon)
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)pDlgParam->hSmallIcon);

	if(pDlgParam->hLargeIcon)
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)pDlgParam->hLargeIcon);

	pDlgParam->hUpdateIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_UPDATE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

	pDlgParam->hBgBrush = CreateBgBrush(hWnd);

	pDlgParam->bHideWnd = (FindCmdLineSwitch(L"-hidewnd") != 0);

	CustomizeSystemMenu(hWnd);

	InitNotifyIconData(hWnd, pDlgParam);
	Shell_NotifyIcon(NIM_ADD, &pDlgParam->nid);
	Shell_NotifyIcon(NIM_SETVERSION, &pDlgParam->nid);

	// Options
	if(!LoadOptions(pDlgParam->nOptions))
		pDlgParam->dwInitErrors[pDlgParam->nInitErrorsCount++] = IDS_ERROR_LOADCONF;

	InitDlg(hWnd, pDlgParam->nOptions);

	// Inject
	uErrorID = ExplorerInject(hWnd, UWM_EJECTED_FROM_EXPLORER, twSettings.nLanguage, pDlgParam->nOptions, szIniFile);
	if(uErrorID)
	{
		// Don't show an error message if the taskbar wasn't found
		if(uErrorID != IDS_INJERROR_NOTBAR)
			pDlgParam->uInjectionErrorID = uErrorID;

		EnableOptions(hWnd, FALSE);
	}

	// Done
	pDlgParam->bInitialized = TRUE;

	return FALSE;
}

LRESULT OnNcHitTest(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	LRESULT result = DefWindowProc(hWnd, uMsg, wParam, lParam);
	if(result == HTCLIENT)
		result = HTCAPTION;

	SetWindowLongPtr(hWnd, DWLP_MSGRESULT, result);
	return TRUE;
}

LRESULT OnCtlColorDlg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	if(IsHighContrastOn())
		return 0;

	COLORREF bkColor = GetSysColor(COLOR_WINDOW);
	if(GetRValue(bkColor) + GetGValue(bkColor) + GetBValue(bkColor) < 128 * 3)
	{
		// Dark background, leave it to keep text readable
		return 0;
	}

	return (LRESULT)pDlgParam->hBgBrush;
}

LRESULT OnCtlColorStatic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	RECT rc;

	if(IsHighContrastOn())
		return 0;

	COLORREF bkColor = GetSysColor(COLOR_WINDOW);
	if(GetRValue(bkColor) + GetGValue(bkColor) + GetBValue(bkColor) < 128 * 3)
	{
		// Dark background, leave it to keep text readable
		return 0;
	}

	SetBkMode((HDC)wParam, TRANSPARENT);
	GetWindowRect((HWND)lParam, &rc);
	MapWindowPoints(NULL, hWnd, (LPPOINT)&rc, 2);
	SetBrushOrgEx((HDC)wParam, -rc.left, -rc.top, NULL);
	return (LRESULT)pDlgParam->hBgBrush;
}

LRESULT OnDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM* pDlgParam)
{
	if(pDlgParam->hBgBrush)
		DeleteObject(pDlgParam->hBgBrush);

	pDlgParam->hBgBrush = CreateBgBrush(hWnd);
	return 0;
}

LRESULT OnUNotifyIcon(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	switch(LOWORD(lParam))
	{
	case NIN_SELECT:
	case NIN_KEYSELECT:
		if(twSettings.bTrayOpensInspector)
		{
			if(IsWindowEnabled(hWnd))
			{
				if(ExplorerIsInjected())
					SendInspectorMessageFromTray(ExplorerGetTaskbarWnd(), pDlgParam->uTweakerMsg);
				else
					MessageBox(hWnd, LoadStrFromRsrc(IDS_INFO_NOTINJ), LoadStrFromRsrc(IDS_INFO_NOTINJ_CAPT), MB_ICONASTERISK);
			}
			else
			{
				ShowWindow(hWnd, SW_SHOWNORMAL);
				SetForegroundWindow(GetLastActivePopup(hWnd));
			}
		}
		else
		{
			if(!IsWindowVisible(hWnd))
			{
				ShowWindow(hWnd, SW_SHOWNORMAL);
				SetForegroundWindow(GetLastActivePopup(hWnd));
			}
			else if(IsWindowEnabled(hWnd))
				ShowWindow(hWnd, SW_HIDE);
			else
				SetForegroundWindow(GetLastActivePopup(hWnd));
		}
		break;

	case WM_CONTEXTMENU:
		if(IsWindowEnabled(hWnd))
		{
			SetForegroundWindow(hWnd);

			HMENU hMenu = MakeTrayRightClickMenu(FALSE);
			if(hMenu)
			{
				POINT pt;
				GetCursorPos(&pt);
				TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
				DestroyMenu(hMenu);
			}
		}
		else
		{
			ShowWindow(hWnd, SW_SHOWNORMAL);
			SetForegroundWindow(GetLastActivePopup(hWnd));
		}
		break;
	}

	return FALSE;
}

LRESULT OnUSettingsDlg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	switch(wParam)
	{
	case SETTINGS_HIDETRAY_CHANGED:
		UpdateNotifyIconData(hWnd, pDlgParam);

		Shell_NotifyIcon(NIM_MODIFY, &pDlgParam->nid);
		break;

	case SETTINGS_TRAY_CHANGED:
		break;

	case SETTINGS_LANG_CHANGED:
		break;

	case SETTINGS_UPDATE_CHANGED:
		// Omitted from public code
		break;

	case SETTINGS_UPDATE_AUTO_CHANGED:
		break;
	}

	return FALSE;
}

LRESULT OnUAdvancedOptsDlg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	switch(wParam)
	{
	case ADV_OPTS_OPTIONSEX:
		if(ExplorerIsInjected())
			PostMessage(ExplorerGetTaskbarWnd(), pDlgParam->uTweakerMsg, wParam, MSG_DLL_RELOAD_OPTIONSEX);
		break;

	case ADV_OPTS_MOUSE_BUTTON_CONTROL:
		if(ExplorerIsInjected())
			PostMessage(ExplorerGetTaskbarWnd(), pDlgParam->uTweakerMsg, wParam, MSG_DLL_RELOAD_MOUSE_BUTTON_CONTROL);
		break;

	case ADV_OPTS_KEYBOARD_SHORTCUTS:
		if(ExplorerIsInjected())
			PostMessage(ExplorerGetTaskbarWnd(), pDlgParam->uTweakerMsg, wParam, MSG_DLL_RELOAD_KEYBOARD_SHORTCUTS);
		break;

	case ADV_OPTS_DESTROYED:
		pDlgParam->hAdvancedOptionsWnd = NULL;
		break;
	}

	return FALSE;
}

LRESULT OnUEjectedFromExplorer(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM* pDlgParam)
{
	EnableOptions(hWnd, FALSE);
	return 0;
}

LRESULT OnCopyData(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	COPYDATASTRUCT *cds = (COPYDATASTRUCT *)lParam;

	if(ExplorerIsInjected() && (HWND)wParam == ExplorerGetTaskbarWnd() && cds->dwData == 0xDEADBEEF)
	{
		WCHAR *pString = (WCHAR *)HeapAlloc(GetProcessHeap(), 0, cds->cbData);
		if(pString)
			lstrcpy(pString, (WCHAR *)cds->lpData);

		ReplyMessage(0);

		HWND hPopup = IsWindowEnabled(hWnd) ? hWnd : GetLastActivePopup(hWnd);
		SetForegroundWindow(hPopup);

		if(pString)
		{
			MessageBox(hPopup, pString, NULL, MB_ICONHAND);
			HeapFree(GetProcessHeap(), 0, pString);
		}
		else
			MessageBox(hPopup, L"(Allocation failed)", NULL, MB_ICONHAND);
	}

	return FALSE;
}

LRESULT OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	LANGID lang;

	if(LOWORD(wParam) >= ID_TRAY___FIRST___ && LOWORD(wParam) <= ID_TRAY___LAST___)
	{
		if(!IsWindowEnabled(hWnd))
			return FALSE;
	}

	switch(LOWORD(wParam))
	{
	case ID_TRAY_UPDATE:
		// Omitted from public code
		break;

	case ID_TRAY_TWEAKS:
		if(!IsWindowVisible(hWnd))
		{
			ShowWindow(hWnd, SW_SHOWNORMAL);
			SetForegroundWindow(hWnd);
		}
		else
			ShowWindow(hWnd, SW_HIDE);
		break;

	case ID_TRAY_ADVANCED:
		ShowAdvancedOptionsDialog(hWnd, pDlgParam);
		break;

	case ID_TRAY_EXIT:
		SendMessage(hWnd, UWM_EXIT, 0, 0);
		break;

	case ID_TRAY_INSPECTOR:
	case IDC_INSPECTOR:
		if(ExplorerIsInjected())
			SendInspectorMessage(ExplorerGetTaskbarWnd(), pDlgParam->uTweakerMsg);
		else
			MessageBox(hWnd, LoadStrFromRsrc(IDS_INFO_NOTINJ), LoadStrFromRsrc(IDS_INFO_NOTINJ_CAPT), MB_ICONASTERISK);
		break;

	case ID_TRAY_SETTINGS:
	case IDC_SETTINGS:
		lang = twSettings.nLanguage;
		ShowSettings(hWnd, UWM_SETTING_DLG);
		if(lang != twSettings.nLanguage && ApplyLanguage(twSettings.nLanguage))
		{
			if(ExplorerIsInjected())
				SendLangMessage(ExplorerGetTaskbarWnd(), pDlgParam->uTweakerMsg, twSettings.nLanguage);

			pDlgParam->bLangChanged = TRUE;
			SendMessage(hWnd, UWM_EXIT, 0, 1);
		}
		break;

	case IDC_SHOWHELP:
		if(!ShowHelp(hWnd))
			MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SHOWHELP), NULL, MB_ICONHAND);
		break;

	case IDC_ABOUT:
		AboutMsgBox(hWnd);
		break;

	case IDCANCEL:
		ShowWindow(hWnd, SW_HIDE);
		break;

	default:
		if(LOWORD(wParam) >= IDC___OPTION_FIRST___ && LOWORD(wParam) <= IDC___OPTION_LAST___)
		{
			if(OptionsUpdFromDlg(hWnd, LOWORD(wParam), HIWORD(wParam), pDlgParam->nOptions))
			{
				if(ExplorerIsInjected())
					SendOptionsMessage(ExplorerGetTaskbarWnd(), hWnd, pDlgParam->nOptions);

				if(!SaveOptions(pDlgParam->nOptions))
					MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SAVECONF), NULL, MB_ICONHAND);
			}
		}
		break;
	}

	return FALSE;
}

LRESULT OnSysCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	switch(LOWORD(wParam))
	{
	case SYSTEM_MENU_ADVANCED:
		ShowAdvancedOptionsDialog(hWnd, pDlgParam);
		break;
	}

	return FALSE;
}

LRESULT OnUExitBlockAcquire(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	pDlgParam->nExitBlockCount++;
	return FALSE;
}

LRESULT OnUExitBlockRelease(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	pDlgParam->nExitBlockCount--;

	if(pDlgParam->bClosing && pDlgParam->nExitBlockCount == 0)
		DestroyWindow(hWnd);

	return FALSE;
}

LRESULT OnUExit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	if(!pDlgParam->bClosing)
	{
		pDlgParam->bClosing = TRUE;
		pDlgParam->nClosingResult = (int)lParam;
		pDlgParam->bHideWnd = !IsWindowVisible(hWnd);

		if(pDlgParam->nExitBlockCount == 0)
		{
			DestroyWindow(hWnd);
		}
	}

	return FALSE;
}

LRESULT OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	if(pDlgParam->bLangChanged)
	{
		// No need to clean up everything, we'll be back
		Shell_NotifyIcon(NIM_DELETE, &pDlgParam->nid);

		if(pDlgParam->hBgBrush)
			DeleteObject(pDlgParam->hBgBrush);

		PostQuitMessage(pDlgParam->nClosingResult);

		return FALSE;
	}

	if(ExplorerIsInjected())
		ExplorerCleanup();

	Shell_NotifyIcon(NIM_DELETE, &pDlgParam->nid);

	if(pDlgParam->hSmallIcon)
		DestroyIcon(pDlgParam->hSmallIcon);

	// hSmallIcon, hLargeIcon can be equal due to the code in the LoadCustomIcon function
	if(pDlgParam->hLargeIcon && pDlgParam->hLargeIcon != pDlgParam->hSmallIcon)
		DestroyIcon(pDlgParam->hLargeIcon);

	if(pDlgParam->hUpdateIcon)
		DestroyIcon(pDlgParam->hUpdateIcon);

	if(pDlgParam->hBgBrush)
		DeleteObject(pDlgParam->hBgBrush);

	PostQuitMessage(pDlgParam->nClosingResult);

	return FALSE;
}

LRESULT OnRegisteredTaskbarCreated(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	Shell_NotifyIcon(NIM_ADD, &pDlgParam->nid);
	Shell_NotifyIcon(NIM_SETVERSION, &pDlgParam->nid);

	if(!ExplorerIsInjected() && !pDlgParam->uInjectionErrorID)
	{
		UINT uErrorID = ExplorerInject(hWnd, UWM_EJECTED_FROM_EXPLORER, twSettings.nLanguage, pDlgParam->nOptions, szIniFile);
		if(uErrorID)
		{
			// Don't show an error message if the taskbar wasn't found
			if(uErrorID != IDS_INJERROR_NOTBAR)
			{
				pDlgParam->uInjectionErrorID = uErrorID; // Prevents infinite crashes and restarts of explorer
				InjectionErrorMsgBox(IsWindowEnabled(hWnd) ? hWnd : GetLastActivePopup(hWnd), uErrorID);
			}
		}
		else
			EnableOptions(hWnd, TRUE);
	}

	return FALSE;
}

LRESULT OnRegisteredTweakerMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam)
{
	HWND hPopup;

	switch(lParam)
	{
	case MSG_EXE_SHOWWINDOW:
		hPopup = IsWindowEnabled(hWnd) ? hWnd : GetLastActivePopup(hWnd);

		if(wParam == VER_FILE_VERSION_LONG)
		{
			ShowWindow(hWnd, SW_SHOWNORMAL);
			SetForegroundWindow(hPopup);
		}
		else
		{
			SetForegroundWindow(hPopup);
			MessageBox(hPopup, LoadStrFromRsrc(IDS_INFO_DIFFVER), LoadStrFromRsrc(IDS_INFO_DIFFVER_CAPT), MB_ICONASTERISK);
		}
		break;

	case MSG_EXE_CLOSE:
		SendMessage(hWnd, UWM_EXIT, 0, 0);
		break;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// Functions

BOOL LoadCustomIcon(HICON *phSmall, HICON *phLarge)
{
	// If the [7TT_file_name].ico file exists, use it as the tray icon

	HICON hSmallIcon = NULL, hLargeIcon = NULL;

	WCHAR szFileName[MAX_PATH];
	lstrcpy(szFileName, szLauncherPath);
	int nFilePathLen = lstrlen(szFileName);
	if(nFilePathLen <= 4 || lstrcmpi(szFileName + nFilePathLen - 4, L".exe") != 0)
	{
		return FALSE;
	}

	lstrcpy(szFileName + nFilePathLen - 4, L".ico");

	hSmallIcon = (HICON)LoadImage(
		NULL,
		szFileName,
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_LOADFROMFILE
	);

	hLargeIcon = (HICON)LoadImage(
		NULL,
		szFileName,
		IMAGE_ICON,
		GetSystemMetrics(SM_CXICON),
		GetSystemMetrics(SM_CYICON),
		LR_LOADFROMFILE
	);

	if(!hSmallIcon && !hLargeIcon)
	{
		return FALSE;
	}

	if(!hSmallIcon)
		hSmallIcon = hLargeIcon;
	else if(!hLargeIcon)
		hLargeIcon = hSmallIcon;

	*phSmall = hSmallIcon;
	*phLarge = hLargeIcon;

	return TRUE;
}

HBRUSH CreateBgBrush(HWND hWnd)
{
	HBITMAP hBitmap1, hBitmap2, hBitmap3;
	int height1, height2, height3;
	HDC hdcBitmap1, hdcBitmap2, hdcBitmap3;
	HBITMAP hPrevBitmap1, hPrevBitmap2, hPrevBitmap3;
	HDC hdc;
	int height_client;
	HDC hdcBg;
	HBITMAP hBgBitmap;
	HBITMAP hPrevBgBitmap;
	int height_middle, height_middle_extra_needed, middle_bmp_count;
	int height_top_cut, height_bottom_cut;
	HBRUSH hBgBrush;
	BITMAP bm;
	RECT rc;
	int i;

	hBgBrush = NULL;

	hBitmap1 = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BG1), IMAGE_BITMAP, 0, 0, 0);
	if(hBitmap1)
	{
		GetObject(hBitmap1, sizeof(BITMAP), &bm);
		height1 = bm.bmHeight;

		hBitmap2 = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BG2), IMAGE_BITMAP, 0, 0, 0);
		if(hBitmap2)
		{
			GetObject(hBitmap2, sizeof(BITMAP), &bm);
			height2 = bm.bmHeight;

			hBitmap3 = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BG3), IMAGE_BITMAP, 0, 0, 0);
			if(hBitmap3)
			{
				GetObject(hBitmap3, sizeof(BITMAP), &bm);
				height3 = bm.bmHeight;

				if(height2 < height1 && height2 < height3)
				{
					hdc = GetDC(hWnd);

					GetClientRect(hWnd, &rc);
					height_client = rc.bottom-rc.top;

					hdcBitmap1 = CreateCompatibleDC(hdc);
					hPrevBitmap1 = (HBITMAP)SelectObject(hdcBitmap1, hBitmap1);

					hdcBitmap2 = CreateCompatibleDC(hdc);
					hPrevBitmap2 = (HBITMAP)SelectObject(hdcBitmap2, hBitmap2);

					hdcBitmap3 = CreateCompatibleDC(hdc);
					hPrevBitmap3 = (HBITMAP)SelectObject(hdcBitmap3, hBitmap3);

					hdcBg = CreateCompatibleDC(hdc);
					hBgBitmap = CreateCompatibleBitmap(hdc, 1, height_client);
					hPrevBgBitmap = (HBITMAP)SelectObject(hdcBg, hBgBitmap);

					if(height_client <= height1)
					{
						BitBlt(hdcBg, 0, 0, 1, height_client, hdcBitmap1, 0, 0, SRCCOPY);
					}
					else if(height_client <= height1 + height3)
					{
						BitBlt(hdcBg, 0, 0, 1, height1, hdcBitmap1, 0, 0, SRCCOPY);
						BitBlt(hdcBg, 0, height1, 1, height_client-height1, hdcBitmap3, 0, 0, SRCCOPY);
					}
					else
					{
						height_middle = height_client - (height1 + height3);
						height_middle_extra_needed = height2 - (((height_middle - 1) % height2) + 1);
						middle_bmp_count = (height_middle + height_middle_extra_needed) / height2;

						height_top_cut = height_middle_extra_needed/2;
						height_bottom_cut = height_middle_extra_needed - height_top_cut;

						BitBlt(hdcBg, 0, 0, 1, height1-height_top_cut, hdcBitmap1, 0, height_top_cut, SRCCOPY);

						for(i=0; i<middle_bmp_count; i++)
							BitBlt(hdcBg, 0, (height1-height_top_cut)+height2*i, 1, height2, hdcBitmap2, 0, 0, SRCCOPY);

						BitBlt(hdcBg, 0, height_client-(height3-height_bottom_cut),
							1, height3-height_bottom_cut, hdcBitmap3, 0, 0, SRCCOPY);
					}

					hBgBrush = CreatePatternBrush(hBgBitmap);

					SelectObject(hdcBg, hPrevBgBitmap);
					DeleteDC(hdcBg);
					DeleteObject(hBgBitmap);

					SelectObject(hdcBitmap3, hPrevBitmap3);
					DeleteDC(hdcBitmap3);

					SelectObject(hdcBitmap2, hPrevBitmap2);
					DeleteDC(hdcBitmap2);

					SelectObject(hdcBitmap1, hPrevBitmap1);
					DeleteDC(hdcBitmap1);

					ReleaseDC(hWnd, hdc);
				}

				DeleteObject(hBitmap3);
			}

			DeleteObject(hBitmap2);
		}

		DeleteObject(hBitmap1);
	}

	return hBgBrush;
}

BOOL IsHighContrastOn(void)
{
	HIGHCONTRAST highcontrast;

	highcontrast.cbSize = sizeof(HIGHCONTRAST);

	if(SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRAST), &highcontrast, 0))
	{
		if(highcontrast.dwFlags & HCF_HIGHCONTRASTON)
		{
			return TRUE;
		}
	}

	return FALSE;
}

int FindCmdLineSwitch(WCHAR *pSwitch)
{
	int i;

	for(i=1; i<argc; i++)
		if(lstrcmpi(argv[i], pSwitch) == 0)
			return i;

	return 0;
}

void CustomizeSystemMenu(HWND hWnd)
{
	// Let's insert a menu to the window
	HMENU system_menu = GetSystemMenu(hWnd, FALSE);
	int index = GetMenuItemCount(system_menu) - 1;
	if(index < 0)
	{
		// Paranoia check
		index = 0;
	}

	// First add the separator
	MENUITEMINFO menu_info;
	ZeroMemory(&menu_info, sizeof(MENUITEMINFO));
	menu_info.cbSize = sizeof(MENUITEMINFO);
	menu_info.fMask = MIIM_FTYPE;
	menu_info.fType = MFT_SEPARATOR;
	InsertMenuItem(system_menu, index, TRUE, &menu_info);

	// Then add the actual menu
	menu_info.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING | MIIM_STATE;
	menu_info.fType = MFT_STRING;
	menu_info.fState = MFS_ENABLED;
	menu_info.wID = SYSTEM_MENU_ADVANCED;
	menu_info.dwTypeData = LoadStrFromRsrc(IDS_RCMENU_ADVANCED_OPTIONS);
	InsertMenuItem(system_menu, index, TRUE, &menu_info);
}

void InitNotifyIconData(HWND hWnd, DLG_PARAM *pDlgParam)
{
	NOTIFYICONDATA *p_nid;

	p_nid = &pDlgParam->nid;

	p_nid->cbSize = sizeof(NOTIFYICONDATA);
	p_nid->hWnd = hWnd;
	p_nid->uID = 0;
	p_nid->uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP|NIF_STATE|NIF_SHOWTIP;
	p_nid->uCallbackMessage = UWM_NOTIFYICON;
	p_nid->hIcon = pDlgParam->hSmallIcon;
	lstrcpy(p_nid->szTip, L"7+ Taskbar Tweaker");
	p_nid->dwState = twSettings.bHideTray ? NIS_HIDDEN : 0;
	p_nid->dwStateMask = NIS_HIDDEN;
	lstrcpy(p_nid->szInfo, LoadStrFromRsrc(IDS_UPDATE_BALLOON_TEXT));
	p_nid->uVersion = NOTIFYICON_VERSION_4;
	lstrcpy(p_nid->szInfoTitle, LoadStrFromRsrc(IDS_UPDATE_TEXT));
	p_nid->dwInfoFlags = NIIF_INFO|NIIF_RESPECT_QUIET_TIME;

	UpdateNotifyIconData(hWnd, pDlgParam);
}

void UpdateNotifyIconData(HWND hWnd, DLG_PARAM *pDlgParam)
{
	NOTIFYICONDATA *p_nid;

	p_nid = &pDlgParam->nid;

	if(FALSE) // if update is available, omitted from public code
	{
		p_nid->uFlags |= NIF_INFO;
		p_nid->uFlags &= ~NIF_SHOWTIP;
		p_nid->dwState = 0;
		p_nid->hIcon = pDlgParam->hUpdateIcon;
	}
	else
	{
		p_nid->uFlags &= ~NIF_INFO;
		p_nid->uFlags |= NIF_SHOWTIP;
		p_nid->dwState = twSettings.bHideTray ? NIS_HIDDEN : 0;
		p_nid->hIcon = pDlgParam->hSmallIcon;
	}
}

void ShowAdvancedOptionsDialog(HWND hWnd, DLG_PARAM *pDlgParam)
{
	if(!pDlgParam->hAdvancedOptionsWnd)
		pDlgParam->hAdvancedOptionsWnd = CreateAdvancedOptionsDialog(hWnd, UWM_ADVANCED_OPTS_DLG);
	else
		SetForegroundWindow(pDlgParam->hAdvancedOptionsWnd);
}

HMENU MakeTrayRightClickMenu(BOOL bUpdateAvailable)
{
	HMENU hMenu = CreatePopupMenu();
	if(!hMenu)
		return NULL;

	if(bUpdateAvailable)
	{
		AppendMenu(hMenu, MF_STRING, ID_TRAY_UPDATE, LoadStrFromRsrc(IDS_RCMENU_UPDATE));
		AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
	}

	AppendMenu(hMenu, MF_STRING, ID_TRAY_TWEAKS, LoadStrFromRsrc(IDS_RCMENU_TWEAKER));
	AppendMenu(hMenu, MF_STRING, ID_TRAY_SETTINGS, LoadStrFromRsrc(IDS_RCMENU_SETTINGS));
	AppendMenu(hMenu, MF_STRING, ID_TRAY_ADVANCED, LoadStrFromRsrc(IDS_RCMENU_ADVANCED_OPTIONS));
	AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
	AppendMenu(hMenu, MF_STRING, ID_TRAY_INSPECTOR, LoadStrFromRsrc(IDS_RCMENU_TASKBAR_INSPECTOR));
	AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
	AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, LoadStrFromRsrc(IDS_RCMENU_EXIT));

	return hMenu;
}

BOOL ShowHelp(HWND hWnd)
{
	LANGID langid;

	langid = GetThreadUILanguage();

	if(ShowHelpOfLang(hWnd, langid))
		return TRUE;

	if(langid == MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
		return FALSE;

	return ShowHelpOfLang(hWnd, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
}

BOOL ShowHelpOfLang(HWND hWnd, LANGID langid)
{
	WCHAR szLocaleName[LOCALE_NAME_MAX_LENGTH];
	int nLocaleNameLen;
	WCHAR szFilePath[MAX_PATH];
	int nFilePathLen;
	DWORD dwFileAttributes;

	nLocaleNameLen = LCIDToLocaleName(MAKELCID(langid, SORT_DEFAULT), szLocaleName, LOCALE_NAME_MAX_LENGTH, 0);
	if(nLocaleNameLen == 0)
		return FALSE;

	lstrcpy(szFilePath, szLauncherPath);
	nFilePathLen = lstrlen(szFilePath);

	do
	{
		nFilePathLen--;

		if(nFilePathLen < 0)
			return FALSE;
	}
	while(szFilePath[nFilePathLen] != L'\\');

	nFilePathLen++;
	szFilePath[nFilePathLen] = L'\0';

	if(nFilePathLen+(sizeof("help\\")-1)+nLocaleNameLen+(sizeof(".chm")-1) > MAX_PATH-1)
		return FALSE;

	lstrcat(szFilePath, L"help\\");
	lstrcat(szFilePath, szLocaleName);
	lstrcat(szFilePath, L".chm");

	dwFileAttributes = GetFileAttributes(szFilePath);
	if(dwFileAttributes == INVALID_FILE_ATTRIBUTES || (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;

	return !((int)(UINT_PTR)ShellExecute(hWnd, NULL, szFilePath, NULL, NULL, SW_SHOWNORMAL) <= 32);
}

void AboutMsgBox(HWND hWnd)
{
	TASKDIALOGCONFIG tdcTaskDialogConfig;

	ZeroMemory(&tdcTaskDialogConfig, sizeof(TASKDIALOGCONFIG));

	tdcTaskDialogConfig.cbSize = sizeof(TASKDIALOGCONFIG);
	tdcTaskDialogConfig.hwndParent = hWnd;
	tdcTaskDialogConfig.hInstance = GetModuleHandle(NULL);
	tdcTaskDialogConfig.dwFlags = TDF_ENABLE_HYPERLINKS|TDF_ALLOW_DIALOG_CANCELLATION|TDF_SIZE_TO_CONTENT;
	tdcTaskDialogConfig.dwCommonButtons = TDCBF_OK_BUTTON;
	tdcTaskDialogConfig.pszWindowTitle = LoadStrFromRsrc(IDS_ABOUT_CAPT);
	tdcTaskDialogConfig.pszMainIcon = MAKEINTRESOURCE(IDI_MAIN);
	tdcTaskDialogConfig.pszMainInstruction = L"\x202A" L"7+ Taskbar Tweaker v" VER_FILE_VERSION_WSTR
#if VERSION_BUILD != 0
		L" (beta)"
#endif // VERSION_BUILD
		L"\x202C"; // Note: string is marked with Unicode LEFT-TO-RIGHT EMBEDDING
	tdcTaskDialogConfig.pszContent = LoadStrFromRsrc(IDS_ABOUT_TEXT);
	tdcTaskDialogConfig.pfCallback = AboutMsgTaskDialogCallbackProc;

	if(GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
		tdcTaskDialogConfig.dwFlags |= TDF_RTL_LAYOUT;

	TaskDialogIndirect(&tdcTaskDialogConfig, NULL, NULL, NULL);
}

void InjectionErrorMsgBox(HWND hWnd, UINT uErrorID)
{
	DWORD dwExtraParam = (uErrorID & 0xFFF00000) >> (4 * 5);
	if(!dwExtraParam)
	{
		MessageBox(hWnd, LoadStrFromRsrc(uErrorID), NULL, MB_ICONHAND);
		return;
	}

	UINT uStrId = uErrorID & 0x000FFFFF;
	WCHAR *pStr = LoadStrFromRsrc(uStrId);

	WCHAR *pErrorDescription = NULL;

	switch(dwExtraParam)
	{
	case INJ_ERR_BEFORE_RUN:
	case INJ_ERR_BEFORE_GETMODULEHANDLE:
	case INJ_ERR_BEFORE_LOADLIBRARY:
	case INJ_ERR_BEFORE_GETPROCADDR:
	case INJ_ERR_GETMODULEHANDLE:
	case INJ_ERR_LOADLIBRARY:
	case INJ_ERR_GETPROCADDR:
		pErrorDescription = L"Explorer failed to run the library loading code.";
		break;

	case LIB_ERR_INIT_ALREADY_CALLED:
		pErrorDescription = L"The library is already loaded.";
		break;

	case LIB_ERR_LIB_VER_MISMATCH:
		pErrorDescription = L"The library version doesn't match, try to reinstall the tweaker.";
		break;

	case LIB_ERR_WIN_VER_MISMATCH:
		pErrorDescription = L"Unknown explorer version.";
		break;

	case INJ_ERR_BEFORE_LIBINIT:
	case LIB_ERR_FIND_IMPORT_1:
	case LIB_ERR_FIND_IMPORT_2:
	case LIB_ERR_WND_TASKBAR:
	case LIB_ERR_WND_TASKBAND:
	case LIB_ERR_WND_TASKSW:
	case LIB_ERR_WND_TRAYNOTIFY:
	case LIB_ERR_WND_TASKLIST:
	case LIB_ERR_WND_THUMB:
	case LIB_ERR_WND_TRAYOVERFLOWTOOLBAR:
	case LIB_ERR_WND_TRAYTEMPORARYTOOLBAR:
	case LIB_ERR_WND_TRAYTOOLBAR:
	case LIB_ERR_WND_TRAYCLOCK:
	case LIB_ERR_WND_SHOWDESKTOP:
	case LIB_ERR_WND_W7STARTBTN:
	case LIB_ERR_MSG_DLL_INIT:
	case LIB_ERR_WAITTHREAD:
	case LIB_ERR_EXTHREAD_MINHOOK:
	case LIB_ERR_EXTHREAD_MINHOOK_PRELOADED:
	case LIB_ERR_EXTHREAD_MOUSECTRL:
	case LIB_ERR_EXTHREAD_KEYBDHOTKEYS:
	case LIB_ERR_EXTHREAD_APPIDLISTS:
	case LIB_ERR_EXTHREAD_COMFUNCHOOK:
	case LIB_ERR_EXTHREAD_DPAHOOK:
	case LIB_ERR_EXTHREAD_REFRESHTASKBAR:
	case LIB_ERR_EXTHREAD_MINHOOK_APPLY:
	case EXE_ERR_READ_PROC_MEM:
		pErrorDescription = L"Library initialization failed, perhaps your Windows version is not supported.";
		break;

	case EXE_ERR_VIRTUAL_ALLOC:
	case EXE_ERR_WRITE_PROC_MEM:
	case EXE_ERR_CREATE_REMOTE_THREAD:
		pErrorDescription = L"Failed to inject the library into explorer. "
			L"This is usually caused by an antivirus, which denies access to explorer due to security concerns. "
			L"Try whitelisting the tweaker or contact your antivirus vendor for support.";
		break;
	}

	WCHAR szBuffer[1024 + 1];
	wsprintf(szBuffer, L"%s (%u)%s%s", pStr, dwExtraParam,
		pErrorDescription ? L"\n\n" : L"",
		pErrorDescription ? pErrorDescription : L""
	);

	MessageBox(hWnd, szBuffer, NULL, MB_ICONHAND);
}

HRESULT CALLBACK AboutMsgTaskDialogCallbackProc(HWND hWnd, UINT uNotification, WPARAM wParam, LPARAM lParam, LONG_PTR dwRefData)
{
	if(uNotification == TDN_HYPERLINK_CLICKED)
	{
		if((int)(UINT_PTR)ShellExecute(hWnd, NULL, (WCHAR *)lParam, NULL, NULL, SW_SHOWNORMAL) <= 32)
			MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_LINK), NULL, MB_ICONHAND);
	}

	return S_OK;
}

BOOL ApplyLanguage(LANGID new_language_id)
{
	WCHAR *pOldLangName, *pNewLangName;

	pOldLangName = LoadStrFromRsrc(IDS_LANGUAGE);
	SetThreadUILanguage(new_language_id);
	pNewLangName = LoadStrFromRsrc(IDS_LANGUAGE);

	return pOldLangName != pNewLangName;
}

LRESULT SendOptionsMessage(HWND hWnd, HWND hSenderWnd, int pOptions[OPTS_COUNT])
{
	COPYDATASTRUCT cds;

	cds.dwData = 0xDEADBEEF;
	cds.cbData = OPTS_BUFF;
	cds.lpData = pOptions;

	return SendMessage(hWnd, WM_COPYDATA, (WPARAM)hSenderWnd, (LPARAM)&cds);
}

LRESULT SendInspectorMessage(HWND hWnd, UINT uTweakerMsg)
{
	DWORD dwProcess;

	GetWindowThreadProcessId(hWnd, &dwProcess);
	AllowSetForegroundWindow(dwProcess);

	return SendMessage(hWnd, uTweakerMsg, 0, MSG_DLL_INSPECTOR);
}

LRESULT SendInspectorMessageFromTray(HWND hWnd, UINT uTweakerMsg)
{
	DWORD dwProcess;

	GetWindowThreadProcessId(hWnd, &dwProcess);
	AllowSetForegroundWindow(dwProcess);

	return SendMessage(hWnd, uTweakerMsg, 0, MSG_DLL_INSPECTOR_FROM_TRAY);
}

LRESULT SendLangMessage(HWND hWnd, UINT uTweakerMsg, LANGID langid)
{
	return SendMessage(hWnd, uTweakerMsg, (LPARAM)langid, MSG_DLL_SETLANG);
}

LRESULT SendNewHwndMessage(HWND hWnd, UINT uTweakerMsg, HWND hTweakerWnd)
{
	return SendMessage(hWnd, uTweakerMsg, (LPARAM)hTweakerWnd, MSG_DLL_SETHWND);
}
