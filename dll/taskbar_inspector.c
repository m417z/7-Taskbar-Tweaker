#include "stdafx.h"
#include "taskbar_inspector.h"
#include "explorer_vars.h"
#include "functions.h"
#include "wnd_proc.h"
#include "taskbar_refresh.h"
#include "appid_lists.h"
#include "resource.h"
#include "portable_settings.h"

#define UWM_INSP_FILLLIST               WM_APP
#define UWM_INSP_FINALLYCLOSE           (WM_APP+1)
#define UWM_INSP_CALLFUNC               (WM_APP+2)
#define UWM_INSP_CALLFUNC_PARAM         (WM_APP+3)

// Right click menu of an Application ID
#define APPIDMENU_LABELNEVER            40001
#define APPIDMENU_LABELALWAYS           40002
#define APPIDMENU_LABELDEF              40003
#define APPIDMENU_GROUPNEVER            40004
#define APPIDMENU_GROUPALWAYS           40005
#define APPIDMENU_GROUPDEF              40006
#define APPIDMENU_GROUPPINNEDNEVER      40007
#define APPIDMENU_GROUPPINNEDALWAYS     40008
#define APPIDMENU_GROUPPINNEDDEF        40009
#define APPIDMENU_COMBINENEVER          40010
#define APPIDMENU_COMBINEALWAYS         40011
#define APPIDMENU_COMBINEDEF            40012
#define APPIDMENU_SORT                  40013

// Right click menu of window(s)
#define WNDMENU_RESTORE                 40001
#define WNDMENU_MINIMIZE                40002
#define WNDMENU_MAXIMIZE                40003
#define WNDMENU_CLOSE                   40004
#define WNDMENU_CASCADE                 40005
#define WNDMENU_TILEH                   40006
#define WNDMENU_TILEV                   40007
#define WNDMENU_DELAPPID                40008
#define WNDMENU_NEWAPPID                40009 // <-- must be the last in the list

typedef struct {
	HDPA pdpa;
	int index;
	int extra;
} DPA_HOOK_PARAM;

// superglobals
extern LANGID language_id;
extern HINSTANCE hDllInst;
extern UINT uTweakerMsg;
extern HWND hTaskbarWnd, hTaskSwWnd, hTaskListWnd;
extern LONG_PTR lpTaskbarLongPtr, lpTaskSwLongPtr, lpTaskListLongPtr;

static volatile HANDLE hInspThread;
static volatile HWND hInspWnd;
static volatile DWORD dwInspectorCloseTime;
static HWND hInspListWnd;
static LONG_PTR lpInspMMTaskListLongPtr;
static int nRefreshing;
static POINT ptRefreshOrigin;

// Hook globals
static BOOL bHooksEnabled;
static BOOL bNewButton;
static HDPA hNewButtonDpa;
static int hook_proc_call_counter;
static BOOL bHookCloseInspector;

static DWORD WINAPI InspectorThread(void *pParameter);
static LRESULT CALLBACK DlgInspectorProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void MoveResizeWindowCenterMouse(HWND hWnd, long w, long h);
static void InitWindowPos(HWND hWnd, BOOL bShowCaption, long *p_dlg_min_w, long *p_dlg_min_h, long *p_dlg_last_cw, long *p_dlg_last_ch);
static BOOL LoadWindowSize(long *pw, long *ph);
static BOOL SaveWindowSize(long w, long h);
static BOOL InitList(HINSTANCE hInst, HWND hListWnd);
static BOOL CALLBACK EnumThreadWndProc(HWND hWnd, LPARAM lParam);

// List filling
static BOOL FillList(LONG_PTR lpMMTaskListLongPtr);
static BOOL ExplorerThreadFillList(LONG_PTR lpMMTaskListLongPtr);
static void AddButtonGroupToList(HWND hListWnd, int list_index, LONG_PTR *button_group);
static void AddButtonToList(HWND hListWnd, int list_index, HIMAGELIST hImageList, LONG_PTR *button);
static void ListUpdateIconsTexts(HWND hListWnd);
static BOOL ListUpdateWindowItem(HWND hListWnd, HIMAGELIST hImageList, LVITEM *plvi);
static HICON GetWindowIcon(HWND hWnd);

// Mouse/keybd operations
static BOOL ListDrag(HWND hWnd, HWND hListWnd, int x_pos, int y_pos);
static BOOL ListEnter(HWND hListWnd, int index);
static BOOL ListRightClick(HWND hWnd, HWND hListWnd, int x_pos, int y_pos, int *pRefreshTaskbar);
static BOOL AppIdPopupMenu(HWND hWnd, HWND hListWnd, int selected_index, int x, int y, int *pRefreshTaskbar);
static BOOL SortGroup(HWND hListWnd, int selected_index);
static BOOL WindowsPopupMenu(HWND hWnd, HWND hListWnd, UINT uSelectedWndCount, int x, int y, int *pRefreshTaskbar);
static void ListPostSysCmdSelected(HWND hListWnd, WPARAM wCommandParam);
static HWND *ListGetHwndArraySelected(HWND hListWnd, UINT *puSelectedWndCount);
static void ListSetAppIdSelected(HWND hListWnd, WCHAR *pAppId);
static BOOL ListCopySelectedToClipboard(HWND hListWnd);

// List functions
static void ListView_MoveItem(HWND hListWnd, int iIndexFrom, int iIndexTo);
static BOOL ImageListRemoveAndListViewUpdate(HIMAGELIST hImageList, int image_index, HWND hListWnd);

// Inspector functions
static void BeforeTaskbarRefresh();
static void AfterTaskbarRefresh();

// Hooks
static void DPA_InsertPtrHook_NewGroup(DPA_HOOK_PARAM *p_dpa_hook_param);
static void DPA_InsertPtrHook_Group(DPA_HOOK_PARAM *p_dpa_hook_param);
static void DPA_InsertPtrHook_ButtonOfNewGroup(DPA_HOOK_PARAM *p_dpa_hook_param);
static void DPA_InsertPtrHook_Button(DPA_HOOK_PARAM *p_dpa_hook_param);
static void DPA_DeletePtrHook_Group(DPA_HOOK_PARAM *p_dpa_hook_param);
static void DPA_DeletePtrHook_Button(DPA_HOOK_PARAM *p_dpa_hook_param);

BOOL ShowInspectorDlg()
{
	if(hInspThread)
	{
		if(hInspWnd)
			SetForegroundWindow(hInspWnd);

		return TRUE;
	}

	hInspThread = CreateThread(NULL, 0, InspectorThread, NULL, CREATE_SUSPENDED, NULL);
	if(!hInspThread)
		return FALSE;

	ResumeThread(hInspThread);

	return TRUE;
}

