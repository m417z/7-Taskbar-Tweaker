#include "stdafx.h"
#include "com_func_hook.h"
#include "explorer_vars.h"
#include "functions.h"
#include "options_def.h"
#include "options_ex.h"
#include "pointer_redirection.h"
#include "taskbar_refresh.h"
#include "appid_lists.h"
#include "taskbar_inspector.h"
#include "MinHook/MinHook.h"

// OLE drag and drop function definitions
typedef HRESULT __stdcall OLE_DRAG_ENTER_PROC
(IDropTarget *This, IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
typedef HRESULT __stdcall OLE_DRAG_OVER_PROC
(IDropTarget *This, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
typedef HRESULT __stdcall OLE_DROP_PROC
(IDropTarget *This, IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

// superglobals
extern int nOptions[OPTS_COUNT];
extern int nOptionsEx[OPTS_EX_COUNT];
extern DWORD dwTaskbarThreadId;
extern HWND hTaskbarWnd, hTaskSwWnd, hTaskListWnd, hThumbnailWnd;
extern LONG_PTR lpTaskbarLongPtr, lpTaskSwLongPtr, lpTaskListLongPtr, lpThumbnailLongPtr;
extern void **ppDrawThemeBackground;
extern MODULEINFO ExplorerModuleInfo;

// hooked functions
static void **ppOleDragEnter;
static void *pOleDragEnter;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prOleDragEnter);
static void **ppOleDragOver;
static void *pOleDragOver;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prOleDragOver);
static void **ppOleDrop;
static void *pOleDrop;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prOleDrop);
static void **ppGetUserPreferences;
static void *pGetUserPreferences;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prGetUserPreferences);
static void **ppIsHorizontal;
static void *pIsHorizontal;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prIsHorizontal);
static void **ppGetIconId;
static void *pGetIconId;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prGetIconId);
static void **ppSwitchTo;
static void *pSwitchTo;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prSwitchTo);
static void **ppGetIconSize;
static void *pGetIconSize;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prGetIconSize);
static void **ppCurrentVirtualDesktopChanged;
static void *pCurrentVirtualDesktopChanged;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prCurrentVirtualDesktopChanged);
static void **ppCurrentVirtualDesktopChangedAnimated;
static void *pCurrentVirtualDesktopChangedAnimated;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prCurrentVirtualDesktopChangedAnimated);
static void **ppTaskBandExec;
static void *pTaskBandExec;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prTaskBandExec);
static void **ppStartAnimation;
static void *pStartAnimation;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prStartAnimation);
static void **ppGetStuckPlace;
static void *pGetStuckPlace;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prGetStuckPlace);
static void **ppTaskListWndInitialize;
static void *pTaskListWndInitialize;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prTaskListWndInitialize);
static void **ppTaskCreated;
static void *pTaskCreated;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prTaskCreated);
static void **ppActivateTask;
static void *pActivateTask;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prActivateTask);
static void **ppTaskDestroyed;
static void *pTaskDestroyed;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prTaskDestroyed);
static void **ppTaskInclusionChanged;
static void *pTaskInclusionChanged;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prTaskInclusionChanged);
static void **ppGetButtonHeight;
static void *pGetButtonHeight;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prGetButtonHeight);
static void **ppDismissHoverUI;
static void *pDismissHoverUI;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prDismissHoverUI);
static void **ppShowDestinationMenu;
static void *pShowDestinationMenu;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prShowDestinationMenu);
static void **ppOnDestinationMenuDismissed;
static void *pOnDestinationMenuDismissed;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prOnDestinationMenuDismissed);
static void **ppDisplayUI;
static void *pDisplayUI;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prDisplayUI);
static void **ppGetThumbRectFromIndex;
static void *pGetThumbRectFromIndex;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prGetThumbRectFromIndex);
static void **ppThumbIndexFromPoint;
static void *pThumbIndexFromPoint;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prThumbIndexFromPoint);
static void **ppDestroyThumbnail;
static void *pDestroyThumbnail;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prDestroyThumbnail);
static void **ppDoesWindowMatch;
static void *pDoesWindowMatch;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prDoesWindowMatch);
static void **ppTaskItemSetWindow;
static void *pTaskItemSetWindow;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prTaskItemSetWindow);
static void **ppTaskItemGetWindow;
static void *pTaskItemGetWindow;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prTaskItemGetWindow);
static void **ppButtonGroupRemoveTaskItem;
static void *pButtonGroupRemoveTaskItem;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prButtonGroupRemoveTaskItem);
static void **ppGetIdealSpan;
static void *pGetIdealSpan;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prGetIdealSpan);
static void **ppSetLocation;
static void *pSetLocation;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prSetLocation);
static void **ppRender;
static void *pRender;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prRender);
static void **ppButtonGroupCanGlom;
static void *pButtonGroupCanGlom;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prButtonGroupCanGlom);
static void **ppButtonGroupHotTracking;
static void *pButtonGroupHotTracking;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prButtonGroupHotTracking);
static void **ppButtonGroupHotTrackOut;
static void *pButtonGroupHotTrackOut;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prButtonGroupHotTrackOut);
static void **ppButtonGroupStartItemAnimation;
static void *pButtonGroupStartItemAnimation;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prButtonGroupStartItemAnimation);
static void **ppButtonGroupHasItemAnimation;
static void *pButtonGroupHasItemAnimation;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prButtonGroupHasItemAnimation);
static void **ppShouldShowToolTip;
static void *pShouldShowToolTip;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prShouldShowToolTip);
static void **ppSecondaryGetUserPreferences;
static void *pSecondaryGetUserPreferences;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prSecondaryGetUserPreferences);
static void **ppSecondaryIsHorizontal;
static void *pSecondaryIsHorizontal;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prSecondaryIsHorizontal);

// hook vars
static BOOL bTaskGroupFunctionsHooked;
static BOOL bTaskItemFunctionsHooked;
static BOOL bTaskBtnGroupFunctionsHooked;
static BOOL bSecondaryTaskbarFunctionsHooked;
static volatile int nHookProcCallCounter;

// Temporary hooks
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prDrawThemeBackground);
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prTaskGroupRelease);
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prTaskItemRelease);

static void(*hackf1_SubclassSecondaryTaskListWindows)(LONG_PTR);
static LONG_PTR lpUntrackedDecombinedTaskListLongPtr;
static LONG_PTR *button_group_untracked_decombined;
static HWND hDragWithinGroupsWnd;
static BOOL bDragWithinGroupsDone;
static LONG_PTR lpRightDragDetachTaskListLongPtr;
static LONG_PTR *right_drag_detach_group;
static LONG_PTR lpRightDragAttachTaskListLongPtr;
static LONG_PTR *right_drag_attach_group;
static DWORD dwUserPrefSetBits, dwUserPrefRemoveBits;
static BOOL bThumbNoDismiss;
static HWND hCreatedThumb, hCreatedThumbParent;
static int decombine_without_labels_hack; // <= Windows 8.1.1: The magic happens in: CTaskBtnGroup::_DrawRegularButton; Windows 10: we check assembly
static int selective_combining_hack; // <= Windows 8.1.1: The magic happens in: CTaskListWnd::_CheckNeedScrollbars; Windows 10: we check assembly
static int nSwitchToOption;
static LONG_PTR lpSwitchToMMTaskListLongPtr;
static BOOL bInMouseMove;
static BOOL bInGetIdealSpan;
static BOOL bInHandleDelayInitStuff;
static BOOL bCustomDestinationMenuActionDone;
static LONG_PTR *last_active_task_item;
static int nThumbnailListReverseHack = -1;
static BOOL bTaskListCapturedAndActiveWndChanged;
static LONG_PTR lpCapturedTaskListLongPtr;
static LONG_PTR *button_group_active_after_capture;
static DWORD dwDragTickCount;
static LONG_PTR *task_group_virtual_desktop_released;
static LONG_PTR *task_item_virtual_desktop_released;
static int nDoesWindowMatchCalls;
static BOOL bHadCurrentVirtualDesktopChangedAnimated;
static BOOL bTaskItemGetWindowReturnNull;
static LONG_PTR lpTaskItemGetWindowSavedValue;

static BOOL HookFunctions();
static void UnhookFunctions();
static BOOL HookTaskGroupFunctions();
static void UnhookTaskGroupFunctions();
static BOOL HookTaskItemFunctions();
static void UnhookTaskItemFunctions();
static BOOL HookTaskBtnGroupFunctions();
static void UnhookTaskBtnGroupFunctions();
static BOOL HookSecondaryTaskbarFunctions(LONG_PTR lpSecondaryTaskListLongPtr);
static void UnhookSecondaryTaskbarFunctions();
static HRESULT __stdcall OleDragEnterHook(IDropTarget *This, IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
static HRESULT __stdcall OleDragOverHook(IDropTarget *This, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
static HRESULT __stdcall OleDropHook(IDropTarget *This, IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
static LONG_PTR __stdcall GetUserPreferencesHook(LONG_PTR var1, DWORD *pdwPreferences);
static LONG_PTR __stdcall IsHorizontalHook(LONG_PTR this_ptr);
static LONG_PTR __stdcall GetIconIdHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item, LONG_PTR var4, LONG_PTR var5);
static LONG_PTR __stdcall SwitchToHook(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR *task_item, BOOL bSwitchTo);
static LONG_PTR __stdcall SwitchToHook2(LONG_PTR this_ptr, LONG_PTR *task_item, BOOL bSwitchTo);
static void SwitchToExecOption(LONG_PTR *task_item, int nOption);
static LONG_PTR __stdcall GetIconSizeHook(LONG_PTR var1, LONG_PTR var2);
static LONG_PTR __stdcall GetIconSizeHook2(LONG_PTR var1, LONG_PTR var2, LONG_PTR var3);
static void GetIconSizeAndGetButtonHeightHack();
static LONG_PTR __stdcall CurrentVirtualDesktopChangedHook(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3);
static LONG_PTR __stdcall CurrentVirtualDesktopChangedAnimatedHook(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3);
static LONG_PTR __stdcall TaskBandExecHook(LONG_PTR this_ptr, GUID *pGuid, LONG_PTR var3, LONG_PTR var4, LONG_PTR var5, LONG_PTR var6);
static void HideAllTaskbarItems();
static LONG_PTR __stdcall StartAnimationHook(LONG_PTR var1, void *pObject, int nAnimation);
static int __stdcall GetStuckPlaceHook(LONG_PTR this_ptr);
static HRESULT __stdcall TaskListWndInitializeHook(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3);
static HRESULT __stdcall TaskListWndInitializeHook2(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3, LONG_PTR var4);
static HRESULT __stdcall TaskListWndInitializeHook3(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3, LONG_PTR var4, LONG_PTR var5);
static void OnTaskListWndInitialized(LONG_PTR lpSecondaryTaskListLongPtr);
static LONG_PTR __stdcall TaskCreatedHook(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3);
static LONG_PTR __stdcall ActivateTaskHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item);
static LONG_PTR __stdcall TaskDestroyedHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item);
static LONG_PTR __stdcall TaskInclusionChangedHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item);
static LONG_PTR __stdcall GetButtonHeightHook(LONG_PTR this_ptr, LONG_PTR var2);
static LONG_PTR __stdcall DismissHoverUIHook(LONG_PTR this_ptr, BOOL bHideWithoutAnimation);
static LONG_PTR __stdcall ShowDestinationMenuHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item, LONG_PTR var4, LONG_PTR var5);
static LONG_PTR __stdcall ShowJumpViewHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item, LONG_PTR var4);
static void JumpListExecOption(LONG_PTR lpMMTaskListLongPtr, HWND hMMTaskListWnd, LONG_PTR *task_group, LONG_PTR *task_item, int nOption);
static BOOL StealPinnedFlagIfPossible(LONG_PTR *task_group);
static LONG_PTR __stdcall OnDestinationMenuDismissedHook(LONG_PTR this_ptr);
static LONG_PTR __stdcall DisplayUIHook(LONG_PTR this_ptr, LONG_PTR *button_group, LONG_PTR var3, LONG_PTR var4, DWORD dwFlags);
static LONG_PTR __stdcall GetThumbRectFromIndexHook(LONG_PTR this_ptr, int thumb_index, LONG_PTR var3, RECT *prcResult);
static int __stdcall ThumbIndexFromPointHook(LONG_PTR this_ptr, POINT *ppt);
static LONG_PTR __stdcall DestroyThumbnailHook(LONG_PTR this_ptr, LONG_PTR var2);
static HRESULT __stdcall DoesWindowMatchHook(LONG_PTR *task_group, HWND hCompareWnd, ITEMIDLIST *pCompareItemIdList,
	WCHAR *pCompareAppId, int *pnMatch, LONG_PTR **p_task_item);
static BOOL FindTaskGroupPairInArrayWithSameAppId(LONG_PTR *task_group, LONG_PTR ***pp_task_group, LONG_PTR ***pp_task_group_other);
static int __stdcall TaskItemSetWindowHook(LONG_PTR *task_item, HWND hNewWnd);
static HWND __stdcall TaskItemGetWindowHook(LONG_PTR *task_item);
static LONG_PTR __stdcall ButtonGroupRemoveTaskItemHook(LONG_PTR *button_group, LONG_PTR *task_item);
static LONG_PTR __stdcall GetIdealSpanHook(LONG_PTR *button_group, LONG_PTR var2, LONG_PTR var3,
	LONG_PTR var4, LONG_PTR var5, LONG_PTR var6);
static LONG_PTR __stdcall SetLocationHook(LONG_PTR *button_group, LONG_PTR var2, LONG_PTR var3, RECT *prc);
static LONG_PTR __stdcall RenderHook(LONG_PTR *button_group, LONG_PTR var2, LONG_PTR var3, LONG_PTR var4, LONG_PTR var5);
static LONG_PTR __stdcall RenderHook2(LONG_PTR *button_group, LONG_PTR var2, LONG_PTR var3, LONG_PTR var4, LONG_PTR var5, LONG_PTR var6);
static BOOL __stdcall ButtonGroupCanGlomHook(LONG_PTR *button_group);
static void ButtonGroupSetPrefOnUpdate(LONG_PTR *button_group, BOOL bDecombineWithoutLabelsHack);
static HRESULT __stdcall DrawThemeBackgroundHook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect);
static LONG_PTR __stdcall ButtonGroupHotTrackingHook(LONG_PTR *button_group, BOOL bNewGroup, int button_index, BOOL bAnimation);
static LONG_PTR __stdcall ButtonGroupHotTrackOutHook(LONG_PTR *button_group, BOOL bAnimation);
static LONG_PTR __stdcall ButtonGroupStartItemAnimationHook(LONG_PTR *button_group, int nAnimationInfo, int nAnimationType);
static LONG_PTR __stdcall ButtonGroupHasItemAnimationHook(LONG_PTR *button_group, int nAnimationInfo, int nAnimationType, int *pnResult);
static void OnButtonGroupHotTracking(LONG_PTR *button_group, BOOL bNewGroup, int button_index);
static void OnButtonGroupHotTrackOut(LONG_PTR *button_group);
static BOOL __stdcall ShouldShowToolTipHook(LONG_PTR *button_group, LONG_PTR *task_item);
static LONG_PTR __stdcall SecondaryGetUserPreferencesHook(LONG_PTR var1, DWORD *pdwPreferences);
static LONG_PTR __stdcall SecondaryIsHorizontalHook(LONG_PTR this_ptr);
static DWORD ManipulateUserPreferences(DWORD dwPreferences, void *pReturnAddress);
static BOOL CheckCombineButtonGroup(LONG_PTR *button_group);
static BOOL ButtonGroupCombine(LONG_PTR *button_group, BOOL bCombine);
static void CombineUntrackedDecombined(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group);
static BOOL DragWithinGroup(LONG_PTR *button_group_tracked, int button_index_tracked);
static BOOL DragBetweenGroups(LONG_PTR *button_group_tracked);
static LONG_PTR *ButtonGroupNextToMousePos(LONG_PTR lpMMTaskListLongPtr, HWND hExcludeWnd);
static void ActiveButtonGroupChanged(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *old_button_group_active, LONG_PTR *new_button_group_active);
static void ActiveButtonGroupChangedNonCaptured(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *old_button_group_active, LONG_PTR *new_button_group_active);
static void ButtonGroupActivated(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group);
static void ButtonGroupDeactivated(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group);
static void ButtonGroupDeactivatedNonCaptured(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group);

// Virtual Desktop switching order fix
static void OnButtonGroupInserted(LONG_PTR lpMMTaskListLongPtr, int nButtonGroupIndex);
static ULONG __stdcall TaskGroupReleaseHook(LONG_PTR this_ptr);
static ULONG __stdcall TaskItemReleaseHook(LONG_PTR this_ptr);

// Hooks
static BOOL CreateEnableHook(void **ppTarget, void *const pDetour, void **ppOriginal, POINTER_REDIRECTION *ppr);
static BOOL DisableHook(void **ppTarget, POINTER_REDIRECTION *ppr);

static BOOL MMMoveNearMatching(LONG_PTR lpMMTaskListLongPtr, HWND hButtonWnd);
static BOOL CustomGetLabelAppidListValue(WCHAR *pAppId, int *pnListValue);

BOOL ComFuncHook_Init(void(*hackf1)(LONG_PTR))
{
	// Kinda hack: func to subclasses a secondary taskbar (multimonitor environment)
	hackf1_SubclassSecondaryTaskListWindows = hackf1;

	if(!HookFunctions())
	{
		UnhookFunctions();
		return FALSE;
	}

	return TRUE;
}

void ComFuncHook_Exit()
{
	UnhookFunctions();
}

void ComFuncHook_WaitTillDone()
{
	while(nHookProcCallCounter > 0)
		Sleep(10);
}

