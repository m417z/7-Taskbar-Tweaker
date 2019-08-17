#include "stdafx.h"
#include "sndvol.h"
#include "explorer_vars.h"
#include "options_ex.h"
#include "wnd_proc.h"
#include "functions.h"

static BOOL OpenScrollSndVolInternal(WPARAM wParam, LPARAM lMousePosParam, HWND hVolumeAppWnd, BOOL *pbOpened);
static BOOL ValidateSndVolProcess();
static BOOL ValidateSndVolWnd();
static void CALLBACK CloseSndVolTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
static HWND GetSndVolDlg(HWND hVolumeAppWnd);
static BOOL CALLBACK EnumThreadFindSndVolWnd(HWND hWnd, LPARAM lParam);
static BOOL IsSndVolWndInitialized(HWND hWnd);
static BOOL MoveSndVolCenterMouse(HWND hWnd);

// Mouse hook functions
static void OnSndVolMouseLeaveClose();
static void OnSndVolMouseClick();
static void OnSndVolMouseWheel();

// Tooltip indicator functions
static BOOL ShowSndVolTooltip();
static BOOL HideSndVolTooltip();
static int GetSndVolTrayIconIndex(HWND *phTrayToolbarWnd);

// Modern indicator functions
static BOOL CanUseModernIndicator();
static BOOL ShowSndVolModernIndicator();
static BOOL HideSndVolModernIndicator();
static void EndSndVolModernIndicatorSession();
static BOOL IsCursorUnderSndVolModernIndicatorWnd();
static HWND GetOpenSndVolModernIndicatorWnd();
static HWND GetSndVolTrayControlWnd();
static BOOL CALLBACK EnumThreadFindSndVolTrayControlWnd(HWND hWnd, LPARAM lParam);

extern UINT uTweakerMsg;
extern int nOptionsEx[OPTS_EX_COUNT];
extern HWND hTaskbarWnd;
extern LONG_PTR lpTaskbarLongPtr;

static HANDLE hSndVolProcess;
static HWND hSndVolWnd;
static UINT_PTR nCloseSndVolTimer;
static int nCloseSndVolTimerCount;
static volatile BOOL bCloseOnMouseLeave;
static BOOL bTooltipTimerOn;
static HWND hSndVolModernPreviousForegroundWnd;
static BOOL bSndVolModernLaunched;
static BOOL bSndVolModernAppeared;

