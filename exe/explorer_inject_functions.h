#pragma once

#include <windows.h>
#include "inject_init_struct_def.h"
#include "library_load_errors.h"

typedef struct tagLOAD_LIBRARY_REMOTE_DATA {
	// in
	void *pGetModuleHandle, *pLoadLibrary, *pGetProcAddress, *pFreeLibrary;
	WCHAR szDllName[MAX_PATH];

	// out
	DWORD dwError;

	// in and out
	INJECT_INIT_STRUCT inject_init_struct;
} LOAD_LIBRARY_REMOTE_DATA;

DWORD WINAPI LoadLibraryRemoteProc(LPVOID *pParam);
void *LoadLibraryRemoteProc_EOF(void);
