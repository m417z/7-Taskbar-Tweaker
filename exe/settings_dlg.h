#ifndef _SETTINGS_DLG_H_
#define _SETTINGS_DLG_H_

#include "settings.h"

#define SETTINGS_HIDETRAY_CHANGED       0
#define SETTINGS_TRAY_CHANGED           1
#define SETTINGS_LANG_CHANGED           2
#define SETTINGS_UPDATE_CHANGED         3
#define SETTINGS_UPDATE_AUTO_CHANGED    4

void ShowSettings(HWND hParentWnd, UINT uSettingUpdMsg);
static LRESULT CALLBACK DlgSettingsProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL GetStartupPath(WCHAR pStartupPath[MAX_PATH]);
static BOOL FileNameFromCmdLine(WCHAR *pCmdLine, WCHAR pFileName[MAX_PATH]);
static BOOL SetStartupPath(WCHAR pStartupPath[MAX_PATH]);
static void LoadResLanguages(HWND hWnd);
static BOOL CALLBACK EnumResLangProc(HMODULE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LONG_PTR lParam);
static LPCWSTR FindStringResourceEx(HINSTANCE hInst, UINT uId, UINT langId);

#endif // _SETTINGS_DLG_H_
