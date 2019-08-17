#include "stdafx.h"
#include "advanced_options_dlg.h"

#include "portable_settings.h"
#include "functions.h"
#include "resource.h"

#define IDC_ADV_LIST     2000

// Right click menu
#define RCMENU_NEWITEM   40001
#define RCMENU_MODIFY    40002
#define RCMENU_RENAME    40003
#define RCMENU_DELETE    40004

typedef struct {
	WCHAR *pszSectionName;
	UINT uTabNameResId;
	WCHAR *pszHelpFileSection;
} SECTION_DATA;

typedef struct {
	HWND hListViewWnd;
} TCITEM_EXTRA;

typedef struct {
	TCITEMHEADER header;
	TCITEM_EXTRA extra;
} TCITEM_CUSTOM;

typedef struct {
	UINT uNotifyMsg;
	SIZE sizeLast;
	SIZE sizeMin;
} DIALOG_PARAM;

static SECTION_DATA SectionData[] = {
	L"OptionsEx", IDS_ADVOPT_SEC_OPTIONSEX, L"OptionsEx",
	L"Mouse Button Control", IDS_ADVOPT_SEC_MOUSE_BUTTON_CONTROL, L"MouseButtonControl",
	L"Keyboard Shortcuts", IDS_ADVOPT_SEC_KEYBOARD_SHORTCUTS, L"KeyboardShortcuts",
};

#define SECTION_COUNT    (sizeof(SectionData) / sizeof(SectionData[0]))

extern WCHAR szLauncherPath[MAX_PATH];

// Label editing
static HWND g_hEditCtrlWnd;
static WNDPROC g_pOldEditCtrlProc;
static HHOOK g_hLabelEditMouseHook;

static DLGTEMPLATE *CloneDlgTemplate(HMODULE hModule, WCHAR *lpName);

// Message processing
static LRESULT CALLBACK AdvancedOptionsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL OnInitDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam);
static BOOL OnNcHitTest(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam);
static BOOL OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam);
static BOOL OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam);
static BOOL OnTabViewNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam);
static BOOL OnListViewNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam);
static BOOL OnContextMenu(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam);
static BOOL OnGetMinMaxInfo(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam);
static BOOL OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam);
static BOOL OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam);

// Functions
static BOOL InitSectionTabs(HWND hWnd);
static BOOL InitListView(HWND hListViewWnd, WCHAR *pszSectionName);
static void DestroySectionTabs(HWND hWnd);
static BOOL ListViewRightClick(HWND hWnd, HWND hListWnd, int x_pos, int y_pos);
static void NewListViewItem(HWND hListViewWnd);
static void DeleteSelectedListViewItems(HWND hListViewWnd);
static void SetListViewModified(HWND hWnd, HWND hListViewWnd);
static BOOL ApplyChanges(HWND hWnd, UINT uNotifyMsg);
static BOOL ApplyListView(HWND hListViewWnd, WCHAR *pszSectionName);
static BOOL ShowHelpSection(HWND hWnd, WCHAR *pszHelpSection);
static BOOL ShowHelpSectionOfLang(HWND hWnd, WCHAR *pszHelpSection, LANGID langid);
static UINT StringToUInt(WCHAR *pszStr);
static void TrimText(WCHAR *pszText);
static HDWP ChildRelativeDeferWindowPos(HDWP hWinPosInfo, HWND hWnd, int nIDDlgItem, int x, int y, int cx, int cy);

// Custom edit control for editing
static BOOL EditListViewData(HWND hListViewWnd, int nItemIndex);
static void EndEditListViewData(HWND hListViewWnd, BOOL bApply);
static void CleanupEditListViewData();
static LRESULT CALLBACK EditCtrlSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam);
static LRESULT CALLBACK EditLabelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

HWND CreateAdvancedOptionsDialog(HWND hParentWnd, UINT uNotifyMsg)
{
	DIALOG_PARAM *pDialogParam = HeapAlloc(GetProcessHeap(), 0, sizeof(DIALOG_PARAM));
	if(!pDialogParam)
		return NULL;

	pDialogParam->uNotifyMsg = uNotifyMsg;

	HWND hDialog = NULL;

	if(GetWindowLong(hParentWnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
	{
		DLGTEMPLATE *pDlgTemplateClone = CloneDlgTemplate(NULL, MAKEINTRESOURCE(IDD_ADVANCED));
		if(pDlgTemplateClone)
		{
			((DWORD *)pDlgTemplateClone)[2] |= WS_EX_LAYOUTRTL; // exStyle of DLGTEMPLATEEX
			//((DWORD *)pDlgTemplateClone)[3] |= 0; // style of DLGTEMPLATEEX

			hDialog = CreateDialogIndirectParam(GetModuleHandle(NULL), pDlgTemplateClone, hParentWnd, (DLGPROC)AdvancedOptionsDlgProc, (LPARAM)pDialogParam);

			HeapFree(GetProcessHeap(), 0, pDlgTemplateClone);
		}
	}
	else
		hDialog = CreateDialogParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADVANCED), hParentWnd, (DLGPROC)AdvancedOptionsDlgProc, (LPARAM)pDialogParam);

	if(!hDialog)
	{
		HeapFree(GetProcessHeap(), 0, pDialogParam);
		return NULL;
	}

	ShowWindow(hDialog, SW_SHOWNORMAL);
	return hDialog;
}