static BOOL HookFunctions()
{
	IDropTarget *dt;
	LONG_PTR *plp;
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;

	// OLE drag and drop interface
	dt = (IDropTarget *)GetProp(hTaskSwWnd, L"OleDropTargetInterface");
	if(!dt)
		return FALSE;

	ppOleDragEnter = (void **)&dt->lpVtbl->DragEnter;
	if(!CreateEnableHook(ppOleDragEnter, OleDragEnterHook, &pOleDragEnter, &prOleDragEnter))
		return FALSE;

	ppOleDragOver = (void **)&dt->lpVtbl->DragOver;
	if(!CreateEnableHook(ppOleDragOver, OleDragOverHook, &pOleDragOver, &prOleDragOver))
		return FALSE;

	ppOleDrop = (void **)&dt->lpVtbl->Drop;
	if(!CreateEnableHook(ppOleDrop, OleDropHook, &pOleDrop, &prOleDrop))
		return FALSE;

	// CTaskBand::GetUserPreferences, IsHorizontal
	plp = *(LONG_PTR **)(lpTaskSwLongPtr + DO4_3264(0x20, 0x40, 0x24, 0x48, ,, 0x28, 0x50));

	ppGetUserPreferences = (void **)&FUNC_CTaskBand_GetUserPreferences(plp);
	if(!CreateEnableHook(ppGetUserPreferences, GetUserPreferencesHook, &pGetUserPreferences, &prGetUserPreferences))
		return FALSE;

	ppIsHorizontal = (void **)&FUNC_CTaskBand_IsHorizontal(plp);
	if(!CreateEnableHook(ppIsHorizontal, IsHorizontalHook, &pIsHorizontal, &prIsHorizontal))
		return FALSE;

	// CTaskBand::GetIconId, SwitchTo, GetIconSize
	plp = *(LONG_PTR **)(lpTaskSwLongPtr + DO4_3264(0x20, 0x40, ,, ,, 0x24, 0x48));

	if(nWinVersion >= WIN_VERSION_10_T1)
	{
		ppGetIconId = (void **)&FUNC_CTaskBand_GetIconId(plp);
		if(!CreateEnableHook(ppGetIconId, GetIconIdHook, &pGetIconId, &prGetIconId))
			return FALSE;
	}

	ppSwitchTo = (void **)&FUNC_CTaskBand_SwitchTo(plp);
	if(nWinVersion >= WIN_VERSION_811)
	{
		if(!CreateEnableHook(ppSwitchTo, SwitchToHook2, &pSwitchTo, &prSwitchTo))
			return FALSE;
	}
	else
	{
		if(!CreateEnableHook(ppSwitchTo, SwitchToHook, &pSwitchTo, &prSwitchTo))
			return FALSE;
	}

	ppGetIconSize = (void **)&FUNC_CTaskBand_GetIconSize(plp);
	if(nWinVersion == WIN_VERSION_81)
	{
		if(!CreateEnableHook(ppGetIconSize, GetIconSizeHook, &pGetIconSize, &prGetIconSize))
			return FALSE;
	}
	else if(nWinVersion == WIN_VERSION_811)
	{
		if(!CreateEnableHook(ppGetIconSize, GetIconSizeHook2, &pGetIconSize, &prGetIconSize))
			return FALSE;
	}
	else
	{
		// GetButtonHeight is hooked instead (see below)
	}

	if(nWinVersion >= WIN_VERSION_10_T1)
	{
		// CTaskBand::CurrentVirtualDesktopChanged, CTaskBand::CurrentVirtualDesktopChangedAnimated
		plp = *(LONG_PTR **)(lpTaskSwLongPtr + DEF3264(0x38, 0x70));

		ppCurrentVirtualDesktopChanged = (void **)&FUNC_CTaskBand_CurrentVirtualDesktopChanged(plp);
		if(!CreateEnableHook(ppCurrentVirtualDesktopChanged, CurrentVirtualDesktopChangedHook, &pCurrentVirtualDesktopChanged, &prCurrentVirtualDesktopChanged))
			return FALSE;

		plp = *(LONG_PTR **)(lpTaskSwLongPtr + DEF3264(0x3C, 0x78));

		ppCurrentVirtualDesktopChangedAnimated = (void **)&FUNC_CTaskBand_CurrentVirtualDesktopChangedAnimated(plp);
		if(!CreateEnableHook(ppCurrentVirtualDesktopChangedAnimated, CurrentVirtualDesktopChangedAnimatedHook, &pCurrentVirtualDesktopChangedAnimated, &prCurrentVirtualDesktopChangedAnimated))
			return FALSE;

		// CTaskBand::Exec
		plp = *(LONG_PTR **)(lpTaskSwLongPtr + DO5_3264(0, 0, ,, ,, ,, 0x20, 0x40));

		ppTaskBandExec = (void **)&FUNC_CTaskBand_Exec(plp);
		if(!CreateEnableHook(ppTaskBandExec, TaskBandExecHook, &pTaskBandExec, &prTaskBandExec))
			return FALSE;
	}

	// StartAnimation
	plp = *(LONG_PTR **)(lpTaskListLongPtr + DEF3264(0x1C, 0x38));

	ppStartAnimation = (void **)&FUNC_CTaskListWnd_StartAnimation(plp);
	if(!CreateEnableHook(ppStartAnimation, StartAnimationHook, &pStartAnimation, &prStartAnimation))
		return FALSE;

	// GetStuckPlace
	plp = *(LONG_PTR **)(lpTaskListLongPtr + DEF3264(0x18, 0x30));

	ppGetStuckPlace = (void **)&FUNC_CTaskListWnd_GetStuckPlace(plp);
	if(!CreateEnableHook(ppGetStuckPlace, GetStuckPlaceHook, &pGetStuckPlace, &prGetStuckPlace))
		return FALSE;

	// TaskListWndInitialize (Windows 8+), TaskCreated, ActivateTask, TaskDestroyed (Until Windows 10 R1), TaskInclusionChanged, GetButtonHeight, DismissHoverUI, ShowJumpView
	plp = *(LONG_PTR **)(lpTaskListLongPtr + DEF3264(0x14, 0x28));

	ppTaskListWndInitialize = (void **)&FUNC_CTaskListWnd_Initialize(plp);
	if(nWinVersion >= WIN_VERSION_10_R2)
	{
		if(!CreateEnableHook(ppTaskListWndInitialize, TaskListWndInitializeHook3, &pTaskListWndInitialize, &prTaskListWndInitialize))
			return FALSE;
	}
	else if(nWinVersion >= WIN_VERSION_10_T1)
	{
		if(!CreateEnableHook(ppTaskListWndInitialize, TaskListWndInitializeHook2, &pTaskListWndInitialize, &prTaskListWndInitialize))
			return FALSE;
	}
	else if(nWinVersion >= WIN_VERSION_8)
	{
		if(!CreateEnableHook(ppTaskListWndInitialize, TaskListWndInitializeHook, &pTaskListWndInitialize, &prTaskListWndInitialize))
			return FALSE;
	}

	ppTaskCreated = (void **)&FUNC_CTaskListWnd_TaskCreated(plp);
	if(!CreateEnableHook(ppTaskCreated, TaskCreatedHook, &pTaskCreated, &prTaskCreated))
		return FALSE;

	ppActivateTask = (void **)&FUNC_CTaskListWnd_ActivateTask(plp);
	if(!CreateEnableHook(ppActivateTask, ActivateTaskHook, &pActivateTask, &prActivateTask))
		return FALSE;

	if(nWinVersion <= WIN_VERSION_10_T2)
	{
		ppTaskDestroyed = (void **)&FUNC_CTaskListWnd_TaskDestroyed(plp);
		if(!CreateEnableHook(ppTaskDestroyed, TaskDestroyedHook, &pTaskDestroyed, &prTaskDestroyed))
			return FALSE;
	}

	if(nWinVersion >= WIN_VERSION_10_T1)
	{
		ppTaskInclusionChanged = (void **)&FUNC_CTaskListWnd_TaskInclusionChanged(plp);
		if(!CreateEnableHook(ppTaskInclusionChanged, TaskInclusionChangedHook, &pTaskInclusionChanged, &prTaskInclusionChanged))
			return FALSE;
	}

	if(nWinVersion != WIN_VERSION_81 && nWinVersion != WIN_VERSION_811)
	{
		ppGetButtonHeight = (void **)&FUNC_CTaskListWnd_GetButtonHeight(plp);
		if(!CreateEnableHook(ppGetButtonHeight, GetButtonHeightHook, &pGetButtonHeight, &prGetButtonHeight))
			return FALSE;
	}
	else
	{
		// GetIconSize is hooked instead (see above)
	}

	ppDismissHoverUI = (void **)&FUNC_CTaskListWnd_DismissHoverUI(plp);
	if(!CreateEnableHook(ppDismissHoverUI, DismissHoverUIHook, &pDismissHoverUI, &prDismissHoverUI))
		return FALSE;

	ppShowDestinationMenu = (void **)&FUNC_CTaskListWnd_ShowJumpView(plp);
	if(nWinVersion <= WIN_VERSION_10_T2)
	{
		if(!CreateEnableHook(ppShowDestinationMenu, ShowDestinationMenuHook, &pShowDestinationMenu, &prShowDestinationMenu))
			return FALSE;
	}
	else
	{
		if(!CreateEnableHook(ppShowDestinationMenu, ShowJumpViewHook, &pShowDestinationMenu, &prShowDestinationMenu))
			return FALSE;
	}

	if(nWinVersion <= WIN_VERSION_10_T2)
	{
		// CTaskListWnd::OnDestinationMenuDismissed
		plp = *(LONG_PTR **)(lpTaskListLongPtr + DO5_3264(0x30, 0x60, ,, ,, ,, 0x2C, 0x58));

		ppOnDestinationMenuDismissed = (void **)&FUNC_CTaskListWnd_OnDestinationMenuDismissed(plp);
		if(!CreateEnableHook(ppOnDestinationMenuDismissed, OnDestinationMenuDismissedHook, &pOnDestinationMenuDismissed, &prOnDestinationMenuDismissed))
			return FALSE;
	}
	else
	{
		// Handled in ButtonGroupHasItemAnimationHook
	}

	// CTaskListThumbnailWnd::DisplayUI
	plp = *(LONG_PTR **)lpThumbnailLongPtr;

	ppDisplayUI = (void **)&FUNC_CTaskListThumbnailWnd_DisplayUI(plp);
	if(!CreateEnableHook(ppDisplayUI, DisplayUIHook, &pDisplayUI, &prDisplayUI))
		return FALSE;

	// These two hooks are for OPT_EX_LIST_REVERSE_ORDER, not supported in newer Windows versions.
	if(nWinVersion <= WIN_VERSION_811)
	{
		// CTaskListThumbnailWnd::GetThumbRectFromIndex, CTaskListThumbnailWnd::ThumbIndexFromPoint
		plp = *(LONG_PTR **)(lpThumbnailLongPtr + DO5_3264(0x10, 0x20, ,, ,, ,, 0x08, 0x10));

		ppGetThumbRectFromIndex = (void **)&FUNC_CTaskListThumbnailWnd_GetThumbRectFromIndex(plp);
		if(!CreateEnableHook(ppGetThumbRectFromIndex, GetThumbRectFromIndexHook, &pGetThumbRectFromIndex, &prGetThumbRectFromIndex))
			return FALSE;

		ppThumbIndexFromPoint = (void **)&FUNC_CTaskListThumbnailWnd_ThumbIndexFromPoint(plp);
		if(!CreateEnableHook(ppThumbIndexFromPoint, ThumbIndexFromPointHook, &pThumbIndexFromPoint, &prThumbIndexFromPoint))
			return FALSE;
	}

	// CTaskListThumbnailWnd::DestroyThumbnail
	plp = *(LONG_PTR **)(lpThumbnailLongPtr + DO5_3264(0x18, 0x30, ,, ,, ,, 0x10, 0x20));

	ppDestroyThumbnail = (void **)&FUNC_CTaskListThumbnailWnd_DestroyThumbnail(plp);
	if(!CreateEnableHook(ppDestroyThumbnail, DestroyThumbnailHook, &pDestroyThumbnail, &prDestroyThumbnail))
		return FALSE;

	// TaskGroup functions hook
	if(HookTaskGroupFunctions())
	{
		// TaskItem functions hook
		if(HookTaskItemFunctions())
		{
			// TaskBtnGroup functions hook
			if(HookTaskBtnGroupFunctions())
			{
				// Secondary taskbar functions hook
				lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
				if(!lpSecondaryTaskListLongPtr)
					return TRUE;

				if(HookSecondaryTaskbarFunctions(lpSecondaryTaskListLongPtr))
					return TRUE;
			}
		}
	}

	return FALSE;
}

static void UnhookFunctions()
{
	DisableHook(ppOleDragEnter, &prOleDragEnter);
	DisableHook(ppOleDragOver, &prOleDragOver);
	DisableHook(ppOleDrop, &prOleDrop);
	DisableHook(ppGetUserPreferences, &prGetUserPreferences);
	DisableHook(ppIsHorizontal, &prIsHorizontal);
	if(nWinVersion >= WIN_VERSION_10_T1)
		DisableHook(ppGetIconId, &prGetIconId);
	DisableHook(ppSwitchTo, &prSwitchTo);
	if(nWinVersion == WIN_VERSION_81 || nWinVersion == WIN_VERSION_811)
		DisableHook(ppGetIconSize, &prGetIconSize);
	if(nWinVersion >= WIN_VERSION_10_T1)
	{
		DisableHook(ppCurrentVirtualDesktopChanged, &prCurrentVirtualDesktopChanged);
		DisableHook(ppCurrentVirtualDesktopChangedAnimated, &prCurrentVirtualDesktopChangedAnimated);
		DisableHook(ppTaskBandExec, &prTaskBandExec);
	}
	DisableHook(ppStartAnimation, &prStartAnimation);
	DisableHook(ppGetStuckPlace, &prGetStuckPlace);
	if(nWinVersion >= WIN_VERSION_8)
		DisableHook(ppTaskListWndInitialize, &prTaskListWndInitialize);
	DisableHook(ppTaskCreated, &prTaskCreated);
	DisableHook(ppActivateTask, &prActivateTask);
	if(nWinVersion <= WIN_VERSION_10_T2)
		DisableHook(ppTaskDestroyed, &prTaskDestroyed);
	if(nWinVersion >= WIN_VERSION_10_T1)
		DisableHook(ppTaskInclusionChanged, &prTaskInclusionChanged);
	if(nWinVersion != WIN_VERSION_81 && nWinVersion != WIN_VERSION_811)
		DisableHook(ppGetButtonHeight, &prGetButtonHeight);
	DisableHook(ppDismissHoverUI, &prDismissHoverUI);
	DisableHook(ppShowDestinationMenu, &prShowDestinationMenu);
	if(nWinVersion <= WIN_VERSION_10_T2)
		DisableHook(ppOnDestinationMenuDismissed, &prOnDestinationMenuDismissed);
	DisableHook(ppDisplayUI, &prDisplayUI);
	if(nWinVersion <= WIN_VERSION_811)
	{
		DisableHook(ppGetThumbRectFromIndex, &prGetThumbRectFromIndex);
		DisableHook(ppThumbIndexFromPoint, &prThumbIndexFromPoint);
	}
	DisableHook(ppDestroyThumbnail, &prDestroyThumbnail);

	if(bTaskGroupFunctionsHooked)
		UnhookTaskGroupFunctions();

	if(bTaskItemFunctionsHooked)
		UnhookTaskItemFunctions();

	if(bTaskBtnGroupFunctionsHooked)
		UnhookTaskBtnGroupFunctions();

	if(bSecondaryTaskbarFunctionsHooked)
		UnhookSecondaryTaskbarFunctions();
}

static BOOL HookTaskGroupFunctions()
{
	LONG_PTR *plp = (LONG_PTR *)*EV_TASK_SW_TASK_GROUPS_HDPA();
	if(!plp || (int)plp[0] == 0)
		return TRUE;

	int task_groups_count = (int)plp[0];
	LONG_PTR **task_groups = (LONG_PTR **)plp[1];
	LONG_PTR *task_group;

	int i;
	for(i = 0; i < task_groups_count; i++)
	{
		task_group = (LONG_PTR *)task_groups[i];

		if(nWinVersion >= WIN_VERSION_10_T1)
		{
			LONG_PTR this_ptr = (LONG_PTR)task_group;
			plp = *(LONG_PTR **)this_ptr;

			// CTaskGroup::IsImmersiveGroup(this)
			if(FUNC_CTaskGroup_IsImmersiveGroup(plp)(this_ptr))
			{
				continue;
			}
		}

		break;
	}

	if(i == task_groups_count)
		return TRUE;

	plp = (LONG_PTR *)task_group[0]; // COM functions list

	ppDoesWindowMatch = (void **)&FUNC_CTaskGroup_DoesWindowMatch(plp);

	if(!CreateEnableHook(ppDoesWindowMatch, DoesWindowMatchHook, &pDoesWindowMatch, &prDoesWindowMatch))
	{
		UnhookTaskGroupFunctions();
		return FALSE;
	}

	bTaskGroupFunctionsHooked = TRUE;
	return TRUE;
}

static void UnhookTaskGroupFunctions()
{
	DisableHook(ppDoesWindowMatch, &prDoesWindowMatch);
}

static BOOL HookTaskItemFunctions()
{
	LONG_PTR *plp = (LONG_PTR *)*EV_TASK_SW_TASK_GROUPS_HDPA();
	if(!plp || (int)plp[0] == 0)
		return TRUE;

	int task_groups_count = (int)plp[0];
	LONG_PTR **task_groups = (LONG_PTR **)plp[1];

	int i;
	for(i = 0; i < task_groups_count; i++)
	{
		LONG_PTR *task_group = (LONG_PTR *)task_groups[i];

		if(nWinVersion >= WIN_VERSION_10_T1)
		{
			LONG_PTR this_ptr = (LONG_PTR)task_group;
			plp = *(LONG_PTR **)this_ptr;

			// CTaskGroup::IsImmersiveGroup(this)
			if(FUNC_CTaskGroup_IsImmersiveGroup(plp)(this_ptr))
			{
				continue;
			}
		}

		plp = (LONG_PTR *)task_group[4];
		if(plp && (int)plp[0] > 0)
			break;
	}

	if(i == task_groups_count)
		return TRUE;

	plp = (LONG_PTR *)plp[1]; // CTaskItem DPA array
	plp = (LONG_PTR *)plp[0]; // First array item
	plp = (LONG_PTR *)plp[0]; // COM functions list

	ppTaskItemSetWindow = (void **)&FUNC_CWindowTaskItem_SetWindow(plp);
	ppTaskItemGetWindow = (void **)&FUNC_CWindowTaskItem_GetWindow(plp);

	if(
		!CreateEnableHook(ppTaskItemSetWindow, TaskItemSetWindowHook, &pTaskItemSetWindow, &prTaskItemSetWindow) ||
		!CreateEnableHook(ppTaskItemGetWindow, TaskItemGetWindowHook, &pTaskItemGetWindow, &prTaskItemGetWindow)
	)
	{
		UnhookTaskItemFunctions();
		return FALSE;
	}

	bTaskItemFunctionsHooked = TRUE;
	return TRUE;
}

static void UnhookTaskItemFunctions()
{
	DisableHook(ppTaskItemSetWindow, &prTaskItemSetWindow);
	DisableHook(ppTaskItemGetWindow, &prTaskItemGetWindow);
}

static BOOL HookTaskBtnGroupFunctions()
{
	LONG_PTR *plp;
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;

	plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpTaskListLongPtr);
	if(!plp || (int)plp[0] == 0)
	{
		lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
		while(lpSecondaryTaskListLongPtr)
		{
			plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpSecondaryTaskListLongPtr);
			if(!plp || (int)plp[0] == 0)
				lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
			else
				break;
		}

		if(!lpSecondaryTaskListLongPtr)
			return TRUE;
	}

	plp = (LONG_PTR *)plp[1]; // CTaskBtnGroup DPA array
	plp = (LONG_PTR *)plp[0]; // First array item
	plp = (LONG_PTR *)plp[0]; // COM functions list

	ppButtonGroupRemoveTaskItem = (void **)&FUNC_CTaskBtnGroup_RemoveTaskItem(plp);
	ppGetIdealSpan = (void **)&FUNC_CTaskBtnGroup_GetIdealSpan(plp);
	ppSetLocation = (void **)&FUNC_CTaskBtnGroup_SetLocation(plp);
	ppRender = (void **)&FUNC_CTaskBtnGroup_Render(plp);
	ppButtonGroupCanGlom = (void **)&FUNC_CTaskBtnGroup_CanGlom(plp);
	ppShouldShowToolTip = (void **)&FUNC_CTaskBtnGroup_ShouldShowToolTip(plp);

	void *pRenderHook;
	if(nWinVersion >= WIN_VERSION_10_R4)
		pRenderHook = RenderHook2;
	else
		pRenderHook = RenderHook;

	if(
		!CreateEnableHook(ppButtonGroupRemoveTaskItem, ButtonGroupRemoveTaskItemHook, &pButtonGroupRemoveTaskItem, &prButtonGroupRemoveTaskItem) ||
		!CreateEnableHook(ppGetIdealSpan, GetIdealSpanHook, &pGetIdealSpan, &prGetIdealSpan) ||
		!CreateEnableHook(ppSetLocation, SetLocationHook, &pSetLocation, &prSetLocation) ||
		!CreateEnableHook(ppRender, pRenderHook, &pRender, &prRender) ||
		!CreateEnableHook(ppButtonGroupCanGlom, ButtonGroupCanGlomHook, &pButtonGroupCanGlom, &prButtonGroupCanGlom) ||
		!CreateEnableHook(ppShouldShowToolTip, ShouldShowToolTipHook, &pShouldShowToolTip, &prShouldShowToolTip)
	)
	{
		UnhookTaskBtnGroupFunctions();
		return FALSE;
	}

	if(nWinVersion <= WIN_VERSION_811)
	{
		ppButtonGroupHotTracking = (void **)&FUNC_CTaskBtnGroup_HandleGroupHotTracking(plp);
		ppButtonGroupHotTrackOut = (void **)&FUNC_CTaskBtnGroup_HandleGroupHotTrackOut(plp);

		if(
			!CreateEnableHook(ppButtonGroupHotTracking, ButtonGroupHotTrackingHook, &pButtonGroupHotTracking, &prButtonGroupHotTracking) ||
			!CreateEnableHook(ppButtonGroupHotTrackOut, ButtonGroupHotTrackOutHook, &pButtonGroupHotTrackOut, &prButtonGroupHotTrackOut)
		)
		{
			UnhookTaskBtnGroupFunctions();
			return FALSE;
		}
	}
	else
	{
		ppButtonGroupStartItemAnimation = (void **)&FUNC_CTaskBtnGroup_StartItemAnimation(plp);
		ppButtonGroupHasItemAnimation = (void **)&FUNC_CTaskBtnGroup_HasItemAnimation(plp);

		if(
			!CreateEnableHook(ppButtonGroupStartItemAnimation, ButtonGroupStartItemAnimationHook, &pButtonGroupStartItemAnimation, &prButtonGroupStartItemAnimation) ||
			!CreateEnableHook(ppButtonGroupHasItemAnimation, ButtonGroupHasItemAnimationHook, &pButtonGroupHasItemAnimation, &prButtonGroupHasItemAnimation)
		)
		{
			UnhookTaskBtnGroupFunctions();
			return FALSE;
		}
	}

	bTaskBtnGroupFunctionsHooked = TRUE;
	return TRUE;
}

static void UnhookTaskBtnGroupFunctions()
{
	DisableHook(ppButtonGroupRemoveTaskItem, &prButtonGroupRemoveTaskItem);
	DisableHook(ppGetIdealSpan, &prGetIdealSpan);
	DisableHook(ppSetLocation, &prSetLocation);
	DisableHook(ppRender, &prRender);
	DisableHook(ppButtonGroupCanGlom, &prButtonGroupCanGlom);
	DisableHook(ppShouldShowToolTip, &prShouldShowToolTip);

	if(nWinVersion <= WIN_VERSION_811)
	{
		DisableHook(ppButtonGroupHotTracking, &prButtonGroupHotTracking);
		DisableHook(ppButtonGroupHotTrackOut, &prButtonGroupHotTrackOut);
	}
	else
	{
		DisableHook(ppButtonGroupStartItemAnimation, &prButtonGroupStartItemAnimation);
		DisableHook(ppButtonGroupHasItemAnimation, &prButtonGroupHasItemAnimation);
	}
}

static BOOL HookSecondaryTaskbarFunctions(LONG_PTR lpSecondaryTaskListLongPtr)
{
	LONG_PTR lp;
	LONG_PTR *plp;
	BOOL bSuccess;

	lp = *EV_MM_TASKLIST_TASK_BAND_REF(lpSecondaryTaskListLongPtr);

	// CSecondaryTaskBand::GetUserPreferences, CSecondaryTaskBand::IsHorizontal
	plp = *(LONG_PTR **)lp;

	ppSecondaryGetUserPreferences = (void **)&FUNC_CSecondaryTaskBand_GetUserPreferences(plp);
	ppSecondaryIsHorizontal = (void **)&FUNC_CSecondaryTaskBand_IsHorizontal(plp);

	if(PointerRedirectionGetOriginalPtr(ppSecondaryGetUserPreferences) == PointerRedirectionGetOriginalPtr(ppGetUserPreferences))
	{
		// This happens in Windows 8.1.1
		PatchPtr(ppSecondaryGetUserPreferences, *ppGetUserPreferences);
		bSuccess = TRUE;
	}
	else
		bSuccess = CreateEnableHook(ppSecondaryGetUserPreferences, SecondaryGetUserPreferencesHook, &pSecondaryGetUserPreferences, &prSecondaryGetUserPreferences);

	if(
		!bSuccess ||
		!CreateEnableHook(ppSecondaryIsHorizontal, SecondaryIsHorizontalHook, &pSecondaryIsHorizontal, &prSecondaryIsHorizontal)
	)
	{
		UnhookSecondaryTaskbarFunctions();
		return FALSE;
	}

	bSecondaryTaskbarFunctionsHooked = TRUE;
	return TRUE;
}

static void UnhookSecondaryTaskbarFunctions()
{
	if(PointerRedirectionGetOriginalPtr(ppSecondaryGetUserPreferences) == PointerRedirectionGetOriginalPtr(ppGetUserPreferences))
	{
		// This happens in Windows 8.1.1
		PatchPtr(ppSecondaryGetUserPreferences, *ppGetUserPreferences);
	}
	else
		DisableHook(ppSecondaryGetUserPreferences, &prSecondaryGetUserPreferences);

	DisableHook(ppSecondaryIsHorizontal, &prSecondaryIsHorizontal);
}

static HRESULT __stdcall OleDragEnterHook(IDropTarget *This, IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	LONG_PTR lpMMTaskListLongPtr;
	HRESULT hr;

	nHookProcCallCounter++;

	lpMMTaskListLongPtr = (LONG_PTR)This - DEF3264(0x24, 0x48);
	if(IsMMTaskListLongPtr(lpMMTaskListLongPtr))
	{
		if(nOptions[OPT_DROP] == 1)
			grfKeyState ^= MK_SHIFT;

		hr = ((OLE_DRAG_ENTER_PROC *)pOleDragEnter)(This, pDataObj, grfKeyState, pt, pdwEffect);

		dwDragTickCount = GetTickCount();
	}
	else
		hr = ((OLE_DRAG_ENTER_PROC *)pOleDragEnter)(This, pDataObj, grfKeyState, pt, pdwEffect);

	nHookProcCallCounter--;

	return hr;
}

static HRESULT __stdcall OleDragOverHook(IDropTarget *This, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	LONG_PTR lpMMTaskListLongPtr;
	HRESULT hr;

	nHookProcCallCounter++;

	lpMMTaskListLongPtr = (LONG_PTR)This - DEF3264(0x24, 0x48);
	if(IsMMTaskListLongPtr(lpMMTaskListLongPtr))
	{
		if(nOptions[OPT_DROP] == 1)
			grfKeyState ^= MK_SHIFT;

		hr = ((OLE_DRAG_OVER_PROC *)pOleDragOver)(This, grfKeyState, pt, pdwEffect);

		if(*EV_MM_TASKLIST_TRACKED_BUTTON_GROUP(lpMMTaskListLongPtr))
		{
			dwDragTickCount = GetTickCount();
		}
		else if(nOptionsEx[OPT_EX_SHOW_DESKTOP_ON_HOVER] && GetTickCount() - dwDragTickCount >= 500)
		{
			// CShowDesktopButton::_RaiseDesktop
			PostMessage(hTaskbarWnd, 0x579, 1, 1);
		}
	}
	else
		hr = ((OLE_DRAG_OVER_PROC *)pOleDragOver)(This, grfKeyState, pt, pdwEffect);

	nHookProcCallCounter--;

	return hr;
}

static HRESULT __stdcall OleDropHook(IDropTarget *This, IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	LONG_PTR lpMMTaskListLongPtr;
	HRESULT hr;

	nHookProcCallCounter++;

	lpMMTaskListLongPtr = (LONG_PTR)This - DEF3264(0x24, 0x48);
	if(IsMMTaskListLongPtr(lpMMTaskListLongPtr))
	{
		if(nOptions[OPT_DROP] == 1)
			grfKeyState ^= MK_SHIFT;
	}

	hr = ((OLE_DROP_PROC *)pOleDrop)(This, pDataObj, grfKeyState, pt, pdwEffect);

	nHookProcCallCounter--;

	return hr;
}

