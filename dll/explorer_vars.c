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
// DEF3264: CTray::_MinimizeAll
LONG_PTR EV_TRAY_UI(LONG_PTR lp)
{
	return DO14_3264(lp, lp, ,, ,, ,, ,, ,, ,,
		*(LONG_PTR *)(lp + 0x1C8) - 0x0C, *(LONG_PTR *)(lp + 0x2A8) - 0x18,
		*(LONG_PTR *)(lp + EV_TASKBAR_OFFSET_FIX(0x1E0)) - 0x0C, *(LONG_PTR *)(lp + EV_TASKBAR_OFFSET_FIX(0x2C0)) - 0x18,
		*(LONG_PTR *)(lp + EV_TASKBAR_OFFSET_FIX(0x1E4)) - 0x0C, *(LONG_PTR *)(lp + EV_TASKBAR_OFFSET_FIX(0x2C8)) - 0x18,
		,,
		*(LONG_PTR *)(lp + (nExplorerQFE <= 329 ? 0x1E4 : (nExplorerQFE <= 997 ? 0x1E8 : 0x1EC))) - 0x0C, *(LONG_PTR *)(lp + (nExplorerQFE <= 329 ? 0x2C8 : 0x2D0)) - 0x18,
		*(LONG_PTR *)(lp + (nExplorerQFE <= 388 ? 0x1F4 : (nExplorerQFE <= 572 ? 0x21C : 0x220))) - 0x0C, *(LONG_PTR *)(lp + (nExplorerQFE <= 388 ? 0x2E0 : 0x320)) - 0x18,
		*(LONG_PTR *)(lp + 0x24C) - 0x0C, *(LONG_PTR *)(lp + 0x370) - 0x18);
}

// DEF3264: TrayUI::_HandleSizing (previously CTray::_HandleSizing)
// Until Windows 10
BOOL *EV_TASKBAR_AUTOPOS_FLAG(void)
{
	return (BOOL *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(
		DO5_3264(0x24, 0x3C, ,, ,, 0x2C, 0x4C, 0, 0)));
}

// Since Windows 10
BYTE *EV_TASKBAR_AUTOPOS_FLAG_BYTE(void)
{
	return (BYTE *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO8_3264(0, 0, ,, ,, ,, 0x24, 0x44, ,, 0x28, 0x4C, 0x24, 0x3C)));
}

// DEF3264: CTray::_KickStartAutohide (called from CTray::_SyncThreadProc)
// -or-
// DEF3264: CTray::_SetUnhideTimer
// -or-
// DEF3264: TrayUI::SetAutoHideFlags (value + 0x10/0x20)
// Bit 0x01 - autohide is on, bit 0x02 - taskbar is hidden.
DWORD *EV_TASKBAR_SETTINGS(void)
{
	return (DWORD *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO10_3264(0x40, 0x60, 0x44, 0x68, ,, 0x4C, 0x78, 0x34, 0x60, ,, 0x40, 0x70, 0x10 + 0x20, 0x20 + 0x30, ,, 0x10 + 0x24, 0x20 + 0x30)));
}

// DEF3264: CTray::_SaveTrayAndDesktop
// -or-
// DEF3264: TrayUI::SaveNotificationAreaState (value + 0x0C/0x18)
HWND *EV_TASKBAR_TRAY_NOTIFY_WND(void)
{
	return (HWND *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO10_3264(0xC04, 0xD70, 0xA0, 0xD0, ,, 0xA8, 0xE0, 0x50, 0x88, ,, 0x60, 0xA0, 0x0C + 0x40, 0x18 + 0x58, ,, 0x0C + 0x44, 0x18 + 0x58)));
}

// DEF3264: TrayUI::_ContextMenu (previously CTray::_ContextMenu)
HWND *EV_TASKBAR_TASKBAND_WND(void)
{
	return (HWND *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO14_3264(0xCA8, 0xE58, 0x150, 0x1B0, ,, 0x158, 0x1C0, 0x140, 0x1A8, ,, 0x158, 0x1C8, 0x10C, 0x158, ,, 0x114, 0x160, ,,
			nExplorerQFE <= 997 ? 0x118 : 0x11C, nExplorerQFE <= 997 ? 0x160 : 0x168,
			nExplorerQFE <= 572 ? 0x118 : (nExplorerQFE <= 2193 ? 0x11C : 0x120), nExplorerQFE <= 572 ? 0x160 : 0x168,
			0x120, 0x170)));
}

// DEF3264: TrayUI::_StuckTrayChange (previously CTray::_StuckTrayChange)
int *EV_TASKBAR_POS(void)
{
	return (int *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO14_3264(0xCD8, 0xEB0, 0x180, 0x208, ,, 0x188, 0x218, 0x180, 0x210, ,, 0x198, 0x230, 0x110, 0x160, ,, 0x118, 0x168, ,,
			nExplorerQFE <= 997 ? 0x11C : 0x120, nExplorerQFE <= 997 ? 0x168 : 0x170,
			nExplorerQFE <= 572 ? 0x11C : (nExplorerQFE <= 2193 ? 0x120 : 0x124), nExplorerQFE <= 572 ? 0x168 : 0x170,
			0x124, 0x178)));
}

// DEF3264: TrayUI::_HandleSizing (previously CTray::_HandleSizing)
BOOL *EV_TASKBAR_UNLOCKED_FLAG(void)
{
	return (BOOL *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO14_3264(0xD28, 0xF24, 0x1D0, 0x27C, ,, 0x1D8, 0x28C, 0x1D8, 0x28C, ,, 0x1F0, 0x2AC, 0x158, 0x1C0, ,, 0x160, 0x1C8, ,,
			nExplorerQFE <= 997 ? 0x164 : 0x168, nExplorerQFE <= 997 ? 0x1C8 : 0x1D0,
			nExplorerQFE <= 572 ? 0x164 : (nExplorerQFE <= 2193 ? 0x168 : 0x16C), nExplorerQFE <= 572 ? 0x1C8 : 0x1D0,
			0x16C, 0x1D8)));
}

