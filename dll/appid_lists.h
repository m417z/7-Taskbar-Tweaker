#pragma once

#define AILISTS_COUNT                   4

#define AILIST_LABEL                    0
#define AILIST_GROUP                    1
#define AILIST_GROUPPINNED              2
#define AILIST_COMBINE                  3

#define AILIST_LABEL_NEVER              0
#define AILIST_LABEL_ALWAYS             1
#define AILIST_GROUP_NEVER              0
#define AILIST_GROUP_ALWAYS             1
#define AILIST_GROUPPINNED_NEVER        0
#define AILIST_GROUPPINNED_ALWAYS       1
#define AILIST_COMBINE_NEVER            0
#define AILIST_COMBINE_ALWAYS           1

BOOL LoadAppidLists(void);
void FreeAppidLists(void);
BOOL AddAppidToList(int list, WCHAR *pStr, int nValue, BOOL bMemoryOnly);
BOOL RemoveAppidFromList(int list, WCHAR *pStr, BOOL bMemoryOnly);
BOOL GetAppidListValue(int list, WCHAR *pStr, int *pnValue);
BOOL IsAppidListEmpty(int list);
void ClearAppidLists(void);