static LONG_PTR __stdcall GetUserPreferencesHook(LONG_PTR var1, DWORD *pdwPreferences)
{
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, DWORD *))pGetUserPreferences)(var1, pdwPreferences);

	*pdwPreferences = ManipulateUserPreferences(*pdwPreferences, _AddressOfReturnAddress());

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall IsHorizontalHook(LONG_PTR this_ptr)
{
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	if(nWinVersion <= WIN_VERSION_811)
	{
		// The magic happens in: CTaskListWnd::_CheckNeedScrollbars
		if(selective_combining_hack == 1)
			selective_combining_hack = 2;
		else if(selective_combining_hack >= 3)
			selective_combining_hack = 1;
	}

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR))pIsHorizontal)(this_ptr);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall GetIconIdHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item, LONG_PTR var4, LONG_PTR var5)
{
	// assert(nWinVersion >= WIN_VERSION_10_T1)

	LONG_PTR lpRet;

	nHookProcCallCounter++;

	// If labels are visible, or for a decombined group, task_item is set,
	// and each item gets its own icon. In this case we do nothing.
	//
	// If labels are hidden, and it's a combined group or a single item,
	// task_item is not set, and an icon based on the AppId is set.
	// In this case there's a problem for UWP apps - if more than one such
	// instance is open, only the first instance gets an icon.
	// The other instances get a blank icon. We fix it below by replacing the
	// task group with the task group of the first item of the same AppId on the taskbar.
	if(task_group && !task_item)
	{
		LONG_PTR this_ptr = (LONG_PTR)task_group;
		LONG_PTR *plp = *(LONG_PTR **)this_ptr;

		// CTaskGroup::IsImmersiveGroup(this)
		if(FUNC_CTaskGroup_IsImmersiveGroup(plp)(this_ptr))
		{
			LONG_PTR **p_task_group, **p_task_group_other;
			if(FindTaskGroupPairInArrayWithSameAppId(task_group, &p_task_group, &p_task_group_other))
			{
				if(p_task_group_other < p_task_group)
					task_group = *p_task_group_other;
			}
		}
	}

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *, LONG_PTR, LONG_PTR))pGetIconId)(this_ptr, task_group, task_item, var4, var5);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall SwitchToHook(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR *task_item, BOOL bSwitchTo)
{
	// assert(nWinVersion <= WIN_VERSION_81)

	LONG_PTR lpRet;

	nHookProcCallCounter++;

	if(nSwitchToOption)
	{
		int nOption = nSwitchToOption;
		nSwitchToOption = 0; // Prevent recursive calls and stack overflow
		SwitchToExecOption(task_item, nOption);
		nSwitchToOption = nOption;

		lpRet = 0;
	}
	else
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR *, BOOL))pSwitchTo)
			(this_ptr, var2, task_item, bSwitchTo);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall SwitchToHook2(LONG_PTR this_ptr, LONG_PTR *task_item, BOOL bSwitchTo)
{
	// assert(nWinVersion >= WIN_VERSION_811)

	LONG_PTR lpRet;

	nHookProcCallCounter++;

	if(nSwitchToOption)
	{
		int nOption = nSwitchToOption;
		nSwitchToOption = 0; // Prevent recursive calls and stack overflow
		SwitchToExecOption(task_item, nOption);
		nSwitchToOption = nOption;

		lpRet = 0;
	}
	else
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, BOOL))pSwitchTo)
			(this_ptr, task_item, bSwitchTo);

	nHookProcCallCounter--;

	return lpRet;
}

static void SwitchToExecOption(LONG_PTR *task_item, int nOption)
{
	switch(nOption)
	{
	case 1:
		SwitchToTaskItem(task_item);
		break;

	case 2:
		MinimizeTaskItem(task_item);
		break;

	case 3:
		CloseTaskItem(task_item, TRUE);
		break;

	case 7:
		OpenThumbnailPreview(lpSwitchToMMTaskListLongPtr);
		break;

	case 8:
		TerminateProcessOfTaskItem(task_item);
		break;

	case 13:
		if(CanMaximizeTaskItem(task_item))
			PostMessage(GetTaskItemWnd(task_item), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		break;

	case 14:
		if(CanRestoreTaskItem(task_item))
			PostMessage(GetTaskItemWnd(task_item), WM_SYSCOMMAND, SC_RESTORE, 0);
		break;
	}
}

static LONG_PTR __stdcall GetIconSizeHook(LONG_PTR var1, LONG_PTR var2)
{
	// assert(nWinVersion == WIN_VERSION_81)
	// For other OSes, see GetIconSizeHook2 / GetButtonHeightHook

	LONG_PTR lpRet;

	nHookProcCallCounter++;

	GetIconSizeAndGetButtonHeightHack();

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR))pGetIconSize)(var1, var2);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall GetIconSizeHook2(LONG_PTR var1, LONG_PTR var2, LONG_PTR var3)
{
	// assert(nWinVersion == WIN_VERSION_811)
	// For other OSes, see GetIconSizeHook / GetButtonHeightHook

	LONG_PTR lpRet;

	nHookProcCallCounter++;

	GetIconSizeAndGetButtonHeightHack();

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR))pGetIconSize)(var1, var2, var3);

	nHookProcCallCounter--;

	return lpRet;
}

static void GetIconSizeAndGetButtonHeightHack()
{
	if(nWinVersion <= WIN_VERSION_811)
	{
		// Called from CTaskBtnGroup::_DrawRegularButtonWhiteLight
		// when an item is middle clicked for a new instance
		if(decombine_without_labels_hack == 2)
			decombine_without_labels_hack = 3;
	}

	if(nWinVersion <= WIN_VERSION_811)
	{
		// The magic happens in: CTaskListWnd::_CheckNeedScrollbars
		if(selective_combining_hack == 2)
			selective_combining_hack = 3;
		else if(selective_combining_hack >= 4)
			selective_combining_hack = 0;
	}
}

static LONG_PTR __stdcall CurrentVirtualDesktopChangedHook(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3)
{
	// assert(nWinVersion >= WIN_VERSION_10_T1)

	LONG_PTR lpRet;

	nHookProcCallCounter++;

	if(nOptionsEx[OPT_EX_VIRTUAL_DESKTOP_ORDER_FIX])
	{
		if(!bHadCurrentVirtualDesktopChangedAnimated)
		{
			LONG_PTR *pMainTaskListAnimationManager;
			ANIMATION_MANAGER_ITEM *lpSecondaryTaskListAnimationManagers;
			DisableTaskbarsAnimation(&pMainTaskListAnimationManager, &lpSecondaryTaskListAnimationManagers);

			HideAllTaskbarItems();

			RestoreTaskbarsAnimation(pMainTaskListAnimationManager, lpSecondaryTaskListAnimationManagers);
		}
		else
			bHadCurrentVirtualDesktopChangedAnimated = FALSE;
	}

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR))pCurrentVirtualDesktopChanged)(this_ptr, var2, var3);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall CurrentVirtualDesktopChangedAnimatedHook(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3)
{
	// assert(nWinVersion >= WIN_VERSION_10_T1)

	LONG_PTR lpRet;

	nHookProcCallCounter++;

	if(nOptionsEx[OPT_EX_VIRTUAL_DESKTOP_ORDER_FIX])
	{
		LONG_PTR *pMainTaskListAnimationManager;
		ANIMATION_MANAGER_ITEM *lpSecondaryTaskListAnimationManagers;
		DisableTaskbarsAnimation(&pMainTaskListAnimationManager, &lpSecondaryTaskListAnimationManagers);

		HideAllTaskbarItems();

		RestoreTaskbarsAnimation(pMainTaskListAnimationManager, lpSecondaryTaskListAnimationManagers);

		bHadCurrentVirtualDesktopChangedAnimated = TRUE;
	}

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR))pCurrentVirtualDesktopChangedAnimated)(this_ptr, var2, var3);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall TaskBandExecHook(LONG_PTR this_ptr, GUID *pGuid, LONG_PTR var3, LONG_PTR var4, LONG_PTR var5, LONG_PTR var6)
{
	// assert(nWinVersion >= WIN_VERSION_10_T1)

	LONG_PTR lpRet;

	nHookProcCallCounter++;

	// If called from BandSite_HandleDelayInitStuff -> CTrayBandSite::_BroadcastExec.
	const char *IID_IDeskBand = "\x72\xE1\x0F\xEB\x3A\x1A\xD0\x11\x89\xB3\x00\xA0\xC9\x0A\x90\xAC";
	if(memcmp(pGuid, IID_IDeskBand, sizeof(GUID)) == 0 && var3 == 5 && var4 == 0 && var5 == 0 && var6 == 0)
	{
		bInHandleDelayInitStuff = TRUE;
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, GUID *, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR))pTaskBandExec)(this_ptr, pGuid, var3, var4, var5, var6);
		bInHandleDelayInitStuff = FALSE;
	}
	else
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, GUID *, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR))pTaskBandExec)(this_ptr, pGuid, var3, var4, var5, var6);

	nHookProcCallCounter--;

	return lpRet;
}

static void HideAllTaskbarItems()
{
	// assert(nWinVersion >= WIN_VERSION_10_T1)

	LONG_PTR *plp = (LONG_PTR *)*EV_TASK_SW_TASK_GROUPS_HDPA();
	if(plp && (int)plp[0] > 0)
	{
		int task_groups_count = (int)plp[0];
		LONG_PTR **task_groups = (LONG_PTR **)plp[1];

		for(int i = 0; i < task_groups_count; i++)
		{
			LONG_PTR *task_group = task_groups[i];
			if(!task_group)
				continue;

			plp = (LONG_PTR *)task_group[4];
			if(!plp)
				continue;

			int task_items_count = (int)plp[0];
			LONG_PTR **task_items = (LONG_PTR **)plp[1];

			for(int j = 0; j < task_items_count; j++)
			{
				LONG_PTR *task_item = task_items[j];

				LONG_PTR this_ptr = (LONG_PTR)task_item;
				plp = *(LONG_PTR **)this_ptr;

				// CTaskItem::IsVisibleOnCurrentVirtualDesktop(this)
				BYTE bIsVisible = FUNC_CTaskItem_IsVisibleOnCurrentVirtualDesktop(plp)(this_ptr);
				if(!bIsVisible)
					continue;

				// CTaskItem::SetVisibleOnCurrentVirtualDesktop(this, bool)
				FUNC_CTaskItem_SetVisibleOnCurrentVirtualDesktop(plp)(this_ptr, 0);

				this_ptr = *EV_TASK_SW_MULTI_TASK_LIST_REF();
				plp = *(LONG_PTR **)this_ptr;

				// CTaskListWndMulti::TaskDestroyed(this, ITaskGroup *,ITaskItem *)
				FUNC_CTaskListWndMulti_TaskDestroyed(plp)(this_ptr, task_group, task_item);
			}
		}
	}
}

static LONG_PTR __stdcall StartAnimationHook(LONG_PTR var1, void *pObject, int nAnimation)
{
	LONG_PTR *button_group;
	LONG_PTR *task_group;
	WCHAR *pAppId;
	int nListValue;
	BOOL bNoAnimation;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	bNoAnimation = FALSE;

	if(nAnimation == 4 && pObject)
	{
		button_group = (LONG_PTR *)pObject;

		if((int)button_group[DO2(6, 8)] == 2 && !nOptionsEx[OPT_EX_PINNED_UNGROUPED_ANIMATE_LAUNCH])
		{
			task_group = (LONG_PTR *)button_group[DO2(3, 4)];
			pAppId = *EV_TASKGROUP_APPID(task_group);

			if(pAppId && GetAppidListValue(AILIST_GROUPPINNED, pAppId, &nListValue))
			{
				if(nListValue == AILIST_GROUPPINNED_NEVER)
					bNoAnimation = TRUE;
			}
			else if(nOptions[OPT_GROUPING_NOPINNED] == 1)
				bNoAnimation = TRUE;
		}
	}

	if(bNoAnimation)
		lpRet = 0; // disable animation
	else
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, void *, int))pStartAnimation)(var1, pObject, nAnimation);

	nHookProcCallCounter--;

	return lpRet;
}

static int __stdcall GetStuckPlaceHook(LONG_PTR this_ptr)
{
	LONG_PTR lpMMTaskListLongPtr;
	HWND hMMTaskListWnd;
	int nRet;

	nHookProcCallCounter++;

	lpMMTaskListLongPtr = this_ptr - DEF3264(0x18, 0x30);
	hMMTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);

	if(bInMouseMove && nOptionsEx[OPT_EX_DRAG_TOWARDS_DESKTOP] == 1 &&
		GetCapture() == hMMTaskListWnd && (nOptions[OPT_GROUPING_RIGHTDRAG] == 0 || !hDragWithinGroupsWnd))
	{
		nRet = -1;
	}
	else
		nRet = ((int(__stdcall *)(LONG_PTR))pGetStuckPlace)(this_ptr);

	nHookProcCallCounter--;

	return nRet;
}

static HRESULT __stdcall TaskListWndInitializeHook(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3)
{
	// assert(nWinVersion >= WIN_VERSION_8 && nWinVersion <= WIN_VERSION_811)

	HRESULT hrRet;

	nHookProcCallCounter++;

	hrRet = ((HRESULT(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR))pTaskListWndInitialize)(this_ptr, var2, var3);
	if(SUCCEEDED(hrRet))
	{
		LONG_PTR lpSecondaryTaskListLongPtr = this_ptr - DEF3264(0x14, 0x28);
		OnTaskListWndInitialized(lpSecondaryTaskListLongPtr);
	}

	nHookProcCallCounter--;

	return hrRet;
}

static HRESULT __stdcall TaskListWndInitializeHook2(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3, LONG_PTR var4)
{
	// assert(nWinVersion >= WIN_VERSION_10_T1 && nWinVersion <= WIN_VERSION_10_R1)

	HRESULT hrRet;

	nHookProcCallCounter++;

	hrRet = ((HRESULT(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR))pTaskListWndInitialize)(this_ptr, var2, var3, var4);
	if(SUCCEEDED(hrRet))
	{
		LONG_PTR lpSecondaryTaskListLongPtr = this_ptr - DEF3264(0x14, 0x28);
		OnTaskListWndInitialized(lpSecondaryTaskListLongPtr);
	}

	nHookProcCallCounter--;

	return hrRet;
}

static HRESULT __stdcall TaskListWndInitializeHook3(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3, LONG_PTR var4, LONG_PTR var5)
{
	// assert(nWinVersion >= WIN_VERSION_10_R2)

	HRESULT hrRet;

	nHookProcCallCounter++;

	hrRet = ((HRESULT(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR))pTaskListWndInitialize)(this_ptr, var2, var3, var4, var5);
	if(SUCCEEDED(hrRet))
	{
		LONG_PTR lpSecondaryTaskListLongPtr = this_ptr - DEF3264(0x14, 0x28);
		OnTaskListWndInitialized(lpSecondaryTaskListLongPtr);
	}

	nHookProcCallCounter--;

	return hrRet;
}

static void OnTaskListWndInitialized(LONG_PTR lpSecondaryTaskListLongPtr)
{
	// assert(nWinVersion >= WIN_VERSION_8)

	// Subclass a secondary taskbar when created (multimonitor environment)
	hackf1_SubclassSecondaryTaskListWindows(lpSecondaryTaskListLongPtr);

	// Hook functions
	BOOL bApplyQueuedHooks = FALSE;

	if(!bSecondaryTaskbarFunctionsHooked)
		if(HookSecondaryTaskbarFunctions(lpSecondaryTaskListLongPtr))
			bApplyQueuedHooks = TRUE;

	if(bApplyQueuedHooks)
		MH_ApplyQueued();
}

static LONG_PTR __stdcall TaskCreatedHook(LONG_PTR this_ptr, LONG_PTR var2, LONG_PTR var3)
{
	BOOL bApplyQueuedHooks;
	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	// Hook functions
	bApplyQueuedHooks = FALSE;

	if(!bTaskGroupFunctionsHooked)
		if(HookTaskGroupFunctions() && bTaskGroupFunctionsHooked)
			bApplyQueuedHooks = TRUE;

	if(!bTaskItemFunctionsHooked)
		if(HookTaskItemFunctions() && bTaskItemFunctionsHooked)
			bApplyQueuedHooks = TRUE;

	if(!bTaskBtnGroupFunctionsHooked)
		if(HookTaskBtnGroupFunctions() && bTaskBtnGroupFunctionsHooked)
			bApplyQueuedHooks = TRUE;

	if(bApplyQueuedHooks)
		MH_ApplyQueued();

	lpMMTaskListLongPtr = this_ptr - DEF3264(0x14, 0x28);

	if((TaskbarGetPreference(lpMMTaskListLongPtr) & 3) != 1) // if not "Combine when taskbar is full"
	{
		selective_combining_hack = 1;

		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR))pTaskCreated)(this_ptr, var2, var3);

		selective_combining_hack = 0;
	}
	else
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR))pTaskCreated)(this_ptr, var2, var3);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall ActivateTaskHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item)
{
	LONG_PTR lpMMTaskListLongPtr;
	HWND hMMTaskListWnd;
	LONG_PTR *prev_button_group_active, *button_group_active;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpMMTaskListLongPtr = this_ptr - DEF3264(0x14, 0x28);
	hMMTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);

	if(!task_group && !task_item)
	{
		POINT pt;
		GetCursorPos(&pt);
		if(WindowFromPoint(pt) == GetAncestor(hMMTaskListWnd, GA_ROOT))
		{
			last_active_task_item = TaskbarGetActiveTaskItem(lpMMTaskListLongPtr);
		}
	}

	// For nWinVersion >= WIN_VERSION_10_R1, this is handled in ButtonGroupHasItemAnimationHook
	if(nWinVersion <= WIN_VERSION_10_T2 && nOptions[OPT_COMBINING_DEACTIVE] == 1)
	{
		prev_button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *))pActivateTask)(this_ptr, task_group, task_item);

		button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

		if(button_group_active != prev_button_group_active)
			ActiveButtonGroupChanged(lpMMTaskListLongPtr, prev_button_group_active, button_group_active);
	}
	else
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *))pActivateTask)(this_ptr, task_group, task_item);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall TaskDestroyedHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item)
{
	// assert(nWinVersion <= WIN_VERSION_10_T2)

	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR *prev_button_group_active, *button_group_active;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpMMTaskListLongPtr = this_ptr - DEF3264(0x14, 0x28);

	// For nWinVersion >= WIN_VERSION_10_R1, this is handled in ButtonGroupHasItemAnimationHook
	if(nOptions[OPT_COMBINING_DEACTIVE] == 1)
	{
		prev_button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *))pTaskDestroyed)(this_ptr, task_group, task_item);

		if(!ButtonGroupValidate(lpMMTaskListLongPtr, prev_button_group_active))
			prev_button_group_active = NULL; // if already gone

		button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

		if(button_group_active != prev_button_group_active)
			ActiveButtonGroupChanged(lpMMTaskListLongPtr, prev_button_group_active, button_group_active);
	}
	else
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *))pTaskDestroyed)(this_ptr, task_group, task_item);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall TaskInclusionChangedHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item)
{
	// assert(nWinVersion >= WIN_VERSION_10_T1)

	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpMMTaskListLongPtr = this_ptr - DEF3264(0x14, 0x28);

	if((TaskbarGetPreference(lpMMTaskListLongPtr) & 3) != 1) // if not "Combine when taskbar is full"
	{
		selective_combining_hack = 1;

		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *))pTaskInclusionChanged)(this_ptr, task_group, task_item);

		selective_combining_hack = 0;
	}
	else
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *))pTaskInclusionChanged)(this_ptr, task_group, task_item);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall GetButtonHeightHook(LONG_PTR this_ptr, LONG_PTR var2)
{
	// assert(nWinVersion != WIN_VERSION_81 && nWinVersion != WIN_VERSION_811)
	// For other OSes, see GetIconSizeHook / GetIconSizeHook

	LONG_PTR lpRet;

	nHookProcCallCounter++;

	GetIconSizeAndGetButtonHeightHack();

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR))pGetButtonHeight)(this_ptr, var2);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall DismissHoverUIHook(LONG_PTR this_ptr, BOOL bHideWithoutAnimation)
{
	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR lpMMThumbnailLongPtr;
	LONG_PTR *button_group_of_thumb;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	if(bThumbNoDismiss)
	{
		lpMMTaskListLongPtr = this_ptr - DEF3264(0x14, 0x28);

		button_group_of_thumb = *EV_MM_TASKLIST_THUMB_BUTTON_GROUP(lpMMTaskListLongPtr);
		if(button_group_of_thumb)
		{
			lpMMThumbnailLongPtr = *EV_MM_TASKLIST_MM_THUMBNAIL_LONG_PTR(lpMMTaskListLongPtr);

			// Make the thumbnail disappear correctly upon focus loss
			*EV_MM_THUMBNAIL_STICKY_FLAG(lpMMThumbnailLongPtr) = FALSE;
		}

		lpRet = 0;
	}
	else
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, BOOL))pDismissHoverUI)(this_ptr, bHideWithoutAnimation);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall ShowDestinationMenuHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item, LONG_PTR var4, LONG_PTR var5)
{
	// assert(nWinVersion <= WIN_VERSION_10_T2)

	LONG_PTR lpMMTaskListLongPtr;
	HWND hMMTaskListWnd;
	LONG_PTR *prev_button_group_active, *button_group_active;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpMMTaskListLongPtr = this_ptr - DEF3264(0x14, 0x28);
	hMMTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);

	if(bInMouseMove &&
		nOptionsEx[OPT_EX_DRAG_TOWARDS_DESKTOP] >= 2 &&
		nOptionsEx[OPT_EX_DRAG_TOWARDS_DESKTOP] <= 14 &&
		GetCapture() == hMMTaskListWnd)
	{
		if(!bCustomDestinationMenuActionDone)
		{
			bCustomDestinationMenuActionDone = TRUE;
			JumpListExecOption(lpMMTaskListLongPtr, hMMTaskListWnd, task_group, task_item, nOptionsEx[OPT_EX_DRAG_TOWARDS_DESKTOP]);
		}

		lpRet = E_FAIL;
	}
	else
	{
		StealPinnedFlagIfPossible(task_group);

		// For nWinVersion >= WIN_VERSION_10_R1, this is handled in ButtonGroupHasItemAnimationHook
		if(nOptions[OPT_COMBINING_DEACTIVE] == 1)
		{
			prev_button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

			lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *, LONG_PTR, LONG_PTR))pShowDestinationMenu)(this_ptr, task_group, task_item, var4, var5);

			button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

			if(button_group_active != prev_button_group_active)
				ActiveButtonGroupChanged(lpMMTaskListLongPtr, prev_button_group_active, button_group_active);
		}
		else
			lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *, LONG_PTR, LONG_PTR))pShowDestinationMenu)(this_ptr, task_group, task_item, var4, var5);
	}

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall ShowJumpViewHook(LONG_PTR this_ptr, LONG_PTR *task_group, LONG_PTR *task_item, LONG_PTR var4)
{
	// assert(nWinVersion >= WIN_VERSION_10_R1)

	LONG_PTR lpMMTaskListLongPtr;
	HWND hMMTaskListWnd;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpMMTaskListLongPtr = this_ptr - DEF3264(0x14, 0x28);
	hMMTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);

	if(bInMouseMove &&
		nOptionsEx[OPT_EX_DRAG_TOWARDS_DESKTOP] >= 2 &&
		nOptionsEx[OPT_EX_DRAG_TOWARDS_DESKTOP] <= 14 &&
		GetCapture() == hMMTaskListWnd)
	{
		if(!bCustomDestinationMenuActionDone)
		{
			bCustomDestinationMenuActionDone = TRUE;
			JumpListExecOption(lpMMTaskListLongPtr, hMMTaskListWnd, task_group, task_item, nOptionsEx[OPT_EX_DRAG_TOWARDS_DESKTOP]);
		}

		lpRet = E_FAIL;
	}
	else
	{
		StealPinnedFlagIfPossible(task_group);

		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *, LONG_PTR))pShowDestinationMenu)(this_ptr, task_group, task_item, var4);
	}

	nHookProcCallCounter--;

	return lpRet;
}