BOOL OpenScrollSndVol(WPARAM wParam, LPARAM lMousePosParam)
{
	HANDLE hMutex;
	HWND hVolumeAppWnd;
	DWORD dwProcessId;
	WCHAR szCommandLine[sizeof("SndVol.exe -f 4294967295")];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	if(nOptionsEx[OPT_EX_SNDVOL_TOOLTIP])
	{
		AddMasterVolumeLevelScalar((float)(GET_WHEEL_DELTA_WPARAM(wParam)*(0.02 / 120)));
		ShowSndVolTooltip();
		return TRUE;
	}
	else if(CanUseModernIndicator())
	{
		AddMasterVolumeLevelScalar((float)(GET_WHEEL_DELTA_WPARAM(wParam)*(0.02 / 120)));
		ShowSndVolModernIndicator();
		return TRUE;
	}

	if(ValidateSndVolProcess())
	{
		if(WaitForInputIdle(hSndVolProcess, 0) == 0) // If not initializing
		{
			if(ValidateSndVolWnd())
			{
				ScrollSndVol(wParam, lMousePosParam);

				return FALSE; // False because we didn't open it, it was open
			}
			else
			{
				hVolumeAppWnd = FindWindow(L"Windows Volume App Window", L"Windows Volume App Window");
				if(hVolumeAppWnd)
				{
					GetWindowThreadProcessId(hVolumeAppWnd, &dwProcessId);

					if(GetProcessId(hSndVolProcess) == dwProcessId)
					{
						BOOL bOpened;
						if(OpenScrollSndVolInternal(wParam, lMousePosParam, hVolumeAppWnd, &bOpened))
							return bOpened;
					}
				}
			}
		}

		return FALSE;
	}

	hMutex = OpenMutex(SYNCHRONIZE, FALSE, L"Windows Volume App Window");
	if(hMutex)
	{
		CloseHandle(hMutex);

		hVolumeAppWnd = FindWindow(L"Windows Volume App Window", L"Windows Volume App Window");
		if(hVolumeAppWnd)
		{
			GetWindowThreadProcessId(hVolumeAppWnd, &dwProcessId);

			hSndVolProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, dwProcessId);
			if(hSndVolProcess)
			{
				if(WaitForInputIdle(hSndVolProcess, 0) == 0) // if not initializing
				{
					if(ValidateSndVolWnd())
					{
						ScrollSndVol(wParam, lMousePosParam);

						return FALSE; // False because we didn't open it, it was open
					}
					else
					{
						BOOL bOpened;
						if(OpenScrollSndVolInternal(wParam, lMousePosParam, hVolumeAppWnd, &bOpened))
							return bOpened;
					}
				}
			}
		}

		return FALSE;
	}

	wsprintf(szCommandLine, L"SndVol.exe -f %u", (DWORD)lMousePosParam);

	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	if(!CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, ABOVE_NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED, NULL, NULL, &si, &pi))
		return FALSE;

	if(nWinVersion <= WIN_VERSION_7)
		SendMessage(hTaskbarWnd, WM_USER + 12, 0, 0); // Close start menu

	AllowSetForegroundWindow(pi.dwProcessId);
	ResumeThread(pi.hThread);

	CloseHandle(pi.hThread);
	hSndVolProcess = pi.hProcess;

	return TRUE;
}

BOOL IsSndVolOpen()
{
	return ValidateSndVolProcess() && ValidateSndVolWnd();
}

BOOL ScrollSndVol(WPARAM wParam, LPARAM lMousePosParam)
{
	GUITHREADINFO guithreadinfo;

	guithreadinfo.cbSize = sizeof(GUITHREADINFO);

	if(!GetGUIThreadInfo(GetWindowThreadProcessId(hSndVolWnd, NULL), &guithreadinfo))
		return FALSE;

	PostMessage(guithreadinfo.hwndFocus, WM_MOUSEWHEEL, wParam, lMousePosParam);
	return TRUE;
}

void SetSndVolTimer()
{
	nCloseSndVolTimer = SetTimer(NULL, nCloseSndVolTimer, 100, CloseSndVolTimerProc);
	nCloseSndVolTimerCount = 0;
}

void ResetSndVolTimer()
{
	if(nCloseSndVolTimer != 0)
		SetSndVolTimer();
}

void KillSndVolTimer()
{
	if(nCloseSndVolTimer != 0)
	{
		KillTimer(NULL, nCloseSndVolTimer);
		nCloseSndVolTimer = 0;
	}
}

void CleanupSndVol()
{
	KillSndVolTimer();

	if(hSndVolProcess)
	{
		CloseHandle(hSndVolProcess);
		hSndVolProcess = NULL;
		hSndVolWnd = NULL;
	}
}

static BOOL OpenScrollSndVolInternal(WPARAM wParam, LPARAM lMousePosParam, HWND hVolumeAppWnd, BOOL *pbOpened)
{
	HWND hSndVolDlg = GetSndVolDlg(hVolumeAppWnd);
	if(hSndVolDlg)
	{
		if(GetWindowTextLength(hSndVolDlg) == 0) // Volume control
		{
			if(IsSndVolWndInitialized(hSndVolDlg) && MoveSndVolCenterMouse(hSndVolDlg))
			{
				if(nWinVersion <= WIN_VERSION_7)
					SendMessage(hTaskbarWnd, WM_USER + 12, 0, 0); // Close start menu

				SetForegroundWindow(hVolumeAppWnd);
				PostMessage(hVolumeAppWnd, WM_USER + 35, 0, 0);

				*pbOpened = TRUE;
				return TRUE;
			}
		}
		else if(IsWindowVisible(hSndVolDlg)) // Another dialog, e.g. volume mixer
		{
			if(nWinVersion <= WIN_VERSION_7)
				SendMessage(hTaskbarWnd, WM_USER + 12, 0, 0); // Close start menu

			SetForegroundWindow(hVolumeAppWnd);
			PostMessage(hVolumeAppWnd, WM_USER + 35, 0, 0);

			*pbOpened = FALSE;
			return TRUE;
		}
	}

	return FALSE;
}