static DLGTEMPLATE *CloneDlgTemplate(HMODULE hModule, WCHAR *lpName)
{
	HRSRC hDialogRC = FindResource(hModule, lpName, RT_DIALOG);
	if(hDialogRC)
	{
		HGLOBAL hDlgTemplate = LoadResource(hModule, hDialogRC);
		if(hDlgTemplate)
		{
			DLGTEMPLATE *pDlgTemplate = (DLGTEMPLATE *)LockResource(hDlgTemplate);
			if(pDlgTemplate)
			{
				DWORD dwRsrcSize = SizeofResource(hModule, hDialogRC);
				DLGTEMPLATE *pDlgTemplateClone = (DLGTEMPLATE *)HeapAlloc(GetProcessHeap(), 0, dwRsrcSize);
				if(pDlgTemplateClone)
				{
					CopyMemory(pDlgTemplateClone, pDlgTemplate, dwRsrcSize);

					return pDlgTemplateClone;
				}
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// Message processing

static LRESULT CALLBACK AdvancedOptionsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DIALOG_PARAM *pDialogParam;

	if(uMsg == WM_INITDIALOG)
	{
		pDialogParam = (DIALOG_PARAM *)lParam;
		SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR)pDialogParam);
	}
	else
	{
		pDialogParam = (DIALOG_PARAM *)GetWindowLongPtr(hWnd, DWLP_USER);

		if(uMsg == WM_NCDESTROY && pDialogParam)
		{
			HeapFree(GetProcessHeap(), 0, pDialogParam);
			pDialogParam = NULL;
			SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR)pDialogParam);
		}

		if(!pDialogParam)
			return FALSE;
	}

	switch(uMsg)
	{
	case WM_INITDIALOG:
		return OnInitDialog(hWnd, uMsg, wParam, lParam, pDialogParam);

	case WM_NCHITTEST:
		return OnNcHitTest(hWnd, uMsg, wParam, lParam, pDialogParam);

	case WM_COMMAND:
		return OnCommand(hWnd, uMsg, wParam, lParam, pDialogParam);

	case WM_NOTIFY:
		return OnNotify(hWnd, uMsg, wParam, lParam, pDialogParam);

	case WM_CONTEXTMENU:
		return OnContextMenu(hWnd, uMsg, wParam, lParam, pDialogParam);

	case WM_GETMINMAXINFO:
		return OnGetMinMaxInfo(hWnd, uMsg, wParam, lParam, pDialogParam);

	case WM_SIZE:
		return OnSize(hWnd, uMsg, wParam, lParam, pDialogParam);

	case WM_DESTROY:
		return OnDestroy(hWnd, uMsg, wParam, lParam, pDialogParam);
	}

	return FALSE;
}

static BOOL OnInitDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam)
{
	HICON hIcon;
	RECT rc;
	long cur_w, cur_h;
	long min_w, min_h;

	hIcon = (HICON)SendMessage(GetParent(hWnd), WM_GETICON, ICON_SMALL, 0);
	if(hIcon)
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	hIcon = (HICON)SendMessage(GetParent(hWnd), WM_GETICON, ICON_BIG, 0);
	if(hIcon)
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

	SetWindowText(hWnd, LoadStrFromRsrc(IDS_ADVOPT_TITLE));
	SetDlgItemText(hWnd, IDHELP, LoadStrFromRsrc(IDS_ADVOPT_HELP));
	SetDlgItemText(hWnd, IDOK, LoadStrFromRsrc(IDS_ADVOPT_OK));
	SetDlgItemText(hWnd, IDCANCEL, LoadStrFromRsrc(IDS_ADVOPT_CANCEL));
	SetDlgItemText(hWnd, IDC_ADV_APPLY, LoadStrFromRsrc(IDS_ADVOPT_APPLY));

	if(!InitSectionTabs(hWnd))
	{
		MessageBox(hWnd, L"Initialization failed", NULL, MB_ICONERROR);
		DestroyWindow(hWnd);
		return FALSE;
	}

	// Last size
	GetClientRect(hWnd, &rc);
	pDialogParam->sizeLast.cx = rc.right - rc.left;
	pDialogParam->sizeLast.cy = rc.bottom - rc.top;

	// Minimum size
	GetWindowRect(hWnd, &rc);
	cur_w = rc.right - rc.left;
	cur_h = rc.bottom - rc.top;

	GetWindowRect(GetDlgItem(hWnd, IDHELP), &rc);
	min_w = cur_w + rc.right;

	GetWindowRect(GetDlgItem(hWnd, IDOK), &rc);
	min_w -= rc.left;

	GetWindowRect(GetDlgItem(hWnd, IDC_ADV_LIST), &rc);
	min_h = cur_h - (rc.bottom - rc.top);

	pDialogParam->sizeMin.cx = min_w;
	pDialogParam->sizeMin.cy = min_h;

	// Disable apply button
	EnableWindow(GetDlgItem(hWnd, IDC_ADV_APPLY), FALSE);

	return FALSE;
}

static BOOL OnNcHitTest(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam)
{
	LRESULT result = DefWindowProc(hWnd, uMsg, wParam, lParam);
	if(result == HTCLIENT)
		result = HTCAPTION;

	SetWindowLongPtr(hWnd, DWLP_MSGRESULT, result);
	return TRUE;
}

