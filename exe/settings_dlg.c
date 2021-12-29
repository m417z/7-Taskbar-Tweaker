#include "stdafx.h"
#include "settings_dlg.h"
#include "functions.h"
#include "resource.h"

extern WCHAR szLauncherPath[MAX_PATH];
extern WCHAR szIniFile[MAX_PATH];

void ShowSettings(HWND hParentWnd, UINT uSettingUpdMsg)
{
	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), hParentWnd,
		(DLGPROC)DlgSettingsProc, (LPARAM)uSettingUpdMsg);
}

static LRESULT CALLBACK DlgSettingsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static UINT uSettingUpdMsg;
	WCHAR szStartupPath[MAX_PATH];

	switch(uMsg)
	{
	case WM_INITDIALOG:
		uSettingUpdMsg = (UINT)lParam;

		if(GetStartupPath(szStartupPath))
		{
			if(*szStartupPath != L'\0')
			{
				if(lstrcmpi(szStartupPath, szLauncherPath) != 0)
				{
					MessageBox(hWnd, LoadStrFromRsrc(IDS_SETT_INFO_STUP), LoadStrFromRsrc(IDS_SETT_INFO_STUP_CAPT), MB_ICONASTERISK);
					CheckDlgButton(hWnd, IDC_AUTORUN, BST_INDETERMINATE);
				}
				else
					CheckDlgButton(hWnd, IDC_AUTORUN, BST_CHECKED);
			}
		}
		else
			MessageBox(hWnd, L"Could not load startup setting", NULL, MB_ICONEXCLAMATION);

		if(twSettings.bCheckForUpdates)
		{
			CheckDlgButton(hWnd, IDC_UPDCHECK, BST_CHECKED);

			if(twSettings.bAutoCheckForUpdates)
				CheckDlgButton(hWnd, IDC_UPDSILENT, BST_CHECKED);
		}

		if(twSettings.bHideTray)
			CheckDlgButton(hWnd, IDC_HIDETRAY, BST_CHECKED);

		if(twSettings.bTrayOpensInspector)
			CheckDlgButton(hWnd, IDC_TRAY_INSPECTOR, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_TRAY_TWEAKER, BST_CHECKED);

		LoadResLanguages(hWnd);
		break;

	case WM_LBUTTONDOWN:
		SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_AUTORUN:
			switch(IsDlgButtonChecked(hWnd, IDC_AUTORUN))
			{
			case BST_CHECKED:
			case BST_INDETERMINATE:
				if(SetStartupPath(NULL))
					CheckDlgButton(hWnd, IDC_AUTORUN, BST_UNCHECKED);
				else
					MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SAVECONF), NULL, MB_ICONHAND);
				break;

			case BST_UNCHECKED:
				if(*szIniFile != L'\0')
				{
					if(MessageBox(hWnd, LoadStrFromRsrc(IDS_SETT_WARN_PORT),
						LoadStrFromRsrc(IDS_SETT_WARN_PORT_CAPT), MB_ICONEXCLAMATION | MB_YESNO) != IDYES)
					{
						break;
					}
				}

				if(SetStartupPath(szLauncherPath))
					CheckDlgButton(hWnd, IDC_AUTORUN, BST_CHECKED);
				else
					MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SAVECONF), NULL, MB_ICONHAND);
				break;
			}
			break;

		case IDC_UPDCHECK:
			twSettings.bCheckForUpdates = (IsDlgButtonChecked(hWnd, IDC_UPDCHECK) == BST_CHECKED);
			if(!twSettings.bCheckForUpdates && twSettings.bAutoCheckForUpdates)
			{
				CheckDlgButton(hWnd, IDC_UPDSILENT, BST_UNCHECKED);
				twSettings.bAutoCheckForUpdates = FALSE;
				SendMessage(GetParent(hWnd), uSettingUpdMsg, SETTINGS_UPDATE_AUTO_CHANGED, 0);
			}

			SendMessage(GetParent(hWnd), uSettingUpdMsg, SETTINGS_UPDATE_CHANGED, 0);

			if(!SaveTweakerSettings())
				MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SAVECONF), NULL, MB_ICONHAND);
			break;

		case IDC_UPDSILENT:
			twSettings.bAutoCheckForUpdates = (IsDlgButtonChecked(hWnd, IDC_UPDSILENT) == BST_CHECKED);
			if(twSettings.bAutoCheckForUpdates && !twSettings.bCheckForUpdates)
			{
				CheckDlgButton(hWnd, IDC_UPDCHECK, BST_CHECKED);
				twSettings.bCheckForUpdates = TRUE;
				SendMessage(GetParent(hWnd), uSettingUpdMsg, SETTINGS_UPDATE_CHANGED, 0);
			}

			SendMessage(GetParent(hWnd), uSettingUpdMsg, SETTINGS_UPDATE_AUTO_CHANGED, 0);

			if(!SaveTweakerSettings())
				MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SAVECONF), NULL, MB_ICONHAND);
			break;

		case IDC_HIDETRAY:
			twSettings.bHideTray = (IsDlgButtonChecked(hWnd, IDC_HIDETRAY) == BST_CHECKED);

			SendMessage(GetParent(hWnd), uSettingUpdMsg, SETTINGS_HIDETRAY_CHANGED, 0);

			if(!SaveTweakerSettings())
				MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SAVECONF), NULL, MB_ICONHAND);
			break;

		case IDC_TRAY_TWEAKER:
		case IDC_TRAY_INSPECTOR:
			twSettings.bTrayOpensInspector = (LOWORD(wParam) == IDC_TRAY_INSPECTOR);

			SendMessage(GetParent(hWnd), uSettingUpdMsg, SETTINGS_TRAY_CHANGED, 0);

			if(!SaveTweakerSettings())
				MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SAVECONF), NULL, MB_ICONHAND);
			break;

		case IDC_LANG:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				twSettings.nLanguage = (LANGID)SendDlgItemMessage(hWnd, IDC_LANG, CB_GETITEMDATA,
					SendDlgItemMessage(hWnd, IDC_LANG, CB_GETCURSEL, 0, 0), 0);

				SendMessage(GetParent(hWnd), uSettingUpdMsg, SETTINGS_LANG_CHANGED, 0);

				if(!SaveTweakerSettings())
					MessageBox(hWnd, LoadStrFromRsrc(IDS_ERROR_SAVECONF), NULL, MB_ICONHAND);
			}
			break;

		case IDOK:
			EndDialog(hWnd, 0);
			break;
		}
		break;
	}

	return FALSE;
}

