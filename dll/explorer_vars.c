#include "stdafx.h"
#include "explorer_vars.h"

extern LONG_PTR lpTaskbarLongPtr, lpTaskSwLongPtr;

//////////////////////////////////////////////////////////////////////////
// CTray (lpTaskbarLongPtr)

// In Windows 8.1 Update 1, since build 17930, a new virtual table pointer appeared.
// The change was caused by update KB3072318.
// In Windows 10 R3, since 10.0.16299.1480 (after 10.0.16299.1419, not including), some field(s) were added.
// The change was caused by update KB4520006.
// In Windows 10 R4, since 10.0.17763.677 (after 10.0.17763.165, not including), some field(s) were removed.
// The change was caused by update KB4489894.
// In Windows 10 R4, since 10.0.17134.1098 (after 10.0.17134.1038, not including), some field(s) were added.
// The change was caused by update KB4519978.
// In Windows 10 R5, since 10.0.17763.346 (after 10.0.17763.107, not including), some field(s) were removed.
// The change was caused by update KB4482887.
// In Windows 10 R5, since 10.0.17763.831 (after 10.0.17763.771, not including), some field(s) were added.
// The change was caused by update KB4520062.
static LONG_PTR EV_TASKBAR_OFFSET_FIX(LONG_PTR offset)
{
	return offset + (
		(nWinVersion == WIN_VERSION_811 && nExplorerQFE >= 17930 && nExplorerQFE < 20000 && offset >= 8 * sizeof(LONG_PTR))
			? sizeof(LONG_PTR)
			: (
		(nWinVersion == WIN_VERSION_10_R3 && nExplorerQFE >= 1480 && offset >= DEF3264(0x1D8, 0x2B0))
			? sizeof(LONG_PTR)
			: (
		(((nWinVersion == WIN_VERSION_10_R4 && nExplorerQFE >= 677) || (nWinVersion == WIN_VERSION_10_R5 && nExplorerQFE >= 346)) &&
			offset >= DEF3264(0x8C, 0xD0))
			? (
				(((nWinVersion == WIN_VERSION_10_R4 && nExplorerQFE >= 1098) || (nWinVersion == WIN_VERSION_10_R5 && nExplorerQFE >= 831)) &&
					offset >= DEF3264(0x1D8, 0x2B0))
					? 0
					: (-(int)sizeof(LONG_PTR))
			)
			: 0)));
}

static LONG_PTR EV_TRAY_UI_OFFSET_FIX(LONG_PTR offset)
{
	return nWinVersion < WIN_VERSION_10_R2 ? EV_TASKBAR_OFFSET_FIX(offset) : offset;
}

// Pointer to the TrayUI instance, since Windows 10 R2.
LONG_PTR EV_TRAY_UI(LONG_PTR lp)
{
	return DO2_3264(lp, lp, 0, 0 /* omitted from public code */);
}

// Until Windows 10
BOOL *EV_TASKBAR_AUTOPOS_FLAG(void)
{
	return (BOOL *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(
		DO2_3264(0x24, 0x3C, 0, 0 /* omitted from public code */)));
}

// Since Windows 10
BYTE *EV_TASKBAR_AUTOPOS_FLAG_BYTE(void)
{
	return (BYTE *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		0 /* omitted from public code */));
}

// Bit 0x01 - autohide is on, bit 0x02 - taskbar is hidden.
DWORD *EV_TASKBAR_SETTINGS(void)
{
	return (DWORD *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO2_3264(0x40, 0x60, 0, 0 /* omitted from public code */)));
}

HWND *EV_TASKBAR_TRAY_NOTIFY_WND(void)
{
	return (HWND *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO2_3264(0xC04, 0xD70, 0, 0 /* omitted from public code */)));
}

HWND *EV_TASKBAR_TASKBAND_WND(void)
{
	return (HWND *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO2_3264(0xCA8, 0xE58, 0, 0 /* omitted from public code */)));
}