void CloseInspectorDlg()
{
	HANDLE hThread;
	HWND hWnd;

	hThread = InterlockedExchangePointer(&hInspThread, NULL);
	if(hThread)
	{
		while(!(hWnd = hInspWnd))
		{
			if(WaitForSingleObject(hThread, 10) == WAIT_OBJECT_0)
			{
				CloseHandle(hThread);
				return;
			}
		}

		SendMessage(hWnd, WM_CLOSE, 0, 0);
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
}

DWORD GetInspectorCloseTime()
{
	return dwInspectorCloseTime;
}

static DWORD WINAPI InspectorThread(void *pParameter)
{
	HANDLE hThread;

	if(language_id)
		SetThreadUILanguage(language_id);

	DialogBox(hDllInst, MAKEINTRESOURCE(IDD_INSPECTOR), NULL, (DLGPROC)DlgInspectorProc);

	dwInspectorCloseTime = GetTickCount();

	hThread = InterlockedExchangePointer(&hInspThread, NULL);
	if(hThread)
		CloseHandle(hThread);

	return 0;
}

static LRESULT CALLBACK DlgInspectorProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static long dlg_min_w, dlg_min_h, dlg_last_cw, dlg_last_ch;
	static long dlg_saved_w, dlg_saved_h;
	static BOOL bPersistent, bDragging;
	static HMONITOR hInspMonitor;
	LONG_PTR lpMMTaskListLongPtr;
	HMONITOR hMonitor;
	int refresh_taskbar;
	BOOL bFocusFollowsMouse;
	RECT rc;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		nRefreshing = 0;
		bNewButton = FALSE;
		hNewButtonDpa = NULL;

		SystemParametersInfo(SPI_GETACTIVEWINDOWTRACKING, 0, &bFocusFollowsMouse, 0);
		if(bFocusFollowsMouse || (GetKeyState(VK_CONTROL) < 0 && GetKeyState(VK_SHIFT) < 0))
			bPersistent = TRUE;
		else
			bPersistent = FALSE;

		hInspListWnd = GetDlgItem(hWnd, IDC_LIST);

		switch(*LoadStrFromRsrc(IDS_LANG_RTL))
		{
		case L'\0':
		case L'0':
			break;

		default:
			SetWindowLong(hWnd, GWL_EXSTYLE,
				GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYOUTRTL);

			SetWindowLong(GetDlgItem(hWnd, IDC_LIST), GWL_EXSTYLE,
				GetWindowLong(GetDlgItem(hWnd, IDC_LIST), GWL_EXSTYLE) | WS_EX_LAYOUTRTL);
			break;
		}

		if(!SetForegroundWindow(hWnd))
		{
			EndDialog(hWnd, 0);
			break;
		}

		if(!InitList(hDllInst, GetDlgItem(hWnd, IDC_LIST)))
		{
			TweakerSendErrorMsg(L"Taskbar Inspector: Something went wrong during initialization");
			EndDialog(hWnd, 0);
			break;
		}

		InitWindowPos(hWnd, bPersistent, &dlg_min_w, &dlg_min_h, &dlg_last_cw, &dlg_last_ch);

		if(!LoadWindowSize(&dlg_saved_w, &dlg_saved_h) || dlg_saved_w < dlg_min_w || dlg_saved_h < dlg_min_h)
		{
			GetWindowRect(hWnd, &rc);
			dlg_saved_w = rc.right - rc.left;
			dlg_saved_h = rc.bottom - rc.top;
		}

		MoveResizeWindowCenterMouse(hWnd, dlg_saved_w, dlg_saved_h);

		SetTimer(hWnd, 1, 1000, NULL);

		hInspWnd = hWnd;

		hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		hInspMonitor = hMonitor;

		lpMMTaskListLongPtr = MMTaskListLongPtrFromMonitor(hMonitor);

		SendMessage(hWnd, UWM_INSP_FILLLIST, 0, lpMMTaskListLongPtr);

		// Mark as initialized
		SetWindowLongPtr(hWnd, DWLP_USER, 1);
		break;

	case UWM_INSP_FILLLIST:
		lpMMTaskListLongPtr = lParam;

		if(!FillList(lpMMTaskListLongPtr))
		{
			TweakerSendErrorMsg(L"Taskbar Inspector: Something went wrong while filling up the list");
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		break;

	case WM_GETMINMAXINFO:
		((MINMAXINFO *)lParam)->ptMinTrackSize.x = dlg_min_w;
		((MINMAXINFO *)lParam)->ptMinTrackSize.y = dlg_min_h;
		break;

	case WM_SIZE:
		if(wParam != SIZE_RESTORED && wParam != SIZE_MAXIMIZED)
			break;

		// If not initialized
		if(!GetWindowLongPtr(hWnd, DWLP_USER))
			break;

		GetWindowRect(GetDlgItem(hWnd, IDC_LIST), &rc);
		MapWindowPoints(NULL, hWnd, (POINT *)&rc, 2);
		MoveWindow(GetDlgItem(hWnd, IDC_LIST), rc.left, rc.top,
			LOWORD(lParam) - rc.left * 2, HIWORD(lParam) - rc.top * 2, TRUE);

		dlg_last_cw = LOWORD(lParam);
		dlg_last_ch = HIWORD(lParam);
		// slip down

	case WM_MOVE:
		// If not initialized
		if(!GetWindowLongPtr(hWnd, DWLP_USER))
			break;

		hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		if(hMonitor != hInspMonitor)
		{
			hInspMonitor = hMonitor;

			lpMMTaskListLongPtr = MMTaskListLongPtrFromMonitor(hMonitor);
			if(lpMMTaskListLongPtr != lpInspMMTaskListLongPtr)
				SendMessage(hWnd, UWM_INSP_FILLLIST, 0, lpMMTaskListLongPtr);
		}
		break;

	case WM_TIMER:
		if(wParam == 1)
		{
			if(!nRefreshing)
				ListUpdateIconsTexts(GetDlgItem(hWnd, IDC_LIST));
		}
		break;

	case WM_ACTIVATE:
		if((HWND)lParam == NULL && LOWORD(wParam) == WA_INACTIVE && !bPersistent)
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		break;

	case WM_CONTEXTMENU:
		if((HWND)wParam == GetDlgItem(hWnd, IDC_LIST))
		{
			if(ListRightClick(hWnd, GetDlgItem(hWnd, IDC_LIST), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), &refresh_taskbar))
			{
				switch(refresh_taskbar)
				{
				case 1:
					SendMessage(hTaskbarWnd, uTweakerMsg, (WPARAM)MMTaskListRecomputeLayout, MSG_DLL_CALLFUNC);
					break;

				case 2:
					SendMessage(hTaskbarWnd, uTweakerMsg, (WPARAM)RefreshTaskbarHardcore, MSG_DLL_CALLFUNC);
					break;
				}
			}
			else
				TweakerSendErrorMsg(L"Taskbar Inspector: Something went wrong");
		}
		break;

	case WM_NOTIFY:
		if(((NMHDR *)lParam)->idFrom == IDC_LIST)
		{
			switch(((NMHDR *)lParam)->code)
			{
			case NM_DBLCLK:
				if(((NMITEMACTIVATE *)lParam)->iItem != -1)
					ListEnter(GetDlgItem(hWnd, IDC_LIST), ((NMITEMACTIVATE *)lParam)->iItem);
				break;

			case LVN_BEGINDRAG:
				SetCapture(hWnd);
				bDragging = TRUE;
				break;

			case LVN_KEYDOWN:
				switch(((NMLVKEYDOWN *)lParam)->wVKey)
				{
				case 'A':
					if(GetKeyState(VK_CONTROL) < 0) // Ctrl+A - Select all
						ListView_SetItemState(GetDlgItem(hWnd, IDC_LIST), -1, LVIS_SELECTED, LVIS_SELECTED);
					break;

				case 'C':
				case VK_INSERT:
					if(GetKeyState(VK_CONTROL) < 0) // Ctrl+C or Ctrl+Insert - Copy to clipboard
						ListCopySelectedToClipboard(GetDlgItem(hWnd, IDC_LIST));
					break;
				}
				break;
			}
		}
		break;

	case WM_LBUTTONDOWN:
		SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		break;

	case WM_LBUTTONUP:
		if(bDragging)
			ReleaseCapture();
		break;

	case WM_CAPTURECHANGED:
		if(bDragging)
			bDragging = FALSE;
		break;

	case WM_MOUSEMOVE:
		if(bDragging)
			ListDrag(hWnd, GetDlgItem(hWnd, IDC_LIST), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case UWM_INSP_CALLFUNC:
		SetWindowLongPtr(hWnd, DWLP_MSGRESULT, ((LONG_PTR(*)())wParam)());
		return TRUE;

	case UWM_INSP_CALLFUNC_PARAM:
		SetWindowLongPtr(hWnd, DWLP_MSGRESULT, ((LONG_PTR(*)(void *))wParam)((void *)lParam));
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			ListEnter(GetDlgItem(hWnd, IDC_LIST), -1);
			break;

		case IDCANCEL:
			KillTimer(hWnd, 1);
			EnableWindow(hWnd, FALSE);

			bHooksEnabled = FALSE;

			if(hook_proc_call_counter > 0)
			{
				// Inspector will be closed when the DPA hook processing is done
				bHookCloseInspector = TRUE;
			}
			else
				SendMessage(hWnd, UWM_INSP_FINALLYCLOSE, 0, 0);
			break;
		}
		break;

	case UWM_INSP_FINALLYCLOSE:
		GetWindowRect(hWnd, &rc);
		if(dlg_saved_w != rc.right - rc.left || dlg_saved_h != rc.bottom - rc.top)
		{
			SaveWindowSize(rc.right - rc.left, rc.bottom - rc.top);
		}

		EndDialog(hWnd, 0);
		break;

	case WM_DESTROY:
		hInspWnd = NULL;
		break;
	}

	return FALSE;
}

static void MoveResizeWindowCenterMouse(HWND hWnd, long w, long h)
{
	POINT pt;
	SIZE size;
	HMONITOR hMonitor;
	LONG_PTR lpSecondaryTaskListLongPtr;
	LONG_PTR lp;
	HWND hSecondaryTaskbarWnd;
	RECT rcExclude;
	RECT rc, rcList;

	GetCursorPos(&pt);

	size.cx = w;
	size.cy = h;

	hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

	lpSecondaryTaskListLongPtr = MMTaskListLongPtrFromMonitor(hMonitor);
	if(lpSecondaryTaskListLongPtr != lpTaskListLongPtr)
	{
		lp = EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lpSecondaryTaskListLongPtr);
		lp = EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lp);

		hSecondaryTaskbarWnd = *EV_SECONDARY_TASKBAR_HWND(lp);
	}
	else
		hSecondaryTaskbarWnd = hTaskbarWnd;

	GetWindowRect(hSecondaryTaskbarWnd, &rcExclude);

	CalculatePopupWindowPosition(&pt, &size, TPM_CENTERALIGN | TPM_VCENTERALIGN | TPM_WORKAREA, &rcExclude, &rc);

	SetWindowPos(hWnd, NULL, rc.left, rc.top,
		rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);

	GetClientRect(hWnd, &rc);
	GetWindowRect(GetDlgItem(hWnd, IDC_LIST), &rcList);
	MapWindowPoints(NULL, hWnd, (POINT *)&rcList, 2);
	MoveWindow(GetDlgItem(hWnd, IDC_LIST), rcList.left, rcList.top,
		rc.right - rc.left - rcList.left * 2, rc.bottom - rc.top - rcList.top * 2, TRUE);
}

