#include "stdafx.h"
#include "explorer_inject.h"

#include "explorer_inject_functions.h"
#include "version.h"
#include "library_load_errors.h"
#include "functions.h"
#include "resource.h"

#define RSRC_STRING_W_PARAM(string_id, extra_param) ((DWORD)(((DWORD)(((DWORD_PTR)(string_id)) & 0x000FFFFF)) | ((DWORD)((DWORD)(((DWORD_PTR)(extra_param)) & 0x00000FFF))) << 4*5))

static volatile BOOL bInjected = FALSE;
static HWND g_hTweakerWnd;
static UINT g_uEjectedMsg;
static HWND hTaskbarWnd;
static HANDLE hExplorerProcess;
static HANDLE hCleanEvent;
static HANDLE hRemoteWaitThread;
static volatile HANDLE hWaitThread;

static DWORD LoadLibraryInExplorer(HANDLE hProcess, WCHAR *pDllName,
	INJECT_INIT_STRUCT *p_inject_init_struct, HANDLE *phWaitThread);
static DWORD WINAPI WaitThread(LPVOID lpParameter);
static BOOL MyReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize);
static BOOL MyWriteProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize);

UINT ExplorerInject(HWND hTweakerWnd, UINT uEjectedMsg, LANGID langid, int pOptions[OPTS_COUNT], WCHAR *pIniFile)
{
	WCHAR szDllPath[MAX_PATH];
	HANDLE hExplorerIsShellMutex;
	DWORD dwProcessId;
	HANDLE hRemoteEvent, hRemoteCurrentProcess;
	INJECT_INIT_STRUCT inject_init_struct;
	DWORD dwError;
	UINT uError;
	int i;

	g_hTweakerWnd = hTweakerWnd;
	g_uEjectedMsg = uEjectedMsg;

	// Get DLL path
	i = GetModuleFileName(NULL, szDllPath, MAX_PATH);
	while(i-- && szDllPath[i] != L'\\');
	lstrcpy(&szDllPath[i+1], L"inject.dll");

	if(GetFileAttributes(szDllPath) == INVALID_FILE_ATTRIBUTES)
		return IDS_INJERROR_NODLL;

	// Wait until explorer shell is created
	hExplorerIsShellMutex = OpenMutex(SYNCHRONIZE, FALSE, L"Local\\ExplorerIsShellMutex");
	if(hExplorerIsShellMutex)
	{
		switch(WaitForSingleObject(hExplorerIsShellMutex, INFINITE))
		{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED:
			ReleaseMutex(hExplorerIsShellMutex);
			break;
		}

		CloseHandle(hExplorerIsShellMutex);
	}

	// Find and open explorer process
	hTaskbarWnd = FindWindow(L"Shell_TrayWnd", NULL);
	if(!hTaskbarWnd)
		return IDS_INJERROR_NOTBAR;

	GetWindowThreadProcessId(hTaskbarWnd, &dwProcessId);

	hExplorerProcess = OpenProcess(
		PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|
		PROCESS_DUP_HANDLE|PROCESS_QUERY_INFORMATION|SYNCHRONIZE, FALSE, dwProcessId
	);
	if(!hExplorerProcess)
		return IDS_INJERROR_EXPROC;

	// Wait for explorer to initialize in case it didn't
	WaitForInputIdle(hExplorerProcess, INFINITE);

	// Create an event (that we are going to signal on exit)
	hCleanEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Duplicate some handles
	DuplicateHandle(GetCurrentProcess(), hCleanEvent, hExplorerProcess, &hRemoteEvent, EVENT_MODIFY_STATE|SYNCHRONIZE, FALSE, 0);
	DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), hExplorerProcess, &hRemoteCurrentProcess, PROCESS_DUP_HANDLE|SYNCHRONIZE, FALSE, 0);

	// Fill the inject struct
	inject_init_struct.dwVersion = VER_FILE_VERSION_LONG;
	inject_init_struct.hTweakerWnd = hTweakerWnd;
	inject_init_struct.lang = langid;
	inject_init_struct.hTweakerProcess = hRemoteCurrentProcess;
	inject_init_struct.hCleanEvent = hRemoteEvent;
	CopyMemory(inject_init_struct.nOptions, pOptions, OPTS_BUFF);

	if(pIniFile)
		lstrcpy(inject_init_struct.szIniFile, pIniFile);
	else
		*inject_init_struct.szIniFile = L'\0';

	// The real thing, load the dll in explorer
	dwError = LoadLibraryInExplorer(hExplorerProcess, szDllPath, &inject_init_struct, &hRemoteWaitThread);
	if(dwError == 0)
	{
		// Create our thread that will clean stuff up
		hWaitThread = CreateThread(NULL, 0, WaitThread, NULL, CREATE_SUSPENDED, NULL);
		if(hWaitThread)
		{
			ResumeThread(hWaitThread);
			bInjected = TRUE;

			return 0;
		}
		else
			uError = IDS_INJERROR_X3;

		SetEvent(hCleanEvent);

		WaitForSingleObject(hRemoteWaitThread, INFINITE);
		CloseHandle(hRemoteWaitThread);
	}
	else
		uError = RSRC_STRING_W_PARAM(IDS_INJERROR_LOADDLL, dwError);

	CloseHandle(hCleanEvent);
	DuplicateHandle(hExplorerProcess, hRemoteEvent, NULL, NULL, 0, FALSE, DUPLICATE_CLOSE_SOURCE);
	DuplicateHandle(hExplorerProcess, hRemoteCurrentProcess, NULL, NULL, 0, FALSE, DUPLICATE_CLOSE_SOURCE);
	CloseHandle(hExplorerProcess);

	return uError;
}

