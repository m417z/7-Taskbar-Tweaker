#pragma once

#include "MinHook/MinHook.h"

#define MHP_FUNCTION_PARAMS(...) __VA_ARGS__

#define MHP_HOOK_DECLARATIONS_AUX(func_name, ret_type, calling_conv, func_args_w_types, hook_func_args_w_types) \
	typedef ret_type(calling_conv *func_name##_type)(func_args_w_types); \
	typedef ret_type(calling_conv *func_name##_hook_type)(hook_func_args_w_types); \
	extern func_name##_type Original##func_name; \
	void MHP_Hook##func_name(func_name##_hook_type pHookProc);

#define MHP_HOOK_DECLARATIONS(func_name, ret_type, calling_conv, func_args_w_types) \
	MHP_HOOK_DECLARATIONS_AUX(func_name, ret_type, calling_conv, func_args_w_types, MHP_FUNCTION_PARAMS(void *pReturnVar, func_args_w_types))

#define MHP_HOOK_DECLARATIONS_NO_ARGS(func_name, ret_type, calling_conv) \
	MHP_HOOK_DECLARATIONS_AUX(func_name, ret_type, calling_conv, void, void *pReturnVar)

MHP_HOOK_DECLARATIONS_NO_ARGS(GetCapture, HWND, WINAPI);
MHP_HOOK_DECLARATIONS(GetKeyState, SHORT, WINAPI, MHP_FUNCTION_PARAMS(int nVirtKey));
MHP_HOOK_DECLARATIONS(GetWindowLongW, LONG, WINAPI, MHP_FUNCTION_PARAMS(HWND hWnd, int nIndex));
MHP_HOOK_DECLARATIONS(PostMessageW, BOOL, WINAPI, MHP_FUNCTION_PARAMS(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam));
MHP_HOOK_DECLARATIONS(DwmEnableBlurBehindWindow, HRESULT, WINAPI, MHP_FUNCTION_PARAMS(HWND hWnd, const DWM_BLURBEHIND* pBlurBehind));
MHP_HOOK_DECLARATIONS(GetSystemMetrics, int, WINAPI, MHP_FUNCTION_PARAMS(int nIndex));
MHP_HOOK_DECLARATIONS(GetClientRect, BOOL, WINAPI, MHP_FUNCTION_PARAMS(HWND hWnd, LPRECT lpRect));

// Functions
BOOL MHP_Initialize();
void MHP_Uninitialize();
void MHP_WaitTillDone();