static void InitWindowPos(HWND hWnd, BOOL bShowCaption, long *p_dlg_min_w, long *p_dlg_min_h, long *p_dlg_last_cw, long *p_dlg_last_ch)
{
	HWND hListWnd;
	RECT rcClient;
	RECT rcListWnd;
	RECT rc;

	hListWnd = GetDlgItem(hWnd, IDC_LIST);

	GetClientRect(hWnd, &rcClient);
	*p_dlg_last_cw = rcClient.right - rcClient.left;
	*p_dlg_last_ch = rcClient.bottom - rcClient.top;

	if(bShowCaption)
	{
		// Add caption with an exit button
		SetWindowText(hWnd, L"Taskbar Inspector");

		SetWindowLong(hWnd, GWL_STYLE,
			GetWindowLong(hWnd, GWL_STYLE) | DS_MODALFRAME | WS_CAPTION | WS_SYSMENU);

		// Note: this causes WM_SIZE to be sent
		SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}

	GetWindowRect(hListWnd, &rcListWnd);

	GetWindowRect(hWnd, &rc);
	*p_dlg_min_w = rc.right - rc.left - (rcListWnd.right - rcListWnd.left);
	*p_dlg_min_h = rc.bottom - rc.top - (rcListWnd.bottom - rcListWnd.top);
}

static BOOL LoadWindowSize(long *pw, long *ph)
{
	PS_SECTION section;
	int w, h;
	LSTATUS error;

	error = PSOpenSection(NULL, FALSE, &section);
	if(error == ERROR_SUCCESS)
	{
		error = PSGetInt(&section, L"inspector_w", &w);
		if(error == ERROR_SUCCESS)
		{
			error = PSGetInt(&section, L"inspector_h", &h);
		}

		PSCloseSection(&section);
	}

	if(error != ERROR_SUCCESS)
		return FALSE;

	*pw = (long)w;
	*ph = (long)h;

	return TRUE;
}

static BOOL SaveWindowSize(long w, long h)
{
	PS_SECTION section;
	LSTATUS error;

	error = PSOpenSection(NULL, TRUE, &section);
	if(error == ERROR_SUCCESS)
	{
		error = PSSetInt(&section, L"inspector_w", w);
		if(error == ERROR_SUCCESS)
			error = PSSetInt(&section, L"inspector_h", h);

		PSCloseSection(&section);
	}

	return error == ERROR_SUCCESS;
}

static BOOL InitList(HINSTANCE hInst, HWND hListWnd)
{
	LVCOLUMN lvc;
	HIMAGELIST hImageList;
	HICON hIcon;
	ATOM nTooltipClassAtom;
	WNDCLASS wndclass;

	// Columns
	lvc.mask = LVCF_WIDTH;
	lvc.cx = INT_MAX; // Dirty but works (with LVS_EX_AUTOSIZECOLUMNS)

	ListView_InsertColumn(hListWnd, 0, &lvc);

	// Image list and icons
	hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
		ILC_MASK | ILC_COLOR32 | ILC_MIRROR, 1, 1);
	if(!hImageList)
		return FALSE;

	ListView_SetImageList(hListWnd, hImageList, LVSIL_SMALL);

	hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_APPID), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	if(!hIcon)
		return FALSE;

	if(GetWindowLong(hListWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
		MirrorIcon_(&hIcon, NULL);

	ImageList_AddIcon(hImageList, hIcon);
	DestroyIcon(hIcon);

	hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_DEF), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	if(!hIcon)
		return FALSE;

	ImageList_AddIcon(hImageList, hIcon);
	DestroyIcon(hIcon);

	// Styles
	ListView_SetExtendedListViewStyle(hListWnd, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER | LVS_EX_AUTOSIZECOLUMNS);
	SetWindowTheme(hListWnd, L"Explorer", NULL);

	// Fix tooltips not always on top
	nTooltipClassAtom = (ATOM)GetClassInfo(NULL, TOOLTIPS_CLASS, &wndclass);
	if(nTooltipClassAtom)
		EnumThreadWindows(GetWindowThreadProcessId(hListWnd, NULL), EnumThreadWndProc, (LPARAM)nTooltipClassAtom);

	return TRUE;
}

static BOOL CALLBACK EnumThreadWndProc(HWND hWnd, LPARAM lParam)
{
	ATOM nClassAtom;

	nClassAtom = (ATOM)GetClassLong(hWnd, GCW_ATOM);
	if(nClassAtom == (ATOM)lParam)
	{
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
		return FALSE;
	}

	return TRUE;
}

static BOOL FillList(LONG_PTR lpMMTaskListLongPtr)
{
	CALLFUCN_PARAM callfucn_param;

	callfucn_param.pFunction = ExplorerThreadFillList;
	callfucn_param.pParam = (void *)lpMMTaskListLongPtr;

	return (BOOL)SendMessage(hTaskbarWnd, uTweakerMsg, (WPARAM)&callfucn_param, MSG_DLL_CALLFUNC_PARAM);
}