static BOOL GetStartupPath(WCHAR pStartupPath[MAX_PATH])
{
	WCHAR szCommandLine[MAX_PATH + 2 + sizeof(" -hidewnd") - 1];
	HKEY hKey;
	DWORD dwType, dwSize;
	UINT nStringLength;
	LSTATUS error;

	error = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		0, KEY_QUERY_VALUE, &hKey);
	if(error == ERROR_SUCCESS)
	{
		dwSize = (MAX_PATH + 2 + sizeof(" -hidewnd") - 1) * sizeof(WCHAR);

		error = RegQueryValueEx(hKey, L"7 Taskbar Tweaker", NULL, &dwType, (BYTE *)szCommandLine, &dwSize);
		if(error == ERROR_SUCCESS)
		{
			if(dwType == REG_SZ && dwSize > 0 && (dwSize % sizeof(WCHAR)) == 0)
			{
				nStringLength = dwSize / sizeof(WCHAR) - 1;

				if(szCommandLine[nStringLength] != L'\0' || lstrlen(szCommandLine) != nStringLength)
					error = ERROR_FILE_NOT_FOUND;
			}
			else
				error = ERROR_FILE_NOT_FOUND;
		}

		RegCloseKey(hKey);
	}

	switch(error)
	{
	case ERROR_SUCCESS:
		if(FileNameFromCmdLine(szCommandLine, pStartupPath))
			return TRUE;
	case ERROR_FILE_NOT_FOUND:
	case ERROR_MORE_DATA:
		*pStartupPath = L'\0';
		return TRUE;

	default:
		return FALSE;
	}
}

static BOOL FileNameFromCmdLine(WCHAR *pCmdLine, WCHAR pFileName[MAX_PATH])
{
	WCHAR *p_in, *p_out;
	WCHAR c;
	BOOL bInQuote;
	int count;

	p_in = pCmdLine;
	p_out = pFileName;
	bInQuote = FALSE;
	count = 0;

	c = *p_in++;

	while(c == L' ' || c == L'\t')
		c = *p_in++;

	while((c != L'\0' && (bInQuote || (c != L' ' && c != L'\t'))))
	{
		if(c != L'\"')
		{
			*p_out++ = c;
			count++;
			if(count > MAX_PATH - 1)
				return FALSE;
		}
		else
			bInQuote = !bInQuote;

		c = *p_in++;
	}

	*p_out = L'\0';

	return TRUE;
}

