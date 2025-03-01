#include "stdafx.h"
#include "explorer_vars.h"

extern LONG_PTR lpTaskbarLongPtr, lpTaskSwLongPtr;

//////////////////////////////////////////////////////////////////////////
// CTray (lpTaskbarLongPtr)

static LONG_PTR EV_TASKBAR_OFFSET_FIX(LONG_PTR offset)
{
	// In Windows 8.1 Update 1, since build 17930, a new virtual table pointer appeared.
	// The change was caused by update KB3072318.
	if(nWinVersion == WIN_VERSION_811 && nExplorerQFE >= 17930 && nExplorerQFE < 20000 && offset >= 8 * sizeof(LONG_PTR))
		return offset + sizeof(LONG_PTR);

	// In Windows 10 R3, since 10.0.16299.1480 (after 10.0.16299.1419, not including), some field(s) were added.
	// The change was caused by update KB4520006.
	if(nWinVersion == WIN_VERSION_10_R3 && nExplorerQFE >= 1480 && offset >= DEF3264(0x1D8, 0x2B0))
		return offset + sizeof(LONG_PTR);

	// In Windows 10 R4, since 10.0.17763.677 (after 10.0.17763.165, not including), some field(s) were removed.
	// The change was caused by update KB4489894.
	// In Windows 10 R4, since 10.0.17134.1098 (after 10.0.17134.1038, not including), some field(s) were added.
	// The change was caused by update KB4519978.
	// In Windows 10 R5, since 10.0.17763.346 (after 10.0.17763.107, not including), some field(s) were removed.
	// The change was caused by update KB4482887.
	// In Windows 10 R5, since 10.0.17763.831 (after 10.0.17763.771, not including), some field(s) were added.
	// The change was caused by update KB4520062.
	if(((nWinVersion == WIN_VERSION_10_R4 && nExplorerQFE >= 677) || (nWinVersion == WIN_VERSION_10_R5 && nExplorerQFE >= 346)) && offset >= DEF3264(0x8C, 0xD0))
		if(((nWinVersion == WIN_VERSION_10_R4 && nExplorerQFE >= 1098) || (nWinVersion == WIN_VERSION_10_R5 && nExplorerQFE >= 831)) && offset >= DEF3264(0x1D8, 0x2B0))
			return offset;
		else
			return offset - sizeof(LONG_PTR);

	return offset;
}

static LONG_PTR EV_TRAY_UI_OFFSET_FIX(LONG_PTR offset)
{
	return nWinVersion < WIN_VERSION_10_R2 ? EV_TASKBAR_OFFSET_FIX(offset) : offset;
}

