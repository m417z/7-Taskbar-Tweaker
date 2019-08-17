#include "stdafx.h"
#include "options_ex.h"
#include "portable_settings.h"

BOOL LoadOptionsEx(int pOptionsEx[OPTS_EX_COUNT])
{
	struct {
		int nDefaultValue;
		WCHAR *pValueName;
	} sOptionsEx[OPTS_EX_COUNT] = {
		0, L"drag_towards_desktop", // has a temporary GUI deprecation code
		0, L"nocheck_minimize",
		0, L"nocheck_maximize",
		0, L"nocheck_close",
		0, L"pinned_ungrouped_animate_launch",
		0, L"sndvol_tooltip",
		0, L"tray_clock_fix_width",
		1, L"fix_hang_reposition",
		0, L"w7_tasklist_htclient",
		0, L"always_show_thumb_labels",
		0, L"scroll_reverse_cycle",
		0, L"scroll_reverse_minimize",
		0, L"multipage_wheel_scroll",
		0, L"show_desktop_button_size",
		0, L"tray_icons_padding",
		0, L"no_width_limit",
		0, L"w7_show_desktop_classic_corner",
		0, L"list_reverse_order",
		0, L"disable_topmost",
		0, L"multirow_equal_width",
		0, L"scroll_maximize_restore",
		0, L"always_show_tooltip",
		0, L"disable_taskbar_transparency",
		0, L"no_start_btn_spacing",
		0, L"right_drag_toggle_labels",
		0, L"show_desktop_on_hover",
		0, L"disable_items_drag",
		0, L"disable_tray_icons_drag",
		0, L"w10_large_icons",
		0, L"cycle_same_virtual_desktop",
		0, L"virtual_desktop_order_fix",
		0, L"scroll_no_wrap",
		0, L"show_labels",
		0, L"sndvol_classic",
	};

	PS_SECTION section;
	LSTATUS error;
	int i;

	error = PSOpenSection(L"OptionsEx", TRUE, &section);
	if(error == ERROR_SUCCESS)
	{
		for(i = 0; i < OPTS_EX_COUNT && error == ERROR_SUCCESS; i++)
		{
			error = PSGetInt(&section, sOptionsEx[i].pValueName, &pOptionsEx[i]);
			if(error == ERROR_FILE_NOT_FOUND)
			{
				pOptionsEx[i] = sOptionsEx[i].nDefaultValue;
				error = PSSetInt(&section, sOptionsEx[i].pValueName, pOptionsEx[i]);
			}
		}

		PSCloseSection(&section);
	}

	if(error != ERROR_SUCCESS)
	{
		ZeroMemory(pOptionsEx, OPTS_EX_BUFF);
		return FALSE;
	}

	// Temporary GUI deprecation code
	// {
	if(pOptionsEx[OPT_EX_DRAG_TOWARDS_DESKTOP] == 0)
	{
		error = PSOpenSection(L"Options", TRUE, &section);
		if(error == ERROR_SUCCESS)
		{
			int nValue;

			error = PSGetInt(&section, L"1365090339", &nValue);
			if(error == ERROR_SUCCESS)
			{
				if(nValue)
				{
					pOptionsEx[OPT_EX_DRAG_TOWARDS_DESKTOP] = 1;
					PSSetSingleInt(L"OptionsEx", sOptionsEx[OPT_EX_DRAG_TOWARDS_DESKTOP].pValueName, 1);
				}

				PSRemove(&section, L"1365090339");
			}
			else if(error == ERROR_FILE_NOT_FOUND)
				error = ERROR_SUCCESS;

			PSCloseSection(&section);
		}
	}
	// }

	return TRUE;
}
