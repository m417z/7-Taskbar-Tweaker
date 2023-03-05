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

// DEF3264: CTaskThumbnail_CreateInstance
#define EV_TASK_THUMBNAIL_SIZE                                       DO11_3264(0x7C, 0xB0, 0x74, 0xB0, ,, ,, 0x7C, 0xB8, ,, ,, ,, 0x80, 0xC0, ,, 0x88, 0xD0)

// Maximum values for 32-bit and 64-bit of the DEF3264 above.
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
#define FUNC_CTaskBand_GetUserPreferences(plp)                       (plp[DO5(23, 7, , , 6)])

// CTaskBand::IsHorizontal
#define FUNC_CTaskBand_IsHorizontal(plp)                             (plp[DO5(25, 13, , , 12)])

////////////////////

// CTaskBand::ApplicationChanged(this, immersive_application, probably_flags, hwnd_unused)
#define FUNC_CTaskBand_ApplicationChanged(plp)                       ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, LONG_PTR, HWND))plp[DO5(0, , , , 3)])

////////////////////

// CTaskBand::CurrentVirtualDesktopChangedAnimated
#define FUNC_CTaskBand_CurrentVirtualDesktopChangedAnimated(plp)     (plp[3])

////////////////////

// CTaskBand::Exec
#define FUNC_CTaskBand_Exec(plp)                                     (plp[4])

////////////////////

// CTaskBand::ViewVirtualDesktopChanged(this, application_view)
#define FUNC_CTaskBand_ViewVirtualDesktopChanged(plp)                ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR))plp[DO15(0, , , , 7, , , , , , , , , 9, 10)])

// CTaskBand::CurrentVirtualDesktopChanged
#define FUNC_CTaskBand_CurrentVirtualDesktopChanged(plp)             (plp[DO15(0, , , , 8, , , , , , , , , 10, 11)])

////////////////////

// CTaskBand::GetIconId
#define FUNC_CTaskBand_GetIconId(plp)                                (plp[4])

// CTaskBand::SwitchTo(this, task_item, true_means_bring_to_front_false_means_toggle_minimize_restore)
#define FUNC_CTaskBand_SwitchTo(plp)                                 ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, BOOL))plp[DO5(7, , , , 6)])

// CTaskBand::Launch(this, task_group, p_point, run_as_admin)
#define FUNC_CTaskBand_Launch(plp)                                   ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, POINT *, BYTE))plp[DO5(8, , , , 7)])

// Only Windows 7
// CTaskBand::Launch(this, task_group)
#define FUNC_CTaskBand_Launch_w7(plp)                                ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *))plp[10])

// Since Windows 8.1.1
// CTaskBand::CloseItem(this, task_item)
#define FUNC_CTaskBand_CloseItem(plp)                                ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *))plp[DO5(0, 0, 0, 15, 14)])

// Until Windows 10
// CTaskBand::GetIconSize
#define FUNC_CTaskBand_GetIconSize(plp)                              (plp[DO4(0, 0, 20, 23)])

////////////////////

// CSecondaryTaskBand::GetUserPreferences
#define FUNC_CSecondaryTaskBand_GetUserPreferences(plp)              (plp[DO5(7, , , , 6)])

// CSecondaryTaskBand::IsHorizontal
#define FUNC_CSecondaryTaskBand_IsHorizontal(plp)                    (plp[DO5(13, , , , 12)])

////////////////////

// CTaskListWnd::StartAnimation(this, button_group, animation_id)
#define FUNC_CTaskListWnd_StartAnimation(plp)                        ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR))plp[5])

////////////////////

// CTaskListWnd::GetStuckPlace
#define FUNC_CTaskListWnd_GetStuckPlace(plp)                         (plp[4])

// CTaskListWnd::ShowLivePreview(this, hWnd, uFlags)
#define FUNC_CTaskListWnd_ShowLivePreview(plp)                       ((LONG_PTR(__stdcall *)(LONG_PTR, HWND, LONG_PTR))plp[DO5(18, , , , 16)])

////////////////////

// CTaskListWnd::Initialize
#define FUNC_CTaskListWnd_Initialize(plp)                            (plp[3])

