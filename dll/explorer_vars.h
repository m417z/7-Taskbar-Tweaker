#pragma once

#include "functions.h"

//////////////////////////////////////////////////////////////////////////
// CTray (lpTaskbarLongPtr)

// In Windows 8.1 Update 1, since build 17930, a new virtual table pointer appeared.
// The change was caused by update KB3072318.
// In Windows 10 R4, since 10.0.17763.677 (after 10.0.17763.165, not including), some field(s) were removed.
// The change was caused by update KB4489894.
// In Windows 10 R5, since 10.0.17763.346 (after 10.0.17763.107, not including), some field(s) were removed.
// The change was caused by update KB4482887.
#define EV_TASKBAR_OFFSET_FIX(offset)       ((offset) + \
                                                      ((nWinVersion == WIN_VERSION_811 && nExplorerQFE >= 17930 && nExplorerQFE < 20000 && (offset) >= 8 * sizeof(LONG_PTR)) ? sizeof(LONG_PTR) : \
                                                      ((((nWinVersion == WIN_VERSION_10_R4 && nExplorerQFE >= 677) || (nWinVersion == WIN_VERSION_10_R5 && nExplorerQFE >= 346)) \
                                                          && (offset) >= DEF3264(0x8C, 0xD0)) ? (-(int)sizeof(LONG_PTR)) : 0)))
#define EV_TRAY_UI_OFFSET_FIX(offset)       ((nWinVersion < WIN_VERSION_10_R2) ? EV_TASKBAR_OFFSET_FIX(offset) : (offset))

// Pointer to the TrayUI instance, since Windows 10 R2.
#define EV_TRAY_UI(lp)                      DO2_3264((lp), (lp), 0, 0 /* omitted from public code */)

// Until Windows 10
#define EV_TASKBAR_AUTOPOS_FLAG            ((BOOL *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX( \
                                                     DO2_3264(0x24, 0x3C, 0, 0 /* omitted from public code */))))
// Since Windows 10
#define EV_TASKBAR_AUTOPOS_FLAG_BYTE       ((BYTE *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX( \
                                                     0 /* omitted from public code */)))

// Bit 0x01 - autohide is on, bit 0x02 - taskbar is hidden.
#define EV_TASKBAR_SETTINGS               ((DWORD *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX( \
                                                     DO2_3264(0x40, 0x60, 0, 0 /* omitted from public code */))))

#define EV_TASKBAR_TRAY_NOTIFY_WND         ((HWND *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX( \
                                                     DO2_3264(0xC04, 0xD70, 0, 0 /* omitted from public code */))))

#define EV_TASKBAR_TASKBAND_WND            ((HWND *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX( \
                                                     DO2_3264(0xCA8, 0xE58, 0, 0 /* omitted from public code */))))

#define EV_TASKBAR_POS                      ((int *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX( \
                                                     DO2_3264(0xCD8, 0xEB0, 0, 0 /* omitted from public code */))))

#define EV_TASKBAR_UNLOCKED_FLAG           ((BOOL *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX( \
                                                     DO2_3264(0xD28, 0xF24, 0, 0 /* omitted from public code */))))

#define EV_TASKBAR_TOPMOST_EX_FLAG         ((BOOL *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX( \
                                                     DO2_3264(0xD98, 0xFAC, 0, 0 /* omitted from public code */))))

// Until Windows 10
#define EV_TASKBAR_START_BTN_WND           ((HWND *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX( \
                                                     DO2_3264(0xAF8 + 0x18, 0xC20 + 0x28, 0, 0 /* omitted from public code */))))

// Since Windows 10
#define EV_TASKBAR_START_BTN_LONG_PTR       ((LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(0 /* omitted from public code */)))
#define EV_TASKBAR_BACK_LONG_PTR            ((LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(0 /* omitted from public code */)))
#define EV_TASKBAR_SEARCH_LONG_PTR          ((LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(0 /* omitted from public code */)))
#define EV_TASKBAR_CORTANA_LONG_PTR         ((LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(0 /* omitted from public code */)))
#define EV_TASKBAR_EXTRA_BTN_HWNDS             ((HWND **)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(0 /* omitted from public code */)))
#define EV_TASKBAR_MULTITASKING_LONG_PTR    ((LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(0 /* omitted from public code */)))