static BOOL ExplorerThreadFillList(LONG_PTR lpMMTaskListLongPtr)
{
	HWND hListWnd;
	HIMAGELIST hImageList;
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	int button_group_type;
	int buttons_count;
	LONG_PTR **buttons;
	int list_index;
	int i, j;
	BOOL bSuccess;

	plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
	if(plp)
	{
		hListWnd = hInspListWnd;

		if(!nRefreshing)
			SendMessage(hListWnd, WM_SETREDRAW, FALSE, 0);

		hImageList = ListView_GetImageList(hListWnd, LVSIL_SMALL);

		ListView_DeleteAllItems(hListWnd);
		ImageList_SetImageCount(hImageList, 2);

		list_index = 0;

		button_groups_count = (int)plp[0];
		button_groups = (LONG_PTR **)plp[1];

		for(i = 0; i < button_groups_count; i++)
		{
			button_group_type = (int)button_groups[i][DO2(6, 8)];
			if(button_group_type < 0 || button_group_type > 4) // 4 is our hack of removing pinned items gap
				break;

			AddButtonGroupToList(hListWnd, list_index, button_groups[i]);
			list_index++;

			if(button_group_type == 1 || button_group_type == 3)
			{
				plp = (LONG_PTR *)button_groups[i][DO2(5, 7)];
				if(!plp)
					break;

				buttons_count = (int)plp[0];
				buttons = (LONG_PTR **)plp[1];

				for(j = 0; j < buttons_count; j++)
				{
					AddButtonToList(hListWnd, list_index, hImageList, buttons[j]);
					list_index++;
				}
			}
		}

		bSuccess = (i == button_groups_count);

		if(!nRefreshing)
		{
			SendMessage(hListWnd, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(hListWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
	}
	else
		bSuccess = TRUE;

	if(bSuccess)
	{
		lpInspMMTaskListLongPtr = lpMMTaskListLongPtr;
		bHooksEnabled = TRUE;
	}
	else
		bHooksEnabled = FALSE;

	return bSuccess;
}

static void AddButtonGroupToList(HWND hListWnd, int list_index, LONG_PTR *button_group)
{
	LVITEM lvi;
	LONG_PTR *task_group;

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_INDENT;
	lvi.iItem = list_index;
	lvi.iSubItem = 0;

	task_group = (LONG_PTR *)button_group[DO2(3, 4)];
	lvi.pszText = *EV_TASKGROUP_APPID(task_group);
	if(!lvi.pszText)
		lvi.pszText = L"";
	lvi.iImage = 0;
	lvi.lParam = (LPARAM)button_group;
	lvi.iIndent = 0;

	ListView_InsertItem(hListWnd, &lvi);
}

static void AddButtonToList(HWND hListWnd, int list_index, HIMAGELIST hImageList, LONG_PTR *button)
{
	LVITEM lvi;
	LONG_PTR *task_item;
	HWND hButtonWnd;
	WCHAR szBuffer[256];
	HICON hIcon;

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_INDENT;
	lvi.iItem = list_index;
	lvi.iSubItem = 0;

	task_item = (LONG_PTR *)button[DO2(3, 4)];
	hButtonWnd = GetTaskItemWnd(task_item);

	switch(InternalGetWindowText(hButtonWnd, szBuffer, 256))
	{
	case 0:
		lvi.pszText = LoadStrFromRsrc(IDS_NOTITLE);
		break;

	case 255:
		lstrcpy(szBuffer + 255 - 3, L"...");
	default:
		lvi.pszText = szBuffer;
		break;
	}

	hIcon = GetWindowIcon(hButtonWnd);
	if(hIcon)
	{
		lvi.iImage = ImageList_AddIcon(hImageList, hIcon);
		if(lvi.iImage == -1)
			lvi.iImage = 1;
	}
	else
		lvi.iImage = 1;

	lvi.lParam = (LPARAM)task_item;
	lvi.iIndent = 1;

	ListView_InsertItem(hListWnd, &lvi);
}

static void ListUpdateIconsTexts(HWND hListWnd)
{
	HIMAGELIST hImageList;
	LVITEM lvi;

	if(!nRefreshing)
		SendMessage(hListWnd, WM_SETREDRAW, FALSE, 0);

	hImageList = ListView_GetImageList(hListWnd, LVSIL_SMALL);

	lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_INDENT;
	lvi.iItem = 0;
	lvi.iSubItem = 0;

	while(ListView_GetItem(hListWnd, &lvi))
	{
		if(lvi.iIndent == 1)
			ListUpdateWindowItem(hListWnd, hImageList, &lvi);

		lvi.iItem++;
	}

	if(!nRefreshing)
	{
		SendMessage(hListWnd, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hListWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}

static BOOL ListUpdateWindowItem(HWND hListWnd, HIMAGELIST hImageList, LVITEM *plvi)
{
	// input: LVITEM that contains iItem, iImage and lParam

	LVITEM lvi;
	LONG_PTR *task_item;
	HWND hButtonWnd;
	WCHAR szBuffer[256];
	HICON hIcon;
	int iImage, iNewImage;

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.iItem = plvi->iItem;
	lvi.iSubItem = plvi->iSubItem;
	lvi.lParam = plvi->lParam;

	task_item = (LONG_PTR *)plvi->lParam;
	hButtonWnd = GetTaskItemWnd(task_item);

	switch(InternalGetWindowText(hButtonWnd, szBuffer, 256))
	{
	case 0:
		lvi.pszText = LoadStrFromRsrc(IDS_NOTITLE);
		break;

	case 255:
		lstrcpy(szBuffer + 255 - 3, L"...");
	default:
		lvi.pszText = szBuffer;
		break;
	}

	hIcon = GetWindowIcon(hButtonWnd);

	iImage = plvi->iImage;
	if(iImage > 1)
	{
		if(hIcon)
			iNewImage = ImageList_ReplaceIcon(hImageList, iImage, hIcon);
		else
			iNewImage = -1;

		if(iImage != iNewImage)
			ImageListRemoveAndListViewUpdate(hImageList, iImage, hListWnd);
	}
	else
	{
		if(hIcon)
			iNewImage = ImageList_AddIcon(hImageList, hIcon);
		else
			iNewImage = -1;
	}

	if(iNewImage == -1)
		lvi.iImage = 1;
	else
		lvi.iImage = iNewImage;

	return ListView_SetItem(hListWnd, &lvi);
}

static HICON GetWindowIcon(HWND hWnd)
{
	HICON hIcon;

	if(SendMessageTimeout(hWnd, WM_GETICON, ICON_SMALL, 0, SMTO_BLOCK | SMTO_ABORTIFHUNG, 100, (DWORD_PTR *)&hIcon) && hIcon)
		return hIcon;

	hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICONSM);
	if(hIcon)
		return hIcon;

	if(SendMessageTimeout(hWnd, WM_GETICON, ICON_BIG, 0, SMTO_BLOCK | SMTO_ABORTIFHUNG, 100, (DWORD_PTR *)&hIcon) && hIcon)
		return hIcon;

	hIcon = (HICON)GetClassLongPtr(hWnd, GCLP_HICON);
	if(hIcon)
		return hIcon;

	return NULL;
}

static BOOL ListDrag(HWND hWnd, HWND hListWnd, int x_pos, int y_pos)
{
	MOVE_BUTTON_IN_GROUP move_button;
	CALLFUCN_PARAM callfucn_param;
	LVHITTESTINFO lvhti;
	LVITEM lvi;
	int hit_index, sel_index;

	lvi.mask = LVIF_INDENT;
	lvi.iSubItem = 0;

	if(ListView_GetSelectedCount(hListWnd) != 1)
		return FALSE;

	sel_index = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);

	lvi.iItem = sel_index;
	ListView_GetItem(hListWnd, &lvi);
	if(lvi.iIndent == 0)
		return FALSE;

	lvhti.pt.x = x_pos;
	lvhti.pt.y = y_pos;

	MapWindowPoints(hWnd, hListWnd, &lvhti.pt, 1);

	hit_index = ListView_HitTest(hListWnd, &lvhti);
	if(hit_index == -1 || hit_index == sel_index)
		return FALSE;

	if(hit_index > sel_index)
	{
		for(lvi.iItem = sel_index + 1; lvi.iItem <= hit_index; lvi.iItem++)
		{
			ListView_GetItem(hListWnd, &lvi);
			if(lvi.iIndent == 0)
			{
				hit_index = lvi.iItem - 1;
				if(sel_index == hit_index)
					return FALSE;
			}
		}

		lvi.mask = LVIF_PARAM | LVIF_INDENT;
		lvi.iItem = sel_index;
	}
	else
	{
		for(lvi.iItem = sel_index - 1; lvi.iItem >= hit_index; lvi.iItem--)
		{
			ListView_GetItem(hListWnd, &lvi);
			if(lvi.iIndent == 0)
			{
				hit_index = lvi.iItem + 1;
				if(sel_index == hit_index)
					return FALSE;
			}
		}

		lvi.mask = LVIF_PARAM | LVIF_INDENT;
		lvi.iItem = hit_index;
	}

	do
	{
		lvi.iItem--;
		ListView_GetItem(hListWnd, &lvi);
	}
	while(lvi.iIndent != 0);

	move_button.button_group = (LONG_PTR *)lvi.lParam;
	move_button.index_from = sel_index - (lvi.iItem + 1);
	move_button.index_to = hit_index - (lvi.iItem + 1);

	callfucn_param.pFunction = TaskbarMoveButtonInGroup;
	callfucn_param.pParam = &move_button;

	SendMessage(hTaskbarWnd, uTweakerMsg, (WPARAM)&callfucn_param, MSG_DLL_CALLFUNC_PARAM);

	ListView_SetItemState(hListWnd, hit_index, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

	return TRUE;
}

static BOOL ListEnter(HWND hListWnd, int index)
{
	LVITEM lvi;
	BOOL bSelected;

	if(index == -1)
	{
		if(ListView_GetSelectedCount(hListWnd) != 1)
			return FALSE;

		index = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
	}

	lvi.mask = LVIF_PARAM | LVIF_STATE | LVIF_INDENT;
	lvi.iItem = index;
	lvi.iSubItem = 0;
	lvi.stateMask = LVIS_SELECTED;
	ListView_GetItem(hListWnd, &lvi);

	bSelected = ((lvi.state & LVIS_SELECTED) != FALSE);

	if(lvi.iIndent == 0)
	{
		lvi.iItem++;

		while(ListView_GetItem(hListWnd, &lvi) && lvi.iIndent == 1)
		{
			ListView_SetItemState(hListWnd, lvi.iItem, bSelected ? LVIS_SELECTED : 0, LVIS_SELECTED);
			lvi.iItem++;
		}
	}
	else if(bSelected)
		SwitchToTaskItem((LONG_PTR *)lvi.lParam);

	return TRUE;
}

static BOOL ListRightClick(HWND hWnd, HWND hListWnd, int x_pos, int y_pos, int *pRefreshTaskbar)
{
	UINT uSelectedCount, uSelectedWndCount;
	int nSelectedIndex;
	LVITEM lvi;
	RECT rc;
	POINT pt;

	*pRefreshTaskbar = 0;

	// Get nSelectedIndex, uSelectedCount, uSelectedWndCount
	uSelectedCount = ListView_GetSelectedCount(hListWnd);
	if(uSelectedCount == 0)
		return TRUE;

	nSelectedIndex = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);

	lvi.mask = LVIF_INDENT;
	lvi.iItem = nSelectedIndex;
	lvi.iSubItem = 0;
	ListView_GetItem(hListWnd, &lvi);

	if(lvi.iIndent == 1)
		uSelectedWndCount = 1;
	else
		uSelectedWndCount = 0;

	if(uSelectedCount > 1)
	{
		lvi.iItem = ListView_GetNextItem(hListWnd, lvi.iItem, LVNI_SELECTED);
		while(lvi.iItem != -1)
		{
			ListView_GetItem(hListWnd, &lvi);
			if(lvi.iIndent == 1)
				uSelectedWndCount++;

			lvi.iItem = ListView_GetNextItem(hListWnd, lvi.iItem, LVNI_SELECTED);
		}

		if(uSelectedWndCount == 0)
			return TRUE; // multiple appids - nothing to do with them
	}

	// Calc x, y if using keyboard
	if(x_pos == -1 && y_pos == -1)
	{
		if(ListView_IsItemVisible(hListWnd, nSelectedIndex))
			ListView_GetItemRect(hListWnd, nSelectedIndex, &rc, LVIR_ICON);
		else
			ListView_GetItemRect(hListWnd, ListView_GetTopIndex(hListWnd), &rc, LVIR_ICON);

		pt.x = rc.right;
		pt.y = rc.bottom;

		ClientToScreen(hListWnd, &pt);
		x_pos = pt.x;
		y_pos = pt.y;
	}

	// Make and show the menu, do the action
	if(uSelectedCount == 1 && uSelectedWndCount == 0)
		return AppIdPopupMenu(hWnd, hListWnd, nSelectedIndex, x_pos, y_pos, pRefreshTaskbar);
	else if(uSelectedWndCount > 0)
		return WindowsPopupMenu(hWnd, hListWnd, uSelectedWndCount, x_pos, y_pos, pRefreshTaskbar);

	return TRUE;
}

static BOOL AppIdPopupMenu(HWND hWnd, HWND hListWnd, int nSelectedIndex, int x, int y, int *pRefreshTaskbar)
{
	HMENU hMenu;
	HMENU hSubMenu;
	HMENU hSubMenus[4];
	UINT nSubMenusCount;
	LVITEM lvi;
	MENUITEMINFO miiItemInfo;
	WCHAR szAppId[MAX_PATH];
	int nListValue;
	BOOL bRandomGroup;
	int nAppidList;
	WCHAR *pszNever, *pszAlways;
	WCHAR szMenuText[1024 + 1]; // safe for wsprintf
	BOOL bMenuResult;
	int n;

	hMenu = CreatePopupMenu();
	nSubMenusCount = 0;

	lvi.mask = LVIF_TEXT;
	lvi.iItem = nSelectedIndex;
	lvi.iSubItem = 0;
	lvi.pszText = szAppId;
	lvi.cchTextMax = MAX_PATH;
	ListView_GetItem(hListWnd, &lvi);

	miiItemInfo.cbSize = sizeof(MENUITEMINFO);
	miiItemInfo.fMask = MIIM_STATE | MIIM_ID;
	miiItemInfo.fState = MF_STRING | MFS_CHECKED;

	pszNever = LoadStrFromRsrc(IDS_RCMENU_APPID_NEVER);
	pszAlways = LoadStrFromRsrc(IDS_RCMENU_APPID_ALWAYS);

	// Show labels
	hSubMenu = CreatePopupMenu();
	hSubMenus[nSubMenusCount++] = hSubMenu;

	AppendMenu(hSubMenu, MF_STRING, APPIDMENU_LABELNEVER, pszNever);
	AppendMenu(hSubMenu, MF_STRING, APPIDMENU_LABELALWAYS, pszAlways);

	if(GetAppidListValue(AILIST_LABEL, szAppId, &nListValue))
	{
		miiItemInfo.wID = APPIDMENU_LABELDEF;
		SetMenuItemInfo(hSubMenu, nListValue, TRUE, &miiItemInfo);

		n = wsprintf(szMenuText, L"%s: ", LoadStrFromRsrc(IDS_RCMENU_APPID_LABEL));
		StripAmpersand(szMenuText + n, (nListValue == AILIST_LABEL_NEVER) ? pszNever : pszAlways);

		AppendMenu(hMenu, MF_POPUP | MF_CHECKED, (UINT_PTR)hSubMenu, szMenuText);
	}
	else
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, LoadStrFromRsrc(IDS_RCMENU_APPID_LABEL));

	if(!IsAppIdARandomGroup(szAppId))
	{
		bRandomGroup = FALSE;

		// Group
		hSubMenu = CreatePopupMenu();
		hSubMenus[nSubMenusCount++] = hSubMenu;

		AppendMenu(hSubMenu, MF_STRING, APPIDMENU_GROUPNEVER, pszNever);
		AppendMenu(hSubMenu, MF_STRING, APPIDMENU_GROUPALWAYS, pszAlways);

		if(GetAppidListValue(AILIST_GROUP, szAppId, &nListValue))
		{
			miiItemInfo.wID = APPIDMENU_GROUPDEF;
			SetMenuItemInfo(hSubMenu, nListValue, TRUE, &miiItemInfo);

			n = wsprintf(szMenuText, L"%s: ", LoadStrFromRsrc(IDS_RCMENU_APPID_GROUP));
			StripAmpersand(szMenuText + n, (nListValue == AILIST_GROUP_NEVER) ? pszNever : pszAlways);

			AppendMenu(hMenu, MF_POPUP | MF_CHECKED, (UINT_PTR)hSubMenu, szMenuText);
		}
		else
			AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, LoadStrFromRsrc(IDS_RCMENU_APPID_GROUP));

		// Group pinned items
		hSubMenu = CreatePopupMenu();
		hSubMenus[nSubMenusCount++] = hSubMenu;

		AppendMenu(hSubMenu, MF_STRING, APPIDMENU_GROUPPINNEDNEVER, pszNever);
		AppendMenu(hSubMenu, MF_STRING, APPIDMENU_GROUPPINNEDALWAYS, pszAlways);

		if(GetAppidListValue(AILIST_GROUPPINNED, szAppId, &nListValue))
		{
			miiItemInfo.wID = APPIDMENU_GROUPPINNEDDEF;
			SetMenuItemInfo(hSubMenu, nListValue, TRUE, &miiItemInfo);

			n = wsprintf(szMenuText, L"%s: ", LoadStrFromRsrc(IDS_RCMENU_APPID_GROUPPINNED));
			StripAmpersand(szMenuText + n, (nListValue == AILIST_GROUPPINNED_NEVER) ? pszNever : pszAlways);

			AppendMenu(hMenu, MF_POPUP | MF_CHECKED, (UINT_PTR)hSubMenu, szMenuText);
		}
		else
			AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, LoadStrFromRsrc(IDS_RCMENU_APPID_GROUPPINNED));
	}
	else
		bRandomGroup = TRUE;

	// Combine
	hSubMenu = CreatePopupMenu();
	hSubMenus[nSubMenusCount++] = hSubMenu;

	AppendMenu(hSubMenu, MF_STRING, APPIDMENU_COMBINENEVER, pszNever);
	AppendMenu(hSubMenu, MF_STRING, APPIDMENU_COMBINEALWAYS, pszAlways);

	if(GetAppidListValue(AILIST_COMBINE, szAppId, &nListValue))
	{
		miiItemInfo.wID = APPIDMENU_COMBINEDEF;
		SetMenuItemInfo(hSubMenu, nListValue, TRUE, &miiItemInfo);

		n = wsprintf(szMenuText, L"%s: ", LoadStrFromRsrc(IDS_RCMENU_APPID_COMBINE));
		StripAmpersand(szMenuText + n, (nListValue == AILIST_COMBINE_NEVER) ? pszNever : pszAlways);

		AppendMenu(hMenu, MF_POPUP | MF_CHECKED, (UINT_PTR)hSubMenu, szMenuText);
	}
	else
		AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, LoadStrFromRsrc(IDS_RCMENU_APPID_COMBINE));

	// Sort
	lvi.mask = LVIF_INDENT;
	lvi.iItem = nSelectedIndex + 1;
	lvi.iSubItem = 0;

	if(ListView_GetItem(hListWnd, &lvi) && lvi.iIndent == 1)
	{
		lvi.iItem++;

		if(ListView_GetItem(hListWnd, &lvi) && lvi.iIndent == 1)
		{
			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenu(hMenu, MF_STRING, APPIDMENU_SORT, LoadStrFromRsrc(IDS_RCMENU_APPID_SORTITEMS));
		}
	}

	// Go for it!
	bMenuResult = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hWnd, NULL);

	DestroyMenu(hMenu);

	while(nSubMenusCount--)
		DestroyMenu(hSubMenus[nSubMenusCount]);

	if(bMenuResult >= APPIDMENU_LABELNEVER && bMenuResult <= APPIDMENU_LABELDEF)
	{
		nAppidList = AILIST_LABEL;
		*pRefreshTaskbar = 1;
	}
	else if(bMenuResult >= APPIDMENU_GROUPNEVER && bMenuResult <= APPIDMENU_GROUPDEF)
	{
		nAppidList = AILIST_GROUP;
		*pRefreshTaskbar = 2;
	}
	else if(bMenuResult >= APPIDMENU_GROUPPINNEDNEVER && bMenuResult <= APPIDMENU_GROUPPINNEDDEF)
	{
		nAppidList = AILIST_GROUPPINNED;
		*pRefreshTaskbar = 2;
	}
	else if(bMenuResult >= APPIDMENU_COMBINENEVER && bMenuResult <= APPIDMENU_COMBINEDEF)
	{
		nAppidList = AILIST_COMBINE;
		*pRefreshTaskbar = 2;
	}

	switch(bMenuResult)
	{
	case APPIDMENU_LABELNEVER:
	case APPIDMENU_GROUPNEVER:
	case APPIDMENU_GROUPPINNEDNEVER:
	case APPIDMENU_COMBINENEVER:
		// assumes AILIST_LABEL_NEVER == AILIST_GROUP_NEVER == AILIST_GROUPPINNED_NEVER == AILIST_COMBINE_NEVER
		return AddAppidToList(nAppidList, szAppId, AILIST_LABEL_NEVER, bRandomGroup);

	case APPIDMENU_LABELALWAYS:
	case APPIDMENU_GROUPALWAYS:
	case APPIDMENU_GROUPPINNEDALWAYS:
	case APPIDMENU_COMBINEALWAYS:
		// assumes AILIST_LABEL_ALWAYS == AILIST_GROUP_ALWAYS == AILIST_GROUPPINNED_ALWAYS == AILIST_COMBINE_ALWAYS
		return AddAppidToList(nAppidList, szAppId, AILIST_LABEL_ALWAYS, bRandomGroup);

	case APPIDMENU_LABELDEF:
	case APPIDMENU_GROUPDEF:
	case APPIDMENU_GROUPPINNEDDEF:
	case APPIDMENU_COMBINEDEF:
		return RemoveAppidFromList(nAppidList, szAppId, bRandomGroup);

	case APPIDMENU_SORT:
		return SortGroup(hListWnd, nSelectedIndex);
	}

	return TRUE;
}