// DEF3264: CTray::_RecomputeWorkArea -or- CTray::RecomputeWorkArea
// Since Windows 11 22H2, search for the sum in assembly.
BOOL *EV_TASKBAR_TOPMOST_EX_FLAG(void)
{
	return (BOOL *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(
		DO14_3264(0xD98, 0xFAC, 0x210, 0x2CC, ,, 0x218, 0x2DC, 0x210, 0x2D0, ,, 0x228, 0x2F0, 0x1C + 0xD0, 0x38 + 0x130, ,, ,, ,,
			0x1C + (nExplorerQFE <= 997 ? 0xD0 : 0xD4), 0x38 + 0x130,
			0x1C + (nExplorerQFE <= 388 ? 0xD0 : (nExplorerQFE <= 572 ? 0xF4 : 0xF8)), 0x38 + (nExplorerQFE <= 388 ? 0x130 : 0x168),
			0x1C + 0xFC, 0x38 + 0x170)));
}

// Until Windows 10
// DEF3264 1 (W7): CTray::_InitStartButtonEtc
// DEF3264 2 (W7): CStartButton::IsButtonPushed
// DEF3264: CTray::_OnFocusMsg
HWND *EV_TASKBAR_START_BTN_WND(void)
{
	return (HWND *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(
		DO5_3264(0xAF8 + 0x18, 0xC20 + 0x28, ,, 0x2D8, 0x3F8, 0x2F4, 0x428, 0, 0)));
}

// Since Windows 10
// DEF3264: CTray::Unhide
// DEF3264: TrayUI::_UpdatePearlSize
LONG_PTR *EV_TASKBAR_START_BTN_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO16_3264(0, 0, ,, ,, ,, 0x2DC, 0x408, 0x2E0, 0x410, 0x314, 0x448, 0x194, 0x218, 0x19C, 0x220, 0x1A4, 0x228, ,,
			nExplorerQFE <= 997 ? 0x1A8 : 0x1AC, nExplorerQFE <= 997 ? 0x228 : 0x230,
			nExplorerQFE <= 572 ? 0x1A8 : (nExplorerQFE <= 2193 ? 0x1AC : 0x1B0), nExplorerQFE <= 572 ? 0x228 : 0x230,
			0x1B0, 0x238,
			,,
			0x1A8, 0x230)));
}

// DEF3264: TrayUI::TrayUI
LONG_PTR *EV_TASKBAR_BACK_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO16_3264(0, 0, ,, ,, ,, 0x2EC, 0x428, 0x2F0, 0x430, 0x318, 0x450, 0x198, 0x220, 0x1A0, 0x228, 0x1A8, 0x230, ,,
			nExplorerQFE <= 997 ? 0x1AC : (nExplorerQFE <= 1500 ? 0x1B0 : 0x1B4), nExplorerQFE <= 997 ? 0x230 : (nExplorerQFE <= 1500 ? 0x238 : 0x240),
			nExplorerQFE <= 572 ? 0x1AC : (nExplorerQFE <= 928 ? 0x1B0 : (nExplorerQFE <= 2193 ? 0x1B4 : 0x1B8)), nExplorerQFE <= 572 ? 0x230 : (nExplorerQFE <= 928 ? 0x238 : 0x240),
			0x1B8, 0x248,
			,,
			0x1B4, 0x240)));
}

// DEF3264: TrayUI::_HandleSettingChange
LONG_PTR *EV_TASKBAR_SEARCH_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO16_3264(0, 0, ,, ,, ,, 0x2E0, 0x410, 0x2E4, 0x418, 0x31C, 0x458, 0x19C, 0x228, 0x1A4, 0x230, 0x1AC, 0x238, ,,
			nExplorerQFE <= 997 ? 0x1B0 : (nExplorerQFE <= 1500 ? 0x1B4 : 0x1B8), nExplorerQFE <= 997 ? 0x238 : (nExplorerQFE <= 1500 ? 0x240 : 0x248),
			nExplorerQFE <= 572 ? 0x1B0 : (nExplorerQFE <= 928 ? 0x1B4 : (nExplorerQFE <= 2193 ? 0x1B8 : 0x1BC)), nExplorerQFE <= 572 ? 0x238 : (nExplorerQFE <= 928 ? 0x240 : 0x248),
			0x1BC, 0x250,
			,,
			0x1B8, 0x248)));
}

// DEF3264: TrayUI::_HandleSettingChange
LONG_PTR *EV_TASKBAR_CORTANA_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO16_3264(0, 0, ,, ,, ,, ,, ,, ,, ,, ,, ,, ,,
			nExplorerQFE <= 997 ? 0x1B4 : (nExplorerQFE <= 1500 ? 0x1B8 : 0x1BC), nExplorerQFE <= 997 ? 0x240 : (nExplorerQFE <= 1500 ? 0x248 : 0x250),
			nExplorerQFE <= 572 ? 0x1B4 : (nExplorerQFE <= 928 ? 0x1B8 : (nExplorerQFE <= 2193 ? 0x1BC : 0x1C0)), nExplorerQFE <= 572 ? 0x240 : (nExplorerQFE <= 928 ? 0x248 : 0x250),
			0x1C0, 0x258,
			,,
			0, 0)));
}

// DEF3264: TrayUI::_HandleSettingChange
LONG_PTR *EV_TASKBAR_TRAY_SEARCH_CONTROL(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO16_3264(0, 0, ,, ,, ,, 0x2E4, 0x418, 0x2E8, 0x420, 0x320, 0x460, 0x1A0, 0x230, 0x1A8, 0x238, 0x1B0, 0x240, ,,
			nExplorerQFE <= 997 ? 0x1B8 : (nExplorerQFE <= 1500 ? 0x1BC : 0x1B0), nExplorerQFE <= 997 ? 0x248 : (nExplorerQFE <= 1500 ? 0x250 : 0x238),
			nExplorerQFE <= 572 ? 0x1B8 : (nExplorerQFE <= 928 ? 0x1BC : (nExplorerQFE <= 2193 ? 0x1B0 : 0x1B4)), nExplorerQFE <= 572 ? 0x248 : (nExplorerQFE <= 928 ? 0x250 : 0x238),
			0x1B4, 0x240,
			,,
			0x1B0, 0x238)));
}

