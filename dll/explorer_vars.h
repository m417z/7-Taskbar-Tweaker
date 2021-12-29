#pragma once

#include "functions.h"

//////////////////////////////////////////////////////////////////////////
// CTray (lpTaskbarLongPtr)

LONG_PTR EV_TRAY_UI(LONG_PTR lp);
BOOL *EV_TASKBAR_AUTOPOS_FLAG(void);
BYTE *EV_TASKBAR_AUTOPOS_FLAG_BYTE(void);
DWORD *EV_TASKBAR_SETTINGS(void);
HWND *EV_TASKBAR_TRAY_NOTIFY_WND(void);
HWND *EV_TASKBAR_TASKBAND_WND(void);
int *EV_TASKBAR_POS(void);
BOOL *EV_TASKBAR_UNLOCKED_FLAG(void);
BOOL *EV_TASKBAR_TOPMOST_EX_FLAG(void);
HWND *EV_TASKBAR_START_BTN_WND(void);
LONG_PTR *EV_TASKBAR_START_BTN_LONG_PTR(void);
LONG_PTR *EV_TASKBAR_BACK_LONG_PTR(void);
LONG_PTR *EV_TASKBAR_SEARCH_LONG_PTR(void);
LONG_PTR *EV_TASKBAR_CORTANA_LONG_PTR(void);
LONG_PTR *EV_TASKBAR_TRAY_SEARCH_CONTROL(void);
LONG_PTR *EV_TASKBAR_MULTITASKING_LONG_PTR(void);
void *EV_TASKBAR_W7_START_BTN_CLASS(void);
int *EV_TASKBAR_W7_WIDTH_PADDING(void);
int *EV_TASKBAR_W7_HEIGHT_PADDING(void);

//////////////////////////////////////////////////////////////////////////
// CSecondaryTray (lpSecondaryTaskbarLongPtr)

HWND *EV_SECONDARY_TASKBAR_HWND(LONG_PTR lp);
LONG_PTR *EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_REF(LONG_PTR lp);
LONG_PTR EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_LONG_PTR_VALUE(LONG_PTR lp);
int *EV_SECONDARY_TASKBAR_POS(LONG_PTR lp);
HMONITOR *EV_SECONDARY_TASKBAR_MONITOR(LONG_PTR lp);
HWND *EV_SECONDARY_TASKBAR_START_BTN_WND(LONG_PTR lp);
LONG_PTR *EV_SECONDARY_TASKBAR_START_BTN_LONG_PTR(LONG_PTR lp);
LONG_PTR *EV_SECONDARY_TASKBAR_SEARCH_LONG_PTR(LONG_PTR lp);
LONG_PTR *EV_SECONDARY_TASKBAR_CORTANA_LONG_PTR(LONG_PTR lp);
LONG_PTR *EV_SECONDARY_TASKBAR_MULTITASKING_LONG_PTR(LONG_PTR lp);
LONG_PTR *EV_SECONDARY_TASKBAR_CLOCK_LONG_PTR(LONG_PTR lp);

//////////////////////////////////////////////////////////////////////////
// CTaskBand (lpTaskSwLongPtr)

DWORD *EV_TASK_SW_PREFERENCES(void);
HDPA *EV_TASK_SW_TASK_GROUPS_HDPA(void);
LONG_PTR *EV_TASK_SW_MULTI_TASK_LIST_REF(void);
UINT *EV_TASK_SW_SYS_FROSTED_WINDOW_MSG(void);
LONG_PTR *EV_TASK_SW_APP_VIEW_MGR(void);

//////////////////////////////////////////////////////////////////////////
// CSecondaryTaskBand (lpSecondaryTaskBandLongPtr)

HWND *EV_SECONDARY_TASK_BAND_HWND(LONG_PTR lp);
DWORD *EV_SECONDARY_TASK_BAND_PREFERENCES(LONG_PTR lp);
LONG_PTR *EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_REF(LONG_PTR lp);
LONG_PTR EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(LONG_PTR lp);

//////////////////////////////////////////////////////////////////////////
// CTaskListWnd (lpMMTaskListLongPtr)

