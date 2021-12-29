#pragma once

#include "options_def.h"

typedef struct _inject_init_struct {
	// in
	DWORD dwVersion;
	HWND hTweakerWnd;
	LANGID lang;
	HANDLE hTweakerProcess;
	HANDLE hCleanEvent;
	int nOptions[OPTS_COUNT];
	WCHAR szIniFile[MAX_PATH];

	// out
	HANDLE hWaitThread;
} INJECT_INIT_STRUCT;