// Pointer to the TrayUI instance, since Windows 10 R2.
LONG_PTR EV_TRAY_UI(LONG_PTR lp)
{
	if(nWinVersion < WIN_VERSION_10_R2)
		return lp;

	static LONG_PTR offset = 0;

	// Try deducing the offset from assembly, which is more stable than the offset
	// itself regarding changes between updates.
	if(!offset)
	{
		// CTray::SendStateCaptureTelemetry
		LONG_PTR *plp = *(LONG_PTR **)(lpTaskbarLongPtr + DEF3264(0x20, 0x40));

		void *pSendStateCaptureTelemetry = (void *)FUNC_CTray_SendStateCaptureTelemetry(plp);

#ifdef _WIN64
		if(!offset)
		{
			/*
				explorer_10_0_22621_2283_x64!CTray::SendStateCaptureTelemetry:
				4c8bdc          mov     r11,rsp
				49895b10        mov     qword ptr [r11+10h],rbx
				49896b18        mov     qword ptr [r11+18h],rbp
				49897320        mov     qword ptr [r11+20h],rsi
				57              push    rdi
				4883ec20        sub     rsp,20h
				8bea            mov     ebp,edx
				33ff            xor     edi,edi
				488bb130030000  mov     rsi,qword ptr [rcx+330h]
			*/

			BYTE* p = (BYTE *)pSendStateCaptureTelemetry;
			if(
				p[0] == 0x4C && p[1] == 0x8B && p[2] == 0xDC &&
				p[3] == 0x49 && p[4] == 0x89 && p[5] == 0x5B &&
				p[7] == 0x49 && p[8] == 0x89 && p[9] == 0x6B &&
				p[11] == 0x49 && p[12] == 0x89 && p[13] == 0x73 &&
				p[15] == 0x57 &&
				p[16] == 0x48 && p[17] == 0x83 && p[18] == 0xEC &&
				p[20] == 0x8B && p[21] == 0xEA &&
				p[22] == 0x33 && p[23] == 0xFF &&
				p[24] == 0x48 && p[25] == 0x8B && p[26] == 0xB1
			)
			{
				offset = *(DWORD *)(p + 27);
			}
		}

		if(!offset)
		{
			/*
				explorer_10_0_19041_1415_x64!CTray::SendStateCaptureTelemetry:
				48895c2410      mov     qword ptr [rsp+10h],rbx
				57              push    rdi
				4883ec20        sub     rsp,20h
				8bfa            mov     edi,edx
				33db            xor     ebx,ebx
				488b89e0020000  mov     rcx,qword ptr [rcx+2E0h]
			*/

			BYTE* p = (BYTE *)pSendStateCaptureTelemetry;
			if(
				p[0] == 0x48 && p[1] == 0x89 && p[2] == 0x5C && p[3] == 0x24 &&
				p[4] == 0x10 &&
				p[5] == 0x57 &&
				p[6] == 0x48 && p[7] == 0x83 && p[8] == 0xEC &&
				p[10] == 0x8B && p[11] == 0xFA &&
				p[12] == 0x33 && p[13] == 0xDB &&
				p[14] == 0x48 && p[15] == 0x8B && p[16] == 0x89
			)
			{
				offset = *(DWORD *)(p + 17);
			}
		}

		if(!offset)
		{
			/*
				explorer_10_0_17763_4840_x64!CTray::SendStateCaptureTelemetry:
				4c8bdc          mov     r11,rsp
				57              push    rdi
				4883ec30        sub     rsp,30h
				49c743e8feffffff mov     qword ptr [r11-18h],0FFFFFFFFFFFFFFFEh
				49895b10        mov     qword ptr [r11+10h],rbx
				8bfa            mov     edi,edx
				33db            xor     ebx,ebx
				488b8988020000  mov     rcx,qword ptr [rcx+288h]
			*/

			BYTE *p = (BYTE *)pSendStateCaptureTelemetry;
			if(
				p[0] == 0x4C && p[1] == 0x8B && p[2] == 0xDC &&
				p[3] == 0x57 &&
				p[4] == 0x48 && p[5] == 0x83 && p[6] == 0xEC &&
				p[8] == 0x49 && p[9] == 0xC7 && p[10] == 0x43 && p[11] == 0xE8 &&
				p[16] == 0x49 && p[17] == 0x89 && p[18] == 0x5B &&
				p[20] == 0x8B && p[21] == 0xFA &&
				p[22] == 0x33 && p[23] == 0xDB &&
				p[24] == 0x48 && p[25] == 0x8B && p[26] == 0x89
			)
			{
				offset = *(DWORD *)(p + 27);
			}
		}

		if(!offset)
		{
			/*
				explorer_10_0_16299_1992_x64!CTray::SendStateCaptureTelemetry:
				488bc4          mov     rax,rsp
				57              push    rdi
				4883ec30        sub     rsp,30h
				48c740e8feffffff mov     qword ptr [rax-18h],0FFFFFFFFFFFFFFFEh
				48895810        mov     qword ptr [rax+10h],rbx
				48897018        mov     qword ptr [rax+18h],rsi
				8bf2            mov     esi,edx
				33db            xor     ebx,ebx
				488bb988020000  mov     rdi,qword ptr [rcx+288h]
			*/

			BYTE *p = (BYTE *)pSendStateCaptureTelemetry;
			if(
				p[0] == 0x48 && p[1] == 0x8B && p[2] == 0xC4 &&
				p[3] == 0x57 &&
				p[4] == 0x48 && p[5] == 0x83 && p[6] == 0xEC &&
				p[8] == 0x48 && p[9] == 0xC7 && p[10] == 0x40 && p[11] == 0xE8 &&
				p[16] == 0x48 && p[17] == 0x89 && p[18] == 0x58 &&
				p[20] == 0x48 && p[21] == 0x89 && p[22] == 0x70 &&
				p[24] == 0x8B && p[25] == 0xF2 &&
				p[26] == 0x33 && p[27] == 0xDB &&
				p[28] == 0x48 && p[29] == 0x8B && p[30] == 0xB9
			)
			{
				offset = *(DWORD *)(p + 31);
			}
		}

		if(!offset)
		{
			/*
				explorer_10_0_15063_2614_x64!CTray::SendStateCaptureTelemetry:
				4057            push    rdi
				4883ec30        sub     rsp,30h
				48c7442420feffffff mov   qword ptr [rsp+20h],0FFFFFFFFFFFFFFFEh
				48895c2440      mov     qword ptr [rsp+40h],rbx
				4889742448      mov     qword ptr [rsp+48h],rsi
				8bf2            mov     esi,edx
				488bd9          mov     rbx,rcx
				33ff            xor     edi,edi
				e86b95fdff      call    explorer_10_0_15063_2614_x64!wil::Feature<__WilFeatureTraits_Feature_StateTelemetry>::ReportUsageToService (00000001`400745c0)
				488b9b68020000  mov     rbx,qword ptr [rbx+268h]
			*/

			BYTE *p = (BYTE *)pSendStateCaptureTelemetry;
			if(
				p[0] == 0x40 && p[1] == 0x57 &&
				p[2] == 0x48 && p[3] == 0x83 && p[4] == 0xEC &&
				p[6] == 0x48 && p[7] == 0xC7 && p[8] == 0x44 && p[9] == 0x24 && p[10] == 0x20 &&
				p[15] == 0x48 && p[16] == 0x89 && p[17] == 0x5C && p[18] == 0x24 &&
                p[20] == 0x48 && p[21] == 0x89 && p[22] == 0x74 && p[23] == 0x24 &&
				p[25] == 0x8B && p[26] == 0xF2 &&
				p[27] == 0x48 && p[28] == 0x8B && p[29] == 0xD9 &&
				p[30] == 0x33 && p[31] == 0xFF &&
				p[32] == 0xE8 &&
				p[37] == 0x48 && p[38] == 0x8B && p[39] == 0x9B
			)
			{
				offset = *(DWORD *)(p + 40);
			}
		}
#else // !_WIN64
		if(!offset)
		{
			/*
				explorer_10_0_16299_1992_x86!CTray::SendStateCaptureTelemetry:
				6a08            push    8
				b83e984c00      mov     eax,offset explorer_10_0_16299_1992_x86!Windows::Foundation::IPropertyValueStatics::`vcall'{68}'+0x4654 (004c983e)
				e80f3ff8ff      call    explorer_10_0_16299_1992_x86!_EH_prolog3_GS (004b49fb)
				8b4508          mov     eax,dword ptr [ebp+8]
				33ff            xor     edi,edi
				8b98c4010000    mov     ebx,dword ptr [eax+1C4h]
			*/

			BYTE *p = (BYTE *)pSendStateCaptureTelemetry;
			if(
				p[0] == 0x6A &&
				p[2] == 0xB8 &&
				p[7] == 0xE8 &&
				p[12] == 0x8B && p[13] == 0x45 &&
				p[15] == 0x33 && p[16] == 0xFF &&
				p[17] == 0x8B && p[18] == 0x98
			)
			{
				offset = *(DWORD *)(p + 19);
			}
		}

		if(!offset)
		{
			/*
				explorer_10_0_15063_2614_x86!CTray::SendStateCaptureTelemetry:
				6a08            push    8
				b80c5b4c00      mov     eax,offset explorer_10_0_15063_2614_x86!Windows::Foundation::IPropertyValueStatics::`vcall'{68}'+0x4a42 (004c5b0c)
				e895bff8ff      call    explorer_10_0_15063_2614_x86!_EH_prolog3_GS (004bc2b1)
				51              push    ecx
				33ff            xor     edi,edi
				e86c6af6ff      call    explorer_10_0_15063_2614_x86!wil::Feature<__WilFeatureTraits_Feature_StateTelemetry>::ReportUsageToService (00496d90)
				8b4508          mov     eax,dword ptr [ebp+8]
				8b98a8010000    mov     ebx,dword ptr [eax+1A8h]
			*/

			BYTE *p = (BYTE *)pSendStateCaptureTelemetry;
			if(
				p[0] == 0x6A &&
				p[2] == 0xB8 &&
				p[7] == 0xE8 &&
				p[12] == 0x51 &&
				p[13] == 0x33 && p[14] == 0xFF &&
				p[15] == 0xE8 &&
				p[20] == 0x8B && p[21] == 0x45 &&
				p[23] == 0x8B && p[24] == 0x98
			)
			{
				offset = *(DWORD *)(p + 25);
			}
		}
#endif

		if(offset)
			offset += DEF3264(0x20, 0x40);
	}

	if(!offset)
	{
		// DEF3264: CTray::SendStateCaptureTelemetry
		offset = DO17_3264(0, 0, ,, ,, ,, ,, ,, ,,
			0x1C8, 0x2A8,
			EV_TASKBAR_OFFSET_FIX(0x1E0), EV_TASKBAR_OFFSET_FIX(0x2C0),
			EV_TASKBAR_OFFSET_FIX(0x1E4), EV_TASKBAR_OFFSET_FIX(0x2C8),
			,,
			nExplorerQFE <= 329 ? 0x1E4 : (nExplorerQFE <= 997 ? 0x1E8 : 0x1EC),
			nExplorerQFE <= 329 ? 0x2C8 : 0x2D0,
			nExplorerQFE <= 388 ? 0x1F4 : (nExplorerQFE <= 572 ? 0x21C : (nExplorerQFE <= 5487 ? 0x220 : 0x200)),
			nExplorerQFE <= 388 ? 0x2E0 : (nExplorerQFE <= 5487 ? 0x320 : 0x2E0),
			0x370,
			,
			,
			0x408);
	}

	return *(LONG_PTR *)(lp + offset) - DEF3264(0x0C, 0x18);
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
			nExplorerQFE <= 997 ? 0x118 : 0x11C,
			nExplorerQFE <= 997 ? 0x160 : 0x168,
			nExplorerQFE <= 572 ? 0x118 : (nExplorerQFE <= 2193 ? 0x11C : 0x120),
			nExplorerQFE <= 572 ? 0x160 : 0x168,
			0x170)));
}