HWND *EV_MM_TASKLIST_HWND(LONG_PTR lp);
LONG_PTR *EV_MM_TASKLIST_TASK_BAND_REF(LONG_PTR lp);
LONG_PTR EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(LONG_PTR lp);
HDPA *EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(LONG_PTR lp);
HWND *EV_MM_TASKLIST_TOOLTIP_WND(LONG_PTR lp);
BOOL *EV_MM_TASKLIST_THUMB_DISABLING_FLAG(LONG_PTR lp);
LONG_PTR **EV_MM_TASKLIST_TRACKED_BUTTON_GROUP(LONG_PTR lp);
int *EV_MM_TASKLIST_TRACKED_BUTTON_INDEX(LONG_PTR lp);
LONG_PTR **EV_MM_TASKLIST_ACTIVE_BUTTON_GROUP(LONG_PTR lp);
int *EV_MM_TASKLIST_ACTIVE_BUTTON_INDEX(LONG_PTR lp);
LONG_PTR **EV_MM_TASKLIST_PRESSED_BUTTON_GROUP(LONG_PTR lp);
LONG_PTR **EV_MM_TASKLIST_THUMB_BUTTON_GROUP(LONG_PTR lp);
LONG_PTR *EV_MM_TASKLIST_MM_THUMBNAIL_LONG_PTR(LONG_PTR lp);
UINT *EV_MM_TASKLIST_TOOLTIP_TIMER_ID(LONG_PTR lp);
UINT *EV_MM_TASKLIST_THUMB_TIMER_ID(LONG_PTR lp);
HMONITOR *EV_MM_TASKLIST_HMONITOR(LONG_PTR lp);
LONG_PTR *EV_MM_TASKLIST_TASK_ITEM_FILTER(LONG_PTR lp);
DWORD *EV_MM_TASKLIST_DRAG_FLAG(LONG_PTR lp);
LONG_PTR **EV_MM_TASKLIST_ANIMATION_MANAGER(LONG_PTR lp);

//////////////////////////////////////////////////////////////////////////
// CTaskListThumbnailWnd (lpMMThumbnailLongPtr)

HWND *EV_MM_THUMBNAIL_HWND(LONG_PTR lp);
LONG_PTR *EV_MM_THUMBNAIL_MM_TASKLIST_REF(LONG_PTR lp);
LONG_PTR EV_MM_THUMBNAIL_MM_TASKLIST_LONG_PTR_VALUE(LONG_PTR lp);
BYTE *EV_MM_THUMBNAIL_REDRAW_FLAGS(LONG_PTR lp);
LONG_PTR **EV_MM_THUMBNAIL_TASK_GROUP(LONG_PTR lp);
HDPA *EV_MM_THUMBNAIL_THUMBNAILS_HDPA(LONG_PTR lp);
DWORD *EV_MM_THUMBNAIL_NUM_THUMBNAILS(LONG_PTR lp);
int *EV_MM_THUMBNAIL_ACTIVE_THUMB_INDEX(LONG_PTR lp);
int *EV_MM_THUMBNAIL_TRACKED_THUMB_INDEX(LONG_PTR lp);
int *EV_MM_THUMBNAIL_PRESSED_THUMB_INDEX(LONG_PTR lp);
BOOL *EV_MM_THUMBNAIL_STICKY_FLAG(LONG_PTR lp);
BOOL *EV_MM_THUMBNAIL_LIST_FLAG(LONG_PTR lp);
int *EV_MM_THUMBNAIL_LIST_FIRST_VISIBLE_INDEX(LONG_PTR lp);

//////////////////////////////////////////////////////////////////////////
// CTaskThumbnail

#define EV_TASK_THUMBNAIL_SIZE                                       DO2_3264(0x7C, 0xB0, 0, 0 /* omitted from public code */)

#define EV_TASK_THUMBNAIL_SIZE_BUFFER_ALL_WIN_VERSIONS               DEF3264(0x88, 0xD0)

//////////////////////////////////////////////////////////////////////////
// CPearl (start button)

HWND *EV_START_BUTTON_HWND(LONG_PTR lp);