static BOOL SetStartupPath(WCHAR pStartupPath[MAX_PATH])
{
	WCHAR szCommandLine[MAX_PATH + 2 + sizeof(" -hidewnd") - 1];
	HKEY hKey;
	LSTATUS error;

	error = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		0, KEY_SET_VALUE, &hKey);
	if(error == ERROR_SUCCESS)
	{
		if(pStartupPath)
		{
			szCommandLine[0] = L'\"';
			lstrcpy(szCommandLine + 1, pStartupPath);
			lstrcat(szCommandLine, L"\" -hidewnd");

			error = RegSetValueEx(hKey, L"7 Taskbar Tweaker", 0, REG_SZ, (BYTE *)szCommandLine, (lstrlen(szCommandLine) + 1) * sizeof(WCHAR));
		}
		else
		{
			error = RegDeleteValue(hKey, L"7 Taskbar Tweaker");
			if(error == ERROR_FILE_NOT_FOUND)
				error = ERROR_SUCCESS;
		}

		RegCloseKey(hKey);
	}

	return error == ERROR_SUCCESS;
}

static void LoadResLanguages(HWND hWnd)
{
	HWND hComboBoxWnd;
	LPCWSTR pwsz;
	int index;
	WORD wIDLanguage;

	hComboBoxWnd = GetDlgItem(hWnd, IDC_LANG);

	EnumResourceLanguages(NULL, RT_STRING,
		MAKEINTRESOURCE(IDS_LANGUAGE / 16 + 1), EnumResLangProc, (LPARAM)hComboBoxWnd);

	wIDLanguage = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

	pwsz = FindStringResourceEx(NULL, IDS_LANGUAGE, wIDLanguage);
	if(pwsz)
	{
		pwsz++;
		index = (int)SendMessage(hComboBoxWnd, CB_INSERTSTRING, 0, (LPARAM)pwsz);
		if(index != CB_ERR && index != CB_ERRSPACE)
		{
			SendMessage(hComboBoxWnd, CB_SETITEMDATA, index, wIDLanguage);
			if(pwsz == LoadStrFromRsrc(IDS_LANGUAGE))
				SendMessage(hComboBoxWnd, CB_SETCURSEL, index, 0);
		}
	}
}

static BOOL CALLBACK EnumResLangProc(HMODULE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LONG_PTR lParam)
{
	HWND hComboBoxWnd;
	LPCWSTR pwsz;
	int index;

	if(wIDLanguage != MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
	{
		hComboBoxWnd = (HWND)lParam;

		pwsz = FindStringResourceEx(hModule, IDS_LANGUAGE, wIDLanguage);
		if(pwsz)
		{
			pwsz++;
			index = (int)SendMessage(hComboBoxWnd, CB_ADDSTRING, 0, (LPARAM)pwsz);
			if(index != CB_ERR && index != CB_ERRSPACE)
			{
				SendMessage(hComboBoxWnd, CB_SETITEMDATA, index, wIDLanguage);
				if(pwsz == LoadStrFromRsrc(IDS_LANGUAGE))
					SendMessage(hComboBoxWnd, CB_SETCURSEL, index, 0);
			}
		}
	}

	return TRUE;
}

static LPCWSTR FindStringResourceEx(HINSTANCE hInst, UINT uId, UINT langId)
{
	LPCWSTR pwsz;
	HRSRC hrsrc;
	HGLOBAL hglob;
	UINT i;

	pwsz = NULL;

	hrsrc = FindResourceEx(hInst, RT_STRING, MAKEINTRESOURCE(uId / 16 + 1), langId);
	if(hrsrc)
	{
		hglob = LoadResource(hInst, hrsrc);
		if(hglob)
		{
			pwsz = (LPCWSTR)LockResource(hglob);
			if(pwsz)
			{
				// okay now walk the string table
				for(i = 0; i < (uId & 15); i++)
					pwsz += 1 + (UINT)*pwsz;
			}
		}
	}

	return pwsz;
}