// DEF3264: TrayUI::_StuckTrayChange (previously CTray::_StuckTrayChange)
int *EV_TASKBAR_POS(void)
{
	return (int *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO14_3264(0xCD8, 0xEB0, 0x180, 0x208, ,, 0x188, 0x218, 0x180, 0x210, ,, 0x198, 0x230, 0x110, 0x160, ,, 0x118, 0x168, ,,
			nExplorerQFE <= 997 ? 0x11C : 0x120,
			nExplorerQFE <= 997 ? 0x168 : 0x170,
			nExplorerQFE <= 572 ? 0x11C : (nExplorerQFE <= 2193 ? 0x120 : 0x124),
			nExplorerQFE <= 572 ? 0x168 : 0x170,
			0x178)));
}

// DEF3264: TrayUI::_HandleSizing (previously CTray::_HandleSizing)
BOOL *EV_TASKBAR_UNLOCKED_FLAG(void)
{
	return (BOOL *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO14_3264(0xD28, 0xF24, 0x1D0, 0x27C, ,, 0x1D8, 0x28C, 0x1D8, 0x28C, ,, 0x1F0, 0x2AC, 0x158, 0x1C0, ,, 0x160, 0x1C8, ,,
			nExplorerQFE <= 997 ? 0x164 : 0x168,
			nExplorerQFE <= 997 ? 0x1C8 : 0x1D0,
			nExplorerQFE <= 572 ? 0x164 : (nExplorerQFE <= 2193 ? 0x168 : 0x16C),
			nExplorerQFE <= 572 ? 0x1C8 : 0x1D0,
			0x1D8)));
}