int *EV_TASKBAR_POS(void)
{
	return (int *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO2_3264(0xCD8, 0xEB0, 0, 0 /* omitted from public code */)));
}

BOOL *EV_TASKBAR_UNLOCKED_FLAG(void)
{
	return (BOOL *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO2_3264(0xD28, 0xF24, 0, 0 /* omitted from public code */)));
}

BOOL *EV_TASKBAR_TOPMOST_EX_FLAG(void)
{
	return (BOOL *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(
		DO2_3264(0xD98, 0xFAC, 0, 0 /* omitted from public code */)));
}

// Until Windows 10
HWND *EV_TASKBAR_START_BTN_WND(void)
{
	return (HWND *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(
		DO2_3264(0xAF8 + 0x18, 0xC20 + 0x28, 0, 0 /* omitted from public code */)));
}

// Since Windows 10
LONG_PTR *EV_TASKBAR_START_BTN_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		0 /* omitted from public code */));
}

LONG_PTR *EV_TASKBAR_BACK_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		0 /* omitted from public code */));
}

LONG_PTR *EV_TASKBAR_SEARCH_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		0 /* omitted from public code */));
}

LONG_PTR *EV_TASKBAR_CORTANA_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		0 /* omitted from public code */));
}

LONG_PTR *EV_TASKBAR_TRAY_SEARCH_CONTROL(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		0 /* omitted from public code */));
}

LONG_PTR *EV_TASKBAR_MULTITASKING_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		0 /* omitted from public code */));
}

void *EV_TASKBAR_W7_START_BTN_CLASS(void)
{
	return (void *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(DEF3264(0xAF8, 0xC20)));
}

int *EV_TASKBAR_W7_WIDTH_PADDING(void)
{
	return (int *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(DEF3264(0xB18, 0xC58)));
}

int *EV_TASKBAR_W7_HEIGHT_PADDING(void)
{
	return (int *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(DEF3264(0xB1C, 0xC5C)));
}

//////////////////////////////////////////////////////////////////////////
// CSecondaryTray (lpSecondaryTaskbarLongPtr)

HWND *EV_SECONDARY_TASKBAR_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DEF3264(0x04, 0x08));
}

LONG_PTR *EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_REF(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO2_3264(0x34, 0x58, 0, 0 /* omitted from public code */));
}

LONG_PTR EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_LONG_PTR_VALUE(LONG_PTR lp)
{
	return *EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_REF(lp) - DEF3264(0x14, 0x28);
}

int *EV_SECONDARY_TASKBAR_POS(LONG_PTR lp)
{
	return (int *)(lp + DEF3264(0x10, 0x20) + DO2_3264(0x14, 0x18, 0, 0 /* omitted from public code */));
}

HMONITOR *EV_SECONDARY_TASKBAR_MONITOR(LONG_PTR lp)
{
	return (HMONITOR *)(lp + DEF3264(0x10, 0x20) + DO2_3264(0x18, 0x20, 0, 0 /* omitted from public code */));
}

// Until Windows 10
HWND *EV_SECONDARY_TASKBAR_START_BTN_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO2_3264(0xAC, 0xE0, 0, 0 /* omitted from public code */));
}

// Since Windows 10
LONG_PTR *EV_SECONDARY_TASKBAR_START_BTN_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + 0 /* omitted from public code */);
}

LONG_PTR *EV_SECONDARY_TASKBAR_SEARCH_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + 0 /* omitted from public code */);
}

LONG_PTR *EV_SECONDARY_TASKBAR_CORTANA_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + 0 /* omitted from public code */);
}

LONG_PTR *EV_SECONDARY_TASKBAR_MULTITASKING_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + 0 /* omitted from public code */);
}

LONG_PTR *EV_SECONDARY_TASKBAR_CLOCK_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + 0 /* omitted from public code */);
}

//////////////////////////////////////////////////////////////////////////
// CTaskBand (lpTaskSwLongPtr)