static void JumpListExecOption(LONG_PTR lpMMTaskListLongPtr, HWND hMMTaskListWnd, LONG_PTR *task_group, LONG_PTR *task_item, int nOption)
{
	LONG_PTR *button_group;
	BOOL bDismissThumbs = FALSE;

	switch(nOption)
	{
	case 2:
	case 3:
	case 4:
	case 7:
	case 12:
	case 13:
		if(task_item)
		{
			SendMessage(hMMTaskListWnd, WM_LBUTTONUP, 0, MAKELPARAM(-1, -1));

			switch(nOption)
			{
			case 2:
				SwitchToTaskItem(task_item);
				break;

			case 3:
				MinimizeTaskItem(task_item);
				break;

			case 4:
				CloseTaskItem(task_item, TRUE);
				break;

			case 7:
				TerminateProcessOfTaskItem(task_item);
				break;

			case 12:
				if(CanMaximizeTaskItem(task_item))
					PostMessage(GetTaskItemWnd(task_item), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
				break;

			case 13:
				if(CanRestoreTaskItem(task_item))
					PostMessage(GetTaskItemWnd(task_item), WM_SYSCOMMAND, SC_RESTORE, 0);
				break;
			}

			bDismissThumbs = TRUE;
		}
		break;

	case 5:
	case 8:
	case 9:
	case 10:
	case 11:
	case 14:
		button_group = ButtonGroupFromTaskGroup(lpMMTaskListLongPtr, task_group);
		if(button_group)
		{
			SendMessage(hMMTaskListWnd, WM_LBUTTONUP, 0, MAKELPARAM(-1, -1));

			switch(nOption)
			{
			case 5:
				CreateNewInstance(lpMMTaskListLongPtr, button_group);
				break;

			case 8:
				ButtonGroupExecMenuCommand(button_group, SC_MINIMIZE);
				break;

			case 9:
				ButtonGroupExecMenuCommand(button_group, SC_CLOSE);
				break;

			case 10:
				ButtonGroupExecMenuCommand(button_group, SC_MAXIMIZE);
				break;

			case 11:
				ButtonGroupExecMenuCommand(button_group, SC_RESTORE);
				break;

			case 14:
				SortButtonGroupItems(button_group);
				break;
			}

			bDismissThumbs = TRUE;
		}
		break;

	case 6:
		SendMessage(hMMTaskListWnd, WM_LBUTTONUP, 0, MAKELPARAM(-1, -1));
		OpenThumbnailPreview(lpMMTaskListLongPtr);
		break;
	}

	if(bDismissThumbs)
	{
		DismissHoverUI(lpMMTaskListLongPtr, TRUE);

		// Should disable thumbs on some weird situations
		*EV_MM_TASKLIST_THUMB_DISABLING_FLAG(lpMMTaskListLongPtr) = TRUE;
	}
}

static BOOL StealPinnedFlagIfPossible(LONG_PTR *task_group)
{
	// Fixup disabled grouping and pinned items - steal pinned flag if possible
	DWORD dwFlags = *EV_TASKGROUP_FLAGS(task_group);
	if(!(dwFlags & 1)) // if not marked as pinned
	{
		LONG_PTR **p_task_group, **p_task_group_other;
		if(FindTaskGroupPairInArrayWithSameAppId(task_group, &p_task_group, &p_task_group_other))
		{
			LONG_PTR *task_group_other = *p_task_group_other;
			DWORD dwOtherFlags = *EV_TASKGROUP_FLAGS(task_group_other);
			if(dwOtherFlags & 1) // if marked as pinned
			{
				LONG_PTR *plp = (LONG_PTR *)*EV_TASKGROUP_TASKITEMS_HDPA(task_group_other);
				if(plp && (int)plp[0] > 0) // if contains task items, not e.g. a pinned task item without buttons
				{
					//int task_item_other_count = (int)plp[0];
					//LONG_PTR **task_items_other = (LONG_PTR **)plp[1];

					// it's marked as non-pinned, but there are other groups with the same appid; swap pinned flag
					*EV_TASKGROUP_FLAGS(task_group) |= 1;
					*EV_TASKGROUP_FLAGS(task_group_other) &= ~1;

					*p_task_group = task_group_other;
					*p_task_group_other = task_group;

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

static LONG_PTR __stdcall OnDestinationMenuDismissedHook(LONG_PTR this_ptr)
{
	// assert(nWinVersion <= WIN_VERSION_10_T2);
	// For nWinVersion >= WIN_VERSION_10_R1, this is handled in ButtonGroupHasItemAnimationHook

	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR *prev_button_group_active, *button_group_active;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	if(nOptions[OPT_COMBINING_DEACTIVE] == 1)
	{
		lpMMTaskListLongPtr = this_ptr - DO5_3264(0x30, 0x60, ,, ,, ,, 0x2C, 0x58);

		prev_button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR))pOnDestinationMenuDismissed)(this_ptr);

		button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

		if(button_group_active != prev_button_group_active)
			ActiveButtonGroupChanged(lpMMTaskListLongPtr, prev_button_group_active, button_group_active);
	}
	else
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR))pOnDestinationMenuDismissed)(this_ptr);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall DisplayUIHook(LONG_PTR this_ptr, LONG_PTR *button_group, LONG_PTR var3, LONG_PTR var4, DWORD dwFlags)
{
	LONG_PTR *task_group;
	WCHAR *pAppId;
	int button_group_type;
	BOOL bDecombineTemporaryShowLabels;
	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR *button_group_active;
	LONG_PTR *button_group_tracked;
	DWORD dwTaskbarPrefs;
	int nLabelListValue;
	BOOL bProcessed;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	if(nOptionsEx[OPT_EX_ALWAYS_SHOW_THUMB_LABELS])
	{
		dwFlags |= 1;
	}
	else
	{
		bProcessed = FALSE;

		task_group = (LONG_PTR *)button_group[DO2(3, 4)];
		pAppId = *EV_TASKGROUP_APPID(task_group);

		button_group_type = (int)button_group[DO2(6, 8)];
		if(
			button_group_type == 1 &&
			(int)((LONG_PTR *)button_group[DO2(5, 7)])[0] > 1 // buttons_count
		)
		{
			if(nOptions[OPT_COMBINING_DE_LABELS] == 1)
			{
				if(CheckCombineButtonGroup(button_group))
				{
					// multimonitor environment
					if(nWinVersion >= WIN_VERSION_8)
						lpMMTaskListLongPtr = button_group[3];
					else
						lpMMTaskListLongPtr = lpTaskListLongPtr;

					button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);
					button_group_tracked = TaskbarGetTrackedButtonGroup(lpMMTaskListLongPtr);

					bDecombineTemporaryShowLabels = (
						(nOptions[OPT_COMBINING_DEACTIVE] == 1 && button_group == button_group_active) ||
						(nOptions[OPT_COMBINING_DEONHOVER] == 1 && (button_group == button_group_tracked || button_group == button_group_untracked_decombined))
					);

					if(bDecombineTemporaryShowLabels)
					{
						dwTaskbarPrefs = TaskbarGetPreference(lpMMTaskListLongPtr);

						if(!(dwTaskbarPrefs & 2))
							dwFlags &= ~1;
						else if(CustomGetLabelAppidListValue(pAppId, &nLabelListValue))
						{
							if(nLabelListValue == AILIST_LABEL_NEVER)
								dwFlags |= 1;
							else // if(dwLabelListValue == AILIST_LABEL_ALWAYS)
								dwFlags &= ~1;
						}
						else // if(nTaskbarPrefs & 2)
							dwFlags &= ~1;

						bProcessed = TRUE;
					}
				}
			}
		}

		if(!bProcessed)
		{
			if(CustomGetLabelAppidListValue(pAppId, &nLabelListValue))
			{
				if(nLabelListValue == AILIST_LABEL_NEVER)
					dwFlags |= 1;
				else // if(dwLabelListValue == AILIST_LABEL_ALWAYS)
					dwFlags &= ~1;
			}
		}
	}

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR, LONG_PTR, DWORD))pDisplayUI)(this_ptr, button_group, var3, var4, dwFlags);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall GetThumbRectFromIndexHook(LONG_PTR this_ptr, int thumb_index, LONG_PTR var3, RECT *prcResult)
{
	// assert(nWinVersion <= WIN_VERSION_811)

	LONG_PTR lpMMThumbnailLongPtr;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpMMThumbnailLongPtr = this_ptr - DO5_3264(0x10, 0x20, ,, ,, ,, 0x08, 0x10);

	if(nOptionsEx[OPT_EX_LIST_REVERSE_ORDER] && thumb_index >= 0)
	{
		BOOL bShowingList = *EV_MM_THUMBNAIL_LIST_FLAG(lpMMThumbnailLongPtr);
		if(bShowingList)
		{
			LONG_PTR *plp = (LONG_PTR *)*EV_MM_THUMBNAIL_THUMBNAILS_HDPA(lpMMThumbnailLongPtr);
			int thumbs_count = (int)plp[0];
			LONG_PTR **thumbs = (LONG_PTR **)plp[1];

			int first_visible_index = *EV_MM_THUMBNAIL_LIST_FIRST_VISIBLE_INDEX(lpMMThumbnailLongPtr);

			static int nFirstIndex = -1, nLastIndex = -1;
			static BOOL bArrows = FALSE;

			if(nThumbnailListReverseHack == -3 && !bArrows)
			{
				// Don't handle -3 and -4 if there are no arrows
				nThumbnailListReverseHack = -1;
			}

			if(nThumbnailListReverseHack >= 0)
			{
				// Initial test inside ThumbIndexFromPoint, before drawing

				if(thumb_index == nThumbnailListReverseHack + first_visible_index) // a thumb
				{
					if(thumb_index == first_visible_index) // first thumb
					{
						nFirstIndex = first_visible_index;
						nLastIndex = thumbs_count - 1 - first_visible_index;
						bArrows = FALSE;
					}

					nThumbnailListReverseHack++;
				}
				else if(thumb_index == first_visible_index) // first arrow
				{
					bArrows = TRUE;
				}
				else // last arrow
				{
					nLastIndex = thumb_index;
				}
			}
			else if(nThumbnailListReverseHack == -1)
			{
				// Default, reverse

				if(nFirstIndex >= 0 && nLastIndex >= nFirstIndex)
				{
					thumb_index -= first_visible_index;
					thumb_index = nLastIndex - nFirstIndex - thumb_index;
					thumb_index += first_visible_index;
				}
				else
					thumb_index = thumbs_count - 1 - thumb_index;
			}
			else if(nThumbnailListReverseHack == -2)
			{
				// We're inside ThumbIndexFromPoint, don't reverse
			}
			else if(nThumbnailListReverseHack == -3)
			{
				// We're drawing the first arrow, return original coordinates
				nThumbnailListReverseHack = -4;
			}
			else if(nThumbnailListReverseHack == -4)
			{
				// We're drawing the second arrow, return original coordinates
				nThumbnailListReverseHack = -1;
			}
		}
	}

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, int, LONG_PTR, RECT *))pGetThumbRectFromIndex)(this_ptr, thumb_index, var3, prcResult);

	nHookProcCallCounter--;

	return lpRet;
}

static int __stdcall ThumbIndexFromPointHook(LONG_PTR this_ptr, POINT *ppt)
{
	// assert(nWinVersion <= WIN_VERSION_811)

	LONG_PTR lpMMThumbnailLongPtr;
	int nRet;

	nHookProcCallCounter++;

	lpMMThumbnailLongPtr = this_ptr - DO5_3264(0x10, 0x20, ,, ,, ,, 0x08, 0x10);

	if(nOptionsEx[OPT_EX_LIST_REVERSE_ORDER])
	{
		BOOL bShowingList = *EV_MM_THUMBNAIL_LIST_FLAG(lpMMThumbnailLongPtr);
		if(bShowingList)
		{
			if(ppt->x == -13 && ppt->y == -37) // 1337 \m/
			{
				// Drawing, check first and last item
				nThumbnailListReverseHack = 0;
			}
			else
			{
				// Indicate that we're in ThumbIndexFromPoint
				nThumbnailListReverseHack = -2;
			}
		}

		nRet = ((int(__stdcall *)(LONG_PTR, POINT *))pThumbIndexFromPoint)(this_ptr, ppt);

		if(nThumbnailListReverseHack == -2)
		{
			nThumbnailListReverseHack = -1;

			if(nRet >= 0)
			{
				// If we got a thumbnail item (not an arrow), reverse it
				nRet = ((int(__stdcall *)(LONG_PTR, POINT *))pThumbIndexFromPoint)(this_ptr, ppt);
			}
			else if(nRet == -11) // reverse arrows
				nRet = -12;
			else if(nRet == -12) // reverse arrows
				nRet = -11;
		}
		else
			nThumbnailListReverseHack = -1;
	}
	else
		nRet = ((int(__stdcall *)(LONG_PTR, POINT *))pThumbIndexFromPoint)(this_ptr, ppt);

	nHookProcCallCounter--;

	return nRet;
}

static LONG_PTR __stdcall DestroyThumbnailHook(LONG_PTR this_ptr, LONG_PTR var2)
{
	LONG_PTR lpMMThumbnailLongPtr;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR))pDestroyThumbnail)(this_ptr, var2);

	// The DestroyThumbnail function has a bug - if the destroyed thumbnail is
	// active/focused/etc., its index is not adjusted, which can cause an out
	// of bounds access later. This is more likely to happen with right drag,
	// so in order to minimize side effects, only enable it with this option.
	// Steps to reproduce:
	// * Open 3 Notepad instances.
	// * Rapidly detach and re-attach the rightmost item with the right mouse.
	//   Make sure that thumbnails are visible at that time, and try to make
	//   the rightmost item focused most of the time.
	// A crash is likely to happen with this stack trace:
	// * CTaskThumbnail::StartAnimation
	// * CTaskListThumbnailWnd::SetHotItem
	// * CTaskListWnd::_SetHotItem
	// * CTaskListWnd::_HandleMouseMove
	// * CTaskListWnd::v_WndProc
	if(nOptions[OPT_GROUPING_RIGHTDRAG] == 1)
	{
		lpMMThumbnailLongPtr = this_ptr - DO5_3264(0x18, 0x30, ,, ,, ,, 0x10, 0x20);

		LONG_PTR *plp = (LONG_PTR *)*EV_MM_THUMBNAIL_THUMBNAILS_HDPA(lpMMThumbnailLongPtr);
		if(plp)
		{
			int thumbs_count = (int)plp[0];

			int *p_active_index = EV_MM_THUMBNAIL_ACTIVE_THUMB_INDEX(lpMMThumbnailLongPtr);
			if(*p_active_index >= thumbs_count)
				*p_active_index = -10;

			int *p_tracked_index = EV_MM_THUMBNAIL_TRACKED_THUMB_INDEX(lpMMThumbnailLongPtr);
			if(*p_tracked_index >= thumbs_count)
				*p_tracked_index = -10;

			int *p_pressed_index = EV_MM_THUMBNAIL_PRESSED_THUMB_INDEX(lpMMThumbnailLongPtr);
			if(*p_pressed_index >= thumbs_count)
				*p_pressed_index = -10;
		}
	}

	nHookProcCallCounter--;

	return lpRet;
}

static HRESULT __stdcall DoesWindowMatchHook(LONG_PTR *task_group, HWND hCompareWnd, ITEMIDLIST *pCompareItemIdList,
	WCHAR *pCompareAppId, int *pnMatch, LONG_PTR **p_task_item)
{
/*

return:
S_OK if there's a match, E_FAIL otherwise

*pnMatch:
4: hWnd already exists (p_task_item)
3: ITEMIDLIST matches
2: AppId matches, ITEMIDLIST doesn't match
1: AppId matches, ITEMIDLIST is missing

*/

	BOOL bDontGroup;
	BOOL bPinned;
	int nListValue;
	LONG_PTR *plp;
	int task_items_count;
	LONG_PTR **task_items;
	int i;
	HRESULT hrRet;

	nHookProcCallCounter++;
	nDoesWindowMatchCalls++;

	hrRet = ((HRESULT(__stdcall *)(LONG_PTR *, HWND, ITEMIDLIST *, WCHAR *, int *, LONG_PTR **))pDoesWindowMatch)
		(task_group, hCompareWnd, pCompareItemIdList, pCompareAppId, pnMatch, p_task_item);

	if(SUCCEEDED(hrRet) && *pnMatch >= 1 && *pnMatch <= 3) // itemlist or appid match
	{
		bDontGroup = FALSE;
		bPinned = (!task_group[4] || (int)((LONG_PTR *)task_group[4])[0] == 0);

		if(bPinned)
		{
			if(GetAppidListValue(AILIST_GROUPPINNED, pCompareAppId, &nListValue))
			{
				if(nListValue == AILIST_GROUPPINNED_NEVER)
					bDontGroup = TRUE;
			}
			else if(nOptions[OPT_GROUPING_NOPINNED] == 1)
				bDontGroup = TRUE;
		}
		else if(!IsAppIdARandomGroup(pCompareAppId))
		{
			if(GetAppidListValue(AILIST_GROUP, pCompareAppId, &nListValue))
			{
				if(nListValue == AILIST_GROUP_NEVER)
					bDontGroup = TRUE;
			}
			else if(nOptions[OPT_GROUPING] == 1)
				bDontGroup = TRUE;

			if(bDontGroup)
			{
				// Group thumbnail windows (tested on IE9)
				plp = (LONG_PTR *)task_group[4];
				if(plp)
				{
					task_items_count = (int)plp[0];
					task_items = (LONG_PTR **)plp[1];

					if(hCreatedThumb && hCreatedThumbParent && hCompareWnd == hCreatedThumb)
					{
						for(i = 0; i < task_items_count; i++)
						{
							plp = task_items[i];
							if(GetTaskItemWnd(plp) == hCreatedThumbParent)
							{
								bDontGroup = FALSE;
								break;
							}
						}
					}
					else if(hCompareWnd)
					{
						for(i = 0; i < task_items_count; i++)
						{
							plp = *EV_TASKITEM_CONTAINER_TASK_ITEM(task_items[i]);
							if(plp && GetTaskItemWnd(plp) == hCompareWnd)
							{
								bDontGroup = FALSE;
								break;
							}
						}
					}
				}
			}
		}

		if(bDontGroup)
			hrRet = E_FAIL;
	}

	nDoesWindowMatchCalls--;
	nHookProcCallCounter--;

	return hrRet;
}

static BOOL FindTaskGroupPairInArrayWithSameAppId(LONG_PTR *task_group, LONG_PTR ***pp_task_group, LONG_PTR ***pp_task_group_other)
{
	const WCHAR *pszAppId = *EV_TASKGROUP_APPID(task_group);
	if(!pszAppId || *pszAppId == L'\0')
		return FALSE;

	LONG_PTR *plp = (LONG_PTR *)*EV_TASK_SW_TASK_GROUPS_HDPA();
	if(!plp || (int)plp[0] == 0)
		return FALSE;

	int task_groups_count = (int)plp[0];
	LONG_PTR **task_groups = (LONG_PTR **)plp[1];

	BOOL bFound = FALSE, bFoundOther = FALSE;

	for(int i = 0; i < task_groups_count; i++)
	{
		if(!task_groups[i])
			continue;

		if(!bFound)
		{
			if(task_groups[i] == task_group)
			{
				*pp_task_group = task_groups + i;
				bFound = TRUE;
				if(bFoundOther)
					break;

				continue;
			}
		}

		if(!bFoundOther)
		{
			const WCHAR *pszCompareAppId = *EV_TASKGROUP_APPID(task_groups[i]);
			if(pszCompareAppId && lstrcmp(pszAppId, pszCompareAppId) == 0)
			{
				*pp_task_group_other = task_groups + i;
				bFoundOther = TRUE;
				if(bFound)
					break;

				continue;
			}
		}
	}

	return bFound && bFoundOther;
}

static int __stdcall TaskItemSetWindowHook(LONG_PTR *task_item, HWND hNewWnd)
{
	HWND hOldWnd;
	int nRet;

	nHookProcCallCounter++;

	if(nOptionsEx[OPT_EX_FIX_HANG_REPOSITION])
	{
		hOldWnd = GetTaskItemWnd(task_item);

		if(hNewWnd != hOldWnd && IsGhostWindowClass(hNewWnd) && HungWindowFromGhostWindow_(hNewWnd) == hOldWnd)
			hNewWnd = (HWND)((LONG_PTR)hOldWnd | 1);
	}

	nRet = ((int(__stdcall *)(LONG_PTR *, HWND))pTaskItemSetWindow)(task_item, hNewWnd);

	nHookProcCallCounter--;

	return nRet;
}

static HWND __stdcall TaskItemGetWindowHook(LONG_PTR *task_item)
{
	HWND hRetWnd;
	HWND hGhostWnd;

	nHookProcCallCounter++;

	hRetWnd = ((HWND(__stdcall *)(LONG_PTR *))pTaskItemGetWindow)(task_item);

	if(bTaskItemGetWindowReturnNull && nDoesWindowMatchCalls == 0)
	{
		// We use this for a UWP hack.
		// A CImmersiveApp instance is going to be returned,
		// save it for later and replace it with NULL.
		lpTaskItemGetWindowSavedValue = (LONG_PTR)hRetWnd;
		hRetWnd = NULL;
	}
	else
	{
		if(nOptionsEx[OPT_EX_FIX_HANG_REPOSITION])
		{
			if((LONG_PTR)hRetWnd & 1)
			{
				hRetWnd = (HWND)((LONG_PTR)hRetWnd & ~1);

				hGhostWnd = GhostWindowFromHungWindow_(hRetWnd);
				if(!hGhostWnd)
				{
					*EV_TASKITEM_WND(task_item) = hRetWnd;
				}
				else
					hRetWnd = hGhostWnd;
			}
		}
	}

	nHookProcCallCounter--;

	return hRetWnd;
}