// DEF3264: CTray::_RecomputeWorkArea -or- CTray::RecomputeWorkArea
// Since Windows 11 22H2, search for the sum in assembly.
BOOL *EV_TASKBAR_TOPMOST_EX_FLAG(void)
{
	return (BOOL *)(lpTaskbarLongPtr + EV_TASKBAR_OFFSET_FIX(
		DO17_3264(0xD98, 0xFAC, 0x210, 0x2CC, ,, 0x218, 0x2DC, 0x210, 0x2D0, ,, 0x228, 0x2F0, 0x1C + 0xD0, 0x38 + 0x130, ,, ,, ,,
			0x1C + (nExplorerQFE <= 997 ? 0xD0 : 0xD4),
			0x38 + 0x130,
			0x1C + (nExplorerQFE <= 388 ? 0xD0 : (nExplorerQFE <= 572 ? 0xF4 : (nExplorerQFE <= 4123 ? 0xF8 : (nExplorerQFE <= 5487 ? 0xFC : 0xF8)))),
			0x38 + (nExplorerQFE <= 388 ? 0x130 : (nExplorerQFE <= 4123 ? 0x168 : (nExplorerQFE <= 5487 ? 0x170 : 0x168))),
			0x38 + 0x170,
			,
			nExplorerQFE <= 3007 ? 0x1A8 : 0x1D0,
			0x1D0)));
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
			nExplorerQFE <= 997 ? 0x1A8 : 0x1AC,
			nExplorerQFE <= 997 ? 0x228 : 0x230,
			nExplorerQFE <= 572 ? 0x1A8 : (nExplorerQFE <= 2193 ? 0x1AC : 0x1B0),
			nExplorerQFE <= 572 ? 0x228 : 0x230,
			0x238,
			,
			0x230)));
}

