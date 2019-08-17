#pragma once

BOOL ShowInspectorDlg();
void CloseInspectorDlg();
DWORD GetInspectorCloseTime();

BOOL InspectorBeforeTaskbarRefresh();
BOOL InspectorAfterTaskbarRefresh();

void InspectorAfterDPA_Create(int cItemGrow, HDPA hRet);
void InspectorAfterDPA_InsertPtr(HDPA pdpa, int index, void *p, void *pRet);
void InspectorBeforeDPA_DeletePtr(HDPA pdpa, int index);