//////////////////////////////////////////////////////////////////////////
// CTraySearchControl

HWND *EV_TRAY_SEARCH_CONTROL_BUTTON_HWND(LONG_PTR lp);

//////////////////////////////////////////////////////////////////////////
// CTrayButton

HWND *EV_TRAY_BUTTON_HWND(LONG_PTR lp);

//////////////////////////////////////////////////////////////////////////
// CTrayNotify (lpTrayNotifyLongPtr)

HWND *EV_TRAY_NOTIFY_CLOCK_WND(LONG_PTR lp);
LONG_PTR *EV_TRAY_NOTIFY_CLOCK_LONG_PTR(LONG_PTR lp);
HWND *EV_TRAY_NOTIFY_SHOW_DESKTOP_WND(LONG_PTR lp);
HWND *EV_TRAY_NOTIFY_OVERFLOW_TOOLBAR_WND(LONG_PTR lp);
HWND *EV_TRAY_NOTIFY_TEMPORARY_TOOLBAR_WND(LONG_PTR lp);
HWND *EV_TRAY_NOTIFY_TOOLBAR_WND(LONG_PTR lp);
BOOL *EV_TRAY_NOTIFY_CHEVRON_STATE(LONG_PTR lp);
BYTE *EV_TRAY_NOTIFY_PTRDEV_SUPPORTED(LONG_PTR lp);
BYTE *EV_TRAY_NOTIFY_PTRDEV_SUPPORTED_VALID(LONG_PTR lp);
HTHEME *EV_TRAY_NOTIFY_THEME(LONG_PTR lp);
DWORD *EV_TRAY_NOTIFY_DRAG_FLAG(LONG_PTR lp);

//////////////////////////////////////////////////////////////////////////
// CClockCtl (lpTrayClockLongPtr)
// Until Windows 10 R1

WCHAR *EV_TRAY_CLOCK_TEXT(LONG_PTR lp);
BOOL *EV_TRAY_CLOCK_TIMER_ENABLED_FLAG(LONG_PTR lp);
int *EV_TRAY_CLOCK_CACHED_TEXT_SIZE(LONG_PTR lp);

//////////////////////////////////////////////////////////////////////////
// ClockButton (lpTrayClockLongPtr)
// Since Windows 10 R1

HWND *EV_CLOCK_BUTTON_HWND(LONG_PTR lp);
BYTE *EV_CLOCK_BUTTON_SIZES_CACHED(LONG_PTR lp);
BYTE *EV_CLOCK_BUTTON_SHOW_SECONDS(LONG_PTR lp);
WORD *EV_CLOCK_BUTTON_HOURS_CACHE(LONG_PTR lp);
WORD *EV_CLOCK_BUTTON_MINUTES_CACHE(LONG_PTR lp);

//////////////////////////////////////////////////////////////////////////
// CTaskItem

LONG_PTR **EV_TASKITEM_CONTAINER_TASK_ITEM(LONG_PTR *plp);
HWND *EV_TASKITEM_WND(LONG_PTR *plp);

//////////////////////////////////////////////////////////////////////////
// CTaskGroup

HDPA *EV_TASKGROUP_TASKITEMS_HDPA(LONG_PTR *plp);
DWORD *EV_TASKGROUP_FLAGS(LONG_PTR *plp);
WCHAR **EV_TASKGROUP_APPID(LONG_PTR *plp);
int *EV_TASKGROUP_VISUAL_ORDER(LONG_PTR *plp);

//////////////////////////////////////////////////////////////////////////
// CApplicationViewManager

SRWLOCK *EV_APP_VIEW_MGR_APP_ARRAY_LOCK(LONG_PTR lp);
LONG_PTR **EV_APP_VIEW_MGR_APP_ARRAY(LONG_PTR lp);
size_t *EV_APP_VIEW_MGR_APP_ARRAY_SIZE(LONG_PTR lp);

//////////////////////////////////////////////////////////////////////////
// Functions

// CTaskBand::GetUserPreferences
#define FUNC_CTaskBand_GetUserPreferences(plp)                       (plp[DO2(23, 0 /* omitted from public code */)])

