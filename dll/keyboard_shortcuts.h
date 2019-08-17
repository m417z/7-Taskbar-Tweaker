#pragma once

#define KEYBD_SHORTCUT_IS_VALID_VALUE(x) TRUE // (((x)>=1 && (x)<=8) || ((x)>=101 && (x)<=104))

BOOL LoadKeybdShortcuts(WNDPROC pHotKeyWndProc);
void FreeKeybdShortcuts();
BOOL GetKeybdShortcutValue(int nId, int *pnValue);