DWORD *EV_TASK_SW_PREFERENCES(void)
{
	return (DWORD *)(lpTaskSwLongPtr +
		DO2_3264(0x20, 0x40, 0, 0 /* omitted from public code */) +
		DO2_3264(0x04, 0x08, 0, 0 /* omitted from public code */));
}

HDPA *EV_TASK_SW_TASK_GROUPS_HDPA(void)
{
	return (HDPA *)(lpTaskSwLongPtr +
		DO2_3264(0xA8, 0x120, 0, 0 /* omitted from public code */));
}

LONG_PTR *EV_TASK_SW_MULTI_TASK_LIST_REF(void)
{
	return (LONG_PTR *)(lpTaskSwLongPtr +
		DO2_3264(0x8C, 0xF0, 0, 0 /* omitted from public code */));
}

UINT *EV_TASK_SW_SYS_FROSTED_WINDOW_MSG(void)
{
	return (UINT *)(lpTaskSwLongPtr +
		DO2_3264(0x2C, 0x50, 0, 0 /* omitted from public code */));
}

LONG_PTR *EV_TASK_SW_APP_VIEW_MGR(void)
{
	return (LONG_PTR *)(lpTaskSwLongPtr +
		0 /* omitted from public code */);
}

//////////////////////////////////////////////////////////////////////////
// CSecondaryTaskBand (lpSecondaryTaskBandLongPtr)

HWND *EV_SECONDARY_TASK_BAND_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DEF3264(0x04, 0x08));
}

DWORD *EV_SECONDARY_TASK_BAND_PREFERENCES(LONG_PTR lp)
{
	return (DWORD *)(lp + DEF3264(0x14, 0x28) + DEF3264(0x0C, 0x18));
}

LONG_PTR *EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_REF(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DEF3264(0x28, 0x50));
}

LONG_PTR EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_LONG_PTR_VALUE(LONG_PTR lp)
{
	return *EV_SECONDARY_TASK_BAND_SECONDARY_TASKBAR_REF(lp) - DEF3264(0x10, 0x20);
}

//////////////////////////////////////////////////////////////////////////
// CTaskListWnd (lpMMTaskListLongPtr)

HWND *EV_MM_TASKLIST_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DEF3264(0x04, 0x08));
}

LONG_PTR *EV_MM_TASKLIST_TASK_BAND_REF(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO2_3264(0x38, 0x70, 0, 0 /* omitted from public code */));
}

/*LONG_PTR EV_MM_TASKLIST_TASK_BAND_LONG_PTR_VALUE(LONG_PTR lp)
{
	return *EV_MM_TASKLIST_TASK_BAND_REF(lp) - DEF3264(0x28, 0x50);
}*/

LONG_PTR EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(LONG_PTR lp)
{
	return *EV_MM_TASKLIST_TASK_BAND_REF(lp) - DEF3264(0x14, 0x28);
}

HDPA *EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(LONG_PTR lp)
{
	return (HDPA *)(lp + DO2_3264(0x90, 0xE0, 0, 0 /* omitted from public code */));
}

HWND *EV_MM_TASKLIST_TOOLTIP_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO2_3264(0x98, 0xF0, 0, 0 /* omitted from public code */));
}

BOOL *EV_MM_TASKLIST_THUMB_DISABLING_FLAG(LONG_PTR lp)
{
	return (BOOL *)(lp + DO2_3264(0xA0, 0xFC, 0, 0 /* omitted from public code */));
}

LONG_PTR **EV_MM_TASKLIST_TRACKED_BUTTON_GROUP(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO2_3264(0xA8, 0x108, 0, 0 /* omitted from public code */));
}

int *EV_MM_TASKLIST_TRACKED_BUTTON_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO2_3264(0xB0, 0x118, 0, 0 /* omitted from public code */));
}

LONG_PTR **EV_MM_TASKLIST_ACTIVE_BUTTON_GROUP(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO2_3264(0xB4, 0x120, 0, 0 /* omitted from public code */));
}