// CTaskBand::IsHorizontal
#define FUNC_CTaskBand_IsHorizontal(plp)                             (plp[DO2(25, 0 /* omitted from public code */)])

////////////////////

// CTaskBand::ApplicationChanged(this, immersive_application, probably_flags, hwnd_unused)
#define FUNC_CTaskBand_ApplicationChanged(plp)                       ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR, HWND))plp[DO2(0, 0 /* omitted from public code */)])

////////////////////

// CTaskBand::CurrentVirtualDesktopChangedAnimated
#define FUNC_CTaskBand_CurrentVirtualDesktopChangedAnimated(plp)     (plp[3])

////////////////////

// CTaskBand::Exec
#define FUNC_CTaskBand_Exec(plp)                                     (plp[4])

////////////////////

// CTaskBand::ViewVirtualDesktopChanged(this, application_view)
#define FUNC_CTaskBand_ViewVirtualDesktopChanged(plp)                ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR))plp[DO2(0, 0 /* omitted from public code */)])

// CTaskBand::CurrentVirtualDesktopChanged
#define FUNC_CTaskBand_CurrentVirtualDesktopChanged(plp)             (plp[DO2(0, 0 /* omitted from public code */)])

////////////////////

// CTaskBand::GetIconId
#define FUNC_CTaskBand_GetIconId(plp)                                (plp[4])

// CTaskBand::SwitchTo(this, task_item, true_means_bring_to_front_false_means_toggle_minimize_restore)
#define FUNC_CTaskBand_SwitchTo(plp)                                 ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, BOOL))plp[DO2(7, 0 /* omitted from public code */)])

// CTaskBand::Launch(this, task_group, p_point, run_as_admin)
#define FUNC_CTaskBand_Launch(plp)                                   ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, POINT *, BYTE))plp[DO2(8, 0 /* omitted from public code */)])

// Only Windows 7
// CTaskBand::Launch(this, task_group)
#define FUNC_CTaskBand_Launch_w7(plp)                                ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *))plp[10])

// Since Windows 8.1.1
// CTaskBand::CloseItem(this, task_item)
#define FUNC_CTaskBand_CloseItem(plp)                                ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *))plp[DO2(0, 0 /* omitted from public code */)])

// Until Windows 10
// CTaskBand::GetIconSize
#define FUNC_CTaskBand_GetIconSize(plp)                              (plp[DO2(0, 0 /* omitted from public code */)])

////////////////////

// CSecondaryTaskBand::GetUserPreferences
#define FUNC_CSecondaryTaskBand_GetUserPreferences(plp)              (plp[DO2(7, 0 /* omitted from public code */)])

// CSecondaryTaskBand::IsHorizontal
#define FUNC_CSecondaryTaskBand_IsHorizontal(plp)                    (plp[DO2(13, 0 /* omitted from public code */)])

////////////////////

// CTaskListWnd::StartAnimation(this, button_group, animation_id)
#define FUNC_CTaskListWnd_StartAnimation(plp)                        ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR))plp[5])

////////////////////

// CTaskListWnd::GetStuckPlace
#define FUNC_CTaskListWnd_GetStuckPlace(plp)                         (plp[4])

// CTaskListWnd::ShowLivePreview(this, hWnd, uFlags)
#define FUNC_CTaskListWnd_ShowLivePreview(plp)                       ((LONG_PTR(__stdcall *)(LONG_PTR, HWND, LONG_PTR))plp[DO2(18, 0 /* omitted from public code */)])

////////////////////

// CTaskListWnd::Initialize
#define FUNC_CTaskListWnd_Initialize(plp)                            (plp[3])

// CTaskListWnd::TaskCreated
#define FUNC_CTaskListWnd_TaskCreated(plp)                           (plp[DO2(3, 0 /* omitted from public code */)])

// CTaskListWnd::ActivateTask
#define FUNC_CTaskListWnd_ActivateTask(plp)                          (plp[DO2(5, 0 /* omitted from public code */)])