static BOOL ValidateSndVolProcess()
{
	if(!hSndVolProcess)
		return FALSE;

	if(WaitForSingleObject(hSndVolProcess, 0) != WAIT_TIMEOUT)
	{
		CloseHandle(hSndVolProcess);
		hSndVolProcess = NULL;
		hSndVolWnd = NULL;

		return FALSE;
	}

	return TRUE;
}

static BOOL ValidateSndVolWnd()
{
	HWND hForegroundWnd;
	DWORD dwProcessId;
	WCHAR szClass[sizeof("#32770") + 1];

	hForegroundWnd = GetForegroundWindow();

	if(hSndVolWnd == hForegroundWnd)
		return TRUE;

	GetWindowThreadProcessId(hForegroundWnd, &dwProcessId);

	if(GetProcessId(hSndVolProcess) == dwProcessId)
	{
		GetClassName(hForegroundWnd, szClass, sizeof("#32770") + 1);

		if(lstrcmp(szClass, L"#32770") == 0)
		{
			hSndVolWnd = hForegroundWnd;
			bCloseOnMouseLeave = FALSE;

			return TRUE;
		}
	}

	hSndVolWnd = NULL;

	return FALSE;
}

static void CALLBACK CloseSndVolTimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	POINT pt;
	RECT rc;

	if(nOptionsEx[OPT_EX_SNDVOL_TOOLTIP])
	{
		nCloseSndVolTimerCount++;
		if(nCloseSndVolTimerCount < 10)
			return;

		HideSndVolTooltip();
	}
	else if(CanUseModernIndicator())
	{
		if(!bSndVolModernAppeared)
		{
			if(GetOpenSndVolModernIndicatorWnd())
			{
				bSndVolModernAppeared = TRUE;
				nCloseSndVolTimerCount = 1;
				return;
			}
			else
			{
				nCloseSndVolTimerCount++;
				if(nCloseSndVolTimerCount < 10)
					return;
			}
		}
		else if(GetOpenSndVolModernIndicatorWnd())
		{
			if(!IsCursorUnderSndVolModernIndicatorWnd())
				nCloseSndVolTimerCount++;
			else
				nCloseSndVolTimerCount = 0;

			if(nCloseSndVolTimerCount < 10)
				return;

			HideSndVolModernIndicator();
		}

		EndSndVolModernIndicatorSession();
	}
	else
	{
		if(ValidateSndVolProcess())
		{
			if(WaitForInputIdle(hSndVolProcess, 0) != 0)
				return;

			if(ValidateSndVolWnd())
			{
				nCloseSndVolTimerCount++;
				if(nCloseSndVolTimerCount < 10)
					return;

				GetCursorPos(&pt);
				GetWindowRect(hSndVolWnd, &rc);

				if(!PtInRect(&rc, pt))
					PostMessage(hSndVolWnd, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, FALSE), (LPARAM)NULL);
				else
					bCloseOnMouseLeave = TRUE;
			}
		}
	}

	KillTimer(NULL, nCloseSndVolTimer);
	nCloseSndVolTimer = 0;
}

static HWND GetSndVolDlg(HWND hVolumeAppWnd)
{
	HWND hWnd = NULL;
	EnumThreadWindows(GetWindowThreadProcessId(hVolumeAppWnd, NULL), EnumThreadFindSndVolWnd, (LPARAM)&hWnd);
	return hWnd;
}

