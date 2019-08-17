#pragma once

#define MOUSECTRL_IS_VALID_TASKBARITEM(x)    ( ((x) >= 1 && (x) <= 15) )
#define MOUSECTRL_IS_VALID_EMPTYSPACE(x)     TRUE // ( ((x) >= 1 && (x) <= 8) || ((x) >= 101 && (x) <= 104) )
#define MOUSECTRL_IS_VALID_VOLUMEICON(x)     TRUE // Same as above
#define MOUSECTRL_IS_VALID_SHOWDESKTOP(x)    TRUE // Same as above

#define MOUSECTRL_TARGET_TASKBARITEM         1
#define MOUSECTRL_TARGET_EMPTYSPACE          2
#define MOUSECTRL_TARGET_VOLUMEICON          3
#define MOUSECTRL_TARGET_SHOWDESKTOP         4

#define MOUSECTRL_BUTTON_L                   1
#define MOUSECTRL_BUTTON_R                   2
#define MOUSECTRL_BUTTON_M                   3
#define MOUSECTRL_BUTTON_X1                  4
#define MOUSECTRL_BUTTON_X2                  5

#define MOUSECTRL_MOD_DBLCLICK               0x01
#define MOUSECTRL_MOD_CONTROL                0x02
#define MOUSECTRL_MOD_SHIFT                  0x04

#define MOUSECTRL_MAKEKEY(target, button, mod) \
	                                         ((target) | ((button)<<8) | ((mod)<<16)/* | ((sth)<<24)*/)
#define MOUSECTRL_KEYTARGET(key)             ((key) & 0xFF)
#define MOUSECTRL_KEYBUTTON(key)             (((key)>>8) & 0xFF)
#define MOUSECTRL_KEYMOD(key)                (((key)>>16) & 0xFF)

BOOL LoadMouseCtrl();
void FreeMouseCtrl();
BOOL GetMouseCtrlValue(BYTE bTarget, UINT uMsg, WPARAM wParam, int *pnValue);