int *EV_MM_TASKLIST_ACTIVE_BUTTON_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO2_3264(0xB8, 0x128, 0, 0 /* omitted from public code */));
}

LONG_PTR **EV_MM_TASKLIST_PRESSED_BUTTON_GROUP(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO2_3264(0xBC, 0x130, 0, 0 /* omitted from public code */));
}

LONG_PTR **EV_MM_TASKLIST_THUMB_BUTTON_GROUP(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO2_3264(0xE4, 0x168, 0, 0 /* omitted from public code */));
}

LONG_PTR *EV_MM_TASKLIST_MM_THUMBNAIL_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO2_3264(0xE8, 0x170, 0, 0 /* omitted from public code */));
}

UINT *EV_MM_TASKLIST_TOOLTIP_TIMER_ID(LONG_PTR lp)
{
	return (UINT *)(lp + DO2_3264(0xF0, 0x180, 0, 0 /* omitted from public code */));
}

UINT *EV_MM_TASKLIST_THUMB_TIMER_ID(LONG_PTR lp)
{
	return (UINT *)(lp + DO2_3264(0xF8, 0x190, 0, 0 /* omitted from public code */));
}

HMONITOR *EV_MM_TASKLIST_HMONITOR(LONG_PTR lp)
{
	return (HMONITOR *)(lp + DEF3264(0x14, 0x28) + 0 /* omitted from public code */);
}

LONG_PTR *EV_MM_TASKLIST_TASK_ITEM_FILTER(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DEF3264(0x18, 0x30) + 0 /* omitted from public code */);
}

DWORD *EV_MM_TASKLIST_DRAG_FLAG(LONG_PTR lp)
{
	return (DWORD *)(lp + DO2_3264(0x150, 0x210, 0, 0 /* omitted from public code */));
}

LONG_PTR **EV_MM_TASKLIST_ANIMATION_MANAGER(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + 0 /* omitted from public code */);
}

//////////////////////////////////////////////////////////////////////////
// CTaskListThumbnailWnd (lpMMThumbnailLongPtr)

HWND *EV_MM_THUMBNAIL_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DO2_3264(0x30, 0x60, 0, 0 /* omitted from public code */));
}

LONG_PTR *EV_MM_THUMBNAIL_MM_TASKLIST_REF(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO2_3264(0x20, 0x40, 0, 0 /* omitted from public code */));
}

LONG_PTR EV_MM_THUMBNAIL_MM_TASKLIST_LONG_PTR_VALUE(LONG_PTR lp)
{
	return *EV_MM_THUMBNAIL_MM_TASKLIST_REF(lp) - DEF3264(0x18, 0x30);
}

BYTE *EV_MM_THUMBNAIL_REDRAW_FLAGS(LONG_PTR lp)
{
	return (BYTE *)(lp + DO2_3264(0x44, 0x78, 0, 0 /* omitted from public code */));
}

LONG_PTR **EV_MM_THUMBNAIL_TASK_GROUP(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO2_3264(0x64, 0xA8, 0, 0 /* omitted from public code */));
}

HDPA *EV_MM_THUMBNAIL_THUMBNAILS_HDPA(LONG_PTR lp)
{
	return (HDPA *)(lp + DO2_3264(0x68, 0xB0, 0, 0 /* omitted from public code */));
}

DWORD *EV_MM_THUMBNAIL_NUM_THUMBNAILS(LONG_PTR lp)
{
	return (DWORD *)(lp + DO2_3264(0x70, 0xBC, 0, 0 /* omitted from public code */));
}

int *EV_MM_THUMBNAIL_ACTIVE_THUMB_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO2_3264(0x114, 0x168, 0, 0 /* omitted from public code */));
}

int *EV_MM_THUMBNAIL_TRACKED_THUMB_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO2_3264(0x11C, 0x170, 0, 0 /* omitted from public code */));
}