// DEF3264: TrayUI::TrayUI
LONG_PTR *EV_TASKBAR_BACK_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO16_3264(0, 0, ,, ,, ,, 0x2EC, 0x428, 0x2F0, 0x430, 0x318, 0x450, 0x198, 0x220, 0x1A0, 0x228, 0x1A8, 0x230, ,,
			nExplorerQFE <= 997 ? 0x1AC : (nExplorerQFE <= 1500 ? 0x1B0 : 0x1B4),
			nExplorerQFE <= 997 ? 0x230 : (nExplorerQFE <= 1500 ? 0x238 : 0x240),
			nExplorerQFE <= 572 ? 0x1AC : (nExplorerQFE <= 928 ? 0x1B0 : (nExplorerQFE <= 2193 ? 0x1B4 : 0x1B8)),
			nExplorerQFE <= 572 ? 0x230 : (nExplorerQFE <= 928 ? 0x238 : 0x240),
			0x248,
			,
			0x240)));
}

// DEF3264: TrayUI::_HandleSettingChange
LONG_PTR *EV_TASKBAR_SEARCH_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO16_3264(0, 0, ,, ,, ,, 0x2E0, 0x410, 0x2E4, 0x418, 0x31C, 0x458, 0x19C, 0x228, 0x1A4, 0x230, 0x1AC, 0x238, ,,
			nExplorerQFE <= 997 ? 0x1B0 : (nExplorerQFE <= 1500 ? 0x1B4 : 0x1B8),
			nExplorerQFE <= 997 ? 0x238 : (nExplorerQFE <= 1500 ? 0x240 : 0x248),
			nExplorerQFE <= 572 ? 0x1B0 : (nExplorerQFE <= 928 ? 0x1B4 : (nExplorerQFE <= 2193 ? 0x1B8 : 0x1BC)),
			nExplorerQFE <= 572 ? 0x238 : (nExplorerQFE <= 928 ? 0x240 : 0x248),
			0x250,
			,
			0x248)));
}

// DEF3264: TrayUI::_HandleSettingChange
LONG_PTR *EV_TASKBAR_CORTANA_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO16_3264(0, 0, ,, ,, ,, ,, ,, ,, ,, ,, ,, ,,
			nExplorerQFE <= 997 ? 0x1B4 : (nExplorerQFE <= 1500 ? 0x1B8 : 0x1BC),
			nExplorerQFE <= 997 ? 0x240 : (nExplorerQFE <= 1500 ? 0x248 : 0x250),
			nExplorerQFE <= 572 ? 0x1B4 : (nExplorerQFE <= 928 ? 0x1B8 : (nExplorerQFE <= 2193 ? 0x1BC : 0x1C0)),
			nExplorerQFE <= 572 ? 0x240 : (nExplorerQFE <= 928 ? 0x248 : 0x250),
			0x258,
			,
			0)));
}

// DEF3264: TrayUI::_HandleSettingChange
LONG_PTR *EV_TASKBAR_TRAY_SEARCH_CONTROL(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO16_3264(0, 0, ,, ,, ,, 0x2E4, 0x418, 0x2E8, 0x420, 0x320, 0x460, 0x1A0, 0x230, 0x1A8, 0x238, 0x1B0, 0x240, ,,
			nExplorerQFE <= 997 ? 0x1B8 : (nExplorerQFE <= 1500 ? 0x1BC : 0x1B0),
			nExplorerQFE <= 997 ? 0x248 : (nExplorerQFE <= 1500 ? 0x250 : 0x238),
			nExplorerQFE <= 572 ? 0x1B8 : (nExplorerQFE <= 928 ? 0x1BC : (nExplorerQFE <= 2193 ? 0x1B0 : 0x1B4)),
			nExplorerQFE <= 572 ? 0x248 : (nExplorerQFE <= 928 ? 0x250 : 0x238),
			0x240,
			,
			0x238)));
}

// DEF3264: TrayUI::TrayUI
LONG_PTR *EV_TASKBAR_MULTITASKING_LONG_PTR(void)
{
	return (LONG_PTR *)(EV_TRAY_UI(lpTaskbarLongPtr) + EV_TRAY_UI_OFFSET_FIX(
		DO16_3264(0, 0, ,, ,, ,, 0x2E8, 0x420, 0x2EC, 0x428, 0x324, 0x468, 0x1A4, 0x238, 0x1AC, 0x240, 0x1B4, 0x248, ,,
			nExplorerQFE <= 997 ? 0x1BC : 0x1C0,
			nExplorerQFE <= 997 ? 0x250 : 0x258,
			nExplorerQFE <= 572 ? 0x1BC : (nExplorerQFE <= 2193 ? 0x1C0 : 0x1C4),
			nExplorerQFE <= 572 ? 0x250 : 0x258,
			0x260,
			0x268,
			0x258)));
}

