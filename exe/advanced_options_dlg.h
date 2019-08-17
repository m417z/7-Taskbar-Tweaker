#ifndef _ADVANCED_OPTIONS_DLG_H_
#define _ADVANCED_OPTIONS_DLG_H_

#define ADV_OPTS_OPTIONSEX              0
#define ADV_OPTS_MOUSE_BUTTON_CONTROL   1
#define ADV_OPTS_KEYBOARD_SHORTCUTS     2

#define ADV_OPTS_DESTROYED              10

HWND CreateAdvancedOptionsDialog(HWND hParentWnd, UINT uNotifyMsg);

#endif // _ADVANCED_OPTIONS_DLG_H_
