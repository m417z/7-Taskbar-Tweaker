#pragma once

BOOL ComFuncHook_Init(void(*hackf1)(LONG_PTR));
void ComFuncHook_Exit();
void ComFuncHook_WaitTillDone();

void ComFuncVirtualDesktopFixAfterDPA_InsertPtr(HDPA pdpa, int index, void *p, void *pRet);

void ComFuncTaskListBeforeLButtonUp(LONG_PTR lpMMTaskListLongPtr, DWORD *pdwOldUserPrefSetBits, DWORD *pdwOldUserPrefRemoveBits, LONG_PTR **p_prev_button_group_active);
void ComFuncTaskListAfterLButtonUp(LONG_PTR lpMMTaskListLongPtr, DWORD dwOldUserPrefSetBits, DWORD dwOldUserPrefRemoveBits, LONG_PTR *prev_button_group_active);
void ComFuncTaskListBeforeMouseMove();
void ComFuncTaskListAfterMouseMove();
void ComFuncTaskListMouseLeave();
BOOL ComFuncTaskListRightDragInit(LONG_PTR lpMMTaskListLongPtr);
BOOL ComFuncTaskListRightDragProcessed();
void ComFuncTaskListCaptureChanged(LONG_PTR lpMMTaskListLongPtr);
BOOL ComFuncTaskListMouseWheel(LONG_PTR lpMMTaskListLongPtr, short delta);
void ComFuncThumbnailWndBeforePaint(LONG_PTR lpMMThumbnailLongPtr);
void ComFuncSetThumbNoDismiss(BOOL bNoDismiss);
BOOL ComFuncGetThumbNoDismiss();
BOOL ComFuncMoveDetachedToCursor();
BOOL ComFuncIsAttachPending(HWND hButtonWnd);
BOOL ComFuncMoveAttachedToCursor();
void ComFuncMoveNearMatching(HWND hButtonWnd);
void ComFuncSetCreatedThumb(HWND hCreatedThumb, HWND hCreatedThumbParent);
void ComFuncSwitchToHookEnable(int nOption, LONG_PTR lpMMTaskListLongPtr);
void ComFuncSwitchToHookDisable();
LONG_PTR *ComFuncGetLastActiveTaskItem();
void ComFuncResetLastActiveTaskItem();
BOOL ComFuncIsInGetIdealSpan();
BOOL ComFuncIsInHandleDelayInitStuff();
void ComFuncSetTaskItemGetWindowReturnNull(BOOL bSet);
LONG_PTR ComFuncGetSavedTaskItemGetWindow();
