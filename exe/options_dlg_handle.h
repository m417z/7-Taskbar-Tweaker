#ifndef _OPTIONS_DLG_HANDLE_H_
#define _OPTIONS_DLG_HANDLE_H_

#include "options_def.h"
#include "functions.h"
#include "resource.h"

#define CTRL_TYPE_CHECKBOX    0
#define CTRL_TYPE_RADIOBUTTON 1
#define CTRL_TYPE_COMBOBOX    2

typedef struct {
	int nCtrlType;
	int nOption;
	int nExtra; // CHECKBOX: is checked, RADIOBUTTON: option value, COMBOBOX: not used
} CTRL_INFO;

void InitDlg(HWND hWnd, int pOptions[OPTS_COUNT]);
int MeasureStringWidthForCombo(HWND hComboWnd, WCHAR *pString);
BOOL OptionsUpdFromDlg(HWND hWnd, int nCtrlId, int nNotificationCode, int pOptions[OPTS_COUNT]);

#endif // _OPTIONS_DLG_HANDLE_H_