static BOOL CALLBACK EnumThreadFindSndVolWnd(HWND hWnd, LPARAM lParam)
{
	WCHAR szClass[16];

	GetClassName(hWnd, szClass, _countof(szClass));
	if(lstrcmp(szClass, L"#32770") == 0)
	{
		*(HWND *)lParam = hWnd;
		return FALSE;
	}

	return TRUE;
}

static BOOL IsSndVolWndInitialized(HWND hWnd)
{
	HWND hChildDlg;

	hChildDlg = FindWindowEx(hWnd, NULL, L"#32770", NULL);
	if(!hChildDlg)
		return FALSE;

	if(!(GetWindowLong(hChildDlg, GWL_STYLE) & WS_VISIBLE))
		return FALSE;

	return TRUE;
}

static BOOL MoveSndVolCenterMouse(HWND hWnd)
{
	NOTIFYICONIDENTIFIER notifyiconidentifier;
	BOOL bCompositionEnabled;
	POINT pt;
	SIZE size;
	RECT rc, rcExclude, rcInflate;
	int nInflate;

	ZeroMemory(&notifyiconidentifier, sizeof(NOTIFYICONIDENTIFIER));
	notifyiconidentifier.cbSize = sizeof(NOTIFYICONIDENTIFIER);
	memcpy(&notifyiconidentifier.guidItem, "\x73\xAE\x20\x78\xE3\x23\x29\x42\x82\xC1\xE4\x1C\xB6\x7D\x5B\x9C", sizeof(GUID));

	if(Shell_NotifyIconGetRect(&notifyiconidentifier, &rcExclude) != S_OK)
		return FALSE;

	GetCursorPos(&pt);
	GetWindowRect(hWnd, &rc);

	nInflate = 0;

	if(DwmIsCompositionEnabled(&bCompositionEnabled) == S_OK && bCompositionEnabled)
	{
		memcpy(&notifyiconidentifier.guidItem, "\x43\x65\x4B\x96\xAD\xBB\xEE\x44\x84\x8A\x3A\x95\xD8\x59\x51\xEA", sizeof(GUID));

		if(Shell_NotifyIconGetRect(&notifyiconidentifier, &rcInflate) == S_OK)
		{
			nInflate = rcInflate.bottom - rcInflate.top;
			InflateRect(&rc, nInflate, nInflate);
		}
	}

	size.cx = rc.right - rc.left;
	size.cy = rc.bottom - rc.top;

	if(!CalculatePopupWindowPosition(&pt, &size,
		TPM_CENTERALIGN | TPM_VCENTERALIGN | TPM_VERTICAL | TPM_WORKAREA, &rcExclude, &rc))
		return FALSE;

	SetWindowPos(hWnd, NULL, rc.left + nInflate, rc.top + nInflate,
		0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);

	return TRUE;
}

// Mouse hook functions

void OnSndVolMouseMove_MouseHook(POINT pt)
{
	HWND hWnd;

	if(bCloseOnMouseLeave)
	{
		hWnd = WindowFromPoint(pt);

		if(hWnd == hSndVolWnd || IsChild(hSndVolWnd, hWnd))
		{
			bCloseOnMouseLeave = FALSE;
			PostMessage(hTaskbarWnd, uTweakerMsg, (LPARAM)OnSndVolMouseLeaveClose, MSG_DLL_CALLFUNC);
		}
	}
}

void OnSndVolMouseClick_MouseHook(POINT pt)
{
	PostMessage(hTaskbarWnd, uTweakerMsg, (LPARAM)OnSndVolMouseClick, MSG_DLL_CALLFUNC);
}

void OnSndVolMouseWheel_MouseHook(POINT pt)
{
	PostMessage(hTaskbarWnd, uTweakerMsg, (LPARAM)OnSndVolMouseWheel, MSG_DLL_CALLFUNC);
}

