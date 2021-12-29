#include "stdafx.h"
#include "taskbar_refresh.h"
#include "explorer_vars.h"
#include "functions.h"
#include "taskbar_inspector.h"
#include "com_func_hook.h"

typedef struct _refresh_hardcore_item {
	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR *button_group;
	HWND hButtonWnd;
	BOOL bIsImmersive; // Windows 10
	LONG_PTR lpImmersiveApp; // Windows 10
} REFRESH_HARDCORE_ITEM;

typedef struct _refresh_hardcore_param {
	int i;
	int item_count;
	UINT uShellHookMsg;
	REFRESH_HARDCORE_ITEM refresh_items[0];
} REFRESH_HARDCORE_PARAM;

static BOOL RefreshTaskbarHardcore_Begin();
static REFRESH_HARDCORE_PARAM *PopulateRefreshHardcoreParam();
static void TaskListItemCount(LONG_PTR lpMMTaskListLongPtr, int *p_button_wnd_count, int *p_pinned_item_count);
static int TaskListItemPopulate(LONG_PTR lpMMTaskListLongPtr, REFRESH_HARDCORE_ITEM *p_refresh_items);
static void SyncWndCreated();
static void SyncWndDestroyed();

// superglobals
extern HINSTANCE hDllInst;
extern HWND hTaskbarWnd, hTaskSwWnd, hTaskListWnd;
extern LONG_PTR lpTaskbarLongPtr, lpTaskSwLongPtr, lpTaskListLongPtr;

static HWND hSyncWnd;
static volatile BOOL bRefreshHardcoreRunning;
static BOOL bRefreshAgain;
static REFRESH_HARDCORE_PARAM *gp_refresh_param;
static LONG_PTR *pMainTaskListAnimationManager;
static ANIMATION_MANAGER_ITEM *lpSeconadryTaskListAnimationManagers;
static volatile HANDLE hDoneEvent;

BOOL RefreshTaskbarHardcore_Init()
{
	WNDCLASS wc;
	HWND hWnd;

	ZeroMemory(&wc, sizeof(WNDCLASS));

	wc.lpfnWndProc = (WNDPROC)DefWindowProc;
	wc.hInstance = hDllInst;
	wc.lpszClassName = L"7ttRefreshSyncWnd";

	if(!RegisterClass(&wc))
		return FALSE;

	hWnd = CreateWindow(L"7ttRefreshSyncWnd", NULL, WS_POPUP | WS_DISABLED, 0, 0, 0, 0, NULL, NULL, hDllInst, 0);
	if(!hWnd)
	{
		UnregisterClass(L"7ttRefreshSyncWnd", hDllInst);
		return FALSE;
	}

	WndSetAppId(hWnd, L"7ttRefreshSyncWnd");

	hSyncWnd = hWnd;

	return TRUE;
}

void RefreshTaskbarHardcore_Exit()
{
	DestroyWindow(hSyncWnd);
	UnregisterClass(L"7ttRefreshSyncWnd", hDllInst);
}

void RefreshTaskbarHardcore_WaitTillDone()
{
	if(bRefreshHardcoreRunning)
	{
		hDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(bRefreshHardcoreRunning)
			WaitForSingleObject(hDoneEvent, INFINITE);

		CloseHandle(hDoneEvent);
		hDoneEvent = NULL;
	}
}

BOOL RefreshTaskbarHardcore()
{
	if(bRefreshHardcoreRunning)
	{
		bRefreshAgain = TRUE;
		return TRUE;
	}

	bRefreshHardcoreRunning = TRUE;

	return RefreshTaskbarHardcore_Begin();
}

static BOOL RefreshTaskbarHardcore_Begin()
{
	gp_refresh_param = PopulateRefreshHardcoreParam();
	if(!gp_refresh_param)
	{
		bRefreshHardcoreRunning = FALSE;
		return FALSE;
	}

	DisableTaskbarsAnimation(&pMainTaskListAnimationManager, &lpSeconadryTaskListAnimationManagers);
	EnableTaskbars(FALSE);

	InspectorBeforeTaskbarRefresh();

	REFRESH_HARDCORE_ITEM *p_refresh_items = gp_refresh_param->refresh_items;

	// Hide in reverse order, attempts to fix an issue with pinned item reordering.
	// Example: re-enabling grouping, before:
	// [notepad] [pinned explorer] [explorer]
	// after:
	// [pinned explorer group] [notepad]
	for(int i = gp_refresh_param->item_count - 1; i >= 0; i--)
	{
		HWND hWnd = p_refresh_items[i].hButtonWnd;
		if(hWnd)
		{
			if(p_refresh_items[i].bIsImmersive)
			{
				BOOL bOldShellManagedWindowAsNormalWindow = GetProp(hWnd, L"Microsoft.Windows.ShellManagedWindowAsNormalWindow") != NULL;

				ComFuncSetTaskItemGetWindowReturnNull(TRUE);
				SetProp(hWnd, L"Microsoft.Windows.ShellManagedWindowAsNormalWindow", (HANDLE)1);

				// CTaskBand::_HandleOtherWindowDestroyed
				SendMessage(hTaskSwWnd, 0x444, (WPARAM)hWnd, 0);

				p_refresh_items[i].lpImmersiveApp = ComFuncGetSavedTaskItemGetWindow();

				ComFuncSetTaskItemGetWindowReturnNull(FALSE);
				if(!bOldShellManagedWindowAsNormalWindow)
					RemoveProp(hWnd, L"Microsoft.Windows.ShellManagedWindowAsNormalWindow");
			}
			else
			{
				SendMessage(hTaskSwWnd, gp_refresh_param->uShellHookMsg, HSHELL_WINDOWDESTROYED, (LPARAM)hWnd);
			}
		}
	}

	ShowWindow(hSyncWnd, SW_SHOWNA);

	return TRUE;
}