// DEF3264: TrayUI::TrayUI
LONG_PTR *EV_TASKBAR_MULTITASKING_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO16_3264(0, 0, ,, ,, ,, 0x2E8, 0x420, 0x2EC, 0x428, 0x324, 0x468, 0x1A4, 0x238, 0x1AC, 0x240, 0x1B4, 0x248, ,,
			nExplorerQFE <= 997 ? 0x1BC : 0x1C0, nExplorerQFE <= 997 ? 0x250 : 0x258,
			nExplorerQFE <= 572 ? 0x1BC : (nExplorerQFE <= 2193 ? 0x1C0 : 0x1C4), nExplorerQFE <= 572 ? 0x250 : 0x258,
			0x1C4, 0x260,
			0x1C8, 0x268,
			0x1C0, 0x258)));
}

// DEF3264: CTray::_InitStartButtonEtc
void *EV_TASKBAR_W7_START_BTN_CLASS(void)
{
	return (void *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(DEF3264(0xAF8, 0xC20)));
}

// DEF3264: CTray::_HandleSizing
int *EV_TASKBAR_W7_WIDTH_PADDING(void)
{
	return (int *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(DEF3264(0xB18, 0xC58)));
}

// DEF3264: CTray::_HandleSizing
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

// DEF3264: CSecondaryTray::_LoadSettings
LONG_PTR *EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_REF(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO7_3264(0x34, 0x58, ,, ,, 0x3C, 0x68, 0x3C, 0x60, ,, 0x40, 0x68));
}

LONG_PTR EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_LONG_PTR_VALUE(LONG_PTR lp)
{
	return *EV_SECONDARY_TASKBAR_SECONDARY_TASKLIST_REF(lp) - DEF3264(0x14, 0x28);
}

// DEF3264: CSecondaryTray::GetStuckPlace
int *EV_SECONDARY_TASKBAR_POS(LONG_PTR lp)
{
	return (int *)(lp + DEF3264(0x10, 0x20) + DO7_3264(0x14, 0x18, ,, ,, 0x1C, 0x28, 0x18, 0x20, ,, 0x1C, 0x28));
}

// DEF3264: CSecondaryTray::SetMonitor
HMONITOR *EV_SECONDARY_TASKBAR_MONITOR(LONG_PTR lp)
{
	return (HMONITOR *)(lp + DEF3264(0x10, 0x20) + DO7_3264(0x18, 0x20, ,, ,, 0x20, 0x30, 0x1C, 0x28, ,, 0x20, 0x30));
}

// Until Windows 10
// DEF3264: CSecondaryTray::CSecondaryTray
HWND *EV_SECONDARY_TASKBAR_START_BTN_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO5_3264(0xAC, 0xE0, ,, ,, 0xB4, 0xF0, 0, 0));
}

// Since Windows 10
// DEF3264: CSecondaryTray::_CreateSecondaryTray
LONG_PTR *EV_SECONDARY_TASKBAR_START_BTN_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO14_3264(0, 0, ,, ,, ,, 0xB4, 0xE8, 0xB8, 0xF0, 0xB8, 0xF0, ,, ,, ,, ,,
		nExplorerQFE <= 1500 ? 0xB8 : 0xBC, 0xF0,
		nExplorerQFE <= 928 ? 0xB8 : 0xBC, 0xF0,
		0xBC, 0xF0));
}

// DEF3264: CSecondaryTray::CSecondaryTray
LONG_PTR *EV_SECONDARY_TASKBAR_SEARCH_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO16_3264(0, 0, ,, ,, ,, 0xB8, 0xF0, 0xBC, 0xF8, 0xC0, 0x100, ,, ,, ,, ,,
		nExplorerQFE <= 1500 ? 0xC0 : 0xC4, 0x100,
		nExplorerQFE <= 928 ? 0xC0 : 0xC4, 0x100,
		0xC4, 0x100,
		,,
		0xC0, 0xF8));
}

// DEF3264: CSecondaryTray::_OnSettingChange
LONG_PTR *EV_SECONDARY_TASKBAR_CORTANA_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO16_3264(0, 0, ,, ,, ,, ,, ,, ,, ,, ,, ,, ,,
		nExplorerQFE <= 1500 ? 0xC4 : 0xC8, 0x108,
		nExplorerQFE <= 928 ? 0xC4 : 0xC8, 0x108,
		0xC8, 0x108,
		,,
		0, 0));
}

// DEF3264: CSecondaryTray::CSecondaryTray
LONG_PTR *EV_SECONDARY_TASKBAR_MULTITASKING_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO16_3264(0, 0, ,, ,, ,, 0xBC, 0xF8, 0xC0, 0x100, 0xC4, 0x108, ,, ,, ,, ,,
		nExplorerQFE <= 1500 ? 0xC8 : 0xCC, 0x110,
		nExplorerQFE <= 928 ? 0xC8 : 0xCC, 0x110,
		0xCC, 0x110,
		,,
		0xC4, 0x100));
}

// DEF3264: CSecondaryTray::_DestroyClockControlIfApplicable
LONG_PTR *EV_SECONDARY_TASKBAR_CLOCK_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO16_3264(0, 0, ,, ,, ,, ,, ,, 0xC8, 0x110, ,, ,, ,, ,,
		nExplorerQFE <= 1500 ? 0xCC : 0xD4, nExplorerQFE <= 1500 ? 0x118 : 0x120,
		nExplorerQFE <= 928 ? 0xCC : 0xD4, nExplorerQFE <= 928 ? 0x118 : 0x120,
		0xCC, 0x118,
		0xD4, 0x120,
		0xCC, 0x110));
}