// Until Windows 10 R1
// CTaskListWnd::TaskDestroyed
#define FUNC_CTaskListWnd_TaskDestroyed(plp)                         (plp[DO2(7, 0 /* omitted from public code */)])

// CTaskListWnd::TaskInclusionChanged
#define FUNC_CTaskListWnd_TaskInclusionChanged(plp)                  (plp[DO2(0, 0 /* omitted from public code */)])

// CTaskListWnd::GetButtonHeight
#define FUNC_CTaskListWnd_GetButtonHeight(plp)                       (plp[DO2(11, 0 /* omitted from public code */)])

// CTaskListWnd::AutoSize(this)
#define FUNC_CTaskListWnd_AutoSize(plp)                              ((LONG_PTR(__stdcall *)(LONG_PTR))plp[DO2(15, 0 /* omitted from public code */)])

// CTaskListWnd::DismissHoverUI(this, hide_without_animation)
#define FUNC_CTaskListWnd_DismissHoverUI(plp)                        ((LONG_PTR(__stdcall *)(LONG_PTR, BOOL))plp[DO2(27, 0 /* omitted from public code */)])

// CTaskListWnd::ShowJumpView
#define FUNC_CTaskListWnd_ShowJumpView(plp)                          (plp[DO2(30, 0 /* omitted from public code */)])

////////////////////

// Until Windows 10 R1
// CTaskListWnd::OnDestinationMenuDismissed
#define FUNC_CTaskListWnd_OnDestinationMenuDismissed(plp)            (plp[DO2(8, 0 /* omitted from public code */)])

////////////////////

// CTaskListWndMulti::TaskDestroyed(this, ITaskGroup *, ITaskItem *)
#define FUNC_CTaskListWndMulti_TaskDestroyed(plp)                    ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *))plp[DO2(0, 0 /* omitted from public code */)])

////////////////////

// CTaskListThumbnailWnd::DisplayUI
#define FUNC_CTaskListThumbnailWnd_DisplayUI(plp)                    (plp[4])

////////////////////

// Until Windows 10
// CTaskListThumbnailWnd::GetThumbRectFromIndex
#define FUNC_CTaskListThumbnailWnd_GetThumbRectFromIndex(plp)        (plp[7])

// Until Windows 10
// CTaskListThumbnailWnd::ThumbIndexFromPoint(this, ppt)
#define FUNC_CTaskListThumbnailWnd_ThumbIndexFromPoint(plp)          ((int(__stdcall *)(LONG_PTR, POINT *))plp[DO2(13, 0 /* omitted from public code */)])

////////////////////

// CTaskListThumbnailWnd::DestroyThumbnail
#define FUNC_CTaskListThumbnailWnd_DestroyThumbnail(plp)             (plp[5])

////////////////////

// CWindowTaskItem::SetWindow(this, new_hwnd)
#define FUNC_CWindowTaskItem_SetWindow(plp)                          ((LONG_PTR(__stdcall *)(LONG_PTR, HWND))plp[DO2(18, 0 /* omitted from public code */)])

// CWindowTaskItem::GetWindow(this)
#define FUNC_CWindowTaskItem_GetWindow(plp)                          ((HWND(__stdcall *)(LONG_PTR))plp[DO2(19, 0 /* omitted from public code */)])

// CWindowTaskItem::IsImmersive(this)
#define FUNC_CWindowTaskItem_IsImmersive(plp)                        ((BYTE(__stdcall *)(LONG_PTR))plp[DO2(0, 0 /* omitted from public code */)])

////////////////////

// CTaskItem::IsVisibleOnCurrentVirtualDesktop(this)
#define FUNC_CTaskItem_IsVisibleOnCurrentVirtualDesktop(plp)         ((BYTE(__stdcall *)(LONG_PTR))plp[DO2(0, 0 /* omitted from public code */)])

// CTaskItem::SetVisibleOnCurrentVirtualDesktop(this, bool)
#define FUNC_CTaskItem_SetVisibleOnCurrentVirtualDesktop(plp)        ((LONG_PTR(__stdcall *)(LONG_PTR, BYTE))plp[DO2(0, 0 /* omitted from public code */)])

////////////////////