#define EV_TASKBAR_W7_START_BTN_CLASS      ((void *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(DEF3264(0xAF8, 0xC20))))

#define EV_TASKBAR_W7_WIDTH_PADDING         ((int *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(DEF3264(0xB18, 0xC58))))

#define EV_TASKBAR_W7_HEIGHT_PADDING        ((int *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(DEF3264(0xB1C, 0xC5C))))

//////////////////////////////////////////////////////////////////////////
// CSecondaryTray (lpSecondaryTaskbarLongPtr)

#define EV_SECONDARY_TASKBAR_HWND(lp)               ((HWND *)((lp) + DEF3264(0x04, 0x08)))

#define EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_REF(lp) \
                                                ((LONG_PTR *)((lp) + DO2_3264(0x34, 0x58, 0, 0 /* omitted from public code */)))
#define EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_LONG_PTR_VALUE(lp) \
                                                             (*EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_REF(lp) - DEF3264(0x14, 0x28))

#define EV_SECONDARY_TASKBAR_POS(lp)                 ((int *)((lp) + DEF3264(0x10, 0x20) + DO2_3264(0x14, 0x18, 0, 0 /* omitted from public code */)))

#define EV_SECONDARY_TASKBAR_MONITOR(lp)         (HMONITOR *)((lp) + DEF3264(0x10, 0x20) + DO2_3264(0x18, 0x20, 0, 0 /* omitted from public code */))

// Until Windows 10
#define EV_SECONDARY_TASKBAR_START_BTN_WND(lp)      ((HWND *)((lp) + DO2_3264(0xAC, 0xE0, 0, 0 /* omitted from public code */)))

// Since Windows 10
#define EV_SECONDARY_TASKBAR_START_BTN_LONG_PTR(lp)       ((LONG_PTR *)((lp) + 0 /* omitted from public code */))
#define EV_SECONDARY_TASKBAR_SEARCH_LONG_PTR(lp)          ((LONG_PTR *)((lp) + 0 /* omitted from public code */))
#define EV_SECONDARY_TASKBAR_CORTANA_LONG_PTR(lp)         ((LONG_PTR *)((lp) + 0 /* omitted from public code */))
#define EV_SECONDARY_TASKBAR_MULTITASKING_LONG_PTR(lp)    ((LONG_PTR *)((lp) + 0 /* omitted from public code */))
#define EV_SECONDARY_TASKBAR_CLOCK_LONG_PTR(lp)           ((LONG_PTR *)((lp) + 0 /* omitted from public code */))

//////////////////////////////////////////////////////////////////////////
// CTaskBand (lpTaskSwLongPtr)

#define EV_TASK_SW_PREFERENCES                    ((DWORD *)(lpTaskSwLongPtr + \
                                                             DO2_3264(0x20, 0x40, 0, 0 /* omitted from public code */) + \
                                                             DO2_3264(0x04, 0x08, 0, 0 /* omitted from public code */)))

#define EV_TASK_SW_TASK_GROUPS_HDPA                ((HDPA *)(lpTaskSwLongPtr + \
                                                             DO2_3264(0xA8, 0x120, 0, 0 /* omitted from public code */)))

#define EV_TASK_SW_MULTI_TASK_LIST_REF         ((LONG_PTR *)(lpTaskSwLongPtr + \
                                                             DO2_3264(0x8C, 0xF0, 0, 0 /* omitted from public code */)))

#define EV_TASK_SW_SYS_FROSTED_WINDOW_MSG          ((UINT *)(lpTaskSwLongPtr + \
                                                             DO2_3264(0x2C, 0x50, 0, 0 /* omitted from public code */)))

#define EV_TASK_SW_APP_VIEW_MGR                ((LONG_PTR *)(lpTaskSwLongPtr + \
                                                             0 /* omitted from public code */))

//////////////////////////////////////////////////////////////////////////
// CSecondaryTaskBand (lpSecondaryTaskBandLongPtr)

#define EV_SECONDARY_TASK_BAND_HWND(lp)                     ((HWND *)((lp) + DEF3264(0x04, 0x08)))

#define EV_SECONDARY_TASK_BAND_PREFERENCES(lp)             ((DWORD *)((lp) + DEF3264(0x14, 0x28) + DEF3264(0x0C, 0x18)))

