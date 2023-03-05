#include "stdafx.h"
#include "settings.h"
#include "portable_settings.h"

TWEAKER_SETTINGS twSettings;

BOOL LoadTweakerSettings()
{
	PS_SECTION section;
	const WCHAR *pSettingKeys[] = { L"updcheck", L"updcheckauto", L"hidetray", L"lefttrayfunc", L"language" };
	int nValues[] = { 1, 0, 0, 0, 0 };
	BOOL bError = FALSE;
	int n;

	switch(PSOpenSection(NULL, FALSE, &section))
	{
	case ERROR_SUCCESS:
		for(int i = 0; i < _countof(pSettingKeys); i++)
		{
			switch(PSGetInt(&section, pSettingKeys[i], &n))
			{
			case ERROR_SUCCESS:
				nValues[i] = n;
				break;

			default:
				bError = TRUE;
			case ERROR_FILE_NOT_FOUND:
				break;
			}
		}

		PSCloseSection(&section);
		break;

	default:
		bError = TRUE;
	case ERROR_FILE_NOT_FOUND:
		break;
	}

	twSettings.bCheckForUpdates = nValues[0] != 0;
	twSettings.bAutoCheckForUpdates = nValues[1] != 0;
	twSettings.bHideTray = nValues[2] != 0;
	twSettings.bTrayOpensInspector = nValues[3] != 0;
	twSettings.nLanguage = (LANGID)nValues[4];

	return !bError;
}

BOOL SaveTweakerSettings()
{
	PS_SECTION section;
	const WCHAR *pSettingKeys[] = { L"updcheck", L"updcheckauto", L"hidetray", L"lefttrayfunc", L"language" };
	int nValues[] = {
		twSettings.bCheckForUpdates ? 1 : 0,
		twSettings.bAutoCheckForUpdates ? 1 : 0,
		twSettings.bHideTray ? 1 : 0,
		twSettings.bTrayOpensInspector ? 1 : 0,
		twSettings.nLanguage,
	};
	BOOL bError = FALSE;

	switch(PSOpenSection(NULL, TRUE, &section))
	{
	case ERROR_SUCCESS:
		for(int i = 0; i < _countof(pSettingKeys); i++)
		{
			if(PSSetInt(&section, pSettingKeys[i], nValues[i]) != ERROR_SUCCESS)
			{
				bError = TRUE;
			}
		}

		PSCloseSection(&section);
		break;

	default:
		bError = TRUE;
		break;
	}

	return !bError;
}
