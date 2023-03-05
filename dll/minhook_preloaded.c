#include "stdafx.h"
#include "minhook_preloaded.h"

static volatile LONG nHookProcCallCounter;

#define MHP_HOOK_FUNCTION_AUX(func_name, ret_type, calling_conv, func_args_w_types, func_args, hook_func_args) \
	static volatile func_name##_hook_type pHooked##func_name; \
	func_name##_type Original##func_name; \
	static ret_type calling_conv func_name##_MHP(func_args_w_types) \
	{ \
		InterlockedIncrement(&nHookProcCallCounter); \
		ret_type ret; \
		func_name##_hook_type pHooked = pHooked##func_name; \
		if(pHooked) \
			ret = pHooked(hook_func_args); \
		else \
			ret = Original##func_name(func_args); \
		InterlockedDecrement(&nHookProcCallCounter); \
		return ret; \
	} \
	void MHP_Hook##func_name(func_name##_hook_type pHookProc) \
	{ \
		pHooked##func_name = pHookProc; \
	}

#define MHP_HOOK_FUNCTION(func_name, ret_type, calling_conv, func_args_w_types, func_args) \
	MHP_HOOK_FUNCTION_AUX(func_name, ret_type, calling_conv, func_args_w_types, func_args, MHP_FUNCTION_PARAMS(_ReturnAddress(), func_args))

#define MHP_HOOK_FUNCTION_NO_ARGS(func_name, ret_type, calling_conv) \
	MHP_HOOK_FUNCTION_AUX(func_name, ret_type, calling_conv, void, , _ReturnAddress())

#define MHP_CREATE_HOOK(func_name) MH_CreateHook(func_name, func_name##_MHP, (void **)&Original##func_name)

MHP_HOOK_FUNCTION(GetWindowLongW, LONG, WINAPI, MHP_FUNCTION_PARAMS(HWND hWnd, int nIndex), MHP_FUNCTION_PARAMS(hWnd, nIndex));
MHP_HOOK_FUNCTION(PostMessageW, BOOL, WINAPI, MHP_FUNCTION_PARAMS(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam), MHP_FUNCTION_PARAMS(hWnd, Msg, wParam, lParam));
MHP_HOOK_FUNCTION(GetClientRect, BOOL, WINAPI, MHP_FUNCTION_PARAMS(HWND hWnd, LPRECT lpRect), MHP_FUNCTION_PARAMS(hWnd, lpRect));

// Functions

BOOL MHP_Initialize()
{
	if(
		MHP_CREATE_HOOK(GetWindowLongW) != MH_OK ||
		MHP_CREATE_HOOK(PostMessageW) != MH_OK ||
		MHP_CREATE_HOOK(GetClientRect) != MH_OK
	)
	{
		return FALSE;
	}

	if(
		MH_QueueEnableHook(GetWindowLongW) != MH_OK ||
		MH_QueueEnableHook(PostMessageW) != MH_OK ||
		MH_QueueEnableHook(GetClientRect) != MH_OK
	)
	{
		return FALSE;
	}

	return TRUE;
}

void MHP_Uninitialize()
{
	// Note: no need to cleanup MinHook hooks, they will be removed upon uninitialization
	//MH_DisableHook(MH_ALL_HOOKS);
}

void MHP_WaitTillDone()
{
	while(nHookProcCallCounter > 0)
		Sleep(10);
}