int *EV_MM_THUMBNAIL_PRESSED_THUMB_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO2_3264(0x120, 0x174, 0, 0 /* omitted from public code */));
}

BOOL *EV_MM_THUMBNAIL_STICKY_FLAG(LONG_PTR lp)
{
	return (BOOL *)(lp + DO2_3264(0x144, 0x198, 0, 0 /* omitted from public code */));
}

// Until Windows 10 (for OPT_EX_LIST_REVERSE_ORDER)
BOOL *EV_MM_THUMBNAIL_LIST_FLAG(LONG_PTR lp)
{
	return (BOOL *)(lp + DO2_3264(0x1A0, 0x1F4, 0, 0 /* omitted from public code */));
}

// Until Windows 10 (for OPT_EX_LIST_REVERSE_ORDER)
int *EV_MM_THUMBNAIL_LIST_FIRST_VISIBLE_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO2_3264(0x1A8, 0x1FC, 0, 0 /* omitted from public code */));
}

//////////////////////////////////////////////////////////////////////////
// CTaskThumbnail

// Only in header file

//////////////////////////////////////////////////////////////////////////
// CPearl (start button)

HWND *EV_START_BUTTON_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DO2_3264(0x20, 0x38, 0, 0 /* omitted from public code */));
}

//////////////////////////////////////////////////////////////////////////
// CTraySearchControl

HWND *EV_TRAY_SEARCH_CONTROL_BUTTON_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + 0 /* omitted from public code */);
}

//////////////////////////////////////////////////////////////////////////
// CTrayButton

HWND *EV_TRAY_BUTTON_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DEF3264(0x04, 0x08));
}

//////////////////////////////////////////////////////////////////////////
// CTrayNotify (lpTrayNotifyLongPtr)

// Until Windows 10 R1
HWND *EV_TRAY_NOTIFY_CLOCK_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO2_3264(0x18, 0x30, 0, 0 /* omitted from public code */));
}

// Since Windows 10 R1
LONG_PTR *EV_TRAY_NOTIFY_CLOCK_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + 0 /* omitted from public code */);
}

HWND *EV_TRAY_NOTIFY_SHOW_DESKTOP_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO2_3264(0x28, 0x50, 0, 0 /* omitted from public code */));
}

HWND *EV_TRAY_NOTIFY_OVERFLOW_TOOLBAR_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO2_3264(0x2AC, 0x328, 0, 0 /* omitted from public code */));
}

HWND *EV_TRAY_NOTIFY_TEMPORARY_TOOLBAR_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO2_3264(0x2BC, 0x348, 0, 0 /* omitted from public code */));
}

HWND *EV_TRAY_NOTIFY_TOOLBAR_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO2_3264(0x29C, 0x308, 0, 0 /* omitted from public code */));
}

// For 7+ Taskbar Numberer
BOOL *EV_TRAY_NOTIFY_CHEVRON_STATE(LONG_PTR lp)
{
	return (BOOL *)(lp + DO2_3264(0x324, 0x3EC, 0, 0 /* omitted from public code */));
}

BYTE *EV_TRAY_NOTIFY_PTRDEV_SUPPORTED(LONG_PTR lp)
{
	return (BYTE *)(lp + 0 /* omitted from public code */);
}

// TRUE if EV_TRAY_NOTIFY_PTRDEV_SUPPORTED is valid, FALSE otherwise.
BYTE *EV_TRAY_NOTIFY_PTRDEV_SUPPORTED_VALID(LONG_PTR lp)
{
	return (BYTE *)(lp + 0 /* omitted from public code */);
}

// For 7+ Taskbar Numberer
HTHEME *EV_TRAY_NOTIFY_THEME(LONG_PTR lp)
{
	return (HTHEME *)(lp +
		DO2_3264(0x34C, 0x420, 0, 0 /* omitted from public code */));
}

DWORD *EV_TRAY_NOTIFY_DRAG_FLAG(LONG_PTR lp)
{
	return (DWORD *)(lp +
		DO2_3264(0x3BC, 0x4B8, 0, 0 /* omitted from public code */));
}