static LONG_PTR __stdcall ButtonGroupRemoveTaskItemHook(LONG_PTR *button_group, LONG_PTR *task_item)
{
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	LONG_PTR *task_group = (LONG_PTR *)button_group[DO2(3, 4)];
	if(task_group)
	{
		DWORD dwFlags = *EV_TASKGROUP_FLAGS(task_group);
		if(dwFlags & 1) // if marked as pinned
		{
			LONG_PTR **p_task_group, **p_task_group_other;
			if(FindTaskGroupPairInArrayWithSameAppId(task_group, &p_task_group, &p_task_group_other))
			{
				LONG_PTR *task_group_other = *p_task_group_other;
				DWORD dwOtherFlags = *EV_TASKGROUP_FLAGS(task_group_other);
				if(!(dwOtherFlags & 1)) // if not marked as pinned
				{
					// it's marked as pinned, but there are other groups with the same appid; swap pinned flag
					*EV_TASKGROUP_FLAGS(task_group) &= ~1;
					*EV_TASKGROUP_FLAGS(task_group_other) |= 1;

					*p_task_group = task_group_other;
					*p_task_group_other = task_group;
				}
			}
		}
	}

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR *, LONG_PTR *))pButtonGroupRemoveTaskItem)
		(button_group, task_item);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall GetIdealSpanHook(LONG_PTR *button_group, LONG_PTR var2, LONG_PTR var3,
	LONG_PTR var4, LONG_PTR var5, LONG_PTR var6)
{
	int selective_combining_hack_eof;
	int *p_button_group_type;
	int button_group_type;
	BOOL bTypeModified;
	int old_type;
	DWORD dwOldUserPrefSetBits, dwOldUserPrefRemoveBits;
	LONG_PTR lpRet;

	nHookProcCallCounter++;
	bInGetIdealSpan = TRUE;

	selective_combining_hack_eof = 0;
	if(selective_combining_hack >= 1)
	{
		if(selective_combining_hack <= 3)
			selective_combining_hack_eof = 1;

		selective_combining_hack = 0;
	}

	bTypeModified = FALSE;

	dwOldUserPrefSetBits = dwUserPrefSetBits;
	dwOldUserPrefRemoveBits = dwUserPrefRemoveBits;

	p_button_group_type = (int *)&button_group[DO2(6, 8)];
	button_group_type = *p_button_group_type;
	if(button_group_type == 1 || button_group_type == 3)
	{
		ButtonGroupSetPrefOnUpdate(button_group, FALSE);
	}
	else if(button_group_type == 2)
	{
		if(nOptions[OPT_PINNED_REMOVEGAP] == 1)
		{
			dwUserPrefRemoveBits |= 2;

			old_type = *p_button_group_type;
			*p_button_group_type = 4;
			bTypeModified = TRUE;
		}
	}

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR *, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR))pGetIdealSpan)
		(button_group, var2, var3, var4, var5, var6);

	if(bTypeModified)
		*p_button_group_type = old_type;

	dwUserPrefSetBits = dwOldUserPrefSetBits;
	dwUserPrefRemoveBits = dwOldUserPrefRemoveBits;

	if(selective_combining_hack_eof)
		selective_combining_hack = selective_combining_hack_eof;

	bInGetIdealSpan = FALSE;
	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall SetLocationHook(LONG_PTR *button_group, LONG_PTR var2, LONG_PTR var3, RECT *prc)
{
	static int nLastItemCounter = -1;
	static int nTop, nLeft, nIndex, nFirstRowLastIndex;
	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR **button_groups;
	int button_groups_count;
	int button_group_type;
	//LONG_PTR **buttons;
	int buttons_count;
	RECT rcNew;
	RECT *prcSrcRect;
	int nTaskbarPos;
	LONG_PTR *plp, lp;
	int i;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	if(nOptionsEx[OPT_EX_MULTIROW_EQUAL_WIDTH])
	{
		// multimonitor environment
		if(nWinVersion >= WIN_VERSION_8)
			lpMMTaskListLongPtr = button_group[3];
		else
			lpMMTaskListLongPtr = lpTaskListLongPtr;

		if(lpMMTaskListLongPtr != lpTaskListLongPtr)
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

		if(nTaskbarPos == 1 || nTaskbarPos == 3) // Is taskbar on top/bottom of the screen
		{
			plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
			if(plp && (int)plp[0] > 0)
			{
				button_groups_count = (int)plp[0];
				button_groups = (LONG_PTR **)plp[1];

				// Is the target button_group is the first one on the taskbar
				if(button_group == button_groups[0])
				{
					for(i = 0; i < button_groups_count; i++)
					{
						button_group_type = (int)button_groups[i][DO2(6, 8)];
						if(button_group_type != 1)
							break;

						plp = (LONG_PTR *)button_groups[i][DO2(5, 7)];

						buttons_count = (int)plp[0];
						//buttons = (LONG_PTR **)plp[1];

						if(buttons_count != 1)
							break;
					}

					if(i == button_groups_count)
					{
						nLastItemCounter = 0;

						nTop = prc->top;
						nLeft = prc->left;
						nIndex = 0;
						nFirstRowLastIndex = 0;
					}
					else
						nLastItemCounter = -1;
				}
				else if(nLastItemCounter >= 0)
				{
					if(prc->top != nTop)
					{
						if(nFirstRowLastIndex == 0)
							nFirstRowLastIndex = nIndex;

						nTop = prc->top;
						nIndex = 0;
					}
					else if(prc->left <= nLeft)
						nIndex = 0;
					else
						nIndex++;

					nLeft = prc->left;

					if(nFirstRowLastIndex > 0 && nIndex <= nFirstRowLastIndex)
					{
						rcNew = *prc;
						prc = &rcNew;

						prcSrcRect = *(RECT **)(button_groups[nIndex][DO2(4, 6)] + sizeof(LONG_PTR));

						prc->left = prcSrcRect->left;
						prc->right = prcSrcRect->right;
					}

					if(button_group == button_groups[button_groups_count - 1])
					{
						if(nLastItemCounter == 1)
							nLastItemCounter = -1;
						else
							nLastItemCounter++;
					}
				}
			}
			else
				nLastItemCounter = -1;
		}
	}

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR *, LONG_PTR, LONG_PTR, RECT *))pSetLocation)(button_group, var2, var3, prc);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall RenderHook(LONG_PTR *button_group, LONG_PTR var2, LONG_PTR var3, LONG_PTR var4, LONG_PTR var5)
{
	// assert(nWinVersion <= WIN_VERSION_10_R3)

	int button_group_type;
	DWORD dwOldUserPrefSetBits, dwOldUserPrefRemoveBits;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	dwOldUserPrefSetBits = dwUserPrefSetBits;
	dwOldUserPrefRemoveBits = dwUserPrefRemoveBits;

	button_group_type = (int)button_group[DO2(6, 8)];
	if(button_group_type == 1 || button_group_type == 3)
		ButtonGroupSetPrefOnUpdate(button_group, TRUE);

	if(decombine_without_labels_hack)
	{
		if(nWinVersion <= WIN_VERSION_811)
			PointerRedirectionAdd(ppDrawThemeBackground, DrawThemeBackgroundHook, &prDrawThemeBackground);

		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR *, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR))pRender)
			(button_group, var2, var3, var4, var5);

		if(nWinVersion <= WIN_VERSION_811)
			PointerRedirectionRemove(ppDrawThemeBackground, &prDrawThemeBackground);

		decombine_without_labels_hack = 0;
	}
	else
	{
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR *, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR))pRender)
			(button_group, var2, var3, var4, var5);
	}

	dwUserPrefSetBits = dwOldUserPrefSetBits;
	dwUserPrefRemoveBits = dwOldUserPrefRemoveBits;

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall RenderHook2(LONG_PTR *button_group, LONG_PTR var2, LONG_PTR var3, LONG_PTR var4, LONG_PTR var5, LONG_PTR var6)
{
	// assert(nWinVersion >= WIN_VERSION_10_R4)

	int button_group_type;
	DWORD dwOldUserPrefSetBits, dwOldUserPrefRemoveBits;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	dwOldUserPrefSetBits = dwUserPrefSetBits;
	dwOldUserPrefRemoveBits = dwUserPrefRemoveBits;

	button_group_type = (int)button_group[DO2(6, 8)];
	if(button_group_type == 1 || button_group_type == 3)
		ButtonGroupSetPrefOnUpdate(button_group, TRUE);

	if(decombine_without_labels_hack)
	{
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR *, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR))pRender)
			(button_group, var2, var3, var4, var5, var6);

		decombine_without_labels_hack = 0;
	}
	else
	{
		lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR *, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR, LONG_PTR))pRender)
			(button_group, var2, var3, var4, var5, var6);
	}

	dwUserPrefSetBits = dwOldUserPrefSetBits;
	dwUserPrefRemoveBits = dwOldUserPrefRemoveBits;

	nHookProcCallCounter--;

	return lpRet;
}

static BOOL __stdcall ButtonGroupCanGlomHook(LONG_PTR *button_group)
{
	int selective_combining_hack_eof;
	int button_group_type;
	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR *button_group_active;
	LONG_PTR *button_group_tracked;
	BOOL bDecombineTemporary;
	BOOL bRet;

	nHookProcCallCounter++;

	selective_combining_hack_eof = 0;
	if(selective_combining_hack >= 1)
	{
		if(selective_combining_hack <= 3)
			selective_combining_hack_eof = 1;

		selective_combining_hack = 0;
	}

	button_group_type = (int)button_group[DO2(6, 8)];
	if(
		button_group_type == 1 &&
		(int)((LONG_PTR *)button_group[DO2(5, 7)])[0] > 1 // buttons_count
	)
	{
		bRet = CheckCombineButtonGroup(button_group);
		if(bRet)
		{
			// multimonitor environment
			if(nWinVersion >= WIN_VERSION_8)
				lpMMTaskListLongPtr = button_group[3];
			else
				lpMMTaskListLongPtr = lpTaskListLongPtr;

			button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);
			button_group_tracked = TaskbarGetTrackedButtonGroup(lpMMTaskListLongPtr);

			bDecombineTemporary = (
				(nOptions[OPT_COMBINING_DEACTIVE] == 1 && button_group == button_group_active) ||
				(nOptions[OPT_COMBINING_DEONHOVER] == 1 && (button_group == button_group_tracked || button_group == button_group_untracked_decombined))
			);

			// For WIN_VERSION_10_R1, keep the group decombined while we move it on the taskbar.
			if(!bDecombineTemporary &&
				nWinVersion >= WIN_VERSION_10_R1 &&
				nOptions[OPT_COMBINING_DEACTIVE] == 1 &&
				*EV_MM_TASKLIST_DRAG_FLAG(lpMMTaskListLongPtr) == 1 &&
				button_group == *EV_MM_TASKLIST_PRESSED_BUTTON_GROUP(lpMMTaskListLongPtr))
			{
				bDecombineTemporary = TRUE;
			}

			if(bDecombineTemporary)
				bRet = FALSE;
		}
	}
	else
		bRet = FALSE;

	//bRet = ((BOOL (__stdcall *)(LONG_PTR *))pButtonGroupCanGlom)(button_group);

	if(selective_combining_hack_eof)
		selective_combining_hack = selective_combining_hack_eof;

	nHookProcCallCounter--;

	return bRet;
}

static void ButtonGroupSetPrefOnUpdate(LONG_PTR *button_group, BOOL bDecombineWithoutLabelsHack)
{
	LONG_PTR *task_group;
	WCHAR *pAppId;
	int button_group_type;
	LONG_PTR lpMMTaskListLongPtr;
	DWORD dwTaskbarPrefs;
	BOOL bDecombineTemporaryShowLabels;
	LONG_PTR *button_group_active;
	LONG_PTR *button_group_tracked;
	int nLabelListValue;

	task_group = (LONG_PTR *)button_group[DO2(3, 4)];
	pAppId = *EV_TASKGROUP_APPID(task_group);

	button_group_type = (int)button_group[DO2(6, 8)];
	if(
		button_group_type == 1 &&
		(int)((LONG_PTR *)button_group[DO2(5, 7)])[0] > 1 // buttons_count
	)
	{
		// multimonitor environment
		if(nWinVersion >= WIN_VERSION_8)
			lpMMTaskListLongPtr = button_group[3];
		else
			lpMMTaskListLongPtr = lpTaskListLongPtr;

		dwTaskbarPrefs = TaskbarGetPreference(lpMMTaskListLongPtr);

		bDecombineTemporaryShowLabels = FALSE;

		if(nOptions[OPT_COMBINING_DE_LABELS] == 1)
		{
			if(CheckCombineButtonGroup(button_group))
			{
				button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);
				button_group_tracked = TaskbarGetTrackedButtonGroup(lpMMTaskListLongPtr);

				bDecombineTemporaryShowLabels = (
					(nOptions[OPT_COMBINING_DEACTIVE] == 1 && button_group == button_group_active) ||
					(nOptions[OPT_COMBINING_DEONHOVER] == 1 && (button_group == button_group_tracked || button_group == button_group_untracked_decombined))
				);
			}
		}

		if(bDecombineTemporaryShowLabels && !(dwTaskbarPrefs & 2))
		{
			dwUserPrefRemoveBits |= 2;
		}
		else if(CustomGetLabelAppidListValue(pAppId, &nLabelListValue))
		{
			if(nLabelListValue == AILIST_LABEL_NEVER)
			{
				if(bDecombineWithoutLabelsHack)
				{
					dwUserPrefRemoveBits |= 2;
					decombine_without_labels_hack = 1;
				}
				else
					dwUserPrefSetBits |= 2;
			}
			else // if(dwLabelListValue == AILIST_LABEL_ALWAYS)
				dwUserPrefRemoveBits |= 2;
		}
		else if(bDecombineTemporaryShowLabels && (dwTaskbarPrefs & 2))
		{
			dwUserPrefRemoveBits |= 2;
		}
		else if(bDecombineWithoutLabelsHack && (dwTaskbarPrefs & 2))
		{
			dwUserPrefRemoveBits |= 2;
			decombine_without_labels_hack = 1;
		}
	}
	else
	{
		if(CustomGetLabelAppidListValue(pAppId, &nLabelListValue))
		{
			if(nLabelListValue == AILIST_LABEL_NEVER)
				dwUserPrefSetBits |= 2;
			else // if(dwLabelListValue == AILIST_LABEL_ALWAYS)
				dwUserPrefRemoveBits |= 2;
		}
	}
}

static HRESULT __stdcall DrawThemeBackgroundHook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect)
{
	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		switch(iPartId)
		{
		case 5:
		case 6:
		case 7:
		case 8:
			if(decombine_without_labels_hack == 1)
				decombine_without_labels_hack = 2;
			break;
		}
	}

	return ((HRESULT(__stdcall *)(HTHEME, HDC, int, int, LPCRECT, LPCRECT))prDrawThemeBackground.pOriginalAddress)
		(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

static LONG_PTR __stdcall ButtonGroupHotTrackingHook(LONG_PTR *button_group, BOOL bNewGroup, int button_index, BOOL bAnimation)
{
	// assert(nWinVersion <= WIN_VERSION_811)

	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR *, BOOL, int, BOOL))pButtonGroupHotTracking)
		(button_group, bNewGroup, button_index, bAnimation);

	OnButtonGroupHotTracking(button_group, bNewGroup, button_index);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall ButtonGroupHotTrackOutHook(LONG_PTR *button_group, BOOL bAnimation)
{
	// assert(nWinVersion <= WIN_VERSION_811)

	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR *, BOOL))pButtonGroupHotTrackOut)
		(button_group, bAnimation);

	OnButtonGroupHotTrackOut(button_group);

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall ButtonGroupStartItemAnimationHook(LONG_PTR *button_group, int nAnimationInfo, int nAnimationType)
{
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR *, int, int))pButtonGroupStartItemAnimation)
		(button_group, nAnimationInfo, nAnimationType);

	// Do nothing here for now

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall ButtonGroupHasItemAnimationHook(LONG_PTR *button_group, int nAnimationInfo, int nAnimationType, int *pnResult)
{
	static LONG_PTR *tracked_button_group;
	static int tracked_button_index;
	static LONG_PTR *active_button_group;
	static int active_button_index;
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR *, int, int, int *))pButtonGroupHasItemAnimation)
		(button_group, nAnimationInfo, nAnimationType, pnResult);

	int nAnimationTypeHotTracking;
	int nAnimationTypeHotTrackOut;
	int nAnimationTypeActivate;
	int nAnimationTypeDeactivate;

	// CTaskListWnd::_StartPlateAnimations
	if(nWinVersion >= WIN_VERSION_11_21H2)
	{
		nAnimationTypeHotTracking = 14;
		nAnimationTypeHotTrackOut = 15;
		nAnimationTypeActivate = 12;
		nAnimationTypeDeactivate = 13;
	}
	else
	{
		nAnimationTypeHotTracking = 12;
		nAnimationTypeHotTrackOut = 13;
		nAnimationTypeActivate = 10;
		nAnimationTypeDeactivate = 11;
	}

	if(nAnimationType == nAnimationTypeHotTracking)
	{
		OnButtonGroupHotTracking(button_group, tracked_button_group != button_group, nAnimationInfo);

		// nAnimationInfo is -2 for a combined group or for a pinned item.
		// Set to 0 because things can change on track out (e.g. the pinned item is now running, or the combined group is now a single item).
		tracked_button_group = button_group;
		tracked_button_index = (nAnimationInfo == -2) ? 0 : nAnimationInfo;
	}
	else if(nAnimationType == nAnimationTypeHotTrackOut) // Hot track out
	{
		if(tracked_button_group == button_group && tracked_button_index == ((nAnimationInfo == -2) ? 0 : nAnimationInfo))
		{
			tracked_button_group = NULL;
			tracked_button_index = 0;
		}

		if(tracked_button_group != button_group)
		{
			OnButtonGroupHotTrackOut(button_group);
		}
	}
	else if(nWinVersion >= WIN_VERSION_10_R1 && nOptions[OPT_COMBINING_DEACTIVE] == 1)
	{
		if(nAnimationType == nAnimationTypeActivate)
		{
			LONG_PTR lpMMTaskListLongPtr = button_group[3];
			ButtonGroupActivated(lpMMTaskListLongPtr, button_group);

			active_button_group = button_group;
			active_button_index = nAnimationInfo;
		}
		else if(nAnimationType == nAnimationTypeDeactivate)
		{
			if(active_button_group == button_group && active_button_index == nAnimationInfo)
			{
				active_button_group = NULL;
				active_button_index = 0;
			}

			if(active_button_group != button_group)
			{
				LONG_PTR lpMMTaskListLongPtr = button_group[3];
				ButtonGroupDeactivated(lpMMTaskListLongPtr, button_group);
			}
		}
	}

	nHookProcCallCounter--;

	return lpRet;
}

static void OnButtonGroupHotTracking(LONG_PTR *button_group, BOOL bNewGroup, int button_index)
{
	if(button_index != -1)
	{
		if(lpUntrackedDecombinedTaskListLongPtr && button_group_untracked_decombined)
		{
			if(nOptions[OPT_COMBINING_DEONHOVER] == 1 && button_group_untracked_decombined != button_group)
				CombineUntrackedDecombined(lpUntrackedDecombinedTaskListLongPtr, button_group_untracked_decombined);

			// We reset them several lines below.
			//lpUntrackedDecombinedTaskListLongPtr = 0;
			//button_group_untracked_decombined = NULL;
		}

		if(bNewGroup && nOptions[OPT_COMBINING_DEONHOVER] == 1)
		{
			// We set button_group_untracked_decombined here because sometimes
			// button_group is not set as tracked yet at this stage.
			// In this case, CanGlom will return FALSE and ButtonGroupCombine will fail.
			button_group_untracked_decombined = button_group;

			// multimonitor environment
			if(nWinVersion >= WIN_VERSION_8)
			{
				lpUntrackedDecombinedTaskListLongPtr = button_group[3];
			}
			else
			{
				lpUntrackedDecombinedTaskListLongPtr = lpTaskListLongPtr;
			}

			ButtonGroupCombine(button_group, FALSE);
		}

		lpUntrackedDecombinedTaskListLongPtr = 0;
		button_group_untracked_decombined = NULL;

		if(nOptions[OPT_GROUPING_RIGHTDRAG] == 1 && hDragWithinGroupsWnd)
		{
			if(DragWithinGroup(button_group, button_index) || DragBetweenGroups(button_group))
				SetCursor(LoadCursor(NULL, IDC_ARROW));
			else
				SetCursor(LoadCursor(NULL, IDC_NO));

			bDragWithinGroupsDone = TRUE;
		}
	}
}

static void OnButtonGroupHotTrackOut(LONG_PTR *button_group)
{
	LONG_PTR lpMMTaskListLongPtr;
	HWND hMMTaskListWnd;
	int button_group_type;
	int button_count;
	LONG_PTR *button_group_active;
	LONG_PTR *button_group_tracked;
	BOOL bUntrackedDecombined;
	LONG_PTR *plp;
	RECT *prcGroup;
	POINT ptCursor;
	WCHAR szAppId[sizeof("random_group_4294967295")];

	// multimonitor environment
	if(nWinVersion >= WIN_VERSION_8)
	{
		lpMMTaskListLongPtr = button_group[3];
		hMMTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);
	}
	else
	{
		lpMMTaskListLongPtr = lpTaskListLongPtr;
		hMMTaskListWnd = hTaskListWnd;
	}

	if(nOptions[OPT_COMBINING_DEONHOVER] == 1)
	{
		bUntrackedDecombined = FALSE;

		if(GetCapture() == hMMTaskListWnd && (nOptions[OPT_GROUPING_RIGHTDRAG] == 0 || !hDragWithinGroupsWnd))
		{
			bUntrackedDecombined = TRUE;
		}
		else
		{
			// Fixes combine on close
			button_group_type = (int)button_group[DO2(6, 8)];
			if(button_group_type == 1)
			{
				plp = (LONG_PTR *)button_group[DO2(5, 7)];

				button_count = (int)plp[0];
				if(button_count > 1)
				{
					GetCursorPos(&ptCursor);

					if(WindowFromPoint(ptCursor) == hMMTaskListWnd)
					{
						prcGroup = *(RECT **)(button_group[DO2(4, 6)] + sizeof(LONG_PTR));
						MapWindowPoints(NULL, hMMTaskListWnd, &ptCursor, 1);

						if(PtInRect(prcGroup, ptCursor))
							bUntrackedDecombined = TRUE;
					}
				}
			}
		}

		if(bUntrackedDecombined)
		{
			lpUntrackedDecombinedTaskListLongPtr = lpMMTaskListLongPtr;
			button_group_untracked_decombined = button_group;
		}
		else
		{
			button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

			if(nOptions[OPT_COMBINING_DEACTIVE] == 0 || button_group != button_group_active)
				ButtonGroupCombine(button_group, TRUE);
		}
	}

	if(nOptions[OPT_GROUPING_RIGHTDRAG] == 1 && hDragWithinGroupsWnd)
	{
		button_group_tracked = TaskbarGetTrackedButtonGroup(lpMMTaskListLongPtr);
		if(!button_group_tracked)
		{
			wsprintf(szAppId, L"random_group_%u", GetTickCount());
			if(WndSetAppId(hDragWithinGroupsWnd, szAppId))
			{
				lpRightDragDetachTaskListLongPtr = lpMMTaskListLongPtr;
				right_drag_detach_group = ButtonGroupNextToMousePos(lpMMTaskListLongPtr, hDragWithinGroupsWnd);

				SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
			else
				SetCursor(LoadCursor(NULL, IDC_NO));

			bDragWithinGroupsDone = TRUE;
		}
	}
}

static BOOL __stdcall ShouldShowToolTipHook(LONG_PTR *button_group, LONG_PTR *task_item)
{
	BOOL bRet;

	nHookProcCallCounter++;

	if(nOptionsEx[OPT_EX_ALWAYS_SHOW_TOOLTIP] == 1)
		bRet = TRUE;
	else if(nOptionsEx[OPT_EX_ALWAYS_SHOW_TOOLTIP] == 2)
		bRet = FALSE;
	else
		bRet = ((BOOL(__stdcall *)(LONG_PTR *, LONG_PTR *))pShouldShowToolTip)(button_group, task_item);

	nHookProcCallCounter--;

	return bRet;
}

static LONG_PTR __stdcall SecondaryGetUserPreferencesHook(LONG_PTR var1, DWORD *pdwPreferences)
{
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR, DWORD *))pSecondaryGetUserPreferences)(var1, pdwPreferences);

	*pdwPreferences = ManipulateUserPreferences(*pdwPreferences, _AddressOfReturnAddress());

	nHookProcCallCounter--;

	return lpRet;
}

static LONG_PTR __stdcall SecondaryIsHorizontalHook(LONG_PTR this_ptr)
{
	LONG_PTR lpRet;

	nHookProcCallCounter++;

	if(nWinVersion <= WIN_VERSION_811)
	{
		// The magic happens in: CTaskListWnd::_CheckNeedScrollbars
		if(selective_combining_hack == 1)
			selective_combining_hack = 2;
		else if(selective_combining_hack >= 3)
			selective_combining_hack = 1;
	}

	lpRet = ((LONG_PTR(__stdcall *)(LONG_PTR))pSecondaryIsHorizontal)(this_ptr);

	nHookProcCallCounter--;

	return lpRet;
}