// DEF3264: CTray::_InitStartButtonEtc
void *EV_TASKBAR_W7_START_BTN_CLASS(void)
{
	return (void *)(lpTaskbarLongPtr + DEF3264(0xAF8, 0xC20));
}

// DEF3264: CTray::_HandleSizing
int *EV_TASKBAR_W7_WIDTH_PADDING(void)
{
	return (int *)(lpTaskbarLongPtr + DEF3264(0xB18, 0xC58));
}

// DEF3264: CTray::_HandleSizing
int *EV_TASKBAR_W7_HEIGHT_PADDING(void)
{
	return (int *)(lpTaskbarLongPtr + DEF3264(0xB1C, 0xC5C));
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
		nExplorerQFE <= 1500 ? 0xB8 : 0xBC,
		0xF0,
		nExplorerQFE <= 928 ? 0xB8 : 0xBC,
		0xF0,
		0xF0));
}

// DEF3264: CSecondaryTray::CSecondaryTray
LONG_PTR *EV_SECONDARY_TASKBAR_SEARCH_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO16_3264(0, 0, ,, ,, ,, 0xB8, 0xF0, 0xBC, 0xF8, 0xC0, 0x100, ,, ,, ,, ,,
		nExplorerQFE <= 1500 ? 0xC0 : 0xC4,
		0x100,
		nExplorerQFE <= 928 ? 0xC0 : (nExplorerQFE <= 4648 ? 0xC4 : 0xCC),
		nExplorerQFE <= 4648 ? 0x100 : 0x110,
		0x100,
		,
		0xF8));
}

// DEF3264: CSecondaryTray::_OnSettingChange
LONG_PTR *EV_SECONDARY_TASKBAR_CORTANA_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO16_3264(0, 0, ,, ,, ,, ,, ,, ,, ,, ,, ,, ,,
		nExplorerQFE <= 1500 ? 0xC4 : 0xC8,
		0x108,
		nExplorerQFE <= 928 ? 0xC4 : (nExplorerQFE <= 4648 ? 0xC8 : 0xD0),
		nExplorerQFE <= 4648 ? 0x108 : 0x118,
		0x108,
		,
		0));
}

// DEF3264: CSecondaryTray::CSecondaryTray
LONG_PTR *EV_SECONDARY_TASKBAR_MULTITASKING_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO16_3264(0, 0, ,, ,, ,, 0xBC, 0xF8, 0xC0, 0x100, 0xC4, 0x108, ,, ,, ,, ,,
		nExplorerQFE <= 1500 ? 0xC8 : 0xCC,
		0x110,
		nExplorerQFE <= 928 ? 0xC8 : (nExplorerQFE <= 4648 ? 0xCC : 0xD4),
		nExplorerQFE <= 4648 ? 0x110 : 0x120,
		0x110,
		,
		0x100));
}

// DEF3264: CSecondaryTray::_DestroyClockControlIfApplicable
LONG_PTR *EV_SECONDARY_TASKBAR_CLOCK_LONG_PTR(LONG_PTR lp)
{
	return (LONG_PTR *)(lp + DO16_3264(0, 0, ,, ,, ,, ,, ,, 0xC8, 0x110, ,, ,, ,, ,,
		nExplorerQFE <= 1500 ? 0xCC : 0xD4,
		nExplorerQFE <= 1500 ? 0x118 : 0x120,
		nExplorerQFE <= 928 ? 0xCC : (nExplorerQFE <= 4648 ? 0xD4 : 0xDC),
		nExplorerQFE <= 928 ? 0x118 : (nExplorerQFE <= 4648 ? 0x120 : 0x130),
		0x118,
		0x120,
		0x110));
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
		DO16_3264(0, 0, ,, ,, ,, 0x120, 0x1F8, 0x12C, 0x210, 0x118, 0x1F8, 0x128, 0x208, ,, ,, 0x164, 0x260, ,, 0x168, 0x268,
			, , 0x208));
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
		nExplorerQFE <= 1500 ? 0x1CC : 0x3DC,
		nExplorerQFE <= 1500 ? 0x2C0 : 0x4D0,
		nExplorerQFE <= 928 ? 0x1CC : 0x3DC,
		nExplorerQFE <= 928 ? 0x2C0 : 0x4D0,
		0x4D0));
}

//////////////////////////////////////////////////////////////////////////
// CTaskListThumbnailWnd (lpMMThumbnailLongPtr)