// CTaskListWnd::TaskCreated
#define FUNC_CTaskListWnd_TaskCreated(plp)                           (plp[DO5(3, 4, , , 5)])

// CTaskListWnd::ActivateTask
#define FUNC_CTaskListWnd_ActivateTask(plp)                          (plp[DO16(5, 6, , , 7, , , , , , , , 9, , , 8)])

// Until Windows 10 R1
// CTaskListWnd::TaskDestroyed
#define FUNC_CTaskListWnd_TaskDestroyed(plp)                         (plp[DO5(7, 8, , , 9)])

// CTaskListWnd::TaskInclusionChanged
#define FUNC_CTaskListWnd_TaskInclusionChanged(plp)                  (plp[DO16(0, , , , 13, , , , , , , , 15, , , 14)])

// CTaskListWnd::GetButtonHeight
#define FUNC_CTaskListWnd_GetButtonHeight(plp)                       (plp[DO16(11, 14, , , 15, , , , , , , , 17, , , 16)])

// CTaskListWnd::AutoSize(this)
#define FUNC_CTaskListWnd_AutoSize(plp)                              ((LONG_PTR(__stdcall *)(LONG_PTR))plp[DO16(15, 18, , , 19, , , , , , , , 21, , , 20)])

// CTaskListWnd::DismissHoverUI(this, hide_without_animation)
#define FUNC_CTaskListWnd_DismissHoverUI(plp)                        ((LONG_PTR(__stdcall *)(LONG_PTR, BOOL))plp[DO16( \
                                                                         27, 41, , 47, 50, 51, 55, , , 53, , \
                                                                         nExplorerQFE <= 1500 ? 53 : (nExplorerQFE <= 1533 ? 54 : 55), \
                                                                         nExplorerQFE <= 928 ? 55 : (nExplorerQFE <= 964 ? 56 : 57), \
                                                                         57, 58, 57)])

// CTaskListWnd::ShowJumpView
#define FUNC_CTaskListWnd_ShowJumpView(plp)                          (plp[DO16( \
                                                                         30, 44, , 50, 53, 54, 58, , , 56, , \
                                                                         nExplorerQFE <= 1500 ? 56 : (nExplorerQFE <= 1533 ? 57 : 58), \
                                                                         nExplorerQFE <= 928 ? 58 : (nExplorerQFE <= 964 ? 59 : 60), \
                                                                         60, 61, 60)])

////////////////////

// Until Windows 10 R1
// CTaskListWnd::OnDestinationMenuDismissed
#define FUNC_CTaskListWnd_OnDestinationMenuDismissed(plp)            (plp[DO2(8, 5)])

////////////////////

// CTaskListWndMulti::TaskDestroyed(this, ITaskGroup *, ITaskItem *)
#define FUNC_CTaskListWndMulti_TaskDestroyed(plp)                    ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, LONG_PTR *))plp[DO16(0, , , , 9, , , , , , , , 11, , , 10)])

////////////////////

// CTaskListThumbnailWnd::DisplayUI
#define FUNC_CTaskListThumbnailWnd_DisplayUI(plp)                    (plp[4])

////////////////////

// Until Windows 10
// CTaskListThumbnailWnd::GetThumbRectFromIndex
#define FUNC_CTaskListThumbnailWnd_GetThumbRectFromIndex(plp)        (plp[7])

// Until Windows 10
// CTaskListThumbnailWnd::ThumbIndexFromPoint(this, ppt)
#define FUNC_CTaskListThumbnailWnd_ThumbIndexFromPoint(plp)          ((int(__stdcall *)(LONG_PTR, POINT *))plp[DO2(13, 14)])

////////////////////

// CTaskListThumbnailWnd::DestroyThumbnail
#define FUNC_CTaskListThumbnailWnd_DestroyThumbnail(plp)             (plp[5])

////////////////////

// CWindowTaskItem::SetWindow(this, new_hwnd)
#define FUNC_CWindowTaskItem_SetWindow(plp)                          ((LONG_PTR(__stdcall *)(LONG_PTR, HWND))plp[DO7(18, , , 17, 13, , 11)])