static void OnSndVolMouseLeaveClose()
{
	SetSndVolTimer();
}

static void OnSndVolMouseClick()
{
	bCloseOnMouseLeave = FALSE;
	KillSndVolTimer();
}

static void OnSndVolMouseWheel()
{
	ResetSndVolTimer();
}

// Tooltip indicator functions

void OnSndVolTooltipTimer()
{
	HWND hTrayToolbarWnd;
	int index;
	HWND hTooltipWnd;

	if(!bTooltipTimerOn)
		return;

	bTooltipTimerOn = FALSE;

	index = GetSndVolTrayIconIndex(&hTrayToolbarWnd);
	if(index < 0)
		return;

	hTooltipWnd = (HWND)SendMessage(hTrayToolbarWnd, TB_GETTOOLTIPS, 0, 0);
	if(hTooltipWnd)
		ShowWindow(hTooltipWnd, SW_HIDE);
}

static BOOL ShowSndVolTooltip()
{
	HWND hTrayToolbarWnd;
	int index;

	index = GetSndVolTrayIconIndex(&hTrayToolbarWnd);
	if(index < 0)
		return FALSE;

	SendMessage(hTrayToolbarWnd, TB_SETHOTITEM2, -1, 0);
	SendMessage(hTrayToolbarWnd, TB_SETHOTITEM2, index, 0);

	// Show tooltip
	bTooltipTimerOn = TRUE;
	SetTimer(hTrayToolbarWnd, 0, 0, NULL);

	return TRUE;
}

static BOOL HideSndVolTooltip()
{
	HWND hTrayToolbarWnd;
	int index;

	index = GetSndVolTrayIconIndex(&hTrayToolbarWnd);
	if(index < 0)
		return FALSE;

	if(SendMessage(hTrayToolbarWnd, TB_GETHOTITEM, 0, 0) == index)
		SendMessage(hTrayToolbarWnd, TB_SETHOTITEM2, -1, 0);

	return TRUE;
}

static int GetSndVolTrayIconIndex(HWND *phTrayToolbarWnd)
{
	RECT rcSndVolIcon;
	HWND hTrayNotifyWnd, hTrayToolbarWnd;
	LONG_PTR lpTrayNotifyLongPtr;
	POINT pt;
	int index;

	index = -1;

	if(GetSndVolTrayIconRect(&rcSndVolIcon))
	{
		hTrayNotifyWnd = *EV_TASKBAR_TRAY_NOTIFY_WND;
		if(hTrayNotifyWnd)
		{
			lpTrayNotifyLongPtr = GetWindowLongPtr(hTrayNotifyWnd, 0);

			hTrayToolbarWnd = *EV_TRAY_NOTIFY_TOOLBAR_WND(lpTrayNotifyLongPtr);
			if(hTrayToolbarWnd)
			{
				pt.x = rcSndVolIcon.left + (rcSndVolIcon.right - rcSndVolIcon.left) / 2;
				pt.y = rcSndVolIcon.top + (rcSndVolIcon.bottom - rcSndVolIcon.top) / 2;

				MapWindowPoints(NULL, hTrayToolbarWnd, &pt, 1);

				index = (int)SendMessage(hTrayToolbarWnd, TB_HITTEST, 0, (LPARAM)&pt);
				if(index >= 0 && phTrayToolbarWnd)
					*phTrayToolbarWnd = hTrayToolbarWnd;
			}
		}
	}

	return index;
}

// Modern indicator functions

static BOOL CanUseModernIndicator()
{
	if(nWinVersion < WIN_VERSION_10_T1 || nOptionsEx[OPT_EX_SNDVOL_CLASSIC])
		return FALSE;

	DWORD dwEnabled = 1;
	DWORD dwValueSize = sizeof(dwEnabled);
	DWORD dwError = RegGetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\MTCUVC",
		L"EnableMTCUVC", RRF_RT_REG_DWORD, NULL, &dwEnabled, &dwValueSize);

	// We don't check dwError just like Microsoft doesn't at SndVolSSO.dll.

	return dwEnabled != 0;
}