static BOOL OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam)
{
	switch(LOWORD(wParam))
	{
	case IDC_ADV_APPLY:
		if(!ApplyChanges(hWnd, pDialogParam->uNotifyMsg))
			MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SAVECONF), NULL, MB_ICONERROR);
		break;

	case IDOK:
		// The check fixes a listview bug, Q130692
		if((HWND)lParam == GetDlgItem(hWnd, IDOK))
		{
			if(IsWindowEnabled(GetDlgItem(hWnd, IDC_ADV_APPLY)))
			{
				if(!ApplyChanges(hWnd, pDialogParam->uNotifyMsg))
				{
					MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SAVECONF), NULL, MB_ICONERROR);
					break;
				}
			}

			DestroyWindow(hWnd);
		}
		break;

	case IDCANCEL:
		// The check fixes a listview bug, Q130692
		if((HWND)lParam == GetDlgItem(hWnd, IDCANCEL))
		{
			if(IsWindowEnabled(GetDlgItem(hWnd, IDC_ADV_APPLY)))
			{
				if(MessageBox(hWnd, LoadStrFromRsrc(IDS_ADVOPT_EXIT_CONFIRM),
					LoadStrFromRsrc(IDS_ADVOPT_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES)
				{
					break;
				}
			}

			DestroyWindow(hWnd);
		}
		break;

	case IDHELP:
		// The check fixes a listview bug, Q130692
		if((HWND)lParam == GetDlgItem(hWnd, IDHELP))
		{
			int i = TabCtrl_GetCurSel(GetDlgItem(hWnd, IDC_ADV_TAB));
			if(!ShowHelpSection(hWnd, SectionData[i].pszHelpFileSection))
				MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SHOWHELP), NULL, MB_ICONERROR);
		}
		break;
	}

	return FALSE;
}

static BOOL OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam)
{
	NMHDR *phdr = ((NMHDR *)lParam);

	if(phdr->idFrom == IDC_ADV_TAB)
	{
		return OnTabViewNotify(hWnd, uMsg, wParam, lParam, pDialogParam);
	}
	else if(phdr->idFrom >= IDC_ADV_LIST && phdr->idFrom < IDC_ADV_LIST + SECTION_COUNT)
	{
		return OnListViewNotify(hWnd, uMsg, wParam, lParam, pDialogParam);
	}

	return FALSE;
}

static BOOL OnTabViewNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam)
{
	NMHDR *phdr = ((NMHDR *)lParam);
	int nIndex;
	TCITEM_CUSTOM tcitem;

	switch(phdr->code)
	{
	case TCN_SELCHANGING:
	case TCN_SELCHANGE:
		nIndex = TabCtrl_GetCurSel(GetDlgItem(hWnd, IDC_ADV_TAB));

		tcitem.header.mask = TCIF_PARAM;

		TabCtrl_GetItem(GetDlgItem(hWnd, IDC_ADV_TAB), nIndex, &tcitem);

		if(phdr->code == TCN_SELCHANGING)
		{
			ShowWindow(tcitem.extra.hListViewWnd, SW_HIDE);
		}
		else
		{
			ShowWindow(tcitem.extra.hListViewWnd, SW_SHOW);
			if(GetFocus() != phdr->hwndFrom)
			{
				SendMessage(hWnd, WM_NEXTDLGCTL, (WPARAM)tcitem.extra.hListViewWnd, TRUE);
			}
		}
		break;
	}

	return FALSE;
}

static BOOL OnListViewNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam)
{
	NMHDR *phdr = ((NMHDR *)lParam);

	switch(phdr->code)
	{
	case NM_DBLCLK:
		if(ListView_GetSelectedCount(phdr->hwndFrom) == 1 && 
			((NMITEMACTIVATE *)lParam)->iItem == ListView_GetNextItem(phdr->hwndFrom, -1, LVNI_SELECTED))
		{
			switch(((NMITEMACTIVATE *)lParam)->iSubItem)
			{
			case 0:
				ListView_EditLabel(phdr->hwndFrom, ((NMITEMACTIVATE *)lParam)->iItem);
				break;

			case 1:
				EditListViewData(phdr->hwndFrom, ((NMITEMACTIVATE *)lParam)->iItem);
				break;
			}
		}
		break;

	case LVN_KEYDOWN:
		switch(((NMLVKEYDOWN *)lParam)->wVKey)
		{
		case 'A':
			if(GetKeyState(VK_CONTROL) < 0) // Ctrl+A - Select all
				ListView_SetItemState(phdr->hwndFrom, -1, LVIS_SELECTED, LVIS_SELECTED);
			break;

		case 'N':
			if(GetKeyState(VK_CONTROL) < 0) // Ctrl+N - New item
			{
				NewListViewItem(phdr->hwndFrom);
				SetListViewModified(hWnd, phdr->hwndFrom);
			}
			break;

		case VK_F2:
			if(ListView_GetSelectedCount(phdr->hwndFrom) == 1)
			{
				ListView_EditLabel(phdr->hwndFrom, ListView_GetNextItem(phdr->hwndFrom, -1, LVNI_SELECTED));
			}
			break;

		case VK_DELETE:
			DeleteSelectedListViewItems(phdr->hwndFrom);
			SetListViewModified(hWnd, phdr->hwndFrom);
			break;
		}
		break;

	case LVN_BEGINLABELEDIT:
		SetWindowLongPtr(hWnd, DWLP_MSGRESULT, FALSE);
		return TRUE;

	case LVN_ENDLABELEDIT:
		if(((NMLVDISPINFO *)lParam)->item.pszText)
		{
			LVFINDINFO lvfindinfo;
			int nFoundIndex;

			TrimText(((NMLVDISPINFO *)lParam)->item.pszText);

			if(*((NMLVDISPINFO *)lParam)->item.pszText == L'\0')
			{
				// Ignore empty names
				SetWindowLongPtr(hWnd, DWLP_MSGRESULT, FALSE);
				return TRUE;
			}

			lvfindinfo.flags = LVFI_STRING;
			lvfindinfo.psz = ((NMLVDISPINFO *)lParam)->item.pszText;

			nFoundIndex = ListView_FindItem(phdr->hwndFrom, -1, &lvfindinfo);
			if(nFoundIndex != -1)
			{
				if(nFoundIndex != ListView_GetNextItem(phdr->hwndFrom, -1, LVNI_SELECTED))
					MessageBox(hWnd, LoadStrFromRsrc(IDS_ADVOPT_ERROR_NAME_EXISTS), NULL, MB_ICONERROR);

				SetWindowLongPtr(hWnd, DWLP_MSGRESULT, FALSE);
				return TRUE;
			}

			SetListViewModified(hWnd, phdr->hwndFrom);
		}

		SetWindowLongPtr(hWnd, DWLP_MSGRESULT, TRUE);
		return TRUE;

	case LVN_BEGINSCROLL:
		EndEditListViewData(phdr->hwndFrom, FALSE);
		break;
	}

	return FALSE;
}