static REFRESH_HARDCORE_PARAM *PopulateRefreshHardcoreParam()
{
	BOOL bPopulateSecondaryTaskLists;
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;
	int button_wnd_count, pinned_item_count;
	int item_count;
	REFRESH_HARDCORE_PARAM *p_refresh_param;
	REFRESH_HARDCORE_ITEM *p_refresh_items;

	button_wnd_count = 0;
	pinned_item_count = 0;

	TaskListItemCount(lpTaskListLongPtr, &button_wnd_count, &pinned_item_count);

	if(TaskbarGetPreference(lpTaskListLongPtr) & 0x400)
	{
		bPopulateSecondaryTaskLists = TRUE;

		lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
		while(lpSecondaryTaskListLongPtr)
		{
			TaskListItemCount(lpSecondaryTaskListLongPtr, &button_wnd_count, &pinned_item_count);
			lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
		}
	}
	else
		bPopulateSecondaryTaskLists = FALSE;

	if(button_wnd_count == 0)
		return NULL;

	item_count = button_wnd_count + pinned_item_count;

	p_refresh_param = (REFRESH_HARDCORE_PARAM *)HeapAlloc(GetProcessHeap(), 0,
		offsetof(REFRESH_HARDCORE_PARAM, refresh_items) + sizeof(REFRESH_HARDCORE_ITEM) * item_count);
	if(!p_refresh_param)
		return NULL;

	p_refresh_param->i = 0;
	p_refresh_param->item_count = item_count;
	p_refresh_param->uShellHookMsg = RegisterWindowMessage(L"SHELLHOOK");

	p_refresh_items = p_refresh_param->refresh_items;

	p_refresh_items += TaskListItemPopulate(lpTaskListLongPtr, p_refresh_items);

	if(bPopulateSecondaryTaskLists)
	{
		lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
		while(lpSecondaryTaskListLongPtr)
		{
			p_refresh_items += TaskListItemPopulate(lpSecondaryTaskListLongPtr, p_refresh_items);
			lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
		}
	}

	return p_refresh_param;
}

static void TaskListItemCount(LONG_PTR lpMMTaskListLongPtr, int *p_button_wnd_count, int *p_pinned_item_count)
{
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	int button_group_type;
	int button_wnd_count, pinned_item_count;
	int i;

	plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
	if(plp)
	{
		button_wnd_count = 0;
		pinned_item_count = 0;

		button_groups_count = (int)plp[0];
		button_groups = (LONG_PTR **)plp[1];

		for(i = 0; i < button_groups_count; i++)
		{
			button_group_type = (int)button_groups[i][DO2(6, 8)];
			if(button_group_type == 1 || button_group_type == 3)
			{
				plp = (LONG_PTR *)button_groups[i][DO2(5, 7)];
				button_wnd_count += (int)plp[0];
			}
			else if(button_group_type == 2)
				pinned_item_count++;
		}

		*p_button_wnd_count += button_wnd_count;
		*p_pinned_item_count += pinned_item_count;
	}
}

static int TaskListItemPopulate(LONG_PTR lpMMTaskListLongPtr, REFRESH_HARDCORE_ITEM *p_refresh_items)
{
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	int button_group_type;
	int buttons_count;
	LONG_PTR **buttons;
	int populated;
	int i, j;

	plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
	if(!plp)
		return 0;

	populated = 0;

	button_groups_count = (int)plp[0];
	button_groups = (LONG_PTR **)plp[1];

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
				p_refresh_items->lpMMTaskListLongPtr = lpMMTaskListLongPtr;
				p_refresh_items->button_group = button_groups[i];
				p_refresh_items->hButtonWnd = GetButtonWnd(buttons[j]);

				if(nWinVersion >= WIN_VERSION_10_T1)
				{
					LONG_PTR *task_item = (LONG_PTR *)buttons[j][DO2(3, 4)];

					LONG_PTR this_ptr = (LONG_PTR)task_item;
					LONG_PTR *plp = *(LONG_PTR **)this_ptr;

					// CWindowTaskItem::IsImmersive(this)
					p_refresh_items->bIsImmersive = FUNC_CWindowTaskItem_IsImmersive(plp)(this_ptr) != 0;
				}
				else
					p_refresh_items->bIsImmersive = FALSE;

				p_refresh_items++;
				populated++;
			}
		}
		else if(button_group_type == 2)
		{
			p_refresh_items->lpMMTaskListLongPtr = lpMMTaskListLongPtr;
			p_refresh_items->button_group = button_groups[i];
			p_refresh_items->hButtonWnd = NULL;

			p_refresh_items++;
			populated++;
		}
	}

	return populated;
}