#define EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_REF(lp) \
                                                        ((LONG_PTR *)((lp) + DEF3264(0x28, 0x50)))
#define EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(lp)  (*EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_REF(lp) - DEF3264(0x10, 0x20))

//////////////////////////////////////////////////////////////////////////
// CTaskListWnd (lpMMTaskListLongPtr)

#define EV_MM_TASKLIST_HWND(lp)                             ((HWND *)((lp) + DEF3264(0x04, 0x08)))

#define EV_MM_TASKLIST_TASK_BAND_REF(lp)                ((LONG_PTR *)((lp) + DO2_3264(0x38, 0x70, 0, 0 /* omitted from public code */)))
//#define EV_MM_TASKLIST_TASK_BAND_LONG_PTR_VALUE(lp)                  (*EV_MM_TASKLIST_TASK_BAND_REF(lp) - DEF3264(0x28, 0x50))
#define EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(lp)        (*EV_MM_TASKLIST_TASK_BAND_REF(lp) - DEF3264(0x14, 0x28))

#define EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(lp)               ((HDPA *)((lp) + DO2_3264(0x90, 0xE0, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_TOOLTIP_WND(lp)                      ((HWND *)((lp) + DO2_3264(0x98, 0xF0, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_THUMB_DISABLING_FLAG(lp)             ((BOOL *)((lp) + DO2_3264(0xA0, 0xFC, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_TRACKED_BUTTON_GROUP(lp)        ((LONG_PTR **)((lp) + DO2_3264(0xA8, 0x108, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_TRACKED_BUTTON_INDEX(lp)              ((int *)((lp) + DO2_3264(0xB0, 0x118, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_ACVITE_BUTTON_GROUP(lp)         ((LONG_PTR **)((lp) + DO2_3264(0xB4, 0x120, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_ACVITE_BUTTON_INDEX(lp)               ((int *)((lp) + DO2_3264(0xB8, 0x128, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_PRESSED_BUTTON_GROUP(lp)        ((LONG_PTR **)((lp) + DO2_3264(0xBC, 0x130, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_THUMB_BUTTON_GROUP(lp)          ((LONG_PTR **)((lp) + DO2_3264(0xE4, 0x168, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_MM_THUMBNAIL_LONG_PTR(lp)        ((LONG_PTR *)((lp) + DO2_3264(0xE8, 0x170, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_TOOLTIP_TIMER_ID(lp)                 ((UINT *)((lp) + DO2_3264(0xF0, 0x180, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_THUMB_TIMER_ID(lp)                   ((UINT *)((lp) + DO2_3264(0xF8, 0x190, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_HMONITOR(lp)                     ((HMONITOR *)((lp) + DEF3264(0x14, 0x28) + 0 /* omitted from public code */))

#define EV_MM_TASKLIST_TASK_ITEM_FILTER(lp)             ((LONG_PTR *)((lp) + DEF3264(0x18, 0x30) + 0 /* omitted from public code */))

#define EV_MM_TASKLIST_DRAG_FLAG(lp)                       ((DWORD *)((lp) + DO2_3264(0x150, 0x210, 0, 0 /* omitted from public code */)))

#define EV_MM_TASKLIST_ANIMATION_MANAGER(lp)           ((LONG_PTR **)((lp) + 0 /* omitted from public code */))

//////////////////////////////////////////////////////////////////////////
// CTaskListThumbnailWnd (lpMMThumbnailLongPtr)

#define EV_MM_THUMBNAIL_HWND(lp)                            ((HWND *)((lp) + DO2_3264(0x30, 0x60, 0, 0 /* omitted from public code */)))

#define EV_MM_THUMBNAIL_MM_TASKLIST_REF(lp)             ((LONG_PTR *)((lp) + DO2_3264(0x20, 0x40, 0, 0 /* omitted from public code */)))
#define EV_MM_THUMBNAIL_MM_TASKLIST_LONG_PTR_VALUE(lp)               (*EV_MM_THUMBNAIL_MM_TASKLIST_REF(lp) - DEF3264(0x18, 0x30))

#define EV_MM_THUMBNAIL_REDRAW_FLAGS(lp)                    ((BYTE *)((lp) + DO2_3264(0x44, 0x78, 0, 0 /* omitted from public code */)))