//////////////////////////////////////////////////////////////////////////
// CTaskBand (lpTaskSwLongPtr)

// DEF3264: CTaskBand::GetUserPreferences
DWORD *EV_TASK_SW_PREFERENCES(void)
{
	return (DWORD *)(lpTaskSwLongPtr +
		DO4_3264(0x20, 0x40, 0x24, 0x48, ,, 0x28, 0x50) +
		DO8_3264(0x04, 0x08, ,, ,, 0x0C, 0x18, 0x18, 0x30, 0x1C, 0x38, 0x24, 0x48, 0x2C, 0x58));
}

// DEF3264: CTaskBand::_RealityCheck
HDPA *EV_TASK_SW_TASK_GROUPS_HDPA(void)
{
	return (HDPA *)(lpTaskSwLongPtr +
		DO11_3264(0xA8, 0x120, 0xA4, 0x118, 0xA8, 0x120, 0xC0, 0x140, 0xC0, 0x148, 0xC4, 0x150, 0xC0, 0x158, 0xC8, 0x160, ,, ,, 0xC0, 0x158));
}

// DEF3264: CTaskBand::_DeleteTask
LONG_PTR *EV_TASK_SW_MULTI_TASK_LIST_REF(void)
{
	return (LONG_PTR *)(lpTaskSwLongPtr +
		DO11_3264(0x8C, 0xF0, ,, 0x90, 0xF8, 0xA8, 0x118, 0xA8, 0x120, 0xAC, 0x128, 0xB8, 0x148, 0xBC, 0x148, ,, ,, 0xB4, 0x140));
}

// DEF3264: "SysFrostedWindow"
UINT *EV_TASK_SW_SYS_FROSTED_WINDOW_MSG(void)
{
	return (UINT *)(lpTaskSwLongPtr +
		DO8_3264(0x2C, 0x50, 0x30, 0x58, 0x34, 0x60, 0x40, 0x74, 0x48, 0x88, 0x4C, 0x90, 0x54, 0xA0, 0x5C, 0xB0));
}

// DEF3264: CTaskBand::_UpdateVirtualDesktopInclusion (the overload that takes two arguments, not one!)
LONG_PTR *EV_TASK_SW_APP_VIEW_MGR(void)
{
	return (LONG_PTR *)(lpTaskSwLongPtr +
		DO16_3264(0, 0, ,, ,, ,, 0x120, 0x1F8, 0x12C, 0x210, 0x118, 0x1F8, 0x128, 0x208, ,, ,, 0x164, 0x260, ,, 0x168, 0x268, ,, ,, 0, 0x208));
}

//////////////////////////////////////////////////////////////////////////
// CSecondaryTaskBand (lpSecondaryTaskBandLongPtr)

HWND *EV_SECONDARY_TASK_BAND_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DEF3264(0x04, 0x08));
}

// DEF3264: CSecondaryTaskBand::GetUserPreferences
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
	return (LONG_PTR *)(lp + DO8_3264(0x38, 0x70, 0x3C, 0x78, ,, ,, ,, ,, 0x30, 0x60, 0x34, 0x68));
}

/*LONG_PTR EV_MM_TASKLIST_TASK_BAND_LONG_PTR_VALUE(LONG_PTR lp)
{
	return *EV_MM_TASKLIST_TASK_BAND_REF(lp) - DEF3264(0x28, 0x50);
}*/

LONG_PTR EV_MM_TASKLIST_SECONDARY_TASK_BAND_LONG_PTR_VALUE(LONG_PTR lp)
{
	return *EV_MM_TASKLIST_TASK_BAND_REF(lp) - DEF3264(0x14, 0x28);
}

// DEF3264: CTaskListWnd::_GetTBGroupFromGroup
HDPA *EV_MM_TASKLIST_BUTTON_GROUPS_HDPA(LONG_PTR lp)
{
	return (HDPA *)(lp + DO8_3264(0x90, 0xE0, 0x94, 0xE8, ,, ,, 0x90, 0xE8, ,, 0x84, 0xD0, 0x88, 0xD8));
}

// DEF3264: CTaskListWnd::_ShowToolTip
HWND *EV_MM_TASKLIST_TOOLTIP_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO8_3264(0x98, 0xF0, 0x9C, 0xF8, ,, ,, 0x98, 0xF8, ,, 0x8C, 0xE0, 0x90, 0xE8));
}

// DEF3264: CTaskListWnd::_HandleMouseButtonDown
BOOL *EV_MM_TASKLIST_THUMB_DISABLING_FLAG(LONG_PTR lp)
{
	return (BOOL *)(lp + DO8_3264(0xA0, 0xFC, 0xA4, 0x104, ,, ,, 0xA0, 0x104, ,, 0x94, 0xEC, 0x98, 0xF4));
}

// DEF3264: case 0x4E: CTaskListWnd::v_WndProc (before a call to CTaskListWnd::_PositionToolTip)
LONG_PTR **EV_MM_TASKLIST_TRACKED_BUTTON_GROUP(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO8_3264(0xA8, 0x108, 0xAC, 0x110, ,, ,, 0xA8, 0x110, ,, 0x9C, 0xF8, 0xA0, 0x100));
}

// Until Windows 10 R4
// DEF3264: CTaskListWnd::_SetHotItem -or- CTaskListWnd::_HandleSetFocus
// Since Windows 10 R4
// DEF3264: CTaskListWnd::_SetActiveItem -or- CTaskListWnd::_StartPlateAnimations
int *EV_MM_TASKLIST_TRACKED_BUTTON_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO8_3264(0xB0, 0x118, 0xB4, 0x120, ,, ,, 0xB0, 0x120, ,, 0xA4, 0x108, 0xA8, 0x110));
}