static BOOL ShowSndVolModernIndicator()
{
	if(bSndVolModernLaunched)
		return TRUE; // already launched

	HWND hSndVolModernIndicatorWnd = GetOpenSndVolModernIndicatorWnd();
	if(hSndVolModernIndicatorWnd)
		return TRUE; // already shown

	HWND hForegroundWnd = GetForegroundWindow();
	if(hForegroundWnd && hForegroundWnd != hTaskbarWnd)
		hSndVolModernPreviousForegroundWnd = hForegroundWnd;

	HWND hSndVolTrayControlWnd = GetSndVolTrayControlWnd();
	if(!hSndVolTrayControlWnd)
		return FALSE;

	if(!PostMessage(hSndVolTrayControlWnd, 0x460, 0, MAKELPARAM(NIN_SELECT, 100)))
		return FALSE;

	bSndVolModernLaunched = TRUE;
	return TRUE;
}

static BOOL HideSndVolModernIndicator()
{
	HWND hSndVolModernIndicatorWnd = GetOpenSndVolModernIndicatorWnd();
	if(hSndVolModernIndicatorWnd)
	{
		if(!hSndVolModernPreviousForegroundWnd || !SetForegroundWindow(hSndVolModernPreviousForegroundWnd))
			SetForegroundWindow(hTaskbarWnd);
	}

	return TRUE;
}

static void EndSndVolModernIndicatorSession()
{
	hSndVolModernPreviousForegroundWnd = NULL;
	bSndVolModernLaunched = FALSE;
	bSndVolModernAppeared = FALSE;
}

static BOOL IsCursorUnderSndVolModernIndicatorWnd()
{
	HWND hSndVolModernIndicatorWnd = GetOpenSndVolModernIndicatorWnd();
	if(!hSndVolModernIndicatorWnd)
		return FALSE;

	POINT pt;
	GetCursorPos(&pt);
	return WindowFromPoint(pt) == hSndVolModernIndicatorWnd;
}

static HWND GetOpenSndVolModernIndicatorWnd()
{
	HWND hForegroundWnd = GetForegroundWindow();
	if(!hForegroundWnd)
		return NULL;

	// Check class name
	WCHAR szBuffer[32];
	if(!GetClassName(hForegroundWnd, szBuffer, 32) ||
		wcscmp(szBuffer, L"Windows.UI.Core.CoreWindow") != 0)
		return NULL;

	// Check that the MtcUvc prop exists
	WCHAR szVerifyPropName[sizeof("ApplicationView_CustomWindowTitle#1234567890#MtcUvc")];
	wsprintf(szVerifyPropName, L"ApplicationView_CustomWindowTitle#%u#MtcUvc", (DWORD)(DWORD_PTR)hForegroundWnd);

	SetLastError(0);
	GetProp(hForegroundWnd, szVerifyPropName);
	if(GetLastError() != 0)
		return NULL;

	return hForegroundWnd;
}

static HWND GetSndVolTrayControlWnd()
{
	// The window we're looking for has a class name similar to "ATL:00007FFAECBBD280".
	// It shares a thread with the bluetooth window, which is easier to find by class,
	// so we use that.

	HWND hBluetoothNotificationWnd = FindWindow(L"BluetoothNotificationAreaIconWindowClass", NULL);
	if(!hBluetoothNotificationWnd)
		return NULL;

	HWND hWnd = NULL;
	EnumThreadWindows(GetWindowThreadProcessId(hBluetoothNotificationWnd, NULL), EnumThreadFindSndVolTrayControlWnd, (LPARAM)&hWnd);
	return hWnd;
}