static DWORD ManipulateUserPreferences(DWORD dwPreferences, void **ppAddressOfReturnAddress)
{
	DWORD dwNewPreferences = dwPreferences;

	dwNewPreferences |= dwUserPrefSetBits;
	dwNewPreferences &= ~dwUserPrefRemoveBits;

	void *pReturnAddress = *ppAddressOfReturnAddress;
	BOOL bCalledFromExplorer =
		(ULONG_PTR)pReturnAddress >= (ULONG_PTR)ExplorerModuleInfo.lpBaseOfDll &&
		(ULONG_PTR)pReturnAddress < (ULONG_PTR)ExplorerModuleInfo.lpBaseOfDll + ExplorerModuleInfo.SizeOfImage;

	if(!bCalledFromExplorer)
	{
		if(decombine_without_labels_hack)
		{
			// If GetUserPreferences wasn't called from explorer,
			// return the intended value for tools like StartIsBack.
			dwNewPreferences |= 2;
		}

		return dwNewPreferences;
	}

	if(nWinVersion >= WIN_VERSION_10_T1 && decombine_without_labels_hack == 1)
	{
		// Assembly code below is from CTaskBtnGroup::_DrawRegularButton

		BYTE *pbCode = (BYTE *)pReturnAddress;

#ifdef _WIN64
		if(nWinVersion == WIN_VERSION_10_T1)
		{
			// 8B 54 24 XX              | mov edx,dword ptr ss:[rsp+XX]
			// F6 C2 02                 | test dl,2
			if(pbCode[0] == 0x8B && pbCode[1] == 0x54 && pbCode[2] == 0x24 &&
				pbCode[4] == 0xF6 && pbCode[5] == 0xC2 && pbCode[6] == 0x02)
			{
				pbCode += 7;
			}
			else
				pbCode = NULL;

			// 0F 84 XX XX XX XX        | je XXXXXXXX
			// 89 7C 24 XX              | mov dword ptr ss:[rsp+XX],edi
			if(pbCode &&
				pbCode[0] == 0x0F && pbCode[1] == 0x84 &&
				pbCode[6] == 0x89 && pbCode[7] == 0x7C && pbCode[8] == 0x24)
			{
				decombine_without_labels_hack = 2;
			}
		}
		else if(nWinVersion == WIN_VERSION_10_T2)
		{
			// 8B 54 24 XX              | mov edx,dword ptr ss:[rsp+XX]
			// F6 C2 02                 | test dl,2
			if(pbCode[0] == 0x8B && pbCode[1] == 0x54 && pbCode[2] == 0x24 &&
				pbCode[4] == 0xF6 && pbCode[5] == 0xC2 && pbCode[6] == 0x02)
			{
				pbCode += 7;
			}
			else
				pbCode = NULL;

			// 0F85 XXXXXXXX | JNZ XXXXXXXX
			if(pbCode &&
				pbCode[0] == 0x0F && pbCode[1] == 0x85)
			{
				INT32 nOffset = *(INT32 *)(pbCode + 2);
				pbCode += 6;
				pbCode += nOffset;
			}
			// 75 XX | JNZ SHORT XXXXXXXX
			else if(pbCode &&
				pbCode[0] == 0x75)
			{
				INT8 nOffset = *(INT8 *)(pbCode + 1);
				pbCode += 2;
				pbCode += nOffset;
			}
			else
				pbCode = NULL;

			// 44 89 74 24 XX | mov dword ptr ss:[rsp+XX],r14d
			if(pbCode &&
				pbCode[0] == 0x44 && pbCode[1] == 0x89 && pbCode[2] == 0x74 && pbCode[3] == 0x24)
			{
				decombine_without_labels_hack = 2;
			}
		}
		else if(nWinVersion <= WIN_VERSION_10_R3)
		{
			/*
				lea rax,qword ptr ss:[rbp-18]
				mov qword ptr ss:[rsp+20],rax
				mov rbx,qword ptr ss:[rbp-40]
				mov r9,rbx
				mov r8b,1
				mov rdx,r12
				mov rcx,r14
				call <explorer.private: void __cdecl CTaskBtnGroup::_GetStatesFromRenderInfo(struct BUTTONRE
				lea r9,qword ptr ss:[rbp-18]
				mov r8,r12
				mov rdx,r13
				mov rcx,r14
				call <explorer.private: void __cdecl CTaskBtnGroup::_DrawBasePlate(struct HDC__ * __ptr64,st
				lea r9,qword ptr ss:[rbp-18]
				mov r8,r12
				mov rdx,r13
				mov rcx,r14
				call <explorer.private: void __cdecl CTaskBtnGroup::_DrawIndicator(struct HDC__ * __ptr64,st
				mov eax,dword ptr ss:[rbp-C]
				cmp eax,FFFF
				je explorer.7FF75FBBFA50
				test eax,eax
				jne explorer.7FF75FBBFA68
				lea r9,qword ptr ss:[rbp-18]
				mov r8,r12
				mov rdx,r13
				mov rcx,r14
				call <explorer.private: void __cdecl CTaskBtnGroup::_DrawBar(struct HDC__ * __ptr64,struct B
				cmp dword ptr ds:[r12+E8],0
				jne explorer.7FF75FB2C78C
				mov rax,qword ptr ds:[r14+28]
				mov ecx,edi
				mov dword ptr ss:[rbp-70],ecx
				mov r8,qword ptr ds:[rax+60]
				test r8,r8
				je explorer.7FF75FB2BAA4
				mov rax,qword ptr ds:[r8]
				mov rax,qword ptr ds:[rax+30]
				lea r9,qword ptr ds:[<public: virtual long __cdecl CTaskBand::GetUserPreferences(unsigned lo
				cmp rax,r9
				jne explorer.7FF75FBBFB96 <---------------------------------------------- We're hooking THIS
				mov ecx,dword ptr ds:[r8+48]
				mov dword ptr ss:[rbp-70],ecx
				test cl,2
			*/

			// 8B 4D XX                 | mov ecx,dword ptr ss:[rbp-XX]
			if(pbCode[0] == 0x8B && pbCode[1] == 0x4D)
			{
				pbCode += 3;
			}
			else
				pbCode = NULL;

			// Optional:
			// E9 XXXXXXXX | jmp XXXXXXXX
			if(pbCode &&
				pbCode[0] == 0xE9)
			{
				INT32 nOffset = *(INT32 *)(pbCode + 1);
				pbCode += 5;
				pbCode += nOffset;
			}

			// F6 C1 02                 | test cl,2
			if(pbCode &&
				pbCode[0] == 0xF6 && pbCode[1] == 0xC1 && pbCode[2] == 0x02)
			{
				pbCode += 3;
			}
			else
				pbCode = NULL;

			// 0F 84 XX XX XX XX        | je XXXXXXXX
			// 89 7D XX                 | mov dword ptr ss:[rbp-XX],edi
			// -OR-
			// 0F 84 XX XX XX XX        | je XXXXXXXX
			// 89 7C 24 XX              | mov dword ptr ss:[rsp+XX],edi
			if(pbCode &&
				pbCode[0] == 0x0F && pbCode[1] == 0x84 &&
				pbCode[6] == 0x89 && (pbCode[7] == 0x7D || (pbCode[7] == 0x7C && pbCode[8] == 0x24)))
			{
				decombine_without_labels_hack = 2;
			}
		}
		else if(nWinVersion <= WIN_VERSION_10_R4)
		{
			// 49 8B 5D XX | mov rbx,qword ptr ds:[r13+XX]
			// 8B 44 24 XX | mov eax,dword ptr ss:[rsp+XX]
			if(pbCode[0] == 0x49 && pbCode[1] == 0x8B && pbCode[2] == 0x5D &&
				pbCode[4] == 0x8B && pbCode[5] == 0x44 && pbCode[6] == 0x24)
			{
				pbCode += 8;
			}
			else
				pbCode = NULL;

			// E9 XXXXXXXX | jmp XXXXXXXX
			if(pbCode &&
				pbCode[0] == 0xE9)
			{
				INT32 nOffset = *(INT32 *)(pbCode + 1);
				pbCode += 5;
				pbCode += nOffset;
			}
			else
				pbCode = NULL;

			// 44 8B 27       | mov r12d,dword ptr ds:[rdi]
			// A8 02          | test al,2
			// 0F 84 XXXXXXXX | je XXXXXXXX
			// 89 74 24 XX    | mov dword ptr ss:[rsp+XX],esi
			if(pbCode &&
				pbCode[0] == 0x44 && pbCode[1] == 0x8B && pbCode[2] == 0x27 &&
				pbCode[3] == 0xA8 && pbCode[4] == 0x02 &&
				pbCode[5] == 0x0F && pbCode[6] == 0x84 &&
				pbCode[11] == 0x89 && pbCode[12] == 0x74 && pbCode[13] == 0x24)
			{
				decombine_without_labels_hack = 2;
			}
		}
		else if(nWinVersion <= WIN_VERSION_SERVER_2022)
		{
			// 49 8B 5F XX | mov     rbx,qword ptr [r15+XX] ; <-- registers may differ
			// ^ 49 8B (bin: 01??????) ??
			if(pbCode[0] == 0x49 && pbCode[1] == 0x8B && (pbCode[2] & 0xC0) == 0x40)
			{
				pbCode += 4;
			}
			else
				pbCode = NULL;

			// 8B 45 XX    | mov     eax,dword ptr [rbp-XX]
			if(pbCode &&
				pbCode[0] == 0x8B && pbCode[1] == 0x45)
			{
				pbCode += 3;
			}
			else
				pbCode = NULL;

			// Optional (Win10 has it, WIN_VERSION_SERVER_2022 doesn't):
			// 41 8B 4D 00 | mov     ecx,dword ptr [r13] ; <-- registers may differ
			// ^ (41 or 45) 8B (bin: 01??????) 00
			// -or-
			// 44 8B 27    | mov     r12d,dword ptr [rdi] ; <-- registers may differ
			// ^ 44 8B (bin: 00??????)
			if(pbCode &&
				(pbCode[0] == 0x41 || pbCode[0] == 0x45) && pbCode[1] == 0x8B && (pbCode[2] & 0xC0) == 0x40 && pbCode[3] == 0x00)
			{
				pbCode += 4;
			}
			else if(pbCode &&
				pbCode[0] == 0x44 && pbCode[1] == 0x8B && (pbCode[2] & 0xC0) == 0x00)
			{
				pbCode += 3;
			}

			// Optional (early WIN_VERSION_10_20H1):
			// 89 4D XX    | mov     dword ptr [rbp-XX],ecx
			if(pbCode &&
				pbCode[0] == 0x89 && pbCode[1] == 0x4D)
			{
				pbCode += 3;
			}

			// A8 02          | test    al,2
			if(pbCode &&
				pbCode[0] == 0xA8 && pbCode[1] == 0x02)
			{
				pbCode += 2;
			}
			else
				pbCode = NULL;

			// 0F 84 XXXXXXXX | je      XXXXXXXX
			// -or-
			// 75 XX          | jne     XXXXXXXX
			if(pbCode &&
				pbCode[0] == 0x0F && pbCode[1] == 0x84)
			{
				pbCode += 6;
			}
			else if(pbCode &&
				pbCode[0] == 0x75)
			{
				INT8 nOffset = *(INT8 *)(pbCode + 1);
				pbCode += 2;
				pbCode += nOffset;
			}
			else
				pbCode = NULL;

			// 89 4C 24 XX | mov dword ptr [rsp+XX],ecx ; <-- registers may differ
			// ^ 89 (bin: 01???100) 24 XX
			// -or-
			// 44 89 64 24 XX | mov dword ptr [rsp+XX],r12d ; <-- registers may differ
			// ^ 44 89 (bin: 01???100) 24 XX
			// -or-
			// 89 4D 84 | mov dword ptr ss:[rbp-XX],ecx ; <-- registers may differ
			// ^ 89 (bin: 01???101) XX
			// -or-
			// 44 89 65 84 | mov dword ptr ss:[rbp-XX],r12d ; <-- registers may differ
			// ^ 44 89 (bin: 01???101) XX
			if(pbCode &&
				pbCode[0] == 0x89 && (pbCode[1] & 0xC7) == 0x44 && pbCode[2] == 0x24)
			{
				decombine_without_labels_hack = 2;
			}
			else if(pbCode &&
				pbCode[0] == 0x44 && pbCode[1] == 0x89 && (pbCode[2] & 0xC7) == 0x44 && pbCode[3] == 0x24)
			{
				decombine_without_labels_hack = 2;
			}
			else if(pbCode &&
				pbCode[0] == 0x89 && (pbCode[1] & 0xC7) == 0x45)
			{
				decombine_without_labels_hack = 2;
			}
			else if(pbCode &&
				pbCode[0] == 0x44 && pbCode[1] == 0x89 && (pbCode[2] & 0xC7) == 0x45)
			{
				decombine_without_labels_hack = 2;
			}
		}
		else // if(nWinVersion >= WIN_VERSION_11_21H2)
		{
			// 8B4424 XX  | mov eax,dword ptr ss:[rsp+XX]
			// 48:83C4 28 | add rsp,28
			// C3         | ret
			if(pbCode[0] == 0x8B && pbCode[1] == 0x44 && pbCode[2] == 0x24 &&
				pbCode[4] == 0x48 && pbCode[5] == 0x83 && pbCode[6] == 0xC4 && pbCode[7] == 0x28 &&
				pbCode[8] == 0xC3)
			{
				// Skip the frame of CTaskListWnd::_GetUserPreferences
				// and get the next return address.
				pbCode = *(ppAddressOfReturnAddress + 6);
			}
			else
				pbCode = NULL;

			if(nWinVersion <= WIN_VERSION_11_21H2)
			{
				// 41:84C6                  | test r14b,al
				// 75 XX                    | jne XXXXXXXX
				// 41:8B1424                | mov edx,dword ptr ds:[r12]
				// 3BD6                     | cmp edx,esi
				// 75 XX                    | jne XXXXXXXX
				// 48:85DB                  | test rbx,rbx
				// 75 XX                    | jne XXXXXXXX
				// 83FA 03                  | cmp edx,3
				// 75 XX                    | jne XXXXXXXX
				if(pbCode &&
					pbCode[0] == 0x41 && pbCode[1] == 0x84 && pbCode[2] == 0xC6 &&
					pbCode[3] == 0x75 &&
					pbCode[5] == 0x41 && pbCode[6] == 0x8B && pbCode[7] == 0x14 && pbCode[8] == 0x24 &&
					pbCode[9] == 0x3B && pbCode[10] == 0xD6 &&
					pbCode[11] == 0x75 &&
					pbCode[13] == 0x48 && pbCode[14] == 0x85 && pbCode[15] == 0xDB &&
					pbCode[16] == 0x75 &&
					pbCode[18] == 0x83 && pbCode[19] == 0xFA && pbCode[20] == 0x03 &&
					pbCode[21] == 0x75)
				{
					decombine_without_labels_hack = 2;
				}
			}
			else // if(nWinVersion >= WIN_VERSION_11_22H2)
			{
				// 41:84C6                  | test r14b,al
				// 75 XX                    | jne XXXXXXXX
				// 41:393424                | cmp dword ptr ds:[r12],esi
				// 75 XX                    | jne XXXXXXXX
				// 48:85DB                  | test rbx,rbx
				// 75 XX                    | jne XXXXXXXX
				// 41:833C24 03             | cmp dword ptr ds:[r12],3
				// 75 XX                    | jne XXXXXXXX
				if(pbCode &&
					pbCode[0] == 0x41 && pbCode[1] == 0x84 && pbCode[2] == 0xC6 &&
					pbCode[3] == 0x75 &&
					pbCode[5] == 0x41 && pbCode[6] == 0x39 && pbCode[7] == 0x34 && pbCode[8] == 0x24 &&
					pbCode[9] == 0x75 &&
					pbCode[11] == 0x48 && pbCode[12] == 0x85 && pbCode[13] == 0xDB &&
					pbCode[14] == 0x75 &&
					pbCode[16] == 0x41 && pbCode[17] == 0x83 && pbCode[18] == 0x3C && pbCode[19] == 0x24 && pbCode[20] == 0x03 &&
					pbCode[21] == 0x75)
				{
					decombine_without_labels_hack = 2;
				}
			}
		}
#else // !_WIN64
		if(nWinVersion <= WIN_VERSION_10_R4)
		{
			if(nWinVersion >= WIN_VERSION_10_R4)
			{
				// 8B7B XX       | MOV EDI,DWORD PTR [EBX+XX]
				// 8B8D XXXXXXXX | MOV ECX,DWORD PTR [EBP-XX]
				// 8B85 XXXXXXXX | MOV EAX,DWORD PTR [EBP-XX]
				// 8B00          | MOV EAX,DWORD PTR [EAX]
				// 8985 XXXXXXXX | MOV DWORD PTR [EBP-XX],EAX
				if(pbCode[0] == 0x8B && pbCode[1] == 0x7B &&
					pbCode[3] == 0x8B && pbCode[4] == 0x8D &&
					pbCode[9] == 0x8B && pbCode[10] == 0x85 &&
					pbCode[15] == 0x8B && pbCode[16] == 0x00 &&
					pbCode[17] == 0x89 && pbCode[18] == 0x85)
				{
					pbCode += 23;
				}
				else
					pbCode = NULL;
			}
			else
			{
				// 8B8D XXXXXXXX | MOV ECX,DWORD PTR [EBP-XXXXXXXX]
				if(pbCode[0] == 0x8B && pbCode[1] == 0x8D)
				{
					pbCode += 6;
				}
				else
					pbCode = NULL;
			}

			if(nWinVersion == WIN_VERSION_10_T1)
			{
				// E9 XXXXXXXX | JMP XXXXXXXX
				if(pbCode &&
					pbCode[0] == 0xE9)
				{
					INT32 nOffset = *(INT32 *)(pbCode + 1);
					pbCode += 5;
					pbCode += nOffset;
				}
				else
					pbCode = NULL;
			}

			// F6C1 02 | TEST CL,02
			if(pbCode &&
				pbCode[0] == 0xF6 && pbCode[1] == 0xC1 && pbCode[2] == 0x02)
			{
				pbCode += 3;
			}
		}
		else // if(nWinVersion >= WIN_VERSION_10_R5)
		{
			// 8B73 XX       | MOV ESI,DWORD PTR [EBX+XX]
			// -or-
			// 8B7B XX       | MOV EDI,DWORD PTR [EBX+XX]
			// 8B85 XXXXXXXX | MOV EAX,DWORD PTR [EBP-XXXXXXXX]
			if(pbCode[0] == 0x8B && (pbCode[1] == 0x73 || pbCode[1] == 0x7B) &&
				pbCode[3] == 0x8B && pbCode[4] == 0x85)
			{
				pbCode += 9;
			}
			else
				pbCode = NULL;

			// Optional:
			// E9 XXXXXXXX | JMP XXXXXXXX
			if(pbCode &&
				pbCode[0] == 0xE9)
			{
				INT32 nOffset = *(INT32 *)(pbCode + 1);
				pbCode += 5;
				pbCode += nOffset;
			}

			// 8B0F          | MOV ECX,DWORD PTR [EDI]
			// 898D XXXXXXXX | MOV DWORD PTR [EBP-XXXXXXXX],ECX
			// A8 02         | TEST AL,02
			if(pbCode &&
				pbCode[0] == 0x8B && pbCode[1] == 0x0F &&
				pbCode[2] == 0x89 && pbCode[3] == 0x8D &&
				pbCode[8] == 0xA8 && pbCode[9] == 0x02)
			{
				pbCode += 10;
			}
			// Seen in WIN_VERSION_10_19H1.
			// A8 02 | test al,2
			// 8B07  | mov eax,dword ptr ds:[edi]
			else if(pbCode &&
				pbCode[0] == 0xA8 && pbCode[1] == 0x02 &&
				pbCode[2] == 0x8B && pbCode[3] == 0x07)
			{
				pbCode += 4;
			}
			// 8BB5 XXXXXXXX | mov esi,dword ptr ss:[ebp-XXXXXXXX]
			// 8B0E          | mov ecx,dword ptr ds:[esi]
			// 898D XXXXXXXX | mov dword ptr ss:[ebp-XXXXXXXX],ecx
			// A8 02         | test al,2
			else if(pbCode &&
				pbCode[0] == 0x8B && pbCode[1] == 0xB5 &&
				pbCode[6] == 0x8B && pbCode[7] == 0x0E &&
				pbCode[8] == 0x89 && pbCode[9] == 0x8D &&
				pbCode[14] == 0xA8 && pbCode[15] == 0x02)
			{
				pbCode += 16;
			}
			else
				pbCode = NULL;
		}

		if(pbCode)
		{
			// 0F84 XXXXXXXX | JZ XXXXXXXX
			if(pbCode[0] == 0x0F && pbCode[1] == 0x84)
			{
				pbCode += 6;
			}
			// 74 XX | JZ SHORT XXXXXXXX
			else if(pbCode[0] == 0x74)
			{
				pbCode += 2;
			}
			// 0F85 XXXXXXXX | JNZ XXXXXXXX
			else if(pbCode[0] == 0x0F && pbCode[1] == 0x85)
			{
				INT32 nOffset = *(INT32 *)(pbCode + 2);
				pbCode += 6;
				pbCode += nOffset;
			}
			// 75 XX | JNZ SHORT XXXXXXXX
			else if(pbCode[0] == 0x75)
			{
				INT8 nOffset = *(INT8 *)(pbCode + 1);
				pbCode += 2;
				pbCode += nOffset;
			}
			else
				pbCode = NULL;

			// C785 XXXXXXXX 00000000 | MOV DWORD PTR [EBP-XXXXXXXX],0
			if(pbCode &&
				pbCode[0] == 0xC7 && pbCode[1] == 0x85 && pbCode[6] == 0x00 && pbCode[7] == 0x00 && pbCode[8] == 0x00 && pbCode[9] == 0x00)
			{
				decombine_without_labels_hack = 2;
			}
		}
#endif // WIN64
	}

	// A hack for decombining without labels.
	// How to check whether the hack works:
	// Use "Always combine, hide labels" in Taskbar Properties,
	// and "Don't combine grouped buttons" in the tweaker.
	// Set the taskbar to be vertical and wide.
	// If the hack doesn't work, text labels of the decombined items will show up.
	switch(decombine_without_labels_hack)
	{
	case 2:
		dwNewPreferences |= 2;
		decombine_without_labels_hack = 1;
		break;

	case 3:
		if(nWinVersion <= WIN_VERSION_811)
			decombine_without_labels_hack = 2;
		break;
	}

	if(nWinVersion >= WIN_VERSION_10_T1 && selective_combining_hack == 1)
	{
		// Assembly code below is from CTaskListWnd::_RecomputeLayout

		BYTE *pbCode = (BYTE *)pReturnAddress;

#ifdef _WIN64
		if(nWinVersion <= WIN_VERSION_10_R3)
		{
			// 8B 44 24 XX              | mov eax,dword ptr ss:[rsp+XX]
			if(pbCode[0] == 0x8B && pbCode[1] == 0x44 && pbCode[2] == 0x24)
			{
				pbCode += 4;

				if(nWinVersion == WIN_VERSION_10_T1)
				{
					// 45 33 D2                 | xor r10d,r10d
					if(pbCode[0] == 0x45)
					{
						pbCode += 3;
					}
					else
						pbCode = NULL;
				}
				else if(nWinVersion >= WIN_VERSION_10_R1 && nWinVersion <= WIN_VERSION_10_R2)
				{
					/*
						cmp edi,dword ptr ds:[r14+134]
						jne explorer.7FF75FBBC0AD
						test edi,edi
						jne explorer.7FF75FBBC0B4
						test byte ptr ds:[<Microsoft_Windows_Shell_CoreEnableBits>],1
						jne explorer.7FF75FB250AE
						mov rcx,qword ptr ds:[r14+60]
						lea rdi,qword ptr ds:[<public: virtual long __cdecl CTaskBand::GetUserPreferences(unsigned l
						mov eax,r9d
						mov dword ptr ss:[rsp+44],eax
						test rcx,rcx
						je explorer.7FF75FB24C62
						mov rax,qword ptr ds:[rcx]
						mov rax,qword ptr ds:[rax+30]
						cmp rax,rdi
						jne explorer.7FF75FBBC221 <---------------------------------------------- We're hooking THIS
						mov eax,dword ptr ds:[rcx+48]
						test al,1
						je explorer.7FF75FBBC3CB
						mov rcx,qword ptr ds:[r14+60]
						mov eax,r9d
						mov dword ptr ss:[rsp+44],eax
						test rcx,rcx
						je explorer.7FF75FB24C8D
						mov rax,qword ptr ds:[rcx]
						mov rax,qword ptr ds:[rax+30]
						cmp rax,rdi
						jne explorer.7FF75FBBC23E
						mov eax,dword ptr ds:[rcx+48]
						mov r13,qword ptr ss:[rsp+70]
						test al,2
						je explorer.7FF75FBBC25B
						mov rdi,r9
						test r13,r13
						jle explorer.7FF75FB24D05
						lea rsi,qword ptr ds:[<public: virtual int __cdecl CTaskBtnGroup::CanGlom(void) __ptr64>]
						lea r15,qword ptr ds:[<public: virtual void __cdecl CTaskBtnGroup::Glom(int) __ptr64>]
					*/

					// 41 B8 01 00 00 00        | mov r8d,1
					if(pbCode[0] == 0x41 && pbCode[1] >= 0xB8 && pbCode[1] <= 0xBF &&
						pbCode[2] == 0x01 && pbCode[3] == 0x00 &&
						pbCode[4] == 0x00 && pbCode[5] == 0x00)
					{
						pbCode += 6;
					}
					else
						pbCode = NULL;

					if(nWinVersion == WIN_VERSION_10_R1)
					{
						// 45 33 C9                 | xor r9d,r9d
						if(pbCode &&
							pbCode[0] == 0x45)
						{
							pbCode += 3;
						}
						else
							pbCode = NULL;
					}

					// E9 XXXXXXXX | jmp XXXXXXXX
					if(pbCode &&
						pbCode[0] == 0xE9)
					{
						INT32 nOffset = *(INT32 *)(pbCode + 1);
						pbCode += 5;
						pbCode += nOffset;
					}
					else
						pbCode = NULL;
				}

				// A8 01                    | test al,1
				// 0F 84 XX XX XX XX        | je XXXXXXXX
				if(pbCode &&
					pbCode[0] == 0xA8 && pbCode[1] == 0x01 &&
					pbCode[2] == 0x0F && pbCode[3] == 0x84)
				{
					selective_combining_hack = 3;
				}
			}
		}
		else if(nWinVersion <= WIN_VERSION_10_R4)
		{
			// F6 45 XX 01    | test byte ptr ss:[rbp-XX],1
			// 0F 84 XXXXXXXX | je XXXXXXXX
			if(pbCode[0] == 0xF6 && pbCode[1] == 0x45 && pbCode[3] == 0x01 &&
				pbCode[4] == 0x0F && pbCode[5] == 0x84)
			{
				selective_combining_hack = 3;
			}
		}
		else if(nWinVersion <= WIN_VERSION_10_R5)
		{
			// 8B 44 24 XX | mov eax,dword ptr ss:[rsp+XX]
			// A8 01       | test al,1
			if(pbCode[0] == 0x8B && pbCode[1] == 0x44 && pbCode[2] == 0x24 &&
				pbCode[4] == 0xA8 && pbCode[5] == 0x01)
			{
				pbCode += 6;
			}
			else
				pbCode = NULL;

			// 74 XX          | je XXXXXXXX
			// -or-
			// 0F 84 XXXXXXXX | je XXXXXXXX
			if(pbCode &&
				pbCode[0] == 0x74)
			{
				pbCode += 2;
			}
			else if(pbCode &&
				pbCode[0] == 0x0F && pbCode[1] == 0x84)
			{
				pbCode += 6;
			}
			else
				pbCode = NULL;

			if(pbCode)
			{
				selective_combining_hack = 3;
			}
		}
		else if(nWinVersion <= WIN_VERSION_10_20H1)
		{
			// 8B45 XX | mov eax,dword ptr ss:[rbp-XX]
			// 33D2    | xor edx,edx
			// A8 01   | test al,1
			// -or-
			// 8b5424XX | mov     edx,dword ptr [rsp+XX]
			// f6c201   | test    dl,1
			if(pbCode[0] == 0x8B && pbCode[1] == 0x45 &&
				pbCode[3] == 0x33 && pbCode[4] == 0xD2 &&
				pbCode[5] == 0xA8 && pbCode[6] == 0x01)
			{
				pbCode += 7;
			}
			else if(pbCode[0] == 0x8B && pbCode[1] == 0x54 && pbCode[2] == 0x24 &&
				pbCode[4] == 0xF6 && pbCode[5] == 0xC2 && pbCode[6] == 0x01)
			{
				pbCode += 7;
			}
			else
				pbCode = NULL;

			// 74 XX          | je XXXXXXXX
			// -or-
			// 0F 84 XXXXXXXX | je XXXXXXXX
			if(pbCode &&
				pbCode[0] == 0x74)
			{
				pbCode += 2;
			}
			else if(pbCode &&
				pbCode[0] == 0x0F && pbCode[1] == 0x84)
			{
				pbCode += 6;
			}
			else
				pbCode = NULL;

			if(pbCode)
			{
				selective_combining_hack = 3;
			}
		}
		else // if(nWinVersion >= WIN_VERSION_SERVER_2022)
		{
			// 8B4424 XX  | mov eax,dword ptr ss:[rsp+XX]
			// 48:83C4 28 | add rsp,28
			// C3         | ret
			if(pbCode[0] == 0x8B && pbCode[1] == 0x44 && pbCode[2] == 0x24 &&
				pbCode[4] == 0x48 && pbCode[5] == 0x83 && pbCode[6] == 0xC4 && pbCode[7] == 0x28 &&
				pbCode[8] == 0xC3)
			{
				// Skip the frame of CTaskListWnd::_GetUserPreferences
				// and get the next return address.
				pbCode = *(ppAddressOfReturnAddress + 6);
			}
			else
				pbCode = NULL;

			// WIN_VERSION_SERVER_2022:
			// A8 01      | test al,1
			// -or- (Win11)
			// 41:83CC FF | or r12d,FFFFFFFF
			// 40:84C7    | test dil,al
			// -or- (Win11)
			// 41:83CF FF | or r15d,FFFFFFFF  
			// 40:84C7    | test dil,al
			if(pbCode &&
				pbCode[0] == 0xA8 && pbCode[1] == 0x01)
			{
				pbCode += 2;
			}
			else if(pbCode &&
				pbCode[0] == 0x41 && pbCode[1] == 0x83 &&
				(pbCode[2] == 0xCC || pbCode[2] == 0xCF) && pbCode[3] == 0xFF &&
				pbCode[4] == 0x40 && pbCode[5] == 0x84 && pbCode[6] == 0xC7)
			{
				pbCode += 7;
			}
			else
				pbCode = NULL;

			// 74 XX          | je XXXXXXXX
			// -or-
			// 0F 84 XXXXXXXX | je XXXXXXXX
			if(pbCode &&
				pbCode[0] == 0x74)
			{
				pbCode += 2;
			}
			else if(pbCode &&
				pbCode[0] == 0x0F && pbCode[1] == 0x84)
			{
				pbCode += 6;
			}
			else
				pbCode = NULL;

			if(pbCode)
			{
				selective_combining_hack = 3;
			}
		}
#else // !_WIN64
		// 8B4424 XX | MOV EAX,DWORD PTR [ESP+XX]
		if(pbCode[0] == 0x8B && pbCode[1] == 0x44 && pbCode[2] == 0x24)
		{
			pbCode += 4;
		}
		// 8B8424 XXXXXXXX | MOV EAX,DWORD PTR [ESP+XX]
		else if(pbCode[0] == 0x8B && pbCode[1] == 0x84 && pbCode[2] == 0x24)
		{
			pbCode += 7;
		}
		// 8B85 XXXXXXXX | MOV EAX,DWORD PTR [EBP-XX]
		else if(pbCode[0] == 0x8B && pbCode[1] == 0x85)
		{
			pbCode += 6;
		}
		else
			pbCode = NULL;

		if(nWinVersion == WIN_VERSION_10_T1 || nWinVersion >= WIN_VERSION_10_R5)
		{
			// E9 XXXXXXXX | JMP XXXXXXXX
			if(pbCode &&
				pbCode[0] == 0xE9)
			{
				INT32 nOffset = *(INT32 *)(pbCode + 1);
				pbCode += 5;
				pbCode += nOffset;
			}
			else
				pbCode = NULL;
		}
		else if(nWinVersion >= WIN_VERSION_10_R2 && nWinVersion <= WIN_VERSION_10_R3)
		{
			// 8B7424 XX | MOV ESI,DWORD PTR [ESP+XX]
			if(pbCode &&
				pbCode[0] == 0x8B && pbCode[1] == 0x74 && pbCode[2] == 0x24)
			{
				pbCode += 4;
			}
			else
				pbCode = NULL;
		}

		// A8 01 | TEST AL, 01
		if(pbCode &&
			pbCode[0] == 0xA8 && pbCode[1] == 0x01)
		{
			pbCode += 2;

			// 0F84 XXXXXXXX | JZ XXXXXXXX
			// -OR-
			// 74 XX | JZ SHORT XXXXXXXX
			if((pbCode[0] == 0x0F && pbCode[1] == 0x84) ||
				pbCode[0] == 0x74)
			{
				selective_combining_hack = 3;
			}
		}
#endif // WIN64
	}

	// A hack for selective combining.
	// How to check whether the hack works:
	// Use "Never Combine" in Taskbar Properties,
	// and "Combine grouped buttons" in the tweaker.
	// If the hack doesn't work, the buttons won't combine.
	// Note: in Windows 10, it only works on a horizontal taskbar.
	switch(selective_combining_hack)
	{
	case 2:
		if(nWinVersion <= WIN_VERSION_811)
			selective_combining_hack = 1;
		break;

	case 3:
		dwNewPreferences |= 3;
		selective_combining_hack++;
		break;

	case 4:
		dwNewPreferences |= 3;
		selective_combining_hack = 0;
		break;
	}

	return dwNewPreferences;
}