// DEF3264: CTaskListWnd::_SetActiveItem
LONG_PTR **EV_MM_TASKLIST_ACTIVE_BUTTON_GROUP(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO10_3264(0xB4, 0x120, 0xB8, 0x128, ,, ,, 0xB4, 0x128, ,, 0xA8, 0x110, 0xAC, 0x118, ,, 0xB4, 0x128));
}

// DEF3264: CTaskListWnd::_SetActiveItem
int *EV_MM_TASKLIST_ACTIVE_BUTTON_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO10_3264(0xB8, 0x128, 0xBC, 0x130, ,, ,, 0xB8, 0x130, ,, 0xAC, 0x118, 0xB0, 0x120, ,, 0xB8, 0x130));
}

// DEF3264: CTaskListWnd::_HandleMouseButtonUp
LONG_PTR **EV_MM_TASKLIST_PRESSED_BUTTON_GROUP(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO10_3264(0xBC, 0x130, 0xC0, 0x138, ,, ,, 0xBC, 0x138, ,, 0xB0, 0x120, 0xB4, 0x128, ,, 0xBC, 0x138));
}

// DEF3264: CTaskListWnd::_DisplayExtendedUI
LONG_PTR **EV_MM_TASKLIST_THUMB_BUTTON_GROUP(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO10_3264(0xE4, 0x168, 0xE8, 0x170, ,, 0xEC, 0x178, 0xE8, 0x178, ,, 0xD4, 0x150, 0xD8, 0x158, ,, 0xE0, 0x168));
}

// DEF3264: CTaskListWnd::_DisplayExtendedUI -or- CTaskListWnd::_RefreshThumbnail
LONG_PTR *EV_MM_TASKLIST_MM_THUMBNAIL_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO10_3264(0xE8, 0x170, 0xEC, 0x178, ,, 0xF0, 0x180, 0xEC, 0x180, ,, 0xD8, 0x158, 0xDC, 0x160, ,, 0xE4, 0x170));
}

// DEF3264: CTaskListWnd::_ShowToolTip
UINT *EV_MM_TASKLIST_TOOLTIP_TIMER_ID(LONG_PTR lp)
{
	return (UINT *)(lp + DO10_3264(0xF0, 0x180, 0xF4, 0x188, ,, 0xF8, 0x190, 0xF4, 0x190, ,, 0xE0, 0x168, 0xE4, 0x170, ,, 0xEC, 0x180));
}

// DEF3264: CTaskListWnd::DismissHoverUI (value + 0x14/0x28)
UINT *EV_MM_TASKLIST_THUMB_TIMER_ID(LONG_PTR lp)
{
	return (UINT *)(lp + DO10_3264(0xF8, 0x190, 0xFC, 0x198, ,, 0x100, 0x1A0, 0xFC, 0x1A0, ,, 0xE8, 0x178, 0x14 + 0xD8, 0x28 + 0x158, ,, 0x14 + 0xE0, 0x28 + 0x168));
}

// DEF3264: CTaskListWnd::GetMonitor
HMONITOR *EV_MM_TASKLIST_HMONITOR(LONG_PTR lp)
{
	return (HMONITOR *)(lp + DEF3264(0x14, 0x28) + DO10_3264(0, 0, 0x144, 0x1F0, ,, 0x14C, 0x200, 0x154, 0x208, ,, 0x134, 0x1D0, ,, ,, 0x140, 0x1E8));
}

// DEF3264: CTaskListWnd::OnContextMenu
LONG_PTR *EV_MM_TASKLIST_TASK_ITEM_FILTER(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DEF3264(0x18, 0x30) + DO10_3264(0, 0, ,, ,, ,, 0x16C, 0x228, ,, 0x148, 0x1E8, 0x14C, 0x1F0, ,, 0x158, 0x208));
}

// DEF3264: CTaskListWnd::_HandleMouseButtonUp
DWORD *EV_MM_TASKLIST_DRAG_FLAG(LONG_PTR lp)
{
	return (DWORD *)(lp + DO10_3264(0x150, 0x210, 0x174, 0x240, ,, 0x17C, 0x250, 0x18C, 0x268, ,, 0x168, 0x228, 0x16C, 0x230, ,, 0x178, 0x248));
}

// Until Windows 11
// DEF3264: CTaskBtnGroup::GetLocation (has overloads) -or- CTaskBtnGroup::IsItemBeingRemoved
// Since Windows 11
// DEF3264: CTaskListWnd::HasAnimation (value + 0x38)
LONG_PTR **EV_MM_TASKLIST_ANIMATION_MANAGER(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO14_3264(0, 0, ,, ,, ,, 0x1EC, 0x2F8, ,, 0x1BC, 0x2A0, 0x1C0, 0x2A8, ,, 0x1CC, 0x2C0, ,,
		nExplorerQFE <= 1500 ? 0x1CC : 0x3DC, nExplorerQFE <= 1500 ? 0x2C0 : 0x4D0,
		nExplorerQFE <= 928 ? 0x1CC : 0x3DC, nExplorerQFE <= 928 ? 0x2C0 : 0x4D0,
		0x3DC, 0x4D0));
}

//////////////////////////////////////////////////////////////////////////
// CTaskListThumbnailWnd (lpMMThumbnailLongPtr)

// DEF3264: CTaskListThumbnailWnd::IsVisible
HWND *EV_MM_THUMBNAIL_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DO16_3264(0x30, 0x60, ,, ,, ,, 0x24, 0x40, ,, ,, ,, ,, ,, 0x28, 0x48, ,, ,, ,, ,, 0x20, 0x40));
}

LONG_PTR *EV_MM_THUMBNAIL_MM_TASKLIST_REF(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO11_3264(0x20, 0x40, ,, ,, ,, 0x28, 0x48, ,, ,, ,, ,, ,, 0x2C, 0x50));
}

LONG_PTR EV_MM_THUMBNAIL_MM_TASKLIST_LONG_PTR_VALUE(LONG_PTR lp)
{
	return *EV_MM_THUMBNAIL_MM_TASKLIST_REF(lp) - DEF3264(0x18, 0x30);
}

