#pragma once

BOOL OpenScrollSndVol(WPARAM wParam, LPARAM lMousePosParam);
BOOL IsSndVolOpen();
BOOL ScrollSndVol(WPARAM wParam, LPARAM lMousePosParam);
void SetSndVolTimer();
void ResetSndVolTimer();
void KillSndVolTimer();
void CleanupSndVol();

// Mouse hook functions
void OnSndVolMouseMove_MouseHook(POINT pt);
void OnSndVolMouseClick_MouseHook(POINT pt);
void OnSndVolMouseWheel_MouseHook(POINT pt);

// Tooltip indicator functions
void OnSndVolTooltipTimer();

// Other functions
BOOL GetSndVolTrayIconRect(RECT *prc);
BOOL IsVolMuted(BOOL *pbMuted);
BOOL IsVolMutedAndNotZero(BOOL *pbResult);
BOOL ToggleVolMuted();
BOOL AddMasterVolumeLevelScalar(float fMasterVolumeAdd);