static BOOL OnContextMenu(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam)
{
	if((HWND)wParam == hWnd)
		return FALSE;

	int nCtrlId = GetDlgCtrlID((HWND)wParam);

	if(nCtrlId >= IDC_ADV_LIST && nCtrlId < IDC_ADV_LIST + SECTION_COUNT)
	{
		ListViewRightClick(hWnd, (HWND)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}

	return FALSE;
}

static BOOL OnGetMinMaxInfo(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam)
{
	((MINMAXINFO *)lParam)->ptMinTrackSize.x = pDialogParam->sizeMin.cx;
	((MINMAXINFO *)lParam)->ptMinTrackSize.y = pDialogParam->sizeMin.cy;

	return FALSE;
}

static BOOL OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam)
{
	if(wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED)
	{
		HDWP hDwp;
		SIZE size;
		int i;

		size.cx = LOWORD(lParam);
		size.cy = HIWORD(lParam);

		hDwp = BeginDeferWindowPos(5 + SECTION_COUNT);

		for(i = 0; i < SECTION_COUNT; i++)
		{
			if(hDwp)
				hDwp = ChildRelativeDeferWindowPos(hDwp, hWnd, IDC_ADV_LIST + i, 0, 0, size.cx - pDialogParam->sizeLast.cx, size.cy - pDialogParam->sizeLast.cy);
		}

		if(hDwp)
			hDwp = ChildRelativeDeferWindowPos(hDwp, hWnd, IDC_ADV_TAB, 0, 0, size.cx - pDialogParam->sizeLast.cx, size.cy - pDialogParam->sizeLast.cy);

		if(hDwp)
			hDwp = ChildRelativeDeferWindowPos(hDwp, hWnd, IDHELP, 0, size.cy - pDialogParam->sizeLast.cy, 0, 0);

		if(hDwp)
			hDwp = ChildRelativeDeferWindowPos(hDwp, hWnd, IDOK, size.cx - pDialogParam->sizeLast.cx, size.cy - pDialogParam->sizeLast.cy, 0, 0);

		if(hDwp)
			hDwp = ChildRelativeDeferWindowPos(hDwp, hWnd, IDCANCEL, size.cx - pDialogParam->sizeLast.cx, size.cy - pDialogParam->sizeLast.cy, 0, 0);

		if(hDwp)
			hDwp = ChildRelativeDeferWindowPos(hDwp, hWnd, IDC_ADV_APPLY, size.cx - pDialogParam->sizeLast.cx, size.cy - pDialogParam->sizeLast.cy, 0, 0);

		if(hDwp)
			EndDeferWindowPos(hDwp);

		pDialogParam->sizeLast.cx = size.cx;
		pDialogParam->sizeLast.cy = size.cy;

		for(i = 0; i < SECTION_COUNT; i++)
		{
			HWND hListViewWnd;
			RECT rcListView;
			LVCOLUMN lvcolumn;

			hListViewWnd = GetDlgItem(hWnd, IDC_ADV_LIST + i);
			GetClientRect(hListViewWnd, &rcListView);

			if(!(GetWindowLong(hListViewWnd, GWL_STYLE) & WS_VSCROLL))
				rcListView.right -= GetSystemMetrics(SM_CXVSCROLL);

			lvcolumn.mask = LVCF_WIDTH;
			lvcolumn.cx = rcListView.right - rcListView.left - (20 + 6 * 10);
			ListView_SetColumn(hListViewWnd, 0, &lvcolumn);

			lvcolumn.mask = LVCF_WIDTH;
			lvcolumn.cx = (20 + 6 * 10);
			ListView_SetColumn(hListViewWnd, 1, &lvcolumn);
		}
	}

	return FALSE;
}

