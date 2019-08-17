#include "stdafx.h"
#include "library_load_errors.h"
#include "portable_settings.h"
#include "functions.h"
#include "wnd_proc.h"
#include "options_def.h"
#include "options_ex.h"
#include "version.h"
#include "inject_init_struct_def.h"

// superglobals
HINSTANCE hDllInst;
int nWinVersion;
WORD nExplorerBuild, nExplorerQFE;
LANGID language_id;
HANDLE hTweakerProcess;
HANDLE hCleanEvent;
HWND hTweakerWnd;
UINT uTweakerMsg;
int nOptions[OPTS_COUNT];
int nOptionsEx[OPTS_EX_COUNT];
DWORD dwTaskbarThreadId;
HWND hTaskbarWnd, hTaskBandWnd, hTaskSwWnd, hTaskListWnd, hThumbnailWnd;
LONG_PTR lpTaskbarLongPtr, lpTaskBandLongPtr, lpTaskSwLongPtr, lpTaskListLongPtr, lpThumbnailLongPtr;
void **ppDrawThemeBackground;
MODULEINFO ExplorerModuleInfo;

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		hDllInst = hinstDLL;
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

DWORD WINAPI WaitThread(LPVOID lpParameter)
{
	HANDLE hHandles[] = { hTweakerProcess, hCleanEvent };
	WaitForMultipleObjects(_countof(hHandles), hHandles, FALSE, INFINITE);

	WndProcExit();
	WndProcWaitTillDone();

	CloseHandle(hTweakerProcess);
	CloseHandle(hCleanEvent);

	FreeLibraryAndExitThread(hDllInst, 0);
}

// Exported
DWORD __stdcall Init(INJECT_INIT_STRUCT *p_inject_init_struct)
{
	static volatile BOOL bInitCalled = FALSE;
	DWORD dwError;

	// Make sure this function is called only once
	if(InterlockedExchange((long *)&bInitCalled, TRUE))
		return LIB_ERR_INIT_ALREADY_CALLED;

	// Check tweaker version
	if(p_inject_init_struct->dwVersion != VER_FILE_VERSION_LONG)
		return LIB_ERR_LIB_VER_MISMATCH;

	// Check OS version
	nWinVersion = WIN_VERSION_UNSUPPORTED;

	VS_FIXEDFILEINFO *pFixedFileInfo = GetModuleVersionInfo(NULL, NULL);
	if(pFixedFileInfo)
	{
		// HIWORD(pFixedFileInfo->dwFileVersionMS); // Major version
		// LOWORD(pFixedFileInfo->dwFileVersionMS); // Minor version
		// HIWORD(pFixedFileInfo->dwFileVersionLS); // Build number
		// LOWORD(pFixedFileInfo->dwFileVersionLS); // QFE

		switch(HIWORD(pFixedFileInfo->dwFileVersionMS)) // Major version
		{
		case 6:
			switch(LOWORD(pFixedFileInfo->dwFileVersionMS)) // Minor version
			{
			case 1:
				nWinVersion = WIN_VERSION_7;
				break;

			case 2:
				nWinVersion = WIN_VERSION_8;
				break;

			case 3:
				if(LOWORD(pFixedFileInfo->dwFileVersionLS) < 17000) // QFE
					nWinVersion = WIN_VERSION_81;
				else
					nWinVersion = WIN_VERSION_811;
				break;

			case 4:
				nWinVersion = WIN_VERSION_10_T1;
				break;
			}
			break;

		case 10:
			if(HIWORD(pFixedFileInfo->dwFileVersionLS) <= 10240) // Build number
				nWinVersion = WIN_VERSION_10_T1;
			else if(HIWORD(pFixedFileInfo->dwFileVersionLS) <= 10586)
				nWinVersion = WIN_VERSION_10_T2;
			else if(HIWORD(pFixedFileInfo->dwFileVersionLS) <= 14393)
				nWinVersion = WIN_VERSION_10_R1;
			else if(HIWORD(pFixedFileInfo->dwFileVersionLS) <= 15063)
				nWinVersion = WIN_VERSION_10_R2;
			else if(HIWORD(pFixedFileInfo->dwFileVersionLS) <= 16299)
				nWinVersion = WIN_VERSION_10_R3;
			else if(HIWORD(pFixedFileInfo->dwFileVersionLS) <= 17134)
				nWinVersion = WIN_VERSION_10_R4;
			else if(HIWORD(pFixedFileInfo->dwFileVersionLS) <= 17763)
				nWinVersion = WIN_VERSION_10_R5;
			else
				nWinVersion = WIN_VERSION_10_19H1;
			break;
		}
	}

	if(nWinVersion == WIN_VERSION_UNSUPPORTED)
		return LIB_ERR_WIN_VER_MISMATCH;

	nExplorerBuild = HIWORD(pFixedFileInfo->dwFileVersionLS);
	nExplorerQFE = LOWORD(pFixedFileInfo->dwFileVersionLS);

	// Set some globals
	language_id = p_inject_init_struct->lang;
	hTweakerProcess = p_inject_init_struct->hTweakerProcess;
	hCleanEvent = p_inject_init_struct->hCleanEvent;
	hTweakerWnd = p_inject_init_struct->hTweakerWnd;
	uTweakerMsg = RegisterWindowMessage(L"7 Taskbar Tweaker");

	// Init
	if(p_inject_init_struct->szIniFile[0] == L'\0' || PSInit(PS_INI, p_inject_init_struct->szIniFile) != ERROR_SUCCESS)
		PSInit(PS_REGISTRY, L"7 Taskbar Tweaker");

	dwError = WndProcInit(p_inject_init_struct->nOptions);
	if(dwError)
		return dwError;

	// Create our thread that will clean stuff up
	HANDLE hWaitThread = CreateThread(NULL, 0, WaitThread, NULL, 0, NULL);
	if(!hWaitThread)
	{
		WndProcExit();
		WndProcWaitTillDone();
		return LIB_ERR_WAITTHREAD;
	}

	DuplicateHandle(GetCurrentProcess(), hWaitThread, p_inject_init_struct->hTweakerProcess, &p_inject_init_struct->hWaitThread, SYNCHRONIZE, FALSE, DUPLICATE_CLOSE_SOURCE);

	return 0;
}