// DEF3264: CTaskListThumbnailWnd::_RefreshThumbnail
BYTE *EV_MM_THUMBNAIL_REDRAW_FLAGS(LONG_PTR lp)
{
	return (BYTE *)(lp + DO16_3264(0x44, 0x78, 0x54, 0x88, ,, ,, 0xD8, 0x104, ,, ,, ,, ,, ,, 0xE0, 0x114, ,, ,, ,, ,, 0, 0x10C));
}

// DEF3264: CTaskListThumbnailWnd::_InsertThumbnail
LONG_PTR **EV_MM_THUMBNAIL_TASK_GROUP(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO16_3264(0x64, 0xA8, 0x74, 0xB8, ,, ,, 0xFC, 0x138, ,, ,, ,, ,, ,, 0x104, 0x148, ,, ,, ,, ,, 0, 0x140));
}

// DEF3264: CTaskListThumbnailWnd::_RegisterThumbBars
HDPA *EV_MM_THUMBNAIL_THUMBNAILS_HDPA(LONG_PTR lp)
{
	return (HDPA *)(lp + DO16_3264(0x68, 0xB0, 0x7C, 0xC8, ,, ,, 0x104, 0x148, ,, ,, ,, ,, ,, 0x10C, 0x158, ,, ,, ,, ,, 0, 0x150));
}

// DEF3264: CTaskListThumbnailWnd::CTaskListThumbnailWnd
DWORD *EV_MM_THUMBNAIL_NUM_THUMBNAILS(LONG_PTR lp)
{
	return (DWORD *)(lp + DO16_3264(0x70, 0xBC, 0x84, 0xD4, ,, ,, 0x10C, 0x154, ,, ,, ,, ,, ,, 0x114, 0x164, ,, ,, ,, ,, 0, 0x15C));
}

// DEF3264: CTaskListThumbnailWnd::SetActiveItem
int *EV_MM_THUMBNAIL_ACTIVE_THUMB_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO16_3264(0x114, 0x168, 0x128, 0x180, ,, ,, 0x1CC, 0x228, ,, ,, ,, ,, ,, 0x1D4, 0x238, 0x1D8, 0x238, ,, ,, ,, 0, 0x230));
}

// DEF3264: CTaskListThumbnailWnd::SetHotItem
int *EV_MM_THUMBNAIL_TRACKED_THUMB_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO16_3264(0x11C, 0x170, 0x130, 0x188, ,, ,, 0x1D4, 0x230, ,, ,, ,, ,, ,, 0x1DC, 0x240, 0x1E0, 0x240, ,, ,, ,, 0, 0x238));
}

// DEF3264: CTaskListThumbnailWnd::_DrawScrollArrow
int *EV_MM_THUMBNAIL_PRESSED_THUMB_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO16_3264(0x120, 0x174, 0x134, 0x18C, ,, ,, 0x1D8, 0x234, ,, ,, ,, ,, ,, 0x1E0, 0x244, 0x1E4, 0x244, ,, ,, ,, 0, 0x23C));
}

// DEF3264: CTaskListThumbnailWnd::IsPopup
BOOL *EV_MM_THUMBNAIL_STICKY_FLAG(LONG_PTR lp)
{
	return (BOOL *)(lp + DO16_3264(0x144, 0x198, 0x158, 0x1B0, ,, ,, 0x1FC, 0x258, ,, ,, ,, ,, ,, 0x204, 0x268, 0x208, 0x268, ,, ,, ,, 0, 0x260));
}

// Until Windows 10 (for OPT_EX_LIST_REVERSE_ORDER)
// DEF3264: CTaskListThumbnailWnd::DisplayUI
BOOL *EV_MM_THUMBNAIL_LIST_FLAG(LONG_PTR lp)
{
	return (BOOL *)(lp + DO7_3264(0x1A0, 0x1F4, 0x1B8, 0x214, ,, 0x1BC, 0x218, 0x25C, 0x2C0, ,, 0x238, 0x29C));
}

// Until Windows 10 (for OPT_EX_LIST_REVERSE_ORDER)
// DEF3264: CTaskListThumbnailWnd::_GetMaxVisibleMenuItemCount
int *EV_MM_THUMBNAIL_LIST_FIRST_VISIBLE_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO7_3264(0x1A8, 0x1FC, 0x1C0, 0x21C, ,, 0x1C4, 0x220, 0x264, 0x2C8, ,, 0x240, 0x2A4));
}

//////////////////////////////////////////////////////////////////////////
// CTaskThumbnail

// Only in header file

//////////////////////////////////////////////////////////////////////////
// CPearl (start button)

HWND *EV_START_BUTTON_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DO7_3264(0x20, 0x38, ,, ,, ,, ,, ,, 0x10, 0x18));
}

//////////////////////////////////////////////////////////////////////////
// CTraySearchControl

// DEF3264: CTraySearchControl::ShowWindow
HWND *EV_TRAY_SEARCH_CONTROL_BUTTON_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DO14_3264(0, 0, ,, ,, ,, ,, ,, ,, ,, ,, ,, ,,
		nExplorerQFE <= 1500 ? 0 : 0x0C, nExplorerQFE <= 1500 ? 0 : 0x10,
		nExplorerQFE <= 928 ? 0 : 0x0C, nExplorerQFE <= 928 ? 0 : 0x10,
		0x10, 0x18));
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
// DEF3264: CTrayNotify::_SetClockToolbarThemes
HWND *EV_TRAY_NOTIFY_CLOCK_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO7_3264(0x18, 0x30, ,, ,, ,, 0x20, 0x40, ,, 0, 0));
}

// Since Windows 10 R1
// DEF3264: CTrayNotify::_AllowOverflowAutohide
LONG_PTR *EV_TRAY_NOTIFY_CLOCK_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO8_3264(0, 0, ,, ,, ,, ,, ,, 0x24, 0x48, 0x28, 0x50));
}