// CTaskGroup::DoesWindowMatch
#define FUNC_CTaskGroup_DoesWindowMatch(plp)                         (plp[7])

// CTaskGroup::GetNumTabs(this, a, b)
#define FUNC_CTaskGroup_GetNumTabs(plp)                              ((int(__stdcall *)(LONG_PTR, int *, int *))plp[10])

// Only Windows 7
// CTaskGroup::GroupMenuCommand(this, wCommand)
#define FUNC_CTaskGroup_GroupMenuCommand_w7(plp)                     ((LONG_PTR(__stdcall *)(LONG_PTR, WPARAM))plp[30])

// Only Windows 8
// CTaskGroup::GroupMenuCommand(this, hMonitor, wCommand)
#define FUNC_CTaskGroup_GroupMenuCommand_w8(plp)                     ((LONG_PTR(__stdcall *)(LONG_PTR, HMONITOR, WPARAM))plp[30])

// CTaskGroup::GroupMenuCommand(this, ITaskItemFilter, wCommand)
#define FUNC_CTaskGroup_GroupMenuCommand(plp)                        ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, WPARAM))plp[DO2(0, 0 /* omitted from public code */)])

// CTaskGroup::IsImmersiveGroup(this)
#define FUNC_CTaskGroup_IsImmersiveGroup(plp)                        ((BYTE(__stdcall *)(LONG_PTR))plp[DO2(0, 0 /* omitted from public code */)])

////////////////////

// CTaskBtnGroup::RemoveTaskItem
#define FUNC_CTaskBtnGroup_RemoveTaskItem(plp)                       (plp[DO2(8, 0 /* omitted from public code */)])

// CTaskBtnGroup::GetIdealSpan
#define FUNC_CTaskBtnGroup_GetIdealSpan(plp)                         (plp[DO2(10, 0 /* omitted from public code */)])

// CTaskBtnGroup::GetLocation(this, task_item, p_rect)
#define FUNC_CTaskBtnGroup_GetLocation(plp)                          ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, RECT *))plp[DO2(13, 0 /* omitted from public code */)])

// CTaskBtnGroup::SetLocation
#define FUNC_CTaskBtnGroup_SetLocation(plp)                          (plp[DO2(14, 0 /* omitted from public code */)])

// CTaskBtnGroup::Render
#define FUNC_CTaskBtnGroup_Render(plp)                               (plp[DO2(18, 0 /* omitted from public code */)])

// CTaskBtnGroup::CanGlom
#define FUNC_CTaskBtnGroup_CanGlom(plp)                              (plp[DO2(20, 0 /* omitted from public code */)])

// CTaskBtnGroup::Glom
#define FUNC_CTaskBtnGroup_Glom(plp)                                 ((LONG_PTR(__stdcall *)(LONG_PTR *, BOOL))plp[DO2(22, 0 /* omitted from public code */)])

// Until Windows 10
// CTaskBtnGroup::HandleGroupHotTracking
#define FUNC_CTaskBtnGroup_HandleGroupHotTracking(plp)               (plp[DO2(28, 0 /* omitted from public code */)])

// Until Windows 10
// CTaskBtnGroup::HandleGroupHotTrackOut
#define FUNC_CTaskBtnGroup_HandleGroupHotTrackOut(plp)               (plp[DO2(29, 0 /* omitted from public code */)])

// CTaskBtnGroup::StartItemAnimation
#define FUNC_CTaskBtnGroup_StartItemAnimation(plp)                   (plp[32])

// CTaskBtnGroup::HasItemAnimation
#define FUNC_CTaskBtnGroup_HasItemAnimation(plp)                     (plp[33])

// CTaskBtnGroup::ShouldShowToolTip
#define FUNC_CTaskBtnGroup_ShouldShowToolTip(plp)                    (plp[DO2(34, 0 /* omitted from public code */)])

// CTaskBtnGroup::GetNumStacks(this)
#define FUNC_CTaskBtnGroup_GetNumStacks(plp)                         ((int(__stdcall *)(LONG_PTR))plp[DO2(41, 0 /* omitted from public code */)])