static BOOL OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam)
{
	CleanupEditListViewData();
	DestroySectionTabs(hWnd);

	SendMessage(GetParent(hWnd), pDialogParam->uNotifyMsg, ADV_OPTS_DESTROYED, 0);
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// Functions

static BOOL InitSectionTabs(HWND hWnd)
{
	RECT rcTabDisplay;
	int i;

	TabCtrl_SetItemExtra(GetDlgItem(hWnd, IDC_ADV_TAB), sizeof(TCITEM_EXTRA));

	for(i = 0; i < SECTION_COUNT; i++)
	{
		TCITEM tcitem;

		tcitem.mask = TCIF_TEXT;
		tcitem.pszText = LoadStrFromRsrc(SectionData[i].uTabNameResId);

		TabCtrl_InsertItem(GetDlgItem(hWnd, IDC_ADV_TAB), i, &tcitem);
	}

	GetWindowRect(GetDlgItem(hWnd, IDC_ADV_TAB), &rcTabDisplay);
	MapWindowPoints(NULL, hWnd, (POINT *)&rcTabDisplay, sizeof(RECT) / sizeof(POINT));

	TabCtrl_AdjustRect(GetDlgItem(hWnd, IDC_ADV_TAB), FALSE, &rcTabDisplay);

	// TabCtrl_AdjustRect sucks
	// http://www.progtown.com/topic923181-how-precisely-to-place-dialogue-in-tab-tab-control.html
	rcTabDisplay.left -= 3;
	rcTabDisplay.top -= 1;
	rcTabDisplay.right += 1;
	rcTabDisplay.bottom += 2;

	for(i = 0; i < SECTION_COUNT; i++)
	{
		DWORD dwStyle;
		HWND hListViewWnd;
		TCITEM_CUSTOM tcitem;

		dwStyle = WS_CHILD | WS_TABSTOP | LVS_REPORT | LVS_SORTASCENDING | LVS_EDITLABELS;
		if(i == 0)
			dwStyle |= WS_VISIBLE;

		hListViewWnd = CreateWindow(WC_LISTVIEW, L"", dwStyle,
			rcTabDisplay.left, rcTabDisplay.top,
			rcTabDisplay.right - rcTabDisplay.left, rcTabDisplay.bottom - rcTabDisplay.top,
			hWnd, (HMENU)(UINT_PTR)(IDC_ADV_LIST + i), GetModuleHandle(NULL), NULL);
		if(!hListViewWnd)
		{
			break;
		}

		SetWindowPos(hListViewWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

		if(!InitListView(hListViewWnd, SectionData[i].pszSectionName))
		{
			DestroyWindow(hListViewWnd);
			break;
		}

		dwStyle &= ~LVS_SORTASCENDING;
		SetWindowLong(hListViewWnd, GWL_STYLE, dwStyle);

		tcitem.header.mask = TCIF_PARAM;

		tcitem.extra.hListViewWnd = hListViewWnd;

		TabCtrl_SetItem(GetDlgItem(hWnd, IDC_ADV_TAB), i, &tcitem);
	}

	if(i < SECTION_COUNT)
	{
		int j;

		for(j = 0; j < i; j++)
		{
			TCITEM_CUSTOM tcitem;

			tcitem.header.mask = TCIF_PARAM;

			TabCtrl_GetItem(GetDlgItem(hWnd, IDC_ADV_TAB), j, &tcitem);

			DestroyWindow(tcitem.extra.hListViewWnd);
		}

		return FALSE;
	}

	return TRUE;
}

static BOOL InitListView(HWND hListViewWnd, WCHAR *pszSectionName)
{
	RECT rcListView;
	LVCOLUMN lvcolumn;
	PS_SECTION section;
	LSTATUS error;

	ListView_SetExtendedListViewStyle(hListViewWnd, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER);
	SetWindowTheme(hListViewWnd, L"Explorer", NULL);

	GetClientRect(hListViewWnd, &rcListView);

	if(!(GetWindowLong(hListViewWnd, GWL_STYLE) & WS_VSCROLL))
		rcListView.right -= GetSystemMetrics(SM_CXVSCROLL);

	lvcolumn.mask = LVCF_TEXT | LVCF_WIDTH;
	lvcolumn.pszText = LoadStrFromRsrc(IDS_ADVOPT_LIST_NAME);
	lvcolumn.cx = rcListView.right - rcListView.left - (20 + 6 * 10);
	ListView_InsertColumn(hListViewWnd, 0, &lvcolumn);

	lvcolumn.mask = LVCF_TEXT | LVCF_WIDTH;
	lvcolumn.pszText = LoadStrFromRsrc(IDS_ADVOPT_LIST_DATA);
	lvcolumn.cx = (20 + 6 * 10);
	ListView_InsertColumn(hListViewWnd, 1, &lvcolumn);

	error = PSOpenSection(pszSectionName, FALSE, &section);
	if(error == ERROR_SUCCESS)
	{
		PS_FIND find;

		error = PSFindInit(&section, &find);
		if(error == ERROR_SUCCESS)
		{
			do
			{
				WCHAR szValueName[MAX_PATH];
				UINT uValueNameSize = sizeof(szValueName) / sizeof(WCHAR);
				int nValue;

				error = PSFindNextInt(&section, &find, szValueName, &uValueNameSize, &nValue);
				if(error == ERROR_SUCCESS)
				{
					LVITEM lvitem;
					int nIndex;
					WCHAR szValueText[sizeof("4294967295")];

					lvitem.mask = LVIF_TEXT;
					lvitem.iItem = 0;
					lvitem.iSubItem = 0;
					lvitem.pszText = szValueName;
					nIndex = ListView_InsertItem(hListViewWnd, &lvitem);

					wsprintf(szValueText, L"%u", (unsigned int)nValue);
					ListView_SetItemText(hListViewWnd, nIndex, 1, szValueText);
				}
				else if(error == ERROR_MORE_DATA)
					error = PSFindSkip(&section, &find);
			}
			while(error == ERROR_SUCCESS);

			if(error == ERROR_NO_MORE_ITEMS)
				error = ERROR_SUCCESS;

			PSFindClose(&section, &find);
		}

		PSCloseSection(&section);
	}

	return error == ERROR_SUCCESS;
}

static void DestroySectionTabs(HWND hWnd)
{
	int i;

	for(i = 0; i < SECTION_COUNT; i++)
	{
		TCITEM_CUSTOM tcitem;

		tcitem.header.mask = TCIF_PARAM;

		TabCtrl_GetItem(GetDlgItem(hWnd, IDC_ADV_TAB), i, &tcitem);

		DestroyWindow(tcitem.extra.hListViewWnd);
	}
}

static BOOL ListViewRightClick(HWND hWnd, HWND hListViewWnd, int x_pos, int y_pos)
{
	HMENU hMenu;
	UINT uSelectedCount;
	BOOL bMenuResult;
	int nIndex;

	uSelectedCount = ListView_GetSelectedCount(hListViewWnd);

	// Calc x, y if using keyboard
	if(x_pos == -1 && y_pos == -1)
	{
		POINT pt;
		RECT rc;

		if(ListView_GetItemCount(hListViewWnd) > 0)
		{
			nIndex = ListView_GetTopIndex(hListViewWnd);

			if(uSelectedCount > 0)
				nIndex = ListView_GetNextItem(hListViewWnd, nIndex - 1, LVNI_SELECTED);
			else
				nIndex = ListView_GetNextItem(hListViewWnd, nIndex - 1, LVNI_FOCUSED);

			if(nIndex == -1 || !ListView_IsItemVisible(hListViewWnd, nIndex))
				nIndex = ListView_GetTopIndex(hListViewWnd);

			ListView_GetItemRect(hListViewWnd, nIndex, &rc, LVIR_ICON);
			pt.x = rc.right;
			pt.y = rc.bottom;
		}
		else
		{
			GetClientRect(ListView_GetHeader(hListViewWnd), &rc);
			pt.x = rc.left;
			pt.y = rc.bottom;
		}

		ClientToScreen(hListViewWnd, &pt);
		x_pos = pt.x;
		y_pos = pt.y;
	}

	hMenu = CreatePopupMenu();

	if(uSelectedCount == 1)
	{
		AppendMenu(hMenu, MF_STRING, RCMENU_MODIFY, LoadStrFromRsrc(IDS_ADVOPT_RCMENU_MODIFY));
		AppendMenu(hMenu, MF_STRING, RCMENU_RENAME, LoadStrFromRsrc(IDS_ADVOPT_RCMENU_RENAME));
	}

	if(uSelectedCount > 0)
	{
		AppendMenu(hMenu, MF_STRING, RCMENU_DELETE, LoadStrFromRsrc(IDS_ADVOPT_RCMENU_DELETE));
		AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	}

	AppendMenu(hMenu, MF_STRING, RCMENU_NEWITEM, LoadStrFromRsrc(IDS_ADVOPT_RCMENU_NEW));

	bMenuResult = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, x_pos, y_pos, 0, hWnd, NULL);

	DestroyMenu(hMenu);

	switch(bMenuResult)
	{
	case RCMENU_NEWITEM:
		NewListViewItem(hListViewWnd);
		SetListViewModified(hWnd, hListViewWnd);
		break;

	case RCMENU_MODIFY:
		nIndex = ListView_GetNextItem(hListViewWnd, -1, LVNI_SELECTED);
		EditListViewData(hListViewWnd, nIndex);
		break;

	case RCMENU_RENAME:
		nIndex = ListView_GetNextItem(hListViewWnd, -1, LVNI_SELECTED);
		ListView_EditLabel(hListViewWnd, nIndex);
		break;

	case RCMENU_DELETE:
		DeleteSelectedListViewItems(hListViewWnd);
		SetListViewModified(hWnd, hListViewWnd);
		break;
	}

	return TRUE;
}

static void NewListViewItem(HWND hListViewWnd)
{
	WCHAR szNewItemName[sizeof("New Item #4294967295")];
	UINT nCounter;
	LVFINDINFO lvfindinfo;
	LVITEM lvitem;
	int nIndex;

	nCounter = 0;

	lvfindinfo.flags = LVFI_STRING;
	lvfindinfo.psz = szNewItemName;

	do
	{
		nCounter++;
		wsprintf(szNewItemName, L"New Item #%u", nCounter);
	}
	while(ListView_FindItem(hListViewWnd, -1, &lvfindinfo) != -1);

	lvitem.mask = LVIF_TEXT;
	lvitem.iItem = ListView_GetItemCount(hListViewWnd);
	lvitem.iSubItem = 0;
	lvitem.pszText = szNewItemName;
	nIndex = ListView_InsertItem(hListViewWnd, &lvitem);

	ListView_SetItemText(hListViewWnd, nIndex, 1, L"0");

	ListView_SetItemState(hListViewWnd, -1, 0, LVIS_SELECTED);
	ListView_SetItemState(hListViewWnd, nIndex, LVIS_SELECTED, LVIS_SELECTED);
	ListView_EditLabel(hListViewWnd, nIndex);
}

static void DeleteSelectedListViewItems(HWND hListViewWnd)
{
	SendMessage(hListViewWnd, WM_SETREDRAW, FALSE, 0);

	int nIndex = ListView_GetNextItem(hListViewWnd, -1, LVNI_SELECTED);
	while(nIndex != -1)
	{
		ListView_DeleteItem(hListViewWnd, nIndex);
		nIndex = ListView_GetNextItem(hListViewWnd, nIndex - 1, LVNI_SELECTED);
	}

	SendMessage(hListViewWnd, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(hListViewWnd, NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}

static void SetListViewModified(HWND hWnd, HWND hListViewWnd)
{
	if(GetWindowLongPtr(hListViewWnd, GWLP_USERDATA) == FALSE)
	{
		SetWindowLongPtr(hListViewWnd, GWLP_USERDATA, TRUE);
		EnableWindow(GetDlgItem(hWnd, IDC_ADV_APPLY), TRUE);
	}
}

static BOOL ApplyChanges(HWND hWnd, UINT uNotifyMsg)
{
	TCITEM_CUSTOM tcitem;

	tcitem.header.mask = TCIF_PARAM;

	for(int i = 0; i < SECTION_COUNT; i++)
	{
		TabCtrl_GetItem(GetDlgItem(hWnd, IDC_ADV_TAB), i, &tcitem);

		if(GetWindowLongPtr(tcitem.extra.hListViewWnd, GWLP_USERDATA) != FALSE)
		{
			if(!ApplyListView(tcitem.extra.hListViewWnd, SectionData[i].pszSectionName))
				return FALSE;
		}
	}

	for(int i = 0; i < SECTION_COUNT; i++)
	{
		TabCtrl_GetItem(GetDlgItem(hWnd, IDC_ADV_TAB), i, &tcitem);

		if(GetWindowLongPtr(tcitem.extra.hListViewWnd, GWLP_USERDATA) != FALSE)
		{
			SendMessage(GetParent(hWnd), uNotifyMsg, i, 0);

			SetWindowLongPtr(tcitem.extra.hListViewWnd, GWLP_USERDATA, FALSE);
		}
	}

	if(GetFocus() == GetDlgItem(hWnd, IDC_ADV_APPLY))
	{
		TabCtrl_GetItem(GetDlgItem(hWnd, IDC_ADV_TAB), 
			TabCtrl_GetCurSel(GetDlgItem(hWnd, IDC_ADV_TAB)), &tcitem);

		SendMessage(hWnd, WM_NEXTDLGCTL, (WPARAM)tcitem.extra.hListViewWnd, TRUE);
	}

	EnableWindow(GetDlgItem(hWnd, IDC_ADV_APPLY), FALSE);
	return TRUE;
}

static BOOL ApplyListView(HWND hListViewWnd, WCHAR *pszSectionName)
{
	PS_SECTION section;
	LSTATUS error;

	error = PSRemoveSection(pszSectionName);
	if(error != ERROR_SUCCESS)
		return FALSE;

	error = PSOpenSection(pszSectionName, TRUE, &section);
	if(error == ERROR_SUCCESS)
	{
		WCHAR szName[MAX_PATH];
		WCHAR szValue[sizeof("4294967295")];
		int nItemCount;
		int i;

		nItemCount = ListView_GetItemCount(hListViewWnd);

		for(i = 0; i < nItemCount; i++)
		{
			int nValue;

			ListView_GetItemText(hListViewWnd, i, 0, szName, MAX_PATH);
			ListView_GetItemText(hListViewWnd, i, 1, szValue, sizeof("4294967295"));

			nValue = StringToUInt(szValue);

			PSSetInt(&section, szName, nValue);
		}

		PSCloseSection(&section);
	}

	return error == ERROR_SUCCESS;
}

static BOOL ShowHelpSection(HWND hWnd, WCHAR *pszHelpSection)
{
	LANGID langid;

	langid = GetThreadUILanguage();

	if(ShowHelpSectionOfLang(hWnd, pszHelpSection, langid))
		return TRUE;

	if(langid == MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
		return FALSE;

	return ShowHelpSectionOfLang(hWnd, pszHelpSection, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
}

static BOOL ShowHelpSectionOfLang(HWND hWnd, WCHAR *pszHelpSection, LANGID langid)
{
	WCHAR szLocaleName[LOCALE_NAME_MAX_LENGTH];
	int nLocaleNameLen;
	WCHAR szCommandLine[MAX_PATH*2];
	int nCommandLineLen;
	WCHAR *pszFilePath;
	int nFilePathLen;
	DWORD dwFileAttributes;

	nLocaleNameLen = LCIDToLocaleName(MAKELCID(langid, SORT_DEFAULT), szLocaleName, LOCALE_NAME_MAX_LENGTH, 0);
	if(nLocaleNameLen == 0)
		return FALSE;

	szCommandLine[0] = L'"';
	pszFilePath = szCommandLine + 1;

	lstrcpy(pszFilePath, szLauncherPath);
	nFilePathLen = lstrlen(pszFilePath);

	do
	{
		nFilePathLen--;

		if(nFilePathLen < 0)
			return FALSE;
	}
	while(pszFilePath[nFilePathLen] != L'\\');

	nFilePathLen++;
	pszFilePath[nFilePathLen] = L'\0';

	nFilePathLen += (sizeof("help\\") - 1) + nLocaleNameLen + (sizeof(".chm") - 1);
	if(nFilePathLen > MAX_PATH - 1)
		return FALSE;

	lstrcat(pszFilePath, L"help\\");
	lstrcat(pszFilePath, szLocaleName);
	lstrcat(pszFilePath, L".chm");

	dwFileAttributes = GetFileAttributes(pszFilePath);
	if(dwFileAttributes == INVALID_FILE_ATTRIBUTES || (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return FALSE;

	nCommandLineLen = 1 + nFilePathLen + (sizeof("::/") - 1) + lstrlen(pszHelpSection) + (sizeof(".htm\"") - 1);
	if(nCommandLineLen > MAX_PATH * 2 - 1)
		return FALSE;

	lstrcat(szCommandLine, L"::/");
	lstrcat(szCommandLine, pszHelpSection);
	lstrcat(szCommandLine, L".htm\"");

	return !((int)(UINT_PTR)ShellExecute(hWnd, NULL, L"hh.exe", szCommandLine, NULL, SW_SHOWNORMAL) <= 32);
}

static UINT StringToUInt(WCHAR *pszStr)
{
	int nInt = 0;

	while(*pszStr >= L'0' && *pszStr <= L'9')
	{
		nInt *= 10;
		nInt += *pszStr - L'0';
		pszStr++;
	}

	return nInt;
}

static void TrimText(WCHAR *pszText)
{
	WCHAR *pStart, *pEnd;
	size_t nStringLen;
	size_t nTrimmedLen;

	pStart = pszText;
	while(*pStart == L' ' || *pStart == L'\t')
		pStart++;

	if(*pStart == L'\0')
	{
		*pszText = L'\0';
		return;
	}

	nStringLen = lstrlen(pszText);

	pEnd = pszText + nStringLen;
	do
	{
		pEnd--;
	}
	while(pEnd > pStart && (*pEnd == L' ' || *pEnd == L'\t'));

	nTrimmedLen = pEnd - pStart + 1;

	if(nTrimmedLen != nStringLen)
	{
		if(pStart != pszText)
			MoveMemory(pszText, pStart, nTrimmedLen*sizeof(WCHAR));

		pszText[nTrimmedLen] = L'\0';
	}
}

static HDWP ChildRelativeDeferWindowPos(HDWP hWinPosInfo, HWND hWnd, int nIDDlgItem, int x, int y, int cx, int cy)
{
	HWND hChildWnd;
	RECT rc;

	hChildWnd = GetDlgItem(hWnd, nIDDlgItem);

	GetWindowRect(hChildWnd, &rc);
	MapWindowPoints(NULL, hWnd, (POINT *)&rc, 2);

	return DeferWindowPos(hWinPosInfo, hChildWnd, NULL,
		rc.left + x, rc.top + y, (rc.right - rc.left) + cx, (rc.bottom - rc.top) + cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

//////////////////////////////////////////////////////////////////////////
// Custom edit control for editing

static BOOL EditListViewData(HWND hListViewWnd, int nItemIndex)
{
	HWND hEditCtrlWnd;
	WCHAR szDataText[sizeof("4294967295")];
	RECT rc;

	if(g_hEditCtrlWnd)
		return FALSE;

	ListView_GetItemText(hListViewWnd, nItemIndex, 1, szDataText, sizeof(szDataText) / sizeof(WCHAR));

	ListView_GetSubItemRect(hListViewWnd, nItemIndex, 1, LVIR_BOUNDS, &rc);

	hEditCtrlWnd = CreateWindow(L"edit", szDataText, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hListViewWnd, NULL, NULL, NULL);
	if(!hEditCtrlWnd)
		return FALSE;

	SendMessage(hEditCtrlWnd, WM_SETFONT, (WPARAM)SendMessage(hListViewWnd, WM_GETFONT, 0, 0), FALSE);

	g_pOldEditCtrlProc = (WNDPROC)SetWindowLongPtr(hEditCtrlWnd, GWLP_WNDPROC, (LONG_PTR)EditCtrlSubclassProc);
	if(!g_pOldEditCtrlProc)
	{
		DestroyWindow(hEditCtrlWnd);
		return FALSE;
	}

	g_hLabelEditMouseHook = SetWindowsHookEx(WH_MOUSE, EditLabelMouseProc, NULL, GetCurrentThreadId());
	if(!g_hLabelEditMouseHook)
	{
		DestroyWindow(hEditCtrlWnd);
		return FALSE;
	}

	SendMessage(hEditCtrlWnd, EM_SETLIMITTEXT, sizeof("4294967295")-1, 0);

	SetFocus(hEditCtrlWnd);
	SendMessage(hEditCtrlWnd, EM_SETSEL, 0, -1);

	g_hEditCtrlWnd = hEditCtrlWnd;
	return TRUE;
}

static void EndEditListViewData(HWND hListViewWnd, BOOL bAbort)
{
	HWND hEditCtrlWnd = g_hEditCtrlWnd;
	if(!hEditCtrlWnd)
		return;

	if(!bAbort)
	{
		int nIndex;

		nIndex = ListView_GetNextItem(hListViewWnd, -1, LVNI_SELECTED);
		if(nIndex != -1)
		{
			WCHAR szBuffer[sizeof("4294967295")];

			if(GetWindowText(hEditCtrlWnd, szBuffer, sizeof("4294967295")) > 0)
			{
				WCHAR szOldText[sizeof("4294967295")];

				wsprintf(szBuffer, L"%u", StringToUInt(szBuffer));

				ListView_GetItemText(hListViewWnd, nIndex, 1, szOldText, sizeof("4294967295"));

				if(lstrcmp(szOldText, szBuffer) != 0)
				{
					ListView_SetItemText(hListViewWnd, nIndex, 1, szBuffer);
					SetListViewModified(GetParent(hListViewWnd), hListViewWnd);
				}
			}
		}
	}

	UnhookWindowsHookEx(g_hLabelEditMouseHook);

	SetWindowLongPtr(hEditCtrlWnd, GWLP_WNDPROC, (LONG_PTR)g_pOldEditCtrlProc);

	if(GetFocus() == hEditCtrlWnd)
		SetFocus(GetParent(hListViewWnd));

	DestroyWindow(hEditCtrlWnd);
	g_hEditCtrlWnd = NULL;
}

static void CleanupEditListViewData()
{
	HWND hEditCtrlWnd = g_hEditCtrlWnd;
	if(hEditCtrlWnd)
	{
		UnhookWindowsHookEx(g_hLabelEditMouseHook);
		SetWindowLongPtr(hEditCtrlWnd, GWLP_WNDPROC, (LONG_PTR)g_pOldEditCtrlProc);
		DestroyWindow(hEditCtrlWnd);
		g_hEditCtrlWnd = NULL;
	}
}

static LRESULT CALLBACK EditCtrlSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DIALOG_PARAM *pDialogParam)
{
	switch(uMsg)
	{
	case WM_GETDLGCODE:
		return CallWindowProc(g_pOldEditCtrlProc, hWnd, uMsg, wParam, lParam) | DLGC_WANTALLKEYS;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_RETURN:
			EndEditListViewData(GetParent(hWnd), FALSE);
			return 0;

		case VK_ESCAPE:
			EndEditListViewData(GetParent(hWnd), TRUE);
			return 0;
		}
		break;

	case WM_KILLFOCUS:
		EndEditListViewData(GetParent(hWnd), FALSE);
		break;
	}

	return CallWindowProc(g_pOldEditCtrlProc, hWnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK EditLabelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode < 0)
		return CallNextHookEx(g_hLabelEditMouseHook, nCode, wParam, lParam);

	if(nCode == HC_ACTION)
	{
		if(((MOUSEHOOKSTRUCT *)lParam)->hwnd != g_hEditCtrlWnd &&
			(wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN ||
			wParam == WM_NCLBUTTONDOWN || wParam == WM_NCRBUTTONDOWN || wParam == WM_NCMBUTTONDOWN))
		{
			EndEditListViewData(GetParent(g_hEditCtrlWnd), FALSE);
		}
	}

	return CallNextHookEx(g_hLabelEditMouseHook, nCode, wParam, lParam);
}