static DWORD LoadLibraryInExplorer(HANDLE hProcess, WCHAR *pDllName,
	INJECT_INIT_STRUCT *p_inject_init_struct, HANDLE *phWaitThread)
{
	void *pCode = LoadLibraryRemoteProc;
	void *pCodeEnd = LoadLibraryRemoteProc_EOF;
#ifdef _DEBUG
	if(*(BYTE *)pCode == 0xE9)
	{
		pCode = (void *)((char *)pCode + *(DWORD *)((char *)pCode + 1) + 5);
	}
	if(*(BYTE *)pCodeEnd == 0xE9)
	{
		pCodeEnd = (void *)((char *)pCodeEnd + *(DWORD *)((char *)pCodeEnd + 1) + 5);
	}
#endif // _DEBUG
	size_t cbCodeSize = ((char *)pCodeEnd - (char *)pCode);
	LOAD_LIBRARY_REMOTE_DATA remote_data;
	size_t cbDataSize = sizeof(LOAD_LIBRARY_REMOTE_DATA);
	HANDLE hRemoteThread;
	void *pRemoteCode;
	void *pRemoteData;
	DWORD dwError;

	const int cbCodeSizeAligned = (cbCodeSize + (sizeof(LONG_PTR)-1)) & ~ (sizeof(LONG_PTR)-1);

	remote_data.pGetModuleHandle = GetModuleHandle;
	remote_data.pLoadLibrary = LoadLibrary;
	remote_data.pGetProcAddress = GetProcAddress;
	remote_data.pFreeLibrary = FreeLibrary;
	lstrcpy(remote_data.szDllName, pDllName);
	remote_data.dwError = INJ_ERR_BEFORE_RUN; // in case the injected code fails
	CopyMemory(&remote_data.inject_init_struct, p_inject_init_struct, sizeof(INJECT_INIT_STRUCT));

	// Allocate enough memory in the remote process's address space
	// to hold the binary image of our injection thread, and
	// a copy of the INJTHREADINFO structure

	pRemoteCode = VirtualAllocEx(hProcess, NULL, cbCodeSizeAligned + cbDataSize, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if(pRemoteCode)
	{
		// Write a copy of our injection thread into the remote process
		if(MyWriteProcessMemory(hProcess, pRemoteCode, pCode, cbCodeSize))
		{
			// Write a copy of the INJTHREAD to the remote process. This structure
			// MUST start on an aligned boundary
			pRemoteData = (void *)((BYTE *)pRemoteCode + cbCodeSizeAligned);

			// Put DATA in the remote thread's memory block
			if(MyWriteProcessMemory(hProcess, pRemoteData, &remote_data, cbDataSize))
			{
				// Create the remote thread!!!
				hRemoteThread = CreateRemoteThread(hProcess, NULL, 0,
					(LPTHREAD_START_ROUTINE)pRemoteCode, pRemoteData, 0, NULL);
				if(hRemoteThread)
				{
					// Wait for the thread to terminate
					WaitForSingleObject(hRemoteThread, INFINITE);

					// Read the user-structure back again
					if(MyReadProcessMemory(hProcess, pRemoteData, &remote_data, cbDataSize))
					{
						dwError = remote_data.dwError;
						if(dwError == 0)
							*phWaitThread = remote_data.inject_init_struct.hWaitThread;
					}
					else
						dwError = EXE_ERR_READ_PROC_MEM;

					CloseHandle(hRemoteThread);
				}
				else
					dwError = EXE_ERR_CREATE_REMOTE_THREAD;
			}
			else
				dwError = EXE_ERR_WRITE_PROC_MEM;
		}
		else
			dwError = EXE_ERR_WRITE_PROC_MEM;

		// Free the memory in the remote process
		VirtualFreeEx(hProcess, pRemoteCode, 0, MEM_RELEASE);
	}
	else
		dwError = EXE_ERR_VIRTUAL_ALLOC;

	return dwError;
}

static DWORD WINAPI WaitThread(LPVOID lpParameter)
{
	HANDLE hHandles[2];
	HANDLE hThread;

	hHandles[0] = hCleanEvent;
	hHandles[1] = hExplorerProcess;

	if(WaitForMultipleObjects(2, hHandles, FALSE, INFINITE) == WAIT_OBJECT_0)
		WaitForSingleObject(hRemoteWaitThread, INFINITE);

	CloseHandle(hRemoteWaitThread);
	CloseHandle(hCleanEvent);
	CloseHandle(hExplorerProcess);

	hThread = InterlockedExchangePointer(&hWaitThread, NULL);
	if(hThread)
		CloseHandle(hThread);

	bInjected = FALSE;
	PostMessage(g_hTweakerWnd, g_uEjectedMsg, 0, 0);
	return 0;
}

static BOOL MyReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize)
{
	SIZE_T nNumberOfBytesRead;

	return ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, &nNumberOfBytesRead) &&
		nNumberOfBytesRead == nSize;
}

static BOOL MyWriteProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize)
{
	SIZE_T nNumberOfBytesWritten;

	return WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, &nNumberOfBytesWritten) &&
		nNumberOfBytesWritten == nSize;
}

BOOL ExplorerIsInjected()
{
	return bInjected;
}

HWND ExplorerGetTaskbarWnd()
{
	return hTaskbarWnd;
}

void ExplorerCleanup()
{
	HANDLE hThread;

	SetEvent(hCleanEvent);

	hThread = InterlockedExchangePointer(&hWaitThread, NULL);
	if(hThread)
	{
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
}