static BOOL CALLBACK EnumThreadFindSndVolTrayControlWnd(HWND hWnd, LPARAM lParam)
{
	HMODULE hInstance = (HMODULE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	if(hInstance && hInstance == GetModuleHandle(L"sndvolsso.dll"))
	{
		*(HWND *)lParam = hWnd;
		return FALSE;
	}

	return TRUE;
}

// Other functions

BOOL GetSndVolTrayIconRect(RECT *prc)
{
	NOTIFYICONIDENTIFIER notifyiconidentifier;

	ZeroMemory(&notifyiconidentifier, sizeof(NOTIFYICONIDENTIFIER));
	notifyiconidentifier.cbSize = sizeof(NOTIFYICONIDENTIFIER);
	memcpy(&notifyiconidentifier.guidItem, "\x73\xAE\x20\x78\xE3\x23\x29\x42\x82\xC1\xE4\x1C\xB6\x7D\x5B\x9C", sizeof(GUID));

	return Shell_NotifyIconGetRect(&notifyiconidentifier, prc) == S_OK;
}

const static GUID XIID_IMMDeviceEnumerator = { 0xA95664D2, 0x9614, 0x4F35, { 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6 } };
const static GUID XIID_MMDeviceEnumerator = { 0xBCDE0395, 0xE52F, 0x467C, { 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E } };
const static GUID XIID_IAudioEndpointVolume = { 0x5CDF2C82, 0x841E, 0x4546, { 0x97, 0x22, 0x0C, 0xF7, 0x40, 0x78, 0x22, 0x9A } };

BOOL IsVolMuted(BOOL *pbMuted)
{
	IMMDeviceEnumerator *deviceEnumerator = NULL;
	IMMDevice *defaultDevice = NULL;
	IAudioEndpointVolume *endpointVolume = NULL;
	HRESULT hr;
	BOOL bSuccess = FALSE;

	//hr = CoInitialize(NULL);
	//if(SUCCEEDED(hr))
	{
		hr = CoCreateInstance(&XIID_MMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, &XIID_IMMDeviceEnumerator, (LPVOID *)&deviceEnumerator);
		if(SUCCEEDED(hr))
		{
			hr = deviceEnumerator->lpVtbl->GetDefaultAudioEndpoint(deviceEnumerator, eRender, eConsole, &defaultDevice);
			if(SUCCEEDED(hr))
			{
				hr = defaultDevice->lpVtbl->Activate(defaultDevice, &XIID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
				if(SUCCEEDED(hr))
				{
					if(SUCCEEDED(endpointVolume->lpVtbl->GetMute(endpointVolume, pbMuted)))
						bSuccess = TRUE;

					endpointVolume->lpVtbl->Release(endpointVolume);
				}

				defaultDevice->lpVtbl->Release(defaultDevice);
			}

			deviceEnumerator->lpVtbl->Release(deviceEnumerator);
		}

	//	CoUninitialize();
	}

	return bSuccess;
}

BOOL IsVolMutedAndNotZero(BOOL *pbResult)
{
	IMMDeviceEnumerator *deviceEnumerator = NULL;
	IMMDevice *defaultDevice = NULL;
	IAudioEndpointVolume *endpointVolume = NULL;
	HRESULT hr;
	float fMasterVolume;
	BOOL bMuted;
	BOOL bSuccess = FALSE;

	//hr = CoInitialize(NULL);
	//if(SUCCEEDED(hr))
	{
		hr = CoCreateInstance(&XIID_MMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, &XIID_IMMDeviceEnumerator, (LPVOID *)&deviceEnumerator);
		if(SUCCEEDED(hr))
		{
			hr = deviceEnumerator->lpVtbl->GetDefaultAudioEndpoint(deviceEnumerator, eRender, eConsole, &defaultDevice);
			if(SUCCEEDED(hr))
			{
				hr = defaultDevice->lpVtbl->Activate(defaultDevice, &XIID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
				if(SUCCEEDED(hr))
				{
					if(SUCCEEDED(endpointVolume->lpVtbl->GetMasterVolumeLevelScalar(endpointVolume, &fMasterVolume)) &&
						SUCCEEDED(endpointVolume->lpVtbl->GetMute(endpointVolume, &bMuted)))
					{
						*pbResult = bMuted && (fMasterVolume > 0.005);
						bSuccess = TRUE;
					}

					endpointVolume->lpVtbl->Release(endpointVolume);
				}

				defaultDevice->lpVtbl->Release(defaultDevice);
			}

			deviceEnumerator->lpVtbl->Release(deviceEnumerator);
		}

		//	CoUninitialize();
	}

	return bSuccess;
}

BOOL ToggleVolMuted()
{
	IMMDeviceEnumerator *deviceEnumerator = NULL;
	IMMDevice *defaultDevice = NULL;
	IAudioEndpointVolume *endpointVolume = NULL;
	HRESULT hr;
	BOOL bMuted;
	BOOL bSuccess = FALSE;

	//hr = CoInitialize(NULL);
	//if(SUCCEEDED(hr))
	{
		hr = CoCreateInstance(&XIID_MMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, &XIID_IMMDeviceEnumerator, (LPVOID *)&deviceEnumerator);
		if(SUCCEEDED(hr))
		{
			hr = deviceEnumerator->lpVtbl->GetDefaultAudioEndpoint(deviceEnumerator, eRender, eConsole, &defaultDevice);
			if(SUCCEEDED(hr))
			{
				hr = defaultDevice->lpVtbl->Activate(defaultDevice, &XIID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
				if(SUCCEEDED(hr))
				{
					if(SUCCEEDED(endpointVolume->lpVtbl->GetMute(endpointVolume, &bMuted)))
					{
						if(SUCCEEDED(endpointVolume->lpVtbl->SetMute(endpointVolume, !bMuted, NULL)))
							bSuccess = TRUE;
					}

					endpointVolume->lpVtbl->Release(endpointVolume);
				}

				defaultDevice->lpVtbl->Release(defaultDevice);
			}

			deviceEnumerator->lpVtbl->Release(deviceEnumerator);
		}

	//	CoUninitialize();
	}

	return bSuccess;
}

BOOL AddMasterVolumeLevelScalar(float fMasterVolumeAdd)
{
	IMMDeviceEnumerator *deviceEnumerator = NULL;
	IMMDevice *defaultDevice = NULL;
	IAudioEndpointVolume *endpointVolume = NULL;
	HRESULT hr;
	float fMasterVolume;
	BOOL bSuccess = FALSE;

	//hr = CoInitialize(NULL);
	//if(SUCCEEDED(hr))
	{
		hr = CoCreateInstance(&XIID_MMDeviceEnumerator, NULL, CLSCTX_INPROC_SERVER, &XIID_IMMDeviceEnumerator, (LPVOID *)&deviceEnumerator);
		if(SUCCEEDED(hr))
		{
			hr = deviceEnumerator->lpVtbl->GetDefaultAudioEndpoint(deviceEnumerator, eRender, eConsole, &defaultDevice);
			if(SUCCEEDED(hr))
			{
				hr = defaultDevice->lpVtbl->Activate(defaultDevice, &XIID_IAudioEndpointVolume, CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
				if(SUCCEEDED(hr))
				{
					if(SUCCEEDED(endpointVolume->lpVtbl->GetMasterVolumeLevelScalar(endpointVolume, &fMasterVolume)))
					{
						fMasterVolume += fMasterVolumeAdd;

						if(fMasterVolume < 0.0)
							fMasterVolume = 0.0;
						else if(fMasterVolume > 1.0)
							fMasterVolume = 1.0;

						if(SUCCEEDED(endpointVolume->lpVtbl->SetMasterVolumeLevelScalar(endpointVolume, fMasterVolume, NULL)))
							bSuccess = TRUE;
					}

					endpointVolume->lpVtbl->Release(endpointVolume);
				}

				defaultDevice->lpVtbl->Release(defaultDevice);
			}

			deviceEnumerator->lpVtbl->Release(deviceEnumerator);
		}

	//	CoUninitialize();
	}

	return bSuccess;
}
