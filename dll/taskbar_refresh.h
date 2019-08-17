#pragma once

BOOL RefreshTaskbarHardcore_Init();
void RefreshTaskbarHardcore_Exit();
void RefreshTaskbarHardcore_WaitTillDone();
BOOL RefreshTaskbarHardcore();

BOOL RefreshTaskbarHardcore_ButtonCreating(HWND hCreatingWnd);
void RefreshTaskbarHardcore_ButtonCreated(HWND hCreatedWnd);
void RefreshTaskbarHardcore_ButtonDestroyed(HWND hDestroyedWnd);
