#include "stdafx.h"
#include "options_dlg_handle.h"

static CTRL_INFO ctrl_info[IDC___OPTION_LAST___ - IDC___OPTION_FIRST___ + 1];
static int option_controls[OPTS_COUNT];

void InitDlg(HWND hWnd, int pOptions[OPTS_COUNT])
{
	HWND hCtrlWnd;
	int nCtrlId;
	WCHAR szClassName[16];
	DWORD dwStyle;
	RECT rcSectionGroupbox = { 0 };
	int nOption = -1;
	int nOptionValue;
	int nComboBoxCount = 0;
	UINT uComboBoxTexts[] = {
		IDS_EMPTYCOMBO_1, // OPT_EMPTYDBLCLICK
		IDS_EMPTYCOMBO_1, // OPT_EMPTYMCLICK
	};
	WCHAR *pComboBoxStr;
	int nComboBoxWidth, nComboBoxMaxWidth;
	RECT rc;
	int i;

	for(hCtrlWnd = GetWindow(hWnd, GW_CHILD); hCtrlWnd != NULL; hCtrlWnd = GetWindow(hCtrlWnd, GW_HWNDNEXT))
	{
		nCtrlId = GetDlgCtrlID(hCtrlWnd);
		GetClassName(hCtrlWnd, szClassName, 16);

		if(CompareString(LOCALE_INVARIANT, NORM_IGNORECASE,
			szClassName, -1, TEXT("BUTTON"), -1) == CSTR_EQUAL)
		{
			dwStyle = GetWindowLong(hCtrlWnd, GWL_STYLE);

			switch(dwStyle & BS_TYPEMASK)
			{
			case BS_GROUPBOX:
				GetWindowRect(hCtrlWnd, &rc);

				if(rc.left == rcSectionGroupbox.left && rc.right <= rcSectionGroupbox.right &&
					rc.top >= rcSectionGroupbox.top && rc.bottom <= rcSectionGroupbox.bottom)
				{
					rc.right = rcSectionGroupbox.right;
					rc.bottom = rcSectionGroupbox.bottom;

					MapWindowPoints(NULL, hWnd, (POINT *)&rc, 2);
					MoveWindow(hCtrlWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
				}
				else
					rcSectionGroupbox = rc;
				break;

			case BS_AUTOCHECKBOX:
				nOption++;
				option_controls[nOption] = nCtrlId;

				ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nCtrlType = CTRL_TYPE_CHECKBOX;
				ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nOption = nOption;
				ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nExtra = pOptions[nOption];

				if(pOptions[nOption])
					CheckDlgButton(hWnd, nCtrlId, BST_CHECKED);
				break;

			case BS_AUTORADIOBUTTON:
				if(dwStyle & WS_GROUP)
				{
					nOption++;
					option_controls[nOption] = nCtrlId;

					nOptionValue = 0;
				}
				else
					nOptionValue++;

				ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nCtrlType = CTRL_TYPE_RADIOBUTTON;
				ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nOption = nOption;
				ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nExtra = nOptionValue;

				if(pOptions[nOption] == nOptionValue)
					CheckDlgButton(hWnd, nCtrlId, BST_CHECKED);
				break;
			}
		}
		else if(CompareString(LOCALE_INVARIANT, NORM_IGNORECASE,
			szClassName, -1, TEXT("COMBOBOX"), -1) == CSTR_EQUAL)
		{
			nOption++;
			option_controls[nOption] = nCtrlId;

			ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nCtrlType = CTRL_TYPE_COMBOBOX;
			ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nOption = nOption;

			nComboBoxMaxWidth = 0;

			for(i = 0; i <= opts_max_values[nOption]; i++)
			{
				pComboBoxStr = LoadStrFromRsrc(uComboBoxTexts[nComboBoxCount] + i);

				nComboBoxWidth = MeasureStringWidthForCombo(hCtrlWnd, pComboBoxStr);
				if(nComboBoxWidth > nComboBoxMaxWidth)
					nComboBoxMaxWidth = nComboBoxWidth;

				SendMessage(hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)pComboBoxStr);
			}

			SendMessage(hCtrlWnd, CB_SETDROPPEDWIDTH, nComboBoxMaxWidth + GetSystemMetrics(SM_CXEDGE) * 2, 0);
			SendMessage(hCtrlWnd, CB_SETCURSEL, pOptions[nOption], 0);

			nComboBoxCount++;
		}
	}
}