static void SyncWndCreated()
{
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	REFRESH_HARDCORE_ITEM *p_refresh_items;
	LONG_PTR lpWndLong;
	HWND hWnd;
	int i;

	ShowWindow(hSyncWnd, SW_HIDE);

	p_refresh_items = gp_refresh_param->refresh_items;

	for(i = gp_refresh_param->i; i < gp_refresh_param->item_count; i = (++gp_refresh_param->i))
	{
		plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(p_refresh_items[i].lpMMTaskListLongPtr);
		if(plp)
		{
			button_groups_count = (int)plp[0];
			button_groups = (LONG_PTR **)plp[1];

			if(button_groups_count > 1 && (int)button_groups[0][DO2(6, 8)] == 2 &&
				p_refresh_items[i].button_group == button_groups[0])
			{
				TaskbarMoveGroup(p_refresh_items[i].lpMMTaskListLongPtr, 0, button_groups_count - 1);
			}
		}

		hWnd = p_refresh_items[i].hButtonWnd;
		if(!hWnd)
			continue;

		if(p_refresh_items[i].bIsImmersive && p_refresh_items[i].lpImmersiveApp)
		{
			LONG_PTR this_ptr = (LONG_PTR)(lpTaskSwLongPtr + DEF3264(0x30, 0x60));
			plp = *(LONG_PTR **)this_ptr;

			// CTaskBand::ApplicationChanged(this, immersive_application, probably_flags, hwnd_unused)
			FUNC_CTaskBand_ApplicationChanged(plp)(this_ptr, p_refresh_items[i].lpImmersiveApp, 0x02, hWnd);
		}
		else
		{
			// Fixes SevenDex Dexpot feature
			lpWndLong = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
			if(lpWndLong & WS_EX_TOOLWINDOW)
				SetWindowLongPtr(hWnd, GWL_EXSTYLE, lpWndLong & ~WS_EX_TOOLWINDOW);

			SendMessage(hTaskSwWnd, gp_refresh_param->uShellHookMsg, HSHELL_WINDOWCREATED, (LPARAM)hWnd);
		}

		gp_refresh_param->i++;
		if(gp_refresh_param->i < gp_refresh_param->item_count)
			ShowWindow(hSyncWnd, SW_SHOWNA);

		break;
	}
}

static void SyncWndDestroyed()
{
	if(bRefreshHardcoreRunning && gp_refresh_param->i == gp_refresh_param->item_count)
	{
		HeapFree(GetProcessHeap(), 0, gp_refresh_param);

		EnableTaskbars(TRUE);
		RestoreTaskbarsAnimation(pMainTaskListAnimationManager, lpSeconadryTaskListAnimationManagers);

		InspectorAfterTaskbarRefresh();

		if(bRefreshAgain)
		{
			bRefreshAgain = FALSE;
			RefreshTaskbarHardcore_Begin();
		}
		else
		{
			bRefreshHardcoreRunning = FALSE;
			if(hDoneEvent)
				SetEvent(hDoneEvent);
		}
	}
}

BOOL RefreshTaskbarHardcore_ButtonCreating(HWND hCreatingWnd)
{
	if(bRefreshHardcoreRunning && hCreatingWnd != hSyncWnd)
	{
		REFRESH_HARDCORE_ITEM *p_refresh_items = gp_refresh_param->refresh_items;

		for(int i = gp_refresh_param->i; i < gp_refresh_param->item_count; i++)
		{
			HWND hWnd = p_refresh_items[i].hButtonWnd;
			if(hWnd && hCreatingWnd == hWnd)
				return FALSE; // not now
		}
	}

	return TRUE;
}

void RefreshTaskbarHardcore_ButtonCreated(HWND hCreatedWnd)
{
	if(bRefreshHardcoreRunning && hCreatedWnd == hSyncWnd)
	{
		SyncWndCreated();
	}
}

void RefreshTaskbarHardcore_ButtonDestroyed(HWND hDestroyedWnd)
{
	if(bRefreshHardcoreRunning && hDestroyedWnd == hSyncWnd)
	{
		SyncWndDestroyed();
	}
}