// DEF3264: CTrayNotify::_SizeWindows
HWND *EV_TRAY_NOTIFY_SHOW_DESKTOP_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO8_3264(0x28, 0x50, ,, ,, ,, 0x24, 0x48, ,, 0x28, 0x50, 0x2C, 0x58));
}

// DEF3264: CTrayNotify::_GetItemManager
HWND *EV_TRAY_NOTIFY_OVERFLOW_TOOLBAR_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO8_3264(0x2AC, 0x328, 0xA8, 0x128, ,, ,, 0x38, 0x70, ,, 0x3C, 0x78, 0x40, 0x80));
}

// DEF3264: CTrayNotify::_GetItemManager
HWND *EV_TRAY_NOTIFY_TEMPORARY_TOOLBAR_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO8_3264(0x2BC, 0x348, 0xB8, 0x148, ,, ,, 0x48, 0x90, ,, 0x4C, 0x98, 0x50, 0xA0));
}

// DEF3264: CTrayNotify::_GetItemManager
HWND *EV_TRAY_NOTIFY_TOOLBAR_WND(LONG_PTR lp)
{
	return (HWND *)(lp + DO8_3264(0x29C, 0x308, 0x98, 0x108, ,, ,, 0x58, 0xB0, ,, 0x5C, 0xB8, 0x60, 0xC0));
}

// DEF3264: CTrayNotify::_UpdateChevronState
// For 7+ Taskbar Numberer
BOOL *EV_TRAY_NOTIFY_CHEVRON_STATE(LONG_PTR lp)
{
	return (BOOL *)(lp + DO8_3264(0x324, 0x3EC, 0x124, 0x1EC, ,, ,, 0xB8, 0x148, ,, 0xB8, 0x150, 0xC0, 0x158));
}

// DEF3264: CTrayNotify::_SizeWindows -or- CTrayNotify::_GetNotifyIconToolbarButtonMetrics
// Near IsPointerDeviceSupportedOnHWND or IsPointerDeviceSupportedOnMonitor
BYTE *EV_TRAY_NOTIFY_PTRDEV_SUPPORTED(LONG_PTR lp)
{
	return (BYTE *)(lp + DO8_3264(0, 0, ,, ,, ,, 0xC8, 0x158, ,, 0xC8, 0x160, 0xD0, 0x168));
}

// DEF3264: CTrayNotify::_SizeWindows -or- CTrayNotify::_GetNotifyIconToolbarButtonMetrics
// Near IsPointerDeviceSupportedOnHWND or IsPointerDeviceSupportedOnMonitor
// TRUE if EV_TRAY_NOTIFY_PTRDEV_SUPPORTED is valid, FALSE otherwise.
BYTE *EV_TRAY_NOTIFY_PTRDEV_SUPPORTED_VALID(LONG_PTR lp)
{
	return (BYTE *)(lp + DO8_3264(0, 0, ,, ,, ,, 0xC9, 0x159, ,, 0xC9, 0x161, 0xD1, 0x169));
}

// DEF3264: CTrayNotify::_OpenTheme
// For 7+ Taskbar Numberer
HTHEME *EV_TRAY_NOTIFY_THEME(LONG_PTR lp)
{
	return (HTHEME *)(lp +
		DO14_3264(
			0x34C, 0x420,
			0x14C, 0x220,
			,,
			nExplorerQFE <= 17039 ? 0x14C : 0x150, nExplorerQFE <= 17039 ? 0x220 : 0x228,
			0xF0, 0x1A0,
			,,
			0xF4, 0x1B0,
			0x108, 0x1D0,
			0x10C, 0x1D8,
			0x110, 0x1D8,
			,,
			0x10C, 0x1D0,
			nExplorerQFE <= 572 ? 0x110 : 0x114, nExplorerQFE <= 572 ? 0x1D8 : 0x1E0,
			0x114, 0x1E0));
}

// DEF3264: CTrayNotify::_OnCDNotify -or- CTrayNotify::_OnDragEnd
// For Windows 8.1 Update 1:
// * <= build 17039
//   original
// * builds between the two
//   unknown
// * >= build 17238
//   with fixup
DWORD *EV_TRAY_NOTIFY_DRAG_FLAG(LONG_PTR lp)
{
	return (DWORD *)(lp +
		DO14_3264(
			0x3BC, 0x4B8,
			0x1BC, 0x2C0,
			,,
			nExplorerQFE <= 17039 ? 0x1BC : 0x1C4, nExplorerQFE <= 17039 ? 0x2C0 : 0x2C8,
			0x164, 0x240,
			,,
			0x16C, 0x250,
			0x17C, 0x270,
			0x184, 0x278,
			,,
			,,
			0x184, nExplorerQFE <= 997 ? 0x270 : 0x278,
			nExplorerQFE <= 572 ? 0x184 : 0x18C, nExplorerQFE <= 572 ? 0x278 : 0x280,
			0x188, 0x280));
}

//////////////////////////////////////////////////////////////////////////
// CClockCtl (lpTrayClockLongPtr)
// Until Windows 10 R1

// DEF3264: CClockCtl::_RecalcCurTime
WCHAR *EV_TRAY_CLOCK_TEXT(LONG_PTR lp)
{
	return (WCHAR *)(lp + DO5_3264(0x104, 0x10C, 0x64, 0x6C, ,, ,, 0x70, 0x7C));
}

// DEF3264: CClockCtl::_EnableTimer
BOOL *EV_TRAY_CLOCK_TIMER_ENABLED_FLAG(LONG_PTR lp)
{
	return (BOOL *)(lp + DO5_3264(0x1B4, 0x1C8, 0x11C, 0x134, ,, ,, 0x12C, 0x150));
}

// DEF3264: CClockCtl::_GetClockTextMinSize -or- CClockCtl::_CalcClockTextSizes
int *EV_TRAY_CLOCK_CACHED_TEXT_SIZE(LONG_PTR lp)
{
	return (int *)(lp + DO5_3264(0, 0, 0x134, 0x158, ,, ,, 0x144, 0x178));
}