// CWindowTaskItem::GetWindow(this)
#define FUNC_CWindowTaskItem_GetWindow(plp)                          ((HWND(__stdcall *)(LONG_PTR))plp[DO7(19, , , 18, 14, , 12)])

// CWindowTaskItem::IsImmersive(this)
#define FUNC_CWindowTaskItem_IsImmersive(plp)                        ((BYTE(__stdcall *)(LONG_PTR))plp[DO7(0, , , , 58, , 56)])

////////////////////

// CTaskItem::IsVisibleOnCurrentVirtualDesktop(this)
#define FUNC_CTaskItem_IsVisibleOnCurrentVirtualDesktop(plp)         ((BYTE(__stdcall *)(LONG_PTR))plp[DO16(0, , , , 60, , 58, , , , 60, , , , , 58)])

// CTaskItem::SetVisibleOnCurrentVirtualDesktop(this, bool)
#define FUNC_CTaskItem_SetVisibleOnCurrentVirtualDesktop(plp)        ((LONG_PTR(__stdcall *)(LONG_PTR, BYTE))plp[DO16(0, , , , 61, , 59, , , , 61, , , , , 59)])

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
#define FUNC_CTaskGroup_GroupMenuCommand(plp)                        ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR, WPARAM))plp[DO11(0, , , , 26, , , , , , 27)])

// CTaskGroup::IsImmersiveGroup(this)
#define FUNC_CTaskGroup_IsImmersiveGroup(plp)                        ((BYTE(__stdcall *)(LONG_PTR))plp[DO11(0, , , , 41, , 39, , , , 40)])

////////////////////

// CTaskBtnGroup::RemoveTaskItem
#define FUNC_CTaskBtnGroup_RemoveTaskItem(plp)                       (plp[DO5(8, , , , 9)])

// CTaskBtnGroup::GetIdealSpan
#define FUNC_CTaskBtnGroup_GetIdealSpan(plp)                         (plp[DO5(10, 12, , , 13)])

// CTaskBtnGroup::GetLocation(this, task_item, p_rect)
#define FUNC_CTaskBtnGroup_GetLocation(plp)                          ((LONG_PTR(__stdcall *)(LONG_PTR, LONG_PTR *, RECT *))plp[DO5(13, , , , 14)])

// CTaskBtnGroup::SetLocation
#define FUNC_CTaskBtnGroup_SetLocation(plp)                          (plp[DO5(14, 16, , , 17)])

// CTaskBtnGroup::Render
#define FUNC_CTaskBtnGroup_Render(plp)                               (plp[DO5(18, 20, , , 21)])

// CTaskBtnGroup::CanGlom
#define FUNC_CTaskBtnGroup_CanGlom(plp)                              (plp[DO5(20, 22, , , 23)])

// CTaskBtnGroup::Glom
#define FUNC_CTaskBtnGroup_Glom(plp)                                 ((LONG_PTR(__stdcall *)(LONG_PTR *, BOOL))plp[DO5(22, 24, , , 25)])

// Until Windows 10
// CTaskBtnGroup::HandleGroupHotTracking
#define FUNC_CTaskBtnGroup_HandleGroupHotTracking(plp)               (plp[DO2(28, 30)])

// Until Windows 10
// CTaskBtnGroup::HandleGroupHotTrackOut
#define FUNC_CTaskBtnGroup_HandleGroupHotTrackOut(plp)               (plp[DO2(29, 31)])

// CTaskBtnGroup::StartItemAnimation
#define FUNC_CTaskBtnGroup_StartItemAnimation(plp)                   (plp[32])

// CTaskBtnGroup::HasItemAnimation
#define FUNC_CTaskBtnGroup_HasItemAnimation(plp)                     (plp[33])

// CTaskBtnGroup::ShouldShowToolTip
#define FUNC_CTaskBtnGroup_ShouldShowToolTip(plp)                    (plp[DO2(34, 36)])

// CTaskBtnGroup::GetNumStacks(this)
#define FUNC_CTaskBtnGroup_GetNumStacks(plp)                         ((int(__stdcall *)(LONG_PTR))plp[DO5(41, , , , 40)])