#define EV_MM_THUMBNAIL_TASK_GROUP(lp)                 ((LONG_PTR **)((lp) + DO2_3264(0x64, 0xA8, 0, 0 /* omitted from public code */)))

#define EV_MM_THUMBNAIL_THUMBNAILS_HDPA(lp)                 ((HDPA *)((lp) + DO2_3264(0x68, 0xB0, 0, 0 /* omitted from public code */)))

#define EV_MM_THUMBNAIL_NUM_THUMBNAILS(lp)                 ((DWORD *)((lp) + DO2_3264(0x70, 0xBC, 0, 0 /* omitted from public code */)))

#define EV_MM_THUMBNAIL_ACTIVE_THUMB_INDEX(lp)               ((int *)((lp) + DO2_3264(0x114, 0x168, 0, 0 /* omitted from public code */)))

#define EV_MM_THUMBNAIL_TRACKED_THUMB_INDEX(lp)              ((int *)((lp) + DO2_3264(0x11C, 0x170, 0, 0 /* omitted from public code */)))

#define EV_MM_THUMBNAIL_PRESSED_THUMB_INDEX(lp)              ((int *)((lp) + DO2_3264(0x120, 0x174, 0, 0 /* omitted from public code */)))

#define EV_MM_THUMBNAIL_STICKY_FLAG(lp)                     ((BOOL *)((lp) + DO2_3264(0x144, 0x198, 0, 0 /* omitted from public code */)))

// Until Windows 10 (for OPT_EX_LIST_REVERSE_ORDER)
#define EV_MM_THUMBNAIL_LIST_FLAG(lp)                       ((BOOL *)((lp) + DO2_3264(0x1A0, 0x1F4, 0, 0 /* omitted from public code */)))

// Until Windows 10 (for OPT_EX_LIST_REVERSE_ORDER)
#define EV_MM_THUMBNAIL_LIST_FIRST_VISIBLE_INDEX(lp)         ((int *)((lp) + DO2_3264(0x1A8, 0x1FC, 0, 0 /* omitted from public code */)))

//////////////////////////////////////////////////////////////////////////
// CTaskThumbnail

#define EV_TASK_THUMBNAIL_SIZE                                       DO2_3264(0x7C, 0xB0, 0, 0 /* omitted from public code */)

// Maximum values for 32-bit and 64-bit of the DEF3264 above.
#define EV_TASK_THUMBNAIL_SIZE_BUFFER_ALL_WIN_VERSIONS               DEF3264(0x88, 0xD0)

//////////////////////////////////////////////////////////////////////////
// CPearl (start button)

#define EV_START_BUTTON_HWND(lp)                    ((HWND *)((lp) + DO2_3264(0x20, 0x38, 0, 0 /* omitted from public code */)))

//////////////////////////////////////////////////////////////////////////
// CTrayButton

#define EV_TRAY_BUTTON_HWND(lp)                     ((HWND *)((lp) + DEF3264(0x04, 0x08)))

//////////////////////////////////////////////////////////////////////////
// CTrayNotify (lpTrayNotifyLongPtr)

// Until Windows 10 R1
#define EV_TRAY_NOTIFY_CLOCK_WND(lp)                        ((HWND *)((lp) + DO2_3264(0x18, 0x30, 0, 0 /* omitted from public code */)))

// Since Windows 10 R1
#define EV_TRAY_NOTIFY_CLOCK_LONG_PTR(lp)               ((LONG_PTR *)((lp) + 0 /* omitted from public code */))

#define EV_TRAY_NOTIFY_SHOW_DESKTOP_WND(lp)                 ((HWND *)((lp) + DO2_3264(0x28, 0x50, 0, 0 /* omitted from public code */)))

#define EV_TRAY_NOTIFY_OVERFLOW_TOOLBAR_WND(lp)             ((HWND *)((lp) + DO2_3264(0x2AC, 0x328, 0, 0 /* omitted from public code */)))

#define EV_TRAY_NOTIFY_TEMPORARY_TOOLBAR_WND(lp)            ((HWND *)((lp) + DO2_3264(0x2BC, 0x348, 0, 0 /* omitted from public code */)))