//////////////////////////////////////////////////////////////////////////
// ClockButton (lpTrayClockLongPtr)
// Since Windows 10 R1

HWND *EV_CLOCK_BUTTON_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DEF3264(0x04, 0x08));
}

// DEF3264: ClockButton::v_UpdateTheme
BYTE *EV_CLOCK_BUTTON_SIZES_CACHED(LONG_PTR lp)
{
	return (BYTE *)(lp + DO8_3264(0, 0, ,, ,, ,, ,, ,, 0x60, 0xB0, 0x64, 0xB8));
}

// DEF3264: ClockButton::v_Initialize -or- ClockButton::UpdateTextStringsIfNecessary
BYTE *EV_CLOCK_BUTTON_SHOW_SECONDS(LONG_PTR lp)
{
	return (BYTE *)(lp + DO8_3264(0, 0, ,, ,, ,, ,, ,, 0x84, 0xD4, 0x88, 0xDC));
}

// DEF3264: ClockButton::UpdateTextStringsIfNecessary
WORD *EV_CLOCK_BUTTON_HOURS_CACHE(LONG_PTR lp)
{
	return (WORD *)(lp + DO8_3264(0, 0, ,, ,, ,, ,, ,, 0x8E, 0xE2, 0x8A, 0xDE));
}

// DEF3264: ClockButton::UpdateTextStringsIfNecessary
WORD *EV_CLOCK_BUTTON_MINUTES_CACHE(LONG_PTR lp)
{
	return (WORD *)(lp + DO8_3264(0, 0, ,, ,, ,, ,, ,, 0x90, 0xE4, 0x8C, 0xE0));
}

//////////////////////////////////////////////////////////////////////////
// CTaskItem

// DEF3264: CTaskItem::GetTabContainerItem -or- CTaskItem::SetTabContainerItem
LONG_PTR **EV_TASKITEM_CONTAINER_TASK_ITEM(LONG_PTR *plp)
{
	return (LONG_PTR **)((LONG_PTR)plp + DO5_3264(0x2C, 0x38, 0x38, 0x50, ,, 0x34, 0x48, 0x2C, 0x40));
}

// DEF3264 <= WIN_VERSION_81: CTaskItem::GetWindow
// DEF3264 >= WIN_VERSION_811: CWindowTaskItem::GetWindow -or- CWindowTaskItem::SetWindow
HWND *EV_TASKITEM_WND(LONG_PTR *plp)
{
	return (HWND *)((LONG_PTR)plp + DO14_3264(0x04, 0x08, 0x0C, 0x18, ,, 0x98, 0xE0, ,, ,, ,, ,, ,, 0x9C, 0xE8, ,, ,,
		nExplorerQFE <= 1266 ? 0x9C : (nExplorerQFE <= 1320 ? 0xA0 : 0x9C), nExplorerQFE <= 1266 ? 0xE8 : (nExplorerQFE <= 1320 ? 0xF0 : 0xE8),
		0xA0, 0xF0));
}

//////////////////////////////////////////////////////////////////////////
// CTaskGroup

// DEF3264: CTaskGroup::GetNumItems
HDPA *EV_TASKGROUP_TASKITEMS_HDPA(LONG_PTR *plp)
{
	return (HDPA *)((LONG_PTR)plp + DEF3264(0x10, 0x20));
}

// DEF3264: CTaskGroup::UpdateFlags
DWORD *EV_TASKGROUP_FLAGS(LONG_PTR *plp)
{
	return (DWORD *)((LONG_PTR)plp + DEF3264(0x14, 0x28));
}

// DEF3264: CTaskGroup::SetAppID
WCHAR **EV_TASKGROUP_APPID(LONG_PTR *plp)
{
	return (WCHAR **)((LONG_PTR)plp + DO5_3264(0x28, 0x48, ,, ,, ,, 0x20, 0x40));
}

// DEF3264: CTaskGroup::SetVisualOrder
int *EV_TASKGROUP_VISUAL_ORDER(LONG_PTR *plp)
{
	return (int *)((LONG_PTR)plp + DO5_3264(0x38, 0x5C, ,, ,, ,, 0x30, 0x54));
}

//////////////////////////////////////////////////////////////////////////
// CApplicationViewManager

// DEF3264 (twinui.dll -or- twinui.pcshell.dll): CApplicationViewManager::GetViews
SRWLOCK *EV_APP_VIEW_MGR_APP_ARRAY_LOCK(LONG_PTR lp)
{
	return (SRWLOCK *)(lp + DO14_3264(0, 0, ,, ,, ,, 0x84, 0x100, 0xE4, 0x190, 0xC4, 0x158, 0xD4, 0x168, 0xD8, 0x178, 0xDC, 0x180, 0x12C, 0x200, 0x144, 0x218, ,, 0, 0x210));
}

// DEF3264 (twinui.dll -or- twinui.pcshell.dll): CApplicationViewManager::GetViews
LONG_PTR **EV_APP_VIEW_MGR_APP_ARRAY(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO14_3264(0, 0, ,, ,, ,, 0x88, 0x108, 0xE8, 0x198, 0xC8, 0x160, 0xD8, 0x170, 0xDC, 0x180, 0xE0, 0x188, 0x130, 0x208, 0x148, 0x220, ,, 0, 0x218));
}

// DEF3264 (twinui.dll -or- twinui.pcshell.dll): CApplicationViewManager::GetViews
size_t *EV_APP_VIEW_MGR_APP_ARRAY_SIZE(LONG_PTR lp)
{
	return (size_t *)(lp + DO14_3264(0, 0, ,, ,, ,, 0x8C, 0x110, 0xEC, 0x1A0, 0xCC, 0x168, 0xDC, 0x178, 0xE0, 0x188, 0xE4, 0x190, 0x134, 0x210, 0x14C, 0x228, ,, 0, 0x220));
}
