#pragma once

#include "options_def.h"

#define MSG_DLL_INIT                    (-1)
#define MSG_DLL_UNSUBCLASS              (-2)
#define MSG_DLL_CALLFUNC                (-3)
#define MSG_DLL_CALLFUNC_PARAM          (-4)
#define MSG_DLL_MOUSE_HOOK_WND_IDENT    (-5)

typedef struct _callfunc_param {
	void *pFunction;
	void *pParam;
} CALLFUCN_PARAM;

typedef struct _mouse_hook_wnd_ident_param {
	// In
	const POINT *ppt;
	HWND hMaybeTransWnd;

	// Out
	int nMaybeTransWndIdent;
	HWND hNonTransWnd;
} MOUSE_HOOK_WND_IDENT_PARAM;

DWORD WndProcInit(int pOptions[OPTS_COUNT]);
void WndProcExit(void);
void WndProcWaitTillDone(void);
