#ifndef _EXE_H_
#define _EXE_H_

#include "options_def.h"

#define UWM_EXIT                        WM_APP
#define UWM_SHOW_INIT_ERROR_MSGS        (WM_APP+1)
#define UWM_NOTIFYICON                  (WM_APP+2)
#define UWM_SETTING_DLG                 (WM_APP+3)
#define UWM_UPDATE_CHECKED              (WM_APP+4)
#define UWM_EXITBLOCKACQUIRE            (WM_APP+5)
#define UWM_EXITBLOCKRELEASE            (WM_APP+6)
#define UWM_ADVANCED_OPTS_DLG           (WM_APP+7)
#define UWM_EJECTED_FROM_EXPLORER       (WM_APP+8)

#define SYSTEM_MENU_ADVANCED            40001

#define ID_TRAY___FIRST___				40001
#define ID_TRAY_UPDATE					40001
#define ID_TRAY_TWEAKS					40002
#define ID_TRAY_SETTINGS				40003
#define ID_TRAY_ADVANCED				40004
#define ID_TRAY_INSPECTOR				40005
#define ID_TRAY_EXIT					40006
#define ID_TRAY___LAST___				40006

typedef struct _dlg_param {
	BOOL bInitialized;

	UINT uTaskbarCreatedMsg, uTweakerMsg;
	HICON hSmallIcon, hLargeIcon;
	HICON hUpdateIcon;
	HBRUSH hBgBrush;
	NOTIFYICONDATA nid;

	HWND hAdvancedOptionsWnd;

	// Initializing errors
	DWORD dwInitErrors[2];
	int nInitErrorsCount;

	// Injection error
	UINT uInjectionAttempts;
	DWORD dwInjectionError;
	DWORD dwWantedToShowUpdateDialogTickCount;

	// Settings
	BOOL bHideWnd;
	BOOL bLangChanged;

	// Options
	int nOptions[OPTS_COUNT];

	// Close blocking
	int nExitBlockCount;
	BOOL bClosing;
	int nClosingResult;
} DLG_PARAM;

#ifndef TDF_SIZE_TO_CONTENT
#define TDF_SIZE_TO_CONTENT 0x1000000
#endif

BOOL InitSettings(void);
BOOL Run(void);
LRESULT CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT OnInitDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnNcHitTest(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnCtlColorDlg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnCtlColorStatic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnUNotifyIcon(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnUSettingsDlg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnUAdvancedOptsDlg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnUEjectedFromExplorer(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnCopyData(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnSysCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnUExitBlockAcquire(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnUExitBlockRelease(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnUExit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnRegisteredTaskbarCreated(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);
LRESULT OnRegisteredTweakerMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, DLG_PARAM *pDlgParam);

// Functions
BOOL LoadCustomIcon(HICON *phSmall, HICON *phLarge);
HBRUSH CreateBgBrush(HWND hWnd);
BOOL IsHighContrastOn(void);
int FindCmdLineSwitch(WCHAR *pSwitch);
void CustomizeSystemMenu(HWND hWnd);
void InitNotifyIconData(HWND hWnd, DLG_PARAM *pDlgParam);
void UpdateNotifyIconData(HWND hWnd, DLG_PARAM *pDlgParam);
void ShowAdvancedOptionsDialog(HWND hWnd, DLG_PARAM *pDlgParam);
HMENU MakeTrayRightClickMenu(BOOL bUpdateAvailable);
BOOL ShowHelp(HWND hWnd);
BOOL ShowHelpOfLang(HWND hWnd, LANGID langid);
void AboutMsgBox(HWND hWnd);
void InjectionErrorMsgBox(HWND hWnd, DWORD dwError);
void Windows11UnsupportedMsgBox(HWND hWnd);
HRESULT CALLBACK TaskDialogWithLinksCallbackProc(HWND hWnd, UINT uNotification, WPARAM wParam, LPARAM lParam, LONG_PTR dwRefData);
BOOL ApplyLanguage(LANGID new_language_id);
LRESULT SendOptionsMessage(HWND hWnd, HWND hSenderWnd, int pOptions[OPTS_COUNT]);
LRESULT SendInspectorMessage(HWND hWnd, UINT uTweakerMsg);
LRESULT SendInspectorMessageFromTray(HWND hWnd, UINT uTweakerMsg);
LRESULT SendLangMessage(HWND hWnd, UINT uTweakerMsg, LANGID langid);
LRESULT SendNewHwndMessage(HWND hWnd, UINT uTweakerMsg, HWND hTweakerWnd);

#endif // _EXE_H_
