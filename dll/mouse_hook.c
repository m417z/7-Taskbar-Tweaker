#include "stdafx.h"
#include "mouse_hook.h"
#include "explorer_vars.h"
#include "options_def.h"
#include "options_ex.h"
#include "functions.h"
#include "sndvol.h"
#include "wnd_proc.h"

static DWORD WINAPI MouseHookThread(void *pParameter);
static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

// superglobals
extern HINSTANCE hDllInst;
extern UINT uTweakerMsg;
extern int nOptions[OPTS_COUNT];
extern int nOptionsEx[OPTS_EX_COUNT];
extern DWORD dwTaskbarThreadId;
extern HWND hTaskbarWnd, hTaskBandWnd, hTaskSwWnd, hTaskListWnd, hThumbnailWnd;
extern LONG_PTR lpTaskbarLongPtr, lpTaskBandLongPtr, lpTaskSwLongPtr, lpTaskListLongPtr, lpThumbnailLongPtr;

static volatile HANDLE hMouseHookThread;
static DWORD dwMouseHookThreadId;
static HHOOK hLowLevelMouseHook;
static ATOM wTaskSwitcherClass;

BOOL MouseHook_Init()
{
	if(hMouseHookThread)
	{
		return TRUE;
	}

	BOOL bSuccess = FALSE;

	WNDCLASS wndclass;
	wTaskSwitcherClass = (ATOM)GetClassInfo(GetModuleHandle(NULL), L"TaskSwitcherWnd", &wndclass);

	HANDLE hThreadReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(hThreadReadyEvent)
	{
		hMouseHookThread = CreateThread(NULL, 0, MouseHookThread, (void *)hThreadReadyEvent, CREATE_SUSPENDED, &dwMouseHookThreadId);
		if(hMouseHookThread)
		{
			SetThreadPriority(hMouseHookThread, THREAD_PRIORITY_TIME_CRITICAL);
			ResumeThread(hMouseHookThread);

			WaitForSingleObject(hThreadReadyEvent, INFINITE);

			bSuccess = TRUE;
		}

		CloseHandle(hThreadReadyEvent);
	}

	return bSuccess;
}