//////////////////////////////////////////////////////////////////////////
// CClockCtl (lpTrayClockLongPtr)
// Until Windows 10 R1

WCHAR *EV_TRAY_CLOCK_TEXT(LONG_PTR lp)
{
	return (WCHAR *)(lp + DO2_3264(0x104, 0x10C, 0, 0 /* omitted from public code */));
}

BOOL *EV_TRAY_CLOCK_TIMER_ENABLED_FLAG(LONG_PTR lp)
{
	return (BOOL *)(lp + DO2_3264(0x1B4, 0x1C8, 0, 0 /* omitted from public code */));
}

int *EV_TRAY_CLOCK_CACHED_TEXT_SIZE(LONG_PTR lp)
{
	return (int *)(lp + 0 /* omitted from public code */);
}

//////////////////////////////////////////////////////////////////////////
// ClockButton (lpTrayClockLongPtr)
// Since Windows 10 R1

HWND *EV_CLOCK_BUTTON_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DEF3264(0x04, 0x08));
}

BYTE *EV_CLOCK_BUTTON_SIZES_CACHED(LONG_PTR lp)
{
	return (BYTE *)(lp + 0 /* omitted from public code */);
}

BYTE *EV_CLOCK_BUTTON_SHOW_SECONDS(LONG_PTR lp)
{
	return (BYTE *)(lp + 0 /* omitted from public code */);
}

WORD *EV_CLOCK_BUTTON_HOURS_CACHE(LONG_PTR lp)
{
	return (WORD *)(lp + 0 /* omitted from public code */);
}

WORD *EV_CLOCK_BUTTON_MINUTES_CACHE(LONG_PTR lp)
{
	return (WORD *)(lp + 0 /* omitted from public code */);
}

//////////////////////////////////////////////////////////////////////////
// CTaskItem

LONG_PTR **EV_TASKITEM_CONTAINER_TASK_ITEM(LONG_PTR *plp)
{
	return (LONG_PTR **)((LONG_PTR)plp + DO2_3264(0x2C, 0x38, 0, 0 /* omitted from public code */));
}

HWND *EV_TASKITEM_WND(LONG_PTR *plp)
{
	return (HWND *)((LONG_PTR)plp + DO2_3264(0x04, 0x08, 0, 0 /* omitted from public code */));
}

//////////////////////////////////////////////////////////////////////////
// CTaskGroup

HDPA *EV_TASKGROUP_TASKITEMS_HDPA(LONG_PTR *plp)
{
	return (HDPA *)((LONG_PTR)plp + DEF3264(0x10, 0x20));
}

DWORD *EV_TASKGROUP_FLAGS(LONG_PTR *plp)
{
	return (DWORD *)((LONG_PTR)plp + DEF3264(0x14, 0x28));
}

WCHAR **EV_TASKGROUP_APPID(LONG_PTR *plp)
{
	return (WCHAR **)((LONG_PTR)plp + DO2_3264(0x28, 0x48, 0, 0 /* omitted from public code */));
}

int *EV_TASKGROUP_VISUAL_ORDER(LONG_PTR *plp)
{
	return (int *)((LONG_PTR)plp + DO2_3264(0x38, 0x5C, 0, 0 /* omitted from public code */));
}

//////////////////////////////////////////////////////////////////////////
// CApplicationViewManager

SRWLOCK *EV_APP_VIEW_MGR_APP_ARRAY_LOCK(LONG_PTR lp)
{
	return (SRWLOCK *)(lp + 0 /* omitted from public code */);
}

LONG_PTR **EV_APP_VIEW_MGR_APP_ARRAY(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + 0 /* omitted from public code */);
}

size_t *EV_APP_VIEW_MGR_APP_ARRAY_SIZE(LONG_PTR lp)
{
	return (size_t *)(lp + 0 /* omitted from public code */);
}