// DEF3264: CTaskListThumbnailWnd::IsVisible
HWND *EV_MM_THUMBNAIL_HWND(LONG_PTR lp)
{
	return (HWND *)(lp + DO16_3264(0x30, 0x60, ,, ,, ,, 0x24, 0x40, ,, ,, ,, ,, ,, 0x28, 0x48, ,, ,,
		, , 0x40));
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
	return (BYTE *)(lp + DO16_3264(0x44, 0x78, 0x54, 0x88, ,, ,, 0xD8, 0x104, ,, ,, ,, ,, ,, 0xE0, 0x114, ,, ,,
		, , 0x10C));
}

// DEF3264: CTaskListThumbnailWnd::_InsertThumbnail
LONG_PTR **EV_MM_THUMBNAIL_TASK_GROUP(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO16_3264(0x64, 0xA8, 0x74, 0xB8, ,, ,, 0xFC, 0x138, ,, ,, ,, ,, ,, 0x104, 0x148, ,, ,,
		, , 0x140));
}

// DEF3264: CTaskListThumbnailWnd::_RegisterThumbBars
HDPA *EV_MM_THUMBNAIL_THUMBNAILS_HDPA(LONG_PTR lp)
{
	return (HDPA *)(lp + DO16_3264(0x68, 0xB0, 0x7C, 0xC8, ,, ,, 0x104, 0x148, ,, ,, ,, ,, ,, 0x10C, 0x158, ,, ,,
		, , 0x150));
}

// DEF3264: CTaskListThumbnailWnd::CTaskListThumbnailWnd
DWORD *EV_MM_THUMBNAIL_NUM_THUMBNAILS(LONG_PTR lp)
{
	return (DWORD *)(lp + DO16_3264(0x70, 0xBC, 0x84, 0xD4, ,, ,, 0x10C, 0x154, ,, ,, ,, ,, ,, 0x114, 0x164, ,, ,,
		, , 0x15C));
}

// DEF3264: CTaskListThumbnailWnd::SetActiveItem
int *EV_MM_THUMBNAIL_ACTIVE_THUMB_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO16_3264(0x114, 0x168, 0x128, 0x180, ,, ,, 0x1CC, 0x228, ,, ,, ,, ,, ,, 0x1D4, 0x238, 0x1D8, 0x238, ,,
		, , 0x230));
}

// DEF3264: CTaskListThumbnailWnd::SetHotItem
int *EV_MM_THUMBNAIL_TRACKED_THUMB_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO16_3264(0x11C, 0x170, 0x130, 0x188, ,, ,, 0x1D4, 0x230, ,, ,, ,, ,, ,, 0x1DC, 0x240, 0x1E0, 0x240, ,,
		, , 0x238));
}

// DEF3264: CTaskListThumbnailWnd::_DrawScrollArrow
int *EV_MM_THUMBNAIL_PRESSED_THUMB_INDEX(LONG_PTR lp)
{
	return (int *)(lp + DO16_3264(0x120, 0x174, 0x134, 0x18C, ,, ,, 0x1D8, 0x234, ,, ,, ,, ,, ,, 0x1E0, 0x244, 0x1E4, 0x244, ,,
		, , 0x23C));
}

// DEF3264: CTaskListThumbnailWnd::IsPopup
BOOL *EV_MM_THUMBNAIL_STICKY_FLAG(LONG_PTR lp)
{
	return (BOOL *)(lp + DO16_3264(0x144, 0x198, 0x158, 0x1B0, ,, ,, 0x1FC, 0x258, ,, ,, ,, ,, ,, 0x204, 0x268, 0x208, 0x268, ,,
		, , 0x260));
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
		nExplorerQFE <= 1500 ? 0 : 0x0C,
		nExplorerQFE <= 1500 ? 0 : 0x10,
		nExplorerQFE <= 928 ? 0 : (nExplorerQFE <= 4170 ? 0x0C : 0x34),
		nExplorerQFE <= 928 ? 0 : (nExplorerQFE <= 4170 ? 0x10 : 0x40),
		0x18));
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
	return (BOOL *)(lp + DO14_3264(0x324, 0x3EC, 0x124, 0x1EC, ,, ,, 0xB8, 0x148, ,, 0xB8, 0x150, 0xC0, 0x158, ,, ,, ,, ,,
		nExplorerQFE <= 3693 ? 0xC0 : (nExplorerQFE <= 5487 ? 0xD0 : 0xC0),
		nExplorerQFE <= 3693 ? 0x158 : (nExplorerQFE <= 5487 ? 0x160 : 0x158),
		0x158));
}