int MeasureStringWidthForCombo(HWND hComboWnd, WCHAR *pString)
{
	HDC hDC;
	HFONT font, prevfont;
	SIZE textsize;
	int width;

	hDC = GetDC(hComboWnd);

	font = (HFONT)SendMessage(hComboWnd, WM_GETFONT, 0, 0);
	if(font)
		prevfont = (HFONT)SelectObject(hDC, font);

	if(GetTextExtentPoint32(hDC, pString, lstrlen(pString), &textsize))
		width = textsize.cx;
	else
		width = 0;

	if(font)
		SelectObject(hDC, prevfont);

	ReleaseDC(hComboWnd, hDC);

	return width;
}

BOOL OptionsUpdFromDlg(HWND hWnd, int nCtrlId, int nNotificationCode, int pOptions[OPTS_COUNT])
{
	int nOption;
	int nNewOptionValue;
	OPTS_STRUCT_RULES *pRules;
	int nRuleOption;
	int nRuleOptionValue;
	int nRuleCtrlId;

	switch(ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nCtrlType)
	{
	case CTRL_TYPE_CHECKBOX:
		if(nNotificationCode != BN_CLICKED)
			return FALSE;

		ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nExtra = !ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nExtra;

		nOption = ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nOption;
		nNewOptionValue = ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nExtra;
		break;

	case CTRL_TYPE_RADIOBUTTON:
		if(nNotificationCode != BN_CLICKED)
			return FALSE;

		nOption = ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nOption;
		nNewOptionValue = ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nExtra;
		break;

	case CTRL_TYPE_COMBOBOX:
		if(nNotificationCode != CBN_SELCHANGE)
			return FALSE;

		nOption = ctrl_info[nCtrlId - IDC___OPTION_FIRST___].nOption;
		nNewOptionValue = (int)SendDlgItemMessage(hWnd, nCtrlId, CB_GETCURSEL, 0, 0);
		break;

	default:
		return FALSE;
	}

	if(pOptions[nOption] == nNewOptionValue)
		return FALSE;

	pOptions[nOption] = nNewOptionValue;

	pRules = opts_dependences_rules(pOptions, nOption);
	if(pRules)
	{
		do
		{
			nRuleOption = pRules->nOptIndex;
			nRuleOptionValue = pRules->nOptValue;

			if(pOptions[nRuleOption] != nRuleOptionValue)
			{
				nRuleCtrlId = option_controls[nRuleOption];

				switch(ctrl_info[nRuleCtrlId - IDC___OPTION_FIRST___].nCtrlType)
				{
				case CTRL_TYPE_CHECKBOX:
					ctrl_info[nRuleCtrlId - IDC___OPTION_FIRST___].nExtra = nRuleOptionValue;
					if(nRuleOptionValue)
						CheckDlgButton(hWnd, nRuleCtrlId, BST_CHECKED);
					else
						CheckDlgButton(hWnd, nRuleCtrlId, BST_UNCHECKED);
					break;

				case CTRL_TYPE_RADIOBUTTON:
					CheckDlgButton(hWnd, nRuleCtrlId + pOptions[nRuleOption], BST_UNCHECKED);
					CheckDlgButton(hWnd, nRuleCtrlId + nRuleOptionValue, BST_CHECKED);
					break;

				case CTRL_TYPE_COMBOBOX:
					SendDlgItemMessage(hWnd, nRuleCtrlId, CB_SETCURSEL, nRuleOptionValue, 0);
					break;
				}

				pOptions[nRuleOption] = nRuleOptionValue;
			}

			pRules++;
		}
		while(pRules->nOptIndex != -1);
	}

	return TRUE;
}

void EnableOptions(HWND hWnd, BOOL bEnable)
{
	for(int i = IDC___OPTION_FIRST___; i <= IDC___OPTION_LAST___; i++)
	{
		HWND hOptionWnd = GetDlgItem(hWnd, i);
		EnableWindow(hOptionWnd, bEnable);
	}

	HWND hInspectorWnd = GetDlgItem(hWnd, IDC_INSPECTOR);
	EnableWindow(hInspectorWnd, bEnable);
}