static BOOL CheckCombineButtonGroup(LONG_PTR *button_group)
{
	LONG_PTR *task_group;
	WCHAR *pAppId;
	int nCombineListValue;
	BOOL bCombine;
	LONG_PTR lpMMTaskListLongPtr;

	task_group = (LONG_PTR *)button_group[DO2(3, 4)];
	pAppId = *EV_TASKGROUP_APPID(task_group);

	if(pAppId && GetAppidListValue(AILIST_COMBINE, pAppId, &nCombineListValue))
	{
		if(nCombineListValue == AILIST_COMBINE_NEVER)
			bCombine = FALSE;
		else // if(dwCombineListValue == AILIST_COMBINE_ALWAYS)
			bCombine = TRUE;
	}
	else if(nOptions[OPT_COMBINING] == 1)
	{
		bCombine = TRUE;
	}
	else if(nOptions[OPT_COMBINING] == 2)
	{
		bCombine = FALSE;
	}
	else
	{
		// multimonitor environment
		if(nWinVersion >= WIN_VERSION_8)
			lpMMTaskListLongPtr = button_group[3];
		else
			lpMMTaskListLongPtr = lpTaskListLongPtr;

		if(TaskbarGetPreference(lpMMTaskListLongPtr) & 1)
			bCombine = TRUE;
		else
			bCombine = FALSE;
	}

	return bCombine;
}

static BOOL ButtonGroupCombine(LONG_PTR *button_group, BOOL bCombine)
{
	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR *plp;
	int button_group_type;
	int buttons_count;
	DWORD dwOldUserPrefRemoveBits;

	// multimonitor environment
	if(nWinVersion >= WIN_VERSION_8)
		lpMMTaskListLongPtr = button_group[3];
	else
		lpMMTaskListLongPtr = lpTaskListLongPtr;

	button_group_type = (int)button_group[DO2(6, 8)];
	if(bCombine)
	{
		if(button_group_type != 1)
			return FALSE;

		plp = (LONG_PTR *)button_group[DO2(5, 7)];
		buttons_count = (int)plp[0];
		if(buttons_count < 2)
			return FALSE;
	}
	else if(button_group_type != 3)
		return FALSE;

	if(bCombine && !CheckCombineButtonGroup(button_group))
		return FALSE;

	plp = button_group;
	plp = (LONG_PTR *)plp[0]; // COM functions list

	dwOldUserPrefRemoveBits = dwUserPrefRemoveBits;
	dwUserPrefRemoveBits |= 2;

	// CTaskBtnGroup::Glom
	FUNC_CTaskBtnGroup_Glom(plp)(button_group, bCombine);

	dwUserPrefRemoveBits = dwOldUserPrefRemoveBits;

	if(!(TaskbarGetPreference(lpMMTaskListLongPtr) & 8)) // No animation
		TaskListRecomputeLayout(lpMMTaskListLongPtr);

	return TRUE;
}

static void CombineUntrackedDecombined(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group)
{
	LONG_PTR *button_group_active;

	if(ButtonGroupValidate(lpMMTaskListLongPtr, button_group))
	{
		button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

		if(nOptions[OPT_COMBINING_DEACTIVE] == 0 || button_group != button_group_active)
			ButtonGroupCombine(button_group, TRUE);
	}
}

static BOOL DragWithinGroup(LONG_PTR *button_group_tracked, int button_index_tracked)
{
	LONG_PTR *plp;
	int button_group_type;
	int buttons_count;
	LONG_PTR **buttons;
	LONG_PTR *task_group;
	LONG_PTR *task_item_from, *task_item_to;
	int i;

	button_group_type = (int)button_group_tracked[DO2(6, 8)];
	if(button_group_type == 1 || button_group_type == 3)
	{
		plp = (LONG_PTR *)button_group_tracked[DO2(5, 7)];

		buttons_count = (int)plp[0];
		buttons = (LONG_PTR **)plp[1];

		for(i = 0; i < buttons_count; i++)
		{
			task_item_from = (LONG_PTR *)buttons[i][DO2(3, 4)];

			if(GetTaskItemWnd(task_item_from) == hDragWithinGroupsWnd)
			{
				// dragged hwnd exists in tracked button group

				if(button_index_tracked >= 0 && button_index_tracked < buttons_count && i != button_index_tracked)
				{
					task_group = (LONG_PTR *)button_group_tracked[DO2(3, 4)];
					task_item_to = (LONG_PTR *)buttons[button_index_tracked][DO2(3, 4)];

					TaskbarMoveTaskInGroup(task_group, task_item_from, task_item_to);
				}

				return TRUE;
			}
		}
	}

	return FALSE;
}

static BOOL DragBetweenGroups(LONG_PTR *button_group_tracked)
{
	LONG_PTR *task_group;
	WCHAR *pAppId;
	int button_group_type;
	int nListValue;

	task_group = (LONG_PTR *)button_group_tracked[DO2(3, 4)];
	pAppId = *EV_TASKGROUP_APPID(task_group);
	if(!pAppId)
		return FALSE;

	button_group_type = (int)button_group_tracked[DO2(6, 8)];
	if(button_group_type == 1 || button_group_type == 3)
	{
		if(!IsAppIdARandomGroup(pAppId))
		{
			if(GetAppidListValue(AILIST_GROUP, pAppId, &nListValue))
			{
				if(nListValue == AILIST_GROUP_NEVER)
					return FALSE;
			}
			else if(nOptions[OPT_GROUPING] == 1)
				return FALSE;
		}
	}
	else if(button_group_type == 2)
	{
		if(GetAppidListValue(AILIST_GROUPPINNED, pAppId, &nListValue))
		{
			if(nListValue == AILIST_GROUPPINNED_NEVER)
				return FALSE;
		}
		else if(nOptions[OPT_GROUPING_NOPINNED] == 1)
			return FALSE;
	}

	if(!WndSetAppId(hDragWithinGroupsWnd, pAppId))
		return FALSE;

	if(nWinVersion >= WIN_VERSION_8)
		lpRightDragAttachTaskListLongPtr = button_group_tracked[3];
	else
		lpRightDragAttachTaskListLongPtr = lpTaskListLongPtr;
	right_drag_attach_group = button_group_tracked;

	return TRUE;
}

static LONG_PTR *ButtonGroupNextToMousePos(LONG_PTR lpMMTaskListLongPtr, HWND hExcludeWnd)
{
	LONG_PTR *plp;
	LONG_PTR lp;
	int button_groups_count;
	LONG_PTR **button_groups;
	int button_group_type;
	int buttons_count;
	LONG_PTR **buttons;
	LONG_PTR *task_item;
	int nTaskbarPos;
	BOOL bVertical;
	HWND hMMTaskListWnd;
	RECT rcVisibleTaskList;
	POINT ptCursor;
	RECT *prcGroup;
	POINT ptGroupCenter;
	long last_edge;
	RECT rc1, rc2;
	int i;

	plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
	if(!plp)
		return NULL;

	button_groups_count = (int)plp[0];
	if(button_groups_count == 0)
		return NULL;

	button_groups = (LONG_PTR **)plp[1];

	if(lpMMTaskListLongPtr != lpTaskListLongPtr)
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

	bVertical = FALSE;
	switch(nTaskbarPos)
	{
	case 0:
	case 2:
		bVertical = TRUE;
		break;
	}

	hMMTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);

	GetWindowRect(hMMTaskListWnd, &rc1);
	GetWindowRect(GetParent(hMMTaskListWnd), &rc2);
	if(!IntersectRect(&rcVisibleTaskList, &rc1, &rc2))
		return NULL;

	MapWindowPoints(NULL, hMMTaskListWnd, (POINT *)&rcVisibleTaskList, 2);

	GetCursorPos(&ptCursor);
	MapWindowPoints(NULL, hMMTaskListWnd, &ptCursor, 1);

	// Skip all buttons which are currently not visible (e.g. multipage taskbar)
	for(i = 0; i < button_groups_count; i++)
	{
		prcGroup = *(RECT **)(button_groups[i][DO2(4, 6)] + sizeof(LONG_PTR));
		ptGroupCenter.x = prcGroup->left + (prcGroup->right - prcGroup->left) / 2;
		ptGroupCenter.y = prcGroup->top + (prcGroup->bottom - prcGroup->top) / 2;
		if(PtInRect(&rcVisibleTaskList, ptGroupCenter))
		{
			break;
		}
	}

	last_edge = LONG_MIN;

	for(; i < button_groups_count; i++)
	{
		if(hExcludeWnd)
		{
			button_group_type = (int)button_groups[i][DO2(6, 8)];
			if(button_group_type == 1 || button_group_type == 3)
			{
				plp = (LONG_PTR *)button_groups[i][DO2(5, 7)];

				buttons_count = (int)plp[0];
				if(buttons_count == 1)
				{
					buttons = (LONG_PTR **)plp[1];
					task_item = (LONG_PTR *)buttons[0][DO2(3, 4)];
					if(GetTaskItemWnd(task_item) == hExcludeWnd)
						continue; // skip button group
				}
			}
		}

		prcGroup = *(RECT **)(button_groups[i][DO2(4, 6)] + sizeof(LONG_PTR));
		ptGroupCenter.x = prcGroup->left + (prcGroup->right - prcGroup->left) / 2;
		ptGroupCenter.y = prcGroup->top + (prcGroup->bottom - prcGroup->top) / 2;
		if(!PtInRect(&rcVisibleTaskList, ptGroupCenter))
			return button_groups[i];

		if(bVertical)
		{
			if(prcGroup->top < last_edge)
				return button_groups[i];

			if(ptCursor.y < ptGroupCenter.y)
				return button_groups[i];

			last_edge = prcGroup->bottom;
		}
		else
		{
			if(prcGroup->left < last_edge)
				return button_groups[i];

			if(ptCursor.x < ptGroupCenter.x)
				return button_groups[i];

			last_edge = prcGroup->right;
		}
	}

	return NULL;
}

static void ActiveButtonGroupChanged(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *old_button_group_active, LONG_PTR *new_button_group_active)
{
	if(old_button_group_active)
	{
		ButtonGroupDeactivated(lpMMTaskListLongPtr, old_button_group_active);
	}

	if(new_button_group_active)
	{
		ButtonGroupActivated(lpMMTaskListLongPtr, new_button_group_active);
	}
}

static void ActiveButtonGroupChangedNonCaptured(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *old_button_group_active, LONG_PTR *new_button_group_active)
{
	if(old_button_group_active)
	{
		ButtonGroupDeactivatedNonCaptured(lpMMTaskListLongPtr, old_button_group_active);
	}

	if(new_button_group_active)
	{
		ButtonGroupActivated(lpMMTaskListLongPtr, new_button_group_active);
	}
}

static void ButtonGroupActivated(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group)
{
	if(button_group)
	{
		ButtonGroupCombine(button_group, FALSE);
	}
}

static void ButtonGroupDeactivated(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group)
{
	HWND hMMTaskListWnd = *EV_MM_TASKLIST_HWND(lpMMTaskListLongPtr);
	if(GetCapture() == hMMTaskListWnd)
	{
		// Handle decombining after capture is changed
		if(!bTaskListCapturedAndActiveWndChanged)
		{
			lpCapturedTaskListLongPtr = lpMMTaskListLongPtr;
			button_group_active_after_capture = button_group;
			bTaskListCapturedAndActiveWndChanged = TRUE;
		}
	}
	else
		ButtonGroupDeactivatedNonCaptured(lpMMTaskListLongPtr, button_group);
}

static void ButtonGroupDeactivatedNonCaptured(LONG_PTR lpMMTaskListLongPtr, LONG_PTR *button_group)
{
	if(button_group)
	{
		if(nOptions[OPT_COMBINING_DEONHOVER] == 0)
		{
			ButtonGroupCombine(button_group, TRUE);
		}
		else
		{
			LONG_PTR *button_group_tracked = TaskbarGetTrackedButtonGroup(lpMMTaskListLongPtr);

			if(
				button_group != button_group_tracked &&
				button_group != button_group_untracked_decombined
			)
			{
				ButtonGroupCombine(button_group, TRUE);
			}
		}
	}
}

// Virtual Desktop switching order fix

void ComFuncVirtualDesktopFixAfterDPA_InsertPtr(HDPA pdpa, int index, void *p, void *pRet)
{
	if(nWinVersion >= WIN_VERSION_10_T1 && nOptionsEx[OPT_EX_VIRTUAL_DESKTOP_ORDER_FIX] &&
		index != INT_MAX)
	{
		HDPA hButtonGroupsDpa = *EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpTaskListLongPtr);
		if(hButtonGroupsDpa && pdpa == hButtonGroupsDpa)
		{
			OnButtonGroupInserted(lpTaskListLongPtr, index);
		}
		else if(TaskbarGetPreference(lpTaskListLongPtr) & 0x400) // 0x400: Taskbar where window is open
		{
			SECONDARY_TASK_LIST_GET secondary_task_list_get;
			LONG_PTR lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
			while(lpSecondaryTaskListLongPtr)
			{
				hButtonGroupsDpa = *EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpSecondaryTaskListLongPtr);
				if(hButtonGroupsDpa && pdpa == hButtonGroupsDpa)
				{
					OnButtonGroupInserted(lpSecondaryTaskListLongPtr, index);
					break;
				}

				lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
			}
		}
	}
}