static BOOL SortGroup(HWND hListWnd, int nSelectedIndex)
{
	LVITEM lvi;
	LONG_PTR *button_group;
	CALLFUCN_PARAM callfucn_param;

	lvi.mask = LVIF_PARAM;
	lvi.iItem = nSelectedIndex;
	lvi.iSubItem = 0;

	if(!ListView_GetItem(hListWnd, &lvi) || !lvi.lParam)
		return FALSE;

	button_group = (LONG_PTR *)lvi.lParam;

	callfucn_param.pFunction = SortButtonGroupItems;
	callfucn_param.pParam = button_group;

	SendMessage(hTaskbarWnd, uTweakerMsg, (WPARAM)&callfucn_param, MSG_DLL_CALLFUNC_PARAM);

	return TRUE;
}

static BOOL WindowsPopupMenu(HWND hWnd, HWND hListWnd, UINT uSelectedWndCount, int x, int y, int *pRefreshTaskbar)
{
	HMENU hMenu;
	HMENU hSubMenu;
	LVITEM lvi;
	MENUITEMINFO miiItemInfo;
	WCHAR szAppId[MAX_PATH];
	BOOL bMenuResult;
	HWND *phWndArray;
	int i;

	hMenu = CreatePopupMenu();
	hSubMenu = CreatePopupMenu();

	lvi.mask = LVIF_TEXT | LVIF_INDENT;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.pszText = szAppId;
	lvi.cchTextMax = MAX_PATH;

	for(i = 0; ListView_GetItem(hListWnd, &lvi); i++)
	{
		if(lvi.iIndent == 0)
			AppendMenu(hSubMenu, MF_STRING, WNDMENU_NEWAPPID + i, lvi.pszText);

		lvi.iItem++;
	}

	AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
	wsprintf(szAppId, L"random_group_%u", GetTickCount());
	AppendMenu(hSubMenu, MF_STRING, WNDMENU_NEWAPPID + i, szAppId);
	AppendMenu(hSubMenu, MF_STRING, WNDMENU_DELAPPID, LoadStrFromRsrc(IDS_RCMENU_ID_RESTORE));

	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, LoadStrFromRsrc(IDS_RCMENU_ID_CHANGE));

	if(uSelectedWndCount > 1)
	{
		AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(hMenu, MF_STRING, WNDMENU_CASCADE, LoadStrFromRsrc(IDS_RCMENU_CASCADE));
		AppendMenu(hMenu, MF_STRING, WNDMENU_TILEH, LoadStrFromRsrc(IDS_RCMENU_TILEH));
		AppendMenu(hMenu, MF_STRING, WNDMENU_TILEV, LoadStrFromRsrc(IDS_RCMENU_TILEV));
	}

	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, WNDMENU_RESTORE, LoadStrFromRsrc(IDS_RCMENU_RESTORE));
	AppendMenu(hMenu, MF_STRING, WNDMENU_MINIMIZE, LoadStrFromRsrc(IDS_RCMENU_MINIMIZE));
	AppendMenu(hMenu, MF_STRING, WNDMENU_MAXIMIZE, LoadStrFromRsrc(IDS_RCMENU_MAXIMIZE));
	AppendMenu(hMenu, MF_STRING, WNDMENU_CLOSE, LoadStrFromRsrc(IDS_RCMENU_CLOSE));

	bMenuResult = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, x, y, 0, hWnd, NULL);
	if(bMenuResult >= WNDMENU_NEWAPPID)
	{
		miiItemInfo.cbSize = sizeof(MENUITEMINFO);
		miiItemInfo.fMask = MIIM_STRING;
		miiItemInfo.dwTypeData = szAppId;
		miiItemInfo.cch = MAX_PATH;
		GetMenuItemInfo(hSubMenu, bMenuResult, FALSE, &miiItemInfo);

		bMenuResult = WNDMENU_NEWAPPID;
	}

	DestroyMenu(hMenu);
	DestroyMenu(hSubMenu);

	switch(bMenuResult)
	{
	case WNDMENU_RESTORE:
		ListPostSysCmdSelected(hListWnd, SC_RESTORE);
		return TRUE;

	case WNDMENU_MINIMIZE:
		ListPostSysCmdSelected(hListWnd, SC_MINIMIZE);
		return TRUE;

	case WNDMENU_MAXIMIZE:
		ListPostSysCmdSelected(hListWnd, SC_MAXIMIZE);
		return TRUE;

	case WNDMENU_CLOSE:
		ListPostSysCmdSelected(hListWnd, SC_CLOSE);
		return TRUE;

	case WNDMENU_CASCADE:
		phWndArray = ListGetHwndArraySelected(hListWnd, &uSelectedWndCount);
		if(uSelectedWndCount > 0)
		{
			if(!phWndArray)
				return FALSE;

			CascadeWindows(NULL, 0, NULL, uSelectedWndCount, phWndArray);
			HeapFree(GetProcessHeap(), 0, phWndArray);
		}
		return TRUE;

	case WNDMENU_TILEH:
		phWndArray = ListGetHwndArraySelected(hListWnd, &uSelectedWndCount);
		if(uSelectedWndCount > 0)
		{
			if(!phWndArray)
				return FALSE;

			TileWindows(NULL, MDITILE_HORIZONTAL, NULL, uSelectedWndCount, phWndArray);
			HeapFree(GetProcessHeap(), 0, phWndArray);
		}
		return TRUE;

	case WNDMENU_TILEV:
		phWndArray = ListGetHwndArraySelected(hListWnd, &uSelectedWndCount);
		if(uSelectedWndCount > 0)
		{
			if(!phWndArray)
				return FALSE;

			TileWindows(NULL, MDITILE_VERTICAL, NULL, uSelectedWndCount, phWndArray);
			HeapFree(GetProcessHeap(), 0, phWndArray);
		}
		return TRUE;

	case WNDMENU_DELAPPID:
		ListSetAppIdSelected(hListWnd, NULL);
		break;

	case WNDMENU_NEWAPPID:
		ListSetAppIdSelected(hListWnd, szAppId);
		break;
	}

	return TRUE;
}

