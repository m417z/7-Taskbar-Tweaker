#include "stdafx.h"
#include "dpa_func_hook.h"
#include "MinHook/MinHook.h"
#include "taskbar_inspector.h"
#include "com_func_hook.h"

// superglobals
extern DWORD dwTaskbarThreadId;

static void *pDPA_Create, *pDPA_InsertPtr, *pDPA_DeletePtr;
static volatile LONG nHookProcCallCounter;

static HDPA WINAPI DPA_CreateHook(int cItemGrow);
static void *WINAPI DPA_InsertPtrHook(HDPA pdpa, int index, void *p);
static void *WINAPI DPA_DeletePtrHook(HDPA pdpa, int index);

BOOL DPAFuncHook_Init()
{
	if(MH_CreateHook(DPA_Create, DPA_CreateHook, &pDPA_Create) == MH_OK)
	{
		if(MH_CreateHook(DPA_InsertPtr, DPA_InsertPtrHook, &pDPA_InsertPtr) == MH_OK)
		{
			if(MH_CreateHook(DPA_DeletePtr, DPA_DeletePtrHook, &pDPA_DeletePtr) == MH_OK)
			{
				if(
					MH_QueueEnableHook(DPA_Create) == MH_OK &&
					MH_QueueEnableHook(DPA_InsertPtr) == MH_OK &&
					MH_QueueEnableHook(DPA_DeletePtr) == MH_OK
				)
					return TRUE;

				MH_RemoveHook(DPA_DeletePtr);
			}

			MH_RemoveHook(DPA_InsertPtr);
		}

		MH_RemoveHook(DPA_Create);
	}

	return FALSE;
}

BOOL DPAFuncHook_Exit()
{
	// No need to disable MinHook hooks, they will be disabled upon MinHook uninitialization

	return TRUE;
}

void DPAFuncHook_WaitTillDone()
{
	while(nHookProcCallCounter > 0)
		Sleep(10);
}

static HDPA WINAPI DPA_CreateHook(int cItemGrow)
{
	HDPA hRet;

	InterlockedIncrement(&nHookProcCallCounter);

	hRet = ((HDPA(WINAPI *)(int))pDPA_Create)(cItemGrow);

	if(GetCurrentThreadId() == dwTaskbarThreadId)
		InspectorAfterDPA_Create(cItemGrow, hRet);

	InterlockedDecrement(&nHookProcCallCounter);

	return hRet;
}

static void *WINAPI DPA_InsertPtrHook(HDPA pdpa, int index, void *p)
{
	void *pRet;

	InterlockedIncrement(&nHookProcCallCounter);

	pRet = ((void *(WINAPI *)(HDPA, int, void *))pDPA_InsertPtr)(pdpa, index, p);

	if(GetCurrentThreadId() == dwTaskbarThreadId)
	{
		InspectorAfterDPA_InsertPtr(pdpa, index, p, pRet);
		ComFuncVirtualDesktopFixAfterDPA_InsertPtr(pdpa, index, p, pRet);
	}

	InterlockedDecrement(&nHookProcCallCounter);

	return pRet;
}

static void *WINAPI DPA_DeletePtrHook(HDPA pdpa, int index)
{
	void *pRet;

	InterlockedIncrement(&nHookProcCallCounter);

	if(GetCurrentThreadId() == dwTaskbarThreadId)
		InspectorBeforeDPA_DeletePtr(pdpa, index);

	pRet = ((void *(WINAPI *)(HDPA, int))pDPA_DeletePtr)(pdpa, index);

	InterlockedDecrement(&nHookProcCallCounter);

	return pRet;
}