#define EV_TRAY_NOTIFY_TOOLBAR_WND(lp)                      ((HWND *)((lp) + DO2_3264(0x29C, 0x308, 0, 0 /* omitted from public code */)))

#define EV_TRAY_NOTIFY_PTRDEV_SUPPORTED(lp)                 ((BYTE *)((lp) + 0 /* omitted from public code */))

// TRUE if EV_TRAY_NOTIFY_PTRDEV_SUPPORTED is valid, false otherwise.
#define EV_TRAY_NOTIFY_PTRDEV_SUPPORTED_VALID(lp)           ((BYTE *)((lp) + 0 /* omitted from public code */))

#define EV_TRAY_NOTIFY_DRAG_FLAG(lp)                       ((DWORD *)((lp) + \
                                                                      DO2_3264(0x3BC, 0x4B8, 0, 0 /* omitted from public code */)))

//////////////////////////////////////////////////////////////////////////
// CClockCtl (lpTrayClockLongPtr)
// Until Windows 10 R1

#define EV_TRAY_CLOCK_TEXT(lp)                             ((WCHAR *)((lp) + DO2_3264(0x104, 0x10C, 0, 0 /* omitted from public code */)))

#define EV_TRAY_CLOCK_TIMER_ENABLED_FLAG(lp)                ((BOOL *)((lp) + DO2_3264(0x1B4, 0x1C8, 0, 0 /* omitted from public code */)))

#define EV_TRAY_CLOCK_CACHED_TEXT_SIZE(lp)                   ((int *)((lp) + 0 /* omitted from public code */))

//////////////////////////////////////////////////////////////////////////
// ClockButton (lpTrayClockLongPtr)
// Since Windows 10 R1

#define EV_CLOCK_BUTTON_HWND(lp)                    ((HWND *)((lp) + DEF3264(0x04, 0x08)))

#define EV_CLOCK_BUTTON_SIZES_CACHED(lp)            ((BYTE *)((lp) + 0 /* omitted from public code */))

#define EV_CLOCK_BUTTON_SHOW_SECONDS(lp)            ((BYTE *)((lp) + 0 /* omitted from public code */))

#define EV_CLOCK_BUTTON_HOURS_CACHE(lp)             ((WORD *)((lp) + 0 /* omitted from public code */))

#define EV_CLOCK_BUTTON_MINUTES_CACHE(lp)           ((WORD *)((lp) + 0 /* omitted from public code */))

//////////////////////////////////////////////////////////////////////////
// CTaskItem

#define EV_TASKITEM_CONTAINER_TASK_ITEM(lp)           ((LONG_PTR **)((LONG_PTR)(lp) + DO2_3264(0x2C, 0x38, 0, 0 /* omitted from public code */)))

#define EV_TASKITEM_WND(lp)                                ((HWND *)((LONG_PTR)(lp) + DO2_3264(0x04, 0x08, 0, 0 /* omitted from public code */)))

//////////////////////////////////////////////////////////////////////////
// CTaskGroup

#define EV_TASKGROUP_TASKITEMS_HDPA(lp)                    ((HDPA *)((LONG_PTR)(lp) + DEF3264(0x10, 0x20)))

#define EV_TASKGROUP_FLAGS(lp)                            ((DWORD *)((LONG_PTR)(lp) + DEF3264(0x14, 0x28)))

#define EV_TASKGROUP_APPID(lp)                           ((WCHAR **)((LONG_PTR)(lp) + DO2_3264(0x28, 0x48, 0, 0 /* omitted from public code */)))

#define EV_TASKGROUP_VISUAL_ORDER(lp)                       ((int *)((LONG_PTR)(lp) + DO2_3264(0x38, 0x5C, 0, 0 /* omitted from public code */)))

//////////////////////////////////////////////////////////////////////////
// CApplicationViewManager

#define EV_APP_VIEW_MGR_APP_ARRAY_LOCK(lp)              ((SRWLOCK *)((LONG_PTR)(lp) + 0 /* omitted from public code */))

#define EV_APP_VIEW_MGR_APP_ARRAY(lp)                 ((LONG_PTR **)((LONG_PTR)(lp) + 0 /* omitted from public code */))

#define EV_APP_VIEW_MGR_APP_ARRAY_SIZE(lp)               ((size_t *)((LONG_PTR)(lp) + 0 /* omitted from public code */))