static void ListPostSysCmdSelected(HWND hListWnd, WPARAM wCommandParam)
{
	LVITEM lvi;

	lvi.mask = LVIF_PARAM | LVIF_INDENT;
	lvi.iItem = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
	lvi.iSubItem = 0;

	do
	{
		ListView_GetItem(hListWnd, &lvi);

		if(lvi.iIndent == 1)
		{
			LONG_PTR *task_item = (LONG_PTR *)lvi.lParam;

			switch(wCommandParam)
			{
			case SC_MINIMIZE:
				MinimizeTaskItem(task_item);
				break;

			case SC_MAXIMIZE:
				if(CanMaximizeTaskItem(task_item))
					PostMessage(GetTaskItemWnd(task_item), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
				break;

			case SC_RESTORE:
				if(CanRestoreTaskItem(task_item))
					PostMessage(GetTaskItemWnd(task_item), WM_SYSCOMMAND, SC_RESTORE, 0);
				break;

			case SC_CLOSE:
				CloseTaskItem(task_item, FALSE);
				break;
			}
		}

		lvi.iItem = ListView_GetNextItem(hListWnd, lvi.iItem, LVNI_SELECTED);
	}
	while(lvi.iItem != -1);
}

static HWND *ListGetHwndArraySelected(HWND hListWnd, UINT *puSelectedWndCount)
{
	UINT uSelectedWndCount;
	HWND *phWndArray;
	LVITEM lvi;
	int index;

	uSelectedWndCount = 0;

	lvi.mask = LVIF_INDENT;
	lvi.iItem = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
	lvi.iSubItem = 0;

	while(lvi.iItem != -1)
	{
		ListView_GetItem(hListWnd, &lvi);
		if(lvi.iIndent == 1)
			uSelectedWndCount++;

		lvi.iItem = ListView_GetNextItem(hListWnd, lvi.iItem, LVNI_SELECTED);
	}

	*puSelectedWndCount = uSelectedWndCount;

	if(uSelectedWndCount == 0)
		return NULL;

	phWndArray = (HWND *)HeapAlloc(GetProcessHeap(), 0, uSelectedWndCount*sizeof(HWND));
	if(!phWndArray)
		return NULL;

	index = 0;

	lvi.mask = LVIF_PARAM | LVIF_INDENT;
	lvi.iItem = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
	lvi.iSubItem = 0;

	do
	{
		ListView_GetItem(hListWnd, &lvi);
		if(lvi.iIndent == 1)
			phWndArray[index++] = GetTaskItemWnd((LONG_PTR *)lvi.lParam);

		lvi.iItem = ListView_GetNextItem(hListWnd, lvi.iItem, LVNI_SELECTED);
	}
	while(lvi.iItem != -1);

	return phWndArray;
}

static void ListSetAppIdSelected(HWND hListWnd, WCHAR *pAppId)
{
	LVITEM lvi;

	lvi.mask = LVIF_PARAM | LVIF_INDENT;
	lvi.iItem = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
	lvi.iSubItem = 0;

	do
	{
		ListView_GetItem(hListWnd, &lvi);

		if(lvi.iIndent == 1)
			WndSetAppId(GetTaskItemWnd((LONG_PTR *)lvi.lParam), pAppId);

		lvi.iItem = ListView_GetNextItem(hListWnd, lvi.iItem, LVNI_SELECTED);
	}
	while(lvi.iItem != -1);
}

static BOOL ListCopySelectedToClipboard(HWND hListWnd)
{
	UINT uSelectedCount;
	BOOL bAllIndented;
	HGLOBAL hText, hRealloc;
	WCHAR *pText, *p;
	size_t nTextSize;
	LVITEM lvi;

	uSelectedCount = ListView_GetSelectedCount(hListWnd);
	if(uSelectedCount == 0)
		return FALSE;

	hText = GlobalAlloc(GMEM_MOVEABLE, (uSelectedCount*(1 + (MAX_PATH - 1) + 2) + 1)*sizeof(WCHAR)); // count*(tab+max_len+newline)+null_term
	if(!hText)
		return FALSE;

	// Check whether all items are indented (in such case, no need to copy them with a tab)
	bAllIndented = TRUE; // Assume that all items are indented unless we prove otherwise

	lvi.mask = LVIF_INDENT;
	lvi.iItem = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
	lvi.iSubItem = 0;

	do
	{
		ListView_GetItem(hListWnd, &lvi);

		if(lvi.iIndent == 0)
		{
			bAllIndented = FALSE;
			break;
		}

		lvi.iItem = ListView_GetNextItem(hListWnd, lvi.iItem, LVNI_SELECTED);
	}
	while(lvi.iItem != -1);

	// Make the text
	pText = (WCHAR *)GlobalLock(hText);
	p = pText;

	lvi.iItem = ListView_GetNextItem(hListWnd, -1, LVNI_SELECTED);
	lvi.iSubItem = 0;

	do
	{
		if(!bAllIndented)
		{
			lvi.mask = LVIF_INDENT;

			ListView_GetItem(hListWnd, &lvi);

			if(lvi.iIndent == 1)
			{
				*p++ = L'\t';
			}
		}

		lvi.mask = LVIF_TEXT;
		lvi.pszText = p;
		lvi.cchTextMax = MAX_PATH;

		ListView_GetItem(hListWnd, &lvi);

		p += lstrlen(p);
		*p++ = L'\r';
		*p++ = L'\n';

		lvi.iItem = ListView_GetNextItem(hListWnd, lvi.iItem, LVNI_SELECTED);
	}
	while(lvi.iItem != -1);

	nTextSize = p - pText;
	*p = L'\0';

	GlobalUnlock(hText);

	hRealloc = GlobalReAlloc(hText, (nTextSize + 1)*sizeof(WCHAR), 0);
	if(!hRealloc)
	{
		GlobalFree(hText);
		return FALSE;
	}

	hText = hRealloc;

	// Copy it to clipboard
	if(OpenClipboard(GetParent(hListWnd)))
	{
		if(EmptyClipboard())
		{
			if(SetClipboardData(CF_UNICODETEXT, hText))
			{
				CloseClipboard();
				return TRUE;
			}
		}

		CloseClipboard();
	}

	GlobalFree(hText);
	return FALSE;
}

// List functions

static void ListView_MoveItem(HWND hListWnd, int iIndexFrom, int iIndexTo)
{
	LVITEM lvi;
	WCHAR szBuffer[MAX_PATH];

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM | LVIF_INDENT;
	lvi.iItem = iIndexFrom;
	lvi.iSubItem = 0;
	lvi.stateMask = (UINT)-1;
	lvi.pszText = szBuffer;
	lvi.cchTextMax = MAX_PATH;
	ListView_GetItem(hListWnd, &lvi);

	ListView_DeleteItem(hListWnd, iIndexFrom);

	lvi.iItem = iIndexTo;
	ListView_InsertItem(hListWnd, &lvi);
}

static BOOL ImageListRemoveAndListViewUpdate(HIMAGELIST hImageList, int image_index, HWND hListWnd)
{
	LVITEM lvi;

	if(!ImageList_Remove(hImageList, image_index))
		return FALSE;

	lvi.mask = LVIF_IMAGE;
	lvi.iItem = 0;
	lvi.iSubItem = 0;

	while(ListView_GetItem(hListWnd, &lvi))
	{
		if(lvi.iImage > image_index)
		{
			lvi.iImage--;
			ListView_SetItem(hListWnd, &lvi);
		}

		lvi.iItem++;
	}

	return TRUE;
}

static void BeforeTaskbarRefresh()
{
	HWND hListWnd;

	hListWnd = hInspListWnd;

	if(!nRefreshing)
	{
		ListView_GetOrigin(hListWnd, &ptRefreshOrigin);
		SendMessage(hListWnd, WM_SETREDRAW, FALSE, 0);

		EnableWindow(hListWnd, FALSE);
	}

	nRefreshing++;
}

static void AfterTaskbarRefresh()
{
	HWND hListWnd = hInspListWnd;

	nRefreshing--;

	if(!nRefreshing)
	{
		EnableWindow(hListWnd, TRUE);
		SetFocus(hListWnd);

		POINT pt;
		ListView_GetOrigin(hListWnd, &pt);
		if(pt.x != ptRefreshOrigin.x || pt.y != ptRefreshOrigin.y)
		{
			ListView_Scroll(hListWnd, ptRefreshOrigin.x - pt.x, ptRefreshOrigin.y - pt.y);
		}

		SendMessage(hListWnd, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hListWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}

BOOL InspectorBeforeTaskbarRefresh()
{
	if(hInspWnd)
	{
		SendMessageBlock(hInspWnd, UWM_INSP_CALLFUNC, (WPARAM)BeforeTaskbarRefresh, 0);
		return TRUE;
	}

	return FALSE;
}

BOOL InspectorAfterTaskbarRefresh()
{
	if(hInspWnd)
	{
		SendMessageBlock(hInspWnd, UWM_INSP_CALLFUNC, (WPARAM)AfterTaskbarRefresh, 0);
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// Hooks

void InspectorAfterDPA_Create(int cItemGrow, HDPA hRet)
{
	if(!bHooksEnabled)
		return;

	hook_proc_call_counter++;

	if(bNewButton)
	{
		hNewButtonDpa = hRet;
		bNewButton = FALSE;
	}

	hook_proc_call_counter--;
	if(hook_proc_call_counter == 0 && bHookCloseInspector)
	{
		SendMessage(hInspWnd, UWM_INSP_FINALLYCLOSE, 0, 0);
		bHookCloseInspector = FALSE;
	}
}

void InspectorAfterDPA_InsertPtr(HDPA pdpa, int index, void *p, void *pRet)
{
	if(!bHooksEnabled)
		return;

	hook_proc_call_counter++;

	DPA_HOOK_PARAM dpa_hook_param;
	HDPA hButtonGroupsDpa;
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	LONG_PTR *task_group;
	int i;

	hButtonGroupsDpa = *EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpInspMMTaskListLongPtr);
	if(hButtonGroupsDpa)
	{
		if(pdpa == hButtonGroupsDpa)
		{
			dpa_hook_param.pdpa = pdpa;
			dpa_hook_param.index = index;

			if(index == INT_MAX)
			{
				SendMessage(hInspWnd, UWM_INSP_CALLFUNC_PARAM,
					(WPARAM)DPA_InsertPtrHook_NewGroup, (LPARAM)&dpa_hook_param);

				plp = (LONG_PTR *)hButtonGroupsDpa;

				button_groups_count = (int)plp[0];
				button_groups = (LONG_PTR **)plp[1];

				task_group = (LONG_PTR *)button_groups[button_groups_count - 1][DO2(3, 4)];

				if(task_group[4] && (int)((LONG_PTR *)task_group[4])[0] > 0) // Not pinned
					bNewButton = TRUE;
			}
			else
			{
				SendMessage(hInspWnd, UWM_INSP_CALLFUNC_PARAM,
					(WPARAM)DPA_InsertPtrHook_Group, (LPARAM)&dpa_hook_param);
			}
		}
		else if(pdpa == hNewButtonDpa)
		{
			dpa_hook_param.pdpa = pdpa;
			dpa_hook_param.index = index;

			SendMessage(hInspWnd, UWM_INSP_CALLFUNC_PARAM,
				(WPARAM)DPA_InsertPtrHook_ButtonOfNewGroup, (LPARAM)&dpa_hook_param);

			hNewButtonDpa = NULL;
		}
		else
		{
			plp = (LONG_PTR *)hButtonGroupsDpa;

			button_groups_count = (int)plp[0];
			button_groups = (LONG_PTR **)plp[1];

			for(i = 0; i < button_groups_count; i++)
			{
				plp = (LONG_PTR *)button_groups[i][DO2(5, 7)];
				if(pdpa == (HDPA)plp)
				{
					dpa_hook_param.pdpa = pdpa;
					dpa_hook_param.index = index;
					dpa_hook_param.extra = i;

					SendMessage(hInspWnd, UWM_INSP_CALLFUNC_PARAM,
						(WPARAM)DPA_InsertPtrHook_Button, (LPARAM)&dpa_hook_param);

					break;
				}
			}
		}
	}

	hook_proc_call_counter--;
	if(hook_proc_call_counter == 0 && bHookCloseInspector)
	{
		SendMessage(hInspWnd, UWM_INSP_FINALLYCLOSE, 0, 0);
		bHookCloseInspector = FALSE;
	}
}

void InspectorBeforeDPA_DeletePtr(HDPA pdpa, int index)
{
	if(!bHooksEnabled)
		return;

	hook_proc_call_counter++;

	DPA_HOOK_PARAM dpa_hook_param;
	HDPA hButtonGroupsDpa;
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	int i;

	hButtonGroupsDpa = *EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpInspMMTaskListLongPtr);
	if(hButtonGroupsDpa)
	{
		if(pdpa == hButtonGroupsDpa)
		{
			dpa_hook_param.pdpa = pdpa;
			dpa_hook_param.index = index;

			SendMessage(hInspWnd, UWM_INSP_CALLFUNC_PARAM,
				(WPARAM)DPA_DeletePtrHook_Group, (LPARAM)&dpa_hook_param);
		}
		else
		{
			plp = (LONG_PTR *)hButtonGroupsDpa;

			button_groups_count = (int)plp[0];
			button_groups = (LONG_PTR **)plp[1];

			for(i = 0; i < button_groups_count; i++)
			{
				plp = (LONG_PTR *)button_groups[i][DO2(5, 7)];
				if(pdpa == (HDPA)plp)
				{
					dpa_hook_param.pdpa = pdpa;
					dpa_hook_param.index = index;
					dpa_hook_param.extra = i;

					SendMessage(hInspWnd, UWM_INSP_CALLFUNC_PARAM,
						(WPARAM)DPA_DeletePtrHook_Button, (LPARAM)&dpa_hook_param);

					break;
				}
			}
		}
	}

	hook_proc_call_counter--;
	if(hook_proc_call_counter == 0 && bHookCloseInspector)
	{
		SendMessage(hInspWnd, UWM_INSP_FINALLYCLOSE, 0, 0);
		bHookCloseInspector = FALSE;
	}
}

static void DPA_InsertPtrHook_NewGroup(DPA_HOOK_PARAM *p_dpa_hook_param)
{
	HDPA pdpa;
	HWND hListWnd;
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;

	pdpa = p_dpa_hook_param->pdpa;
	hListWnd = hInspListWnd;

	if(!nRefreshing)
		SendMessage(hListWnd, WM_SETREDRAW, FALSE, 0);

	plp = (LONG_PTR *)pdpa;
	button_groups_count = (int)plp[0];
	button_groups = (LONG_PTR **)plp[1];

	AddButtonGroupToList(hListWnd, ListView_GetItemCount(hListWnd), button_groups[button_groups_count - 1]);

	if(!nRefreshing)
	{
		SendMessage(hListWnd, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hListWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}

static void DPA_InsertPtrHook_Group(DPA_HOOK_PARAM *p_dpa_hook_param)
{
	HDPA pdpa;
	int button_group_index;
	HWND hListWnd;
	HIMAGELIST hImageList;
	LVITEM lvi;
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	int button_group_type;
	int buttons_count;
	LONG_PTR **buttons;
	int i;

	pdpa = p_dpa_hook_param->pdpa;
	button_group_index = p_dpa_hook_param->index;
	hListWnd = hInspListWnd;

	if(!nRefreshing)
		SendMessage(hListWnd, WM_SETREDRAW, FALSE, 0);

	plp = (LONG_PTR *)pdpa;
	button_groups_count = (int)plp[0];
	button_groups = (LONG_PTR **)plp[1];

	if(button_group_index > 0)
	{
		lvi.mask = LVIF_INDENT;
		lvi.iItem = 0;
		lvi.iSubItem = 0;

		for(i = 0; i < button_group_index - 1; i++)
		{
			lvi.iItem++;
			ListView_GetItem(hListWnd, &lvi);

			while(lvi.iIndent == 1)
			{
				lvi.iItem++;
				ListView_GetItem(hListWnd, &lvi);
			}
		}

		do {
			lvi.iItem++;
		} while(ListView_GetItem(hListWnd, &lvi) && lvi.iIndent == 1);
	}
	else
		lvi.iItem = 0;

	AddButtonGroupToList(hListWnd, lvi.iItem, button_groups[button_group_index]);

	button_group_type = (int)button_groups[button_group_index][DO2(6, 8)];
	if(button_group_type == 1 || button_group_type == 3)
	{
		plp = (LONG_PTR *)button_groups[button_group_index][DO2(5, 7)];
		buttons_count = (int)plp[0];
		buttons = (LONG_PTR **)plp[1];

		hImageList = ListView_GetImageList(hListWnd, LVSIL_SMALL);

		for(i = 0; i < buttons_count; i++)
			AddButtonToList(hListWnd, lvi.iItem + 1 + i, hImageList, buttons[i]);
	}

	if(!nRefreshing)
	{
		SendMessage(hListWnd, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hListWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}

static void DPA_InsertPtrHook_ButtonOfNewGroup(DPA_HOOK_PARAM *p_dpa_hook_param)
{
	HDPA pdpa;
	HWND hListWnd;
	HIMAGELIST hImageList;
	LONG_PTR *plp;
	int buttons_count;
	LONG_PTR **buttons;

	pdpa = p_dpa_hook_param->pdpa;
	hListWnd = hInspListWnd;

	if(!nRefreshing)
		SendMessage(hListWnd, WM_SETREDRAW, FALSE, 0);

	plp = (LONG_PTR *)pdpa;

	buttons_count = (int)plp[0];
	buttons = (LONG_PTR **)plp[1];

	hImageList = ListView_GetImageList(hListWnd, LVSIL_SMALL);

	AddButtonToList(hListWnd, ListView_GetItemCount(hListWnd), hImageList, buttons[0]);

	if(!nRefreshing)
	{
		SendMessage(hListWnd, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hListWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}

static void DPA_InsertPtrHook_Button(DPA_HOOK_PARAM *p_dpa_hook_param)
{
	HDPA pdpa;
	int button_index;
	int button_group_index;
	HWND hListWnd;
	HIMAGELIST hImageList;
	LVITEM lvi;
	LONG_PTR *plp;
	int buttons_count;
	LONG_PTR **buttons;
	int i;

	pdpa = p_dpa_hook_param->pdpa;
	button_index = p_dpa_hook_param->index;
	button_group_index = p_dpa_hook_param->extra;
	hListWnd = hInspListWnd;

	if(!nRefreshing)
		SendMessage(hListWnd, WM_SETREDRAW, FALSE, 0);

	plp = (LONG_PTR *)pdpa;
	buttons_count = (int)plp[0];
	buttons = (LONG_PTR **)plp[1];

	lvi.mask = LVIF_INDENT;
	lvi.iItem = 0;
	lvi.iSubItem = 0;

	for(i = 0; i < button_group_index; i++)
	{
		lvi.iItem++;
		ListView_GetItem(hListWnd, &lvi);

		while(lvi.iIndent == 1)
		{
			lvi.iItem++;
			ListView_GetItem(hListWnd, &lvi);
		}
	}

	lvi.iItem++;

	hImageList = ListView_GetImageList(hListWnd, LVSIL_SMALL);

	if(button_index != INT_MAX)
		AddButtonToList(hListWnd, lvi.iItem + button_index, hImageList, buttons[button_index]);
	else
		AddButtonToList(hListWnd, lvi.iItem + buttons_count - 1, hImageList, buttons[buttons_count - 1]);

	if(!nRefreshing)
	{
		SendMessage(hListWnd, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hListWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}

static void DPA_DeletePtrHook_Group(DPA_HOOK_PARAM *p_dpa_hook_param)
{
	int button_group_index;
	HWND hListWnd;
	HIMAGELIST hImageList;
	LVITEM lvi;
	int i;

	button_group_index = p_dpa_hook_param->index;
	hListWnd = hInspListWnd;

	if(!nRefreshing)
		SendMessage(hListWnd, WM_SETREDRAW, FALSE, 0);

	lvi.mask = LVIF_IMAGE | LVIF_INDENT;
	lvi.iItem = 0;
	lvi.iSubItem = 0;

	for(i = 0; i < button_group_index; i++)
	{
		lvi.iItem++;
		ListView_GetItem(hListWnd, &lvi);

		while(lvi.iIndent == 1)
		{
			lvi.iItem++;
			ListView_GetItem(hListWnd, &lvi);
		}
	}

	hImageList = ListView_GetImageList(hListWnd, LVSIL_SMALL);

	do
	{
		ListView_DeleteItem(hListWnd, lvi.iItem);
		if(lvi.iImage > 1)
			ImageListRemoveAndListViewUpdate(hImageList, lvi.iImage, hListWnd);
	}
	while(ListView_GetItem(hListWnd, &lvi) && lvi.iIndent == 1);

	if(!nRefreshing)
	{
		SendMessage(hListWnd, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hListWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}

static void DPA_DeletePtrHook_Button(DPA_HOOK_PARAM *p_dpa_hook_param)
{
	int button_index;
	int button_group_index;
	HWND hListWnd;
	HIMAGELIST hImageList;
	LVITEM lvi;
	int i;

	button_index = p_dpa_hook_param->index;
	button_group_index = p_dpa_hook_param->extra;
	hListWnd = hInspListWnd;

	if(!nRefreshing)
		SendMessage(hListWnd, WM_SETREDRAW, FALSE, 0);

	lvi.mask = LVIF_INDENT;
	lvi.iItem = 0;
	lvi.iSubItem = 0;

	for(i = 0; i < button_group_index; i++)
	{
		lvi.iItem++;
		ListView_GetItem(hListWnd, &lvi);

		while(lvi.iIndent == 1)
		{
			lvi.iItem++;
			ListView_GetItem(hListWnd, &lvi);
		}
	}

	lvi.mask = LVIF_IMAGE;
	lvi.iItem += 1 + button_index;
	ListView_GetItem(hListWnd, &lvi);

	ListView_DeleteItem(hListWnd, lvi.iItem);
	if(lvi.iImage > 1)
	{
		hImageList = ListView_GetImageList(hListWnd, LVSIL_SMALL);
		ImageListRemoveAndListViewUpdate(hImageList, lvi.iImage, hListWnd);
	}

	if(!nRefreshing)
	{
		SendMessage(hListWnd, WM_SETREDRAW, TRUE, 0);
		RedrawWindow(hListWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
	}
}