static void OnButtonGroupInserted(LONG_PTR lpMMTaskListLongPtr, int nButtonGroupIndex)
{
	// assert(nWinVersion >= WIN_VERSION_10_T1)

	HDPA hButtonGroupsDpa = *EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
	if(!hButtonGroupsDpa)
		return;

	LONG_PTR *plp = (LONG_PTR *)hButtonGroupsDpa;
	int button_groups_count = (int)plp[0];
	LONG_PTR **button_groups = (LONG_PTR **)plp[1];
	LONG_PTR *button_group = button_groups[nButtonGroupIndex];

	plp = (LONG_PTR *)button_group[DO2(5, 7)];
	if(!plp)
		return;

	int buttons_count = (int)plp[0];
	LONG_PTR **buttons = (LONG_PTR **)plp[1];
	if(buttons_count == 0)
		return;

	LONG_PTR *task_group = (LONG_PTR *)button_group[DO2(3, 4)];
	plp = (LONG_PTR *)task_group[4];
	if(!plp)
		return;

	int task_items_count = (int)plp[0];
	if(task_items_count == 0)
		return;

	LONG_PTR **task_items = (LONG_PTR **)plp[1];

	plp = *(LONG_PTR **)task_group;
	void **ppTaskGroupRelease = (void **)&plp[2];
	PointerRedirectionAdd(ppTaskGroupRelease, TaskGroupReleaseHook, &prTaskGroupRelease);

	plp = *(LONG_PTR **)task_items[0];
	void **ppTaskItemRelease = (void **)&plp[2];
	PointerRedirectionAdd(ppTaskItemRelease, TaskItemReleaseHook, &prTaskItemRelease);

	LONG_PTR lpAppViewMgr = *EV_TASK_SW_APP_VIEW_MGR();
	SRWLOCK *pArrayLock = EV_APP_VIEW_MGR_APP_ARRAY_LOCK(lpAppViewMgr);

	AcquireSRWLockExclusive(pArrayLock);

	LONG_PTR *lpArray = *EV_APP_VIEW_MGR_APP_ARRAY(lpAppViewMgr);
	size_t nArraySize = *EV_APP_VIEW_MGR_APP_ARRAY_SIZE(lpAppViewMgr);

	int nMatchCount = 0;
	size_t nRightNeighbourItemIndex = nArraySize;

	// Stage one: move all items in lpArray matching the items
	// in the newly inserted group to the beginning of the array.
	// Their amount is maintained in nMatchCount.
	// Also, find nRightNeighbourItemIndex, which is the index of
	// the item in lpArray which will be the first one before the
	// found matching items.

	for(size_t i = 0; i < nArraySize; i++)
	{
		task_group_virtual_desktop_released = NULL;
		task_item_virtual_desktop_released = NULL;

		LONG_PTR this_ptr = (LONG_PTR)(lpTaskSwLongPtr + DO5_3264(0, 0, ,, ,, ,, 0x38, 0x70));
		plp = *(LONG_PTR **)this_ptr;

		ReleaseSRWLockExclusive(pArrayLock);

		// CTaskBand::ViewVirtualDesktopChanged(this, application_view)
		FUNC_CTaskBand_ViewVirtualDesktopChanged(plp)(this_ptr, lpArray[i]);

		AcquireSRWLockExclusive(pArrayLock);

		if(lpArray != *EV_APP_VIEW_MGR_APP_ARRAY(lpAppViewMgr) ||
			nArraySize != *EV_APP_VIEW_MGR_APP_ARRAY_SIZE(lpAppViewMgr))
		{
			// Something went wrong, abort
			nMatchCount = 0;
			break;
		}

		if(!task_group_virtual_desktop_released)
			continue;

		if(task_group_virtual_desktop_released != task_group)
		{
			if(nRightNeighbourItemIndex == nArraySize)
			{
				for(int j = nButtonGroupIndex + 1; j < button_groups_count; j++)
				{
					LONG_PTR *check_button_group = button_groups[j];
					LONG_PTR *check_task_group = (LONG_PTR *)check_button_group[DO2(3, 4)];
					if(task_group_virtual_desktop_released == check_task_group)
					{
						// The current item in lpArray is from the same group
						// of at least one of the items in button_groups to the right
						// of the newly added item.
						nRightNeighbourItemIndex = i - nMatchCount;
						break;
					}
				}
			}

			continue;
		}

		if(!task_item_virtual_desktop_released)
			continue;

		for(int j = 0; j < buttons_count; j++)
		{
			LONG_PTR *button = buttons[j];
			LONG_PTR *task_item = (LONG_PTR *)button[DO2(3, 4)];

			if(task_item_virtual_desktop_released == task_item)
			{
				// The current item in lpArray matches one of the
				// buttons in the newly added item.
				if(i > (size_t)nMatchCount)
				{
					LONG_PTR lpTemp = lpArray[i];
					memmove(&lpArray[nMatchCount + 1], &lpArray[nMatchCount], (i - nMatchCount) * sizeof(LONG_PTR));
					lpArray[nMatchCount] = lpTemp;
				}

				nMatchCount++;
				break;
			}
		}
	}

	PointerRedirectionRemove(ppTaskGroupRelease, &prTaskGroupRelease);
	PointerRedirectionRemove(ppTaskItemRelease, &prTaskItemRelease);

	// Stage two: move the found items before the item in nRightNeighbourItemIndex.

	if(nRightNeighbourItemIndex == nArraySize)
	{
		// By default, move to the right end
		nRightNeighbourItemIndex = nArraySize - nMatchCount;
	}

	if(nMatchCount > 0 && nRightNeighbourItemIndex > 0)
	{
		LONG_PTR *lpBuffer = (LONG_PTR *)HeapAlloc(GetProcessHeap(), 0, nMatchCount * sizeof(LONG_PTR));
		if(lpBuffer)
		{
			memcpy(lpBuffer, lpArray, nMatchCount * sizeof(LONG_PTR));
			memmove(&lpArray[0], &lpArray[nMatchCount], nRightNeighbourItemIndex * sizeof(LONG_PTR));
			memcpy(&lpArray[nRightNeighbourItemIndex], lpBuffer, nMatchCount * sizeof(LONG_PTR));

			HeapFree(GetProcessHeap(), 0, lpBuffer);
		}
	}

	ReleaseSRWLockExclusive(pArrayLock);
}

static ULONG __stdcall TaskGroupReleaseHook(LONG_PTR this_ptr)
{
	ULONG ulRet;

	ulRet = ((ULONG(__stdcall *)(LONG_PTR))prTaskGroupRelease.pOriginalAddress)(this_ptr);
	if(ulRet > 0 && nDoesWindowMatchCalls == 0)
		task_group_virtual_desktop_released = (LONG_PTR *)this_ptr;

	return ulRet;
}

static ULONG __stdcall TaskItemReleaseHook(LONG_PTR this_ptr)
{
	ULONG ulRet;

	ulRet = ((ULONG(__stdcall *)(LONG_PTR))prTaskItemRelease.pOriginalAddress)(this_ptr);
	if(ulRet > 0 && nDoesWindowMatchCalls == 0)
		task_item_virtual_desktop_released = (LONG_PTR *)this_ptr;

	return ulRet;
}

// Hooks

static BOOL CreateEnableHook(void **ppTarget, void *const pDetour, void **ppOriginal, POINTER_REDIRECTION *ppr)
{
	void *pTarget;
	MH_STATUS status;

	pTarget = PointerRedirectionGetOriginalPtr(ppTarget);

	status = MH_CreateHook(pTarget, pDetour, ppOriginal);
	if(status == MH_OK)
	{
		status = MH_QueueEnableHook(pTarget);
		if(status == MH_OK)
		{
			PointerRedirectionAdd(ppTarget, pDetour, ppr);
			return TRUE;
		}

		MH_RemoveHook(pTarget);
	}

	*ppOriginal = NULL;
	return FALSE;
}

static BOOL DisableHook(void **ppTarget, POINTER_REDIRECTION *ppr)
{
	/*void* pTarget;
	MH_STATUS status;*/

	if(ppr->pOriginalAddress)
	{
		PointerRedirectionRemove(ppTarget, ppr);

		// Note: no need to cleanup MinHook hooks, they will be removed upon uninitialization

		/*pTarget = PointerRedirectionGetOriginalPtr(ppTarget);

		status = MH_QueueDisableHook(pTarget);
		if(status != MH_OK)
			return FALSE;*/
	}

	return TRUE;
}

// Public

void ComFuncTaskListBeforeLButtonUp(LONG_PTR lpMMTaskListLongPtr, DWORD *pdwOldUserPrefSetBits, DWORD *pdwOldUserPrefRemoveBits, LONG_PTR **p_prev_button_group_active)
{
	*pdwOldUserPrefSetBits = dwUserPrefSetBits;
	*pdwOldUserPrefRemoveBits = dwUserPrefRemoveBits;
	*p_prev_button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

	// Check drag operation flag
	if(*EV_MM_TASKLIST_DRAG_FLAG(lpMMTaskListLongPtr) == 0)
	{
		// button_group the mouse is pressing on
		LONG_PTR *button_group = *EV_MM_TASKLIST_PRESSED_BUTTON_GROUP(lpMMTaskListLongPtr);
		if(button_group)
		{
			int button_group_type = (int)button_group[DO2(6, 8)];
			if(button_group_type == 1)
			{
				LONG_PTR *plp = (LONG_PTR *)button_group[DO2(5, 7)];

				int buttons_count = (int)plp[0];
				LONG_PTR **buttons = (LONG_PTR **)plp[1];

				if(buttons_count > 1)
					dwUserPrefRemoveBits |= 2;
			}
		}
	}
}

void ComFuncTaskListAfterLButtonUp(LONG_PTR lpMMTaskListLongPtr, DWORD dwOldUserPrefSetBits, DWORD dwOldUserPrefRemoveBits, LONG_PTR *prev_button_group_active)
{
	dwUserPrefSetBits = dwOldUserPrefSetBits;
	dwUserPrefRemoveBits = dwOldUserPrefRemoveBits;

	// For nWinVersion >= WIN_VERSION_10_R1, this is handled in ButtonGroupHasItemAnimationHook
	if(nWinVersion <= WIN_VERSION_10_T2 && nOptions[OPT_COMBINING_DEACTIVE] == 1)
	{
		LONG_PTR *button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

		if(button_group_active != prev_button_group_active)
			ActiveButtonGroupChanged(lpMMTaskListLongPtr, prev_button_group_active, button_group_active);
	}
}

void ComFuncTaskListBeforeMouseMove()
{
	bInMouseMove = TRUE;
}

void ComFuncTaskListAfterMouseMove()
{
	bInMouseMove = FALSE;
}

void ComFuncTaskListMouseLeave()
{
	if(lpUntrackedDecombinedTaskListLongPtr && button_group_untracked_decombined)
	{
		if(nOptions[OPT_COMBINING_DEONHOVER] == 1)
			CombineUntrackedDecombined(lpUntrackedDecombinedTaskListLongPtr, button_group_untracked_decombined);

		lpUntrackedDecombinedTaskListLongPtr = 0;
		button_group_untracked_decombined = NULL;
	}
}

BOOL ComFuncTaskListRightDragInit(LONG_PTR lpMMTaskListLongPtr)
{
	LONG_PTR *button_group_tracked;
	int button_index_tracked;
	LONG_PTR *plp;
	int button_group_type;
	int buttons_count;
	LONG_PTR **buttons;

	button_group_tracked = TaskbarGetTrackedButtonGroup(lpMMTaskListLongPtr);
	if(!button_group_tracked)
		return FALSE;

	button_index_tracked = *EV_MM_TASKLIST_TRACKED_BUTTON_INDEX(lpMMTaskListLongPtr);
	if(button_index_tracked < 0)
		return FALSE;

	button_group_type = (int)button_group_tracked[DO2(6, 8)];
	if(button_group_type != 1)
		return FALSE;

	plp = (LONG_PTR *)button_group_tracked[DO2(5, 7)];
	buttons_count = (int)plp[0];
	if(buttons_count - 1 < button_index_tracked)
		return FALSE;

	buttons = (LONG_PTR **)plp[1];

	hDragWithinGroupsWnd = GetButtonWnd(buttons[button_index_tracked]);

	return TRUE;
}

BOOL ComFuncTaskListRightDragProcessed()
{
	return bDragWithinGroupsDone;
}

void ComFuncTaskListCaptureChanged(LONG_PTR lpMMTaskListLongPtr)
{
	bDragWithinGroupsDone = FALSE;
	hDragWithinGroupsWnd = NULL;

	bCustomDestinationMenuActionDone = FALSE;

	if(bTaskListCapturedAndActiveWndChanged && lpMMTaskListLongPtr == lpCapturedTaskListLongPtr)
	{
		LONG_PTR *prev_button_group_active = button_group_active_after_capture;
		if(!ButtonGroupValidate(lpMMTaskListLongPtr, prev_button_group_active))
			prev_button_group_active = NULL;

		LONG_PTR *button_group_active = TaskbarGetActiveButtonGroup(lpMMTaskListLongPtr);

		if(button_group_active != prev_button_group_active)
			ActiveButtonGroupChangedNonCaptured(lpMMTaskListLongPtr, prev_button_group_active, button_group_active);

		bTaskListCapturedAndActiveWndChanged = FALSE;
		lpCapturedTaskListLongPtr = 0;
		button_group_active_after_capture = NULL;
	}
}

BOOL ComFuncTaskListMouseWheel(LONG_PTR lpMMTaskListLongPtr, short delta)
{
	LONG_PTR *button_group_tracked;
	LONG_PTR *task_group;
	WCHAR *pAppId;
	int nLabelingListValue;
	BOOL bMemoryOnly;

	if(nOptionsEx[OPT_EX_RIGHT_DRAG_TOGGLE_LABELS] &&
		nOptions[OPT_GROUPING_RIGHTDRAG] == 1 && hDragWithinGroupsWnd)
	{
		button_group_tracked = TaskbarGetTrackedButtonGroup(lpMMTaskListLongPtr);
		if(button_group_tracked)
		{
			task_group = (LONG_PTR *)button_group_tracked[DO2(3, 4)];
			pAppId = *EV_TASKGROUP_APPID(task_group);
			if(pAppId)
			{
				if(delta > 0)
					nLabelingListValue = AILIST_LABEL_ALWAYS;
				else
					nLabelingListValue = AILIST_LABEL_NEVER;

				if(IsAppIdARandomGroup(pAppId))
					bMemoryOnly = TRUE;
				else
					bMemoryOnly = FALSE;

				AddAppidToList(AILIST_LABEL, pAppId, nLabelingListValue, bMemoryOnly);
				MMTaskListRecomputeLayout();

				bDragWithinGroupsDone = TRUE;
			}
		}

		return TRUE;
	}

	return FALSE;
}

void ComFuncThumbnailWndBeforePaint(LONG_PTR lpMMThumbnailLongPtr)
{
	if(nOptionsEx[OPT_EX_LIST_REVERSE_ORDER])
	{
		BOOL bShowingList = *EV_MM_THUMBNAIL_LIST_FLAG(lpMMThumbnailLongPtr);
		if(bShowingList)
		{
			POINT pt = { -13, -37 }; // 1337 \m/

			LONG_PTR this_ptr = (LONG_PTR)(lpMMThumbnailLongPtr + DO5_3264(0x10, 0x20, ,, ,, ,, 0x08, 0x10));
			LONG_PTR *plp = *(LONG_PTR **)this_ptr;

			// CTaskListThumbnailWnd::ThumbIndexFromPoint(this, ppt)
			FUNC_CTaskListThumbnailWnd_ThumbIndexFromPoint(plp)(this_ptr, &pt);

			nThumbnailListReverseHack = -3;
		}
	}
}

void ComFuncSetThumbNoDismiss(BOOL bNoDismiss)
{
	bThumbNoDismiss = bNoDismiss;
}

BOOL ComFuncGetThumbNoDismiss()
{
	return bThumbNoDismiss;
}

BOOL ComFuncMoveDetachedToCursor()
{
	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR *right_button_group;
	HDPA hButtonGroupsDpa;
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	int button_group_type;
	int buttons_count;
	LONG_PTR **buttons;
	int i;

	if(!hDragWithinGroupsWnd || !right_drag_detach_group)
		return FALSE;

	lpMMTaskListLongPtr = lpRightDragDetachTaskListLongPtr;
	right_button_group = right_drag_detach_group;
	lpRightDragDetachTaskListLongPtr = 0;
	right_drag_detach_group = NULL;

	plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
	if(!plp)
		return FALSE;

	hButtonGroupsDpa = (HDPA)plp;

	button_groups_count = (int)plp[0];
	button_groups = (LONG_PTR **)plp[1];

	button_group_type = (int)button_groups[button_groups_count - 1][DO2(6, 8)];
	if(button_group_type != 1)
		return FALSE;

	plp = (LONG_PTR *)button_groups[button_groups_count - 1][DO2(5, 7)];
	buttons_count = (int)plp[0];
	buttons = (LONG_PTR **)plp[1];

	if(buttons_count != 1 || GetButtonWnd(buttons[0]) != hDragWithinGroupsWnd)
		return FALSE;

	for(i = 0; i < button_groups_count; i++)
	{
		if(button_groups[i] == right_button_group)
		{
			if(i < button_groups_count - 1)
				TaskbarMoveGroup(lpMMTaskListLongPtr, button_groups_count - 1, i);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL ComFuncIsAttachPending(HWND hButtonWnd)
{
	return hButtonWnd == hDragWithinGroupsWnd && right_drag_attach_group;
}

BOOL ComFuncMoveAttachedToCursor()
{
	LONG_PTR lpMMTaskListLongPtr;
	LONG_PTR *right_button_group;
	LONG_PTR *button_group_tracked;
	int button_index_tracked;

	if(!hDragWithinGroupsWnd || !right_drag_attach_group)
		return FALSE;

	lpMMTaskListLongPtr = lpRightDragAttachTaskListLongPtr;
	right_button_group = right_drag_attach_group;
	lpRightDragAttachTaskListLongPtr = 0;
	right_drag_attach_group = NULL;

	button_group_tracked = TaskbarGetTrackedButtonGroup(lpMMTaskListLongPtr);
	if(button_group_tracked != right_button_group)
		return FALSE;

	button_index_tracked = *EV_MM_TASKLIST_TRACKED_BUTTON_INDEX(lpMMTaskListLongPtr);
	if(button_index_tracked < 0)
		return FALSE;

	return DragWithinGroup(button_group_tracked, button_index_tracked);
}

void ComFuncMoveNearMatching(HWND hButtonWnd)
{
	SECONDARY_TASK_LIST_GET secondary_task_list_get;
	LONG_PTR lpSecondaryTaskListLongPtr;

	MMMoveNearMatching(lpTaskListLongPtr, hButtonWnd);

	if((TaskbarGetPreference(lpTaskListLongPtr) & 0x100) == 0)
	{
		lpSecondaryTaskListLongPtr = SecondaryTaskListGetFirstLongPtr(&secondary_task_list_get);
		while(lpSecondaryTaskListLongPtr)
		{
			MMMoveNearMatching(lpSecondaryTaskListLongPtr, hButtonWnd);
			lpSecondaryTaskListLongPtr = SecondaryTaskListGetNextLongPtr(&secondary_task_list_get);
		}
	}
}

static BOOL MMMoveNearMatching(LONG_PTR lpMMTaskListLongPtr, HWND hButtonWnd)
{
	LONG_PTR *plp;
	int button_groups_count;
	LONG_PTR **button_groups;
	int button_group_type;
	int buttons_count;
	LONG_PTR **buttons;
	LONG_PTR *task_group;
	ITEMIDLIST *pItemIdList;
	WCHAR *pAppId;
	int nMatch;
	HRESULT hr;
	int i;

	if(!bTaskGroupFunctionsHooked)
		return FALSE; // pDoesWindowMatch is not available

	plp = (LONG_PTR *)*EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lpMMTaskListLongPtr);
	if(plp)
	{
		button_groups_count = (int)plp[0];
		button_groups = (LONG_PTR **)plp[1];

		if(button_groups_count > 1)
		{
			button_group_type = (int)button_groups[button_groups_count - 1][DO2(6, 8)];
			if(button_group_type == 1)
			{
				plp = (LONG_PTR *)button_groups[button_groups_count - 1][DO2(5, 7)];

				buttons_count = (int)plp[0];
				buttons = (LONG_PTR **)plp[1];

				if(buttons_count == 1 && GetButtonWnd(buttons[0]) == hButtonWnd)
				{
					task_group = (LONG_PTR *)button_groups[button_groups_count - 1][DO2(3, 4)];

					pItemIdList = (ITEMIDLIST *)task_group[6];

					pAppId = *EV_TASKGROUP_APPID(task_group);
					if(pAppId)
					{
						for(i = (button_groups_count - 1) - 1; i >= 0; i--)
						{
							task_group = (LONG_PTR *)button_groups[i][DO2(3, 4)];

							hr = ((HRESULT(__stdcall *)(LONG_PTR *, HWND, ITEMIDLIST *, WCHAR *, int *, LONG_PTR **))pDoesWindowMatch)
								(task_group, hButtonWnd, pItemIdList, pAppId, &nMatch, NULL);
							if(SUCCEEDED(hr) && nMatch >= 1 && nMatch <= 3)
							{
								if(i + 1 < button_groups_count - 1)
									TaskbarMoveGroup(lpMMTaskListLongPtr, button_groups_count - 1, i + 1);

								return TRUE;
							}
						}
					}
				}
			}
		}
	}

	return FALSE;
}

static BOOL CustomGetLabelAppidListValue(WCHAR *pAppId, int *pnListValue)
{
	if(pAppId && GetAppidListValue(AILIST_LABEL, pAppId, pnListValue))
	{
		return TRUE;
	}
	else if(nOptionsEx[OPT_EX_SHOW_LABELS] == 1)
	{
		*pnListValue = AILIST_LABEL_NEVER;
		return TRUE;
	}
	else if(nOptionsEx[OPT_EX_SHOW_LABELS] == 2)
	{
		*pnListValue = AILIST_LABEL_ALWAYS;
		return TRUE;
	}

	return FALSE;
}

void ComFuncSetCreatedThumb(HWND hThumb, HWND hThumbParent)
{
	hCreatedThumb = hThumb;
	hCreatedThumbParent = hThumbParent;
}

void ComFuncSwitchToHookEnable(int nOption, LONG_PTR lpMMTaskListLongPtr)
{
	nSwitchToOption = nOption;
	lpSwitchToMMTaskListLongPtr = lpMMTaskListLongPtr;
}

void ComFuncSwitchToHookDisable()
{
	nSwitchToOption = 0;
}

LONG_PTR *ComFuncGetLastActiveTaskItem()
{
	return last_active_task_item;
}

void ComFuncResetLastActiveTaskItem()
{
	last_active_task_item = NULL;
}

BOOL ComFuncIsInGetIdealSpan()
{
	return bInGetIdealSpan;
}

BOOL ComFuncIsInHandleDelayInitStuff()
{
	return bInHandleDelayInitStuff;
}

void ComFuncSetTaskItemGetWindowReturnNull(BOOL bSet)
{
	bTaskItemGetWindowReturnNull = bSet;
	lpTaskItemGetWindowSavedValue = 0;
}

LONG_PTR ComFuncGetSavedTaskItemGetWindow()
{
	return lpTaskItemGetWindowSavedValue;
}
