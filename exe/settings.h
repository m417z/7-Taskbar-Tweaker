#pragma once

typedef struct {
	BOOL bCheckForUpdates;
	BOOL bAutoCheckForUpdates;
	BOOL bHideTray;
	BOOL bTrayOpensInspector;
	LANGID nLanguage;
} TWEAKER_SETTINGS;

extern TWEAKER_SETTINGS twSettings;

BOOL LoadTweakerSettings();
BOOL SaveTweakerSettings();