void MouseHook_Exit()
{
	HANDLE hThread = InterlockedExchangePointer(&hMouseHookThread, NULL);
	if(hThread)
	{
		PostThreadMessage(dwMouseHookThreadId, WM_APP, 0, 0);
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
}

static DWORD WINAPI MouseHookThread(void *pParameter)
{
	HANDLE hThreadReadyEvent;
	MSG msg;
	BOOL bRet;
	HANDLE hThread;

	hThreadReadyEvent = (HANDLE)pParameter;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	SetEvent(hThreadReadyEvent);

	hLowLevelMouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(NULL), 0);
	if(hLowLevelMouseHook)
	{
		while((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
		{
			if(bRet == -1)
			{
				msg.wParam = 0;
				break;
			}

			if(msg.hwnd == NULL && msg.message == WM_APP)
			{
				PostQuitMessage(0);
				continue;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		UnhookWindowsHookEx(hLowLevelMouseHook);
	}
	else
		msg.wParam = 0;

	hThread = InterlockedExchangePointer(&hMouseHookThread, NULL);
	if(hThread)
		CloseHandle(hThread);

	return (DWORD)msg.wParam;
}

static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode == HC_ACTION)
	{
		MSLLHOOKSTRUCT *msllHookStruct = (MSLLHOOKSTRUCT *)lParam;
		BOOL bDontCapture = FALSE;

		if(wTaskSwitcherClass)
		{
			HWND hForegroundWnd = GetForegroundWindow();
			if(hForegroundWnd)
			{
				ATOM wClassAtom = (ATOM)GetClassLong(hForegroundWnd, GCW_ATOM);
				if(wClassAtom && wClassAtom == wTaskSwitcherClass)
				{
					bDontCapture = TRUE;
				}
			}
		}

		if(!bDontCapture && wParam == WM_MOUSEWHEEL)
		{
			HWND hMaybeTransWnd = WindowFromPoint(msllHookStruct->pt);
			DWORD dwThreadId, dwProcessId;
			dwThreadId = GetWindowThreadProcessId(hMaybeTransWnd, &dwProcessId);

			if(dwProcessId == GetCurrentProcessId() && dwThreadId == dwTaskbarThreadId)
			{
				// We send MSG_DLL_MOUSE_HOOK_WND_IDENT here, because we want to run WindowFromPoint on the
				// original thread, which in turn will skip transparent windows, by sending WM_NCHITTEST.
				// Reference: http://blogs.msdn.com/b/oldnewthing/archive/2010/12/30/10110077.aspx
				// Also, we call IdentifyTaskbarWindow from the original thread.

				MOUSE_HOOK_WND_IDENT_PARAM identParam;
				identParam.ppt = &msllHookStruct->pt;
				identParam.hMaybeTransWnd = hMaybeTransWnd;

				DWORD_PTR dwMsgResult;
				LRESULT lSendMessageResult = SendMessageTimeout(hTaskbarWnd, uTweakerMsg,
					(WPARAM)&identParam, MSG_DLL_MOUSE_HOOK_WND_IDENT, SMTO_ABORTIFHUNG, 500, &dwMsgResult);

				HWND hNonTransWnd = NULL;
				int nMaybeTransWndIdent = TASKBAR_WINDOW_UNKNOWN;

				if(lSendMessageResult && dwMsgResult)
				{
					dwThreadId = GetWindowThreadProcessId(identParam.hNonTransWnd, &dwProcessId);
					if(dwProcessId == GetCurrentProcessId() && dwThreadId == dwTaskbarThreadId)
					{
						hNonTransWnd = identParam.hNonTransWnd;
						nMaybeTransWndIdent = identParam.nMaybeTransWndIdent;
					}
				}

				BOOL bCaptureWheel = FALSE;
				if(hNonTransWnd)
				{
					switch(nMaybeTransWndIdent)
					{
					case TASKBAR_WINDOW_THUMBNAIL:
					case TASKBAR_SECONDARY_THUMBNAIL:
						if(nOptions[OPT_WHEEL_MINTHUMB] == 1)
							bCaptureWheel = TRUE;
						break;

					case TASKBAR_WINDOW_TASKLIST:
					case TASKBAR_SECONDARY_TASKLIST:
						if(nOptions[OPT_WHEEL_CYCLE] == 1 || nOptions[OPT_WHEEL_MINTASKBAR] == 1 ||
							nOptions[OPT_WHEEL_VOLTASKBAR] == 1 || nOptionsEx[OPT_EX_MULTIPAGE_WHEEL_SCROLL])
							bCaptureWheel = TRUE;
						break;

					case TASKBAR_WINDOW_TASKSW:
						if(nOptions[OPT_WHEEL_VOLTASKBAR] == 1 || nOptionsEx[OPT_EX_MULTIPAGE_WHEEL_SCROLL])
							bCaptureWheel = TRUE;
						break;

					case TASKBAR_WINDOW_TASKBAND:
						if(nOptions[OPT_WHEEL_VOLTASKBAR] == 1)
							bCaptureWheel = TRUE;
						break;

					case TASKBAR_SECONDARY_TASKBAND:
						if(nOptions[OPT_WHEEL_VOLTASKBAR] == 1 || nOptionsEx[OPT_EX_MULTIPAGE_WHEEL_SCROLL])
							bCaptureWheel = TRUE;
						break;

					case TASKBAR_WINDOW_NOTIFY:
						if(nOptions[OPT_WHEEL_VOLNOTIFY] == 1 || nOptions[OPT_WHEEL_VOLTASKBAR] == 1)
							bCaptureWheel = TRUE;
						break;

					case TASKBAR_WINDOW_TASKBAR:
					case TASKBAR_SECONDARY_TASKBAR:
						if(nOptions[OPT_WHEEL_VOLTASKBAR] == 1)
							bCaptureWheel = TRUE;
						break;
					}
				}

				if(bCaptureWheel)
				{
					WORD wVirtualKeys = 0;

					if(GetKeyState(VK_LBUTTON) < 0)
						wVirtualKeys |= MK_LBUTTON;

					if(GetKeyState(VK_RBUTTON) < 0)
						wVirtualKeys |= MK_RBUTTON;

					if(GetKeyState(VK_SHIFT) < 0)
						wVirtualKeys |= MK_SHIFT;

					if(GetKeyState(VK_CONTROL) < 0)
						wVirtualKeys |= MK_CONTROL;

					if(GetKeyState(VK_MBUTTON) < 0)
						wVirtualKeys |= MK_MBUTTON;

					if(GetKeyState(VK_XBUTTON1) < 0)
						wVirtualKeys |= MK_XBUTTON1;

					if(GetKeyState(VK_XBUTTON2) < 0)
						wVirtualKeys |= MK_XBUTTON2;

					PostMessage(hNonTransWnd, WM_MOUSEWHEEL,
						MAKEWPARAM(wVirtualKeys, HIWORD(msllHookStruct->mouseData)),
						MAKELPARAM(msllHookStruct->pt.x, msllHookStruct->pt.y));

					return 1;
				}
			}
		}

		if(nOptions[OPT_WHEEL_VOLTASKBAR] == 1 || nOptions[OPT_WHEEL_VOLNOTIFY] == 1)
		{
			if(IsSndVolOpen())
			{
				switch(wParam)
				{
				case WM_MOUSEMOVE:
					OnSndVolMouseMove_MouseHook(msllHookStruct->pt);
					break;

				case WM_LBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_RBUTTONDOWN:
				case WM_RBUTTONUP:
					OnSndVolMouseClick_MouseHook(msllHookStruct->pt);
					break;

				case WM_MOUSEWHEEL:
					OnSndVolMouseWheel_MouseHook(msllHookStruct->pt);
					break;
				}
			}
		}
	}

	return CallNextHookEx(hLowLevelMouseHook, nCode, wParam, lParam);
}
