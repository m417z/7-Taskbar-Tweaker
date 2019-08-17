#pragma once

// Storage types
#define PS_REGISTRY           0
#define PS_INI                1

#define PS_STORAGE_COUNT      2

// Structs
typedef struct tagPS_SECTION {
	union {
		struct { // Registry
			HKEY hSubKey;
		};
		struct { // Ini
			WCHAR szSection[256];
		};
	};
} PS_SECTION;

typedef struct tagPS_FIND {
	union {
		struct { // Registry
			DWORD dwIndex;
		};
		struct { // Ini
			WCHAR *pValueNames;
			WCHAR *pCurrentValueName;
		};
	};
} PS_FIND;

// Functions
LSTATUS PSInit(int nStorage, const WCHAR *pStorageData);
LSTATUS PSOpenSection(const WCHAR *pSectionName, BOOL bWrite, PS_SECTION *pSection);
LSTATUS PSCloseSection(PS_SECTION *pSection);
LSTATUS PSRemoveSection(const WCHAR *pSectionName);
LSTATUS PSGetString(PS_SECTION *pSection, const WCHAR *pValueName, WCHAR *pReturnedString, UINT *pStringSize);
LSTATUS PSSetString(PS_SECTION *pSection, const WCHAR *pValueName, const WCHAR *pString);
LSTATUS PSGetInt(PS_SECTION *pSection, const WCHAR *pValueName, int *pReturnedInt);
LSTATUS PSSetInt(PS_SECTION *pSection, const WCHAR *pValueName, int nInt);
LSTATUS PSRemove(PS_SECTION *pSection, const WCHAR *pValueName);
LSTATUS PSFindInit(PS_SECTION *pSection, PS_FIND *pFind);
LSTATUS PSFindGetCount(PS_SECTION *pSection, PS_FIND *pFind, UINT *pnCount);
LSTATUS PSFindNextString(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, WCHAR *pReturnedString, UINT *pStringSize);
LSTATUS PSFindNextInt(PS_SECTION *pSection, PS_FIND *pFind,
	WCHAR *pReturnedValueName, UINT *pValueNameSize, int *pReturnedInt);
LSTATUS PSFindSkip(PS_SECTION *pSection, PS_FIND *pFind);
LSTATUS PSFindClose(PS_SECTION *pSection, PS_FIND *pFind);
LSTATUS PSGetSingleString(const WCHAR *pSectionName, const WCHAR *pValueName, WCHAR *pReturnedString, UINT *pStringSize);
LSTATUS PSSetSingleString(const WCHAR *pSectionName, const WCHAR *pValueName, const WCHAR *pString);
LSTATUS PSGetSingleInt(const WCHAR *pSectionName, const WCHAR *pValueName, int *pReturnedInt);
LSTATUS PSSetSingleInt(const WCHAR *pSectionName, const WCHAR *pValueName, int nInt);
LSTATUS PSRemoveSingle(const WCHAR *pSectionName, const WCHAR *pValueName);
