#ifndef _EXPLORER_INJECT_H_
#define _EXPLORER_INJECT_H_

#include "options_def.h"
#include "inject_init_struct_def.h"

DWORD ExplorerInject(HWND hTweakerWnd, UINT uEjectedMsg, LANGID langid, int pOptions[OPTS_COUNT], WCHAR *pIniFile);
BOOL ExplorerIsInjected();
HWND ExplorerGetTaskbarWnd();
void ExplorerCleanup();

#endif // _EXPLORER_INJECT_H_
