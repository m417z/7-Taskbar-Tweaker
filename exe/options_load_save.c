#include "stdafx.h"
#include "options_load_save.h"

BOOL LoadOptions(int pOptions[OPTS_COUNT])
{
	PS_SECTION section;
	WCHAR szConfigurationValue[sizeof("4294967295")];
	LSTATUS error;
	int nOptValue;
	int i;

	error = PSOpenSection(L"Options", FALSE, &section);
	if(error == ERROR_SUCCESS)
	{
		for(i = 0; i < OPTS_COUNT; i++)
		{
			wsprintf(szConfigurationValue, L"%u", opts_configuration_values[i]);

			error = PSGetInt(&section, szConfigurationValue, &nOptValue);
			if(error == ERROR_SUCCESS)
			{
				if(nOptValue < 0 || nOptValue > opts_max_values[i])
					nOptValue = 0;
			}
			else if(error == ERROR_FILE_NOT_FOUND)
			{
				nOptValue = 0;
				error = ERROR_SUCCESS;
			}
			else
				break;

			pOptions[i] = nOptValue;
			CheckOptionDependences(pOptions, i);
		}

		PSCloseSection(&section);
	}

	if(error != ERROR_SUCCESS)
	{
		ZeroMemory(pOptions, OPTS_BUFF);
		return FALSE;
	}

	// Temporary OptionEx deprecation code
	// {
	if(pOptions[OPT_OTHER_NOSTARTBTN] == 0)
	{
		error = PSOpenSection(L"OptionsEx", TRUE, &section);
		if(error == ERROR_SUCCESS)
		{
			error = PSGetInt(&section, L"hide_start_button", &nOptValue);
			if(error == ERROR_SUCCESS)
			{
				if(nOptValue)
				{
					pOptions[OPT_OTHER_NOSTARTBTN] = 1;

					wsprintf(szConfigurationValue, L"%u", opts_configuration_values[OPT_OTHER_NOSTARTBTN]);
					PSSetSingleInt(L"Options", szConfigurationValue, 1);
				}

				PSRemove(&section, L"hide_start_button");
			}
			else if(error == ERROR_FILE_NOT_FOUND)
				error = ERROR_SUCCESS;

			PSCloseSection(&section);
		}
	}
	// }

	return TRUE;
}

BOOL SaveOptions(int pOptions[OPTS_COUNT])
{
	PS_SECTION section;
	WCHAR szConfigurationValue[sizeof("4294967295")];
	LSTATUS error;
	int i;

	error = PSOpenSection(L"Options", TRUE, &section);
	if(error == ERROR_SUCCESS)
	{
		for(i = 0; i < OPTS_COUNT; i++)
		{
			wsprintf(szConfigurationValue, L"%u", opts_configuration_values[i]);

			error = PSSetInt(&section, szConfigurationValue, pOptions[i]);
			if(error != ERROR_SUCCESS)
				break;
		}

		PSCloseSection(&section);
	}

	return error == ERROR_SUCCESS;
}

static void CheckOptionDependences(int pOptions[OPTS_COUNT], int nOption)
{
	OPTS_STRUCT_RULES *pRules;

	pRules = opts_dependences_rules(pOptions, nOption);
	if(pRules)
	{
		do
		{
			pOptions[pRules->nOptIndex] = pRules->nOptValue;
			pRules++;
		}
		while(pRules->nOptIndex != -1);
	}
}