// DEF3264: CTrayNotify::_SizeWindows -or- CTrayNotify::_GetNotifyIconToolbarButtonMetrics
// Near IsPointerDeviceSupportedOnHWND or IsPointerDeviceSupportedOnMonitor
BYTE *EV_TRAY_NOTIFY_PTRDEV_SUPPORTED(LONG_PTR lp)
{
	return (BYTE *)(lp + DO14_3264(0, 0, ,, ,, ,, 0xC8, 0x158, ,, 0xC8, 0x160, 0xD0, 0x168, ,, ,, ,, ,,
		nExplorerQFE <= 3693 ? 0xD0 : (nExplorerQFE <= 5487 ? 0xE0 : 0xD0),
		nExplorerQFE <= 3693 ? 0x168 : (nExplorerQFE <= 5487 ? 0x170 : 0x168),
		0x168));
}

// DEF3264: CTrayNotify::_SizeWindows -or- CTrayNotify::_GetNotifyIconToolbarButtonMetrics
// Near IsPointerDeviceSupportedOnHWND or IsPointerDeviceSupportedOnMonitor
// TRUE if EV_TRAY_NOTIFY_PTRDEV_SUPPORTED is valid, FALSE otherwise.
BYTE *EV_TRAY_NOTIFY_PTRDEV_SUPPORTED_VALID(LONG_PTR lp)
{
	return (BYTE *)(lp + DO14_3264(0, 0, ,, ,, ,, 0xC9, 0x159, ,, 0xC9, 0x161, 0xD1, 0x169, ,, ,, ,, ,,
		nExplorerQFE <= 3693 ? 0xD1 : (nExplorerQFE <= 5487 ? 0xE1 : 0xD1),
		nExplorerQFE <= 3693 ? 0x169 : (nExplorerQFE <= 5487 ? 0x171 : 0x169),
		0x169));
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
			nExplorerQFE <= 572 ? 0x110 : (nExplorerQFE <= 3693 ? 0x114 : (nExplorerQFE <= 5487 ? 0x128 : 0x114)),
			nExplorerQFE <= 572 ? 0x1D8 : (nExplorerQFE <= 3693 ? 0x1E0 : (nExplorerQFE <= 5487 ? 0x1F0 : 0x1E0)),
			0x1E0));
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
			nExplorerQFE <= 572 ? 0x184 : (nExplorerQFE <= 3693 ? 0x18C : (nExplorerQFE <= 5487 ? 0x19C : 0x18C)),
			nExplorerQFE <= 572 ? 0x278 : (nExplorerQFE <= 3693 ? 0x280 : (nExplorerQFE <= 5487 ? 0x290 : 0x280)),
			0x280));
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
		nExplorerQFE <= 1266 ? 0x9C : (nExplorerQFE <= 1320 ? 0xA0 : (nExplorerQFE <= 3271 ? 0x9C : 0xA0)),
		nExplorerQFE <= 1266 ? 0xE8 : (nExplorerQFE <= 1320 ? 0xF0 : (nExplorerQFE <= 3271 ? 0xE8 : 0xF0)),
		0xF0));
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
	return (SRWLOCK *)(lp + DO14_3264(0, 0, ,, ,, ,, 0x84, 0x100, 0xE4, 0x190, 0xC4, 0x158, 0xD4, 0x168, 0xD8, 0x178, 0xDC, 0x180, 0x12C, 0x200, 0x144, 0x218, ,,
		0x210));
}

// DEF3264 (twinui.dll -or- twinui.pcshell.dll): CApplicationViewManager::GetViews
LONG_PTR **EV_APP_VIEW_MGR_APP_ARRAY(LONG_PTR lp)
{
	return (LONG_PTR **)(lp + DO14_3264(0, 0, ,, ,, ,, 0x88, 0x108, 0xE8, 0x198, 0xC8, 0x160, 0xD8, 0x170, 0xDC, 0x180, 0xE0, 0x188, 0x130, 0x208, 0x148, 0x220, ,,
		0x218));
}

// DEF3264 (twinui.dll -or- twinui.pcshell.dll): CApplicationViewManager::GetViews
size_t *EV_APP_VIEW_MGR_APP_ARRAY_SIZE(LONG_PTR lp)
{
	return (size_t *)(lp + DO14_3264(0, 0, ,, ,, ,, 0x8C, 0x110, 0xEC, 0x1A0, 0xCC, 0x168, 0xDC, 0x178, 0xE0, 0x188, 0xE4, 0x190, 0x134, 0x210, 0x14C, 0x228, ,,
		0x220));
}
