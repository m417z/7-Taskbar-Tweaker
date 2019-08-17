#include "explorer_inject_functions.h"

DWORD WINAPI LoadLibraryRemoteProc(LPVOID *pParam)
{
	LOAD_LIBRARY_REMOTE_DATA *pInjData = (LOAD_LIBRARY_REMOTE_DATA *)pParam;
	HMODULE hModule;
	char szInit[] = { 'I', 'n', 'i', 't', '\0' };
	void *pInit;

	pInjData->dwError = INJ_ERR_BEFORE_GETMODULEHANDLE;

	hModule = ((HMODULE(WINAPI *)(LPCTSTR))pInjData->pGetModuleHandle)(pInjData->szDllName);
	if(!hModule)
	{
		pInjData->dwError = INJ_ERR_BEFORE_LOADLIBRARY;

		hModule = ((HMODULE(WINAPI *)(LPCTSTR))pInjData->pLoadLibrary)(pInjData->szDllName);
		if(hModule)
		{
			pInjData->dwError = INJ_ERR_BEFORE_GETPROCADDR;

			pInit = ((FARPROC(WINAPI *)(HMODULE, LPCSTR))pInjData->pGetProcAddress)(hModule, szInit);
			if(pInit)
			{
				pInjData->dwError = INJ_ERR_BEFORE_LIBINIT;

				pInjData->dwError = ((DWORD(__stdcall *)(INJECT_INIT_STRUCT *))pInit)(&pInjData->inject_init_struct);
				if(pInjData->dwError == 0)
					return 0;
			}
			else
				pInjData->dwError = INJ_ERR_GETPROCADDR;

			((BOOL(WINAPI *)(HMODULE))pInjData->pFreeLibrary)(hModule);
		}
		else
			pInjData->dwError = INJ_ERR_LOADLIBRARY;
	}
	else
		pInjData->dwError = INJ_ERR_GETMODULEHANDLE;

	return 1;
}

void *LoadLibraryRemoteProc_EOF(void)
{
	return LoadLibraryRemoteProc;
}
