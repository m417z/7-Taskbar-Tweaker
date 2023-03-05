#include "stdafx.h"
#include "appid_lists.h"

#include "portable_settings.h"
// uthash {
#include "uthash.h"

// undefine the defaults
#undef uthash_fatal
#undef uthash_malloc
#undef uthash_free

// re-define, specifying alternate functions
#define uthash_fatal(msg) __debugbreak();
#define uthash_malloc(sz) HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sz)
#define uthash_free(ptr,sz) HeapFree(GetProcessHeap(), 0, ptr)

// tchar macros
#ifndef HASH_FIND_TSTR
#define HASH_FIND_TSTR(head,findstr,out)                                         \
do {                                                                             \
 unsigned _hf_u_keylen = lstrlen(findstr)*sizeof(TCHAR);                         \
 HASH_FIND(hh,head,findstr,_hf_u_keylen,out);                                    \
} while (0)
#endif // HASH_FIND_TSTR
#ifndef HASH_ADD_TSTR
#define HASH_ADD_TSTR(head,strfield,add)                                         \
do {                                                                             \
 unsigned _hf_u_keylen = lstrlen(add->strfield)*sizeof(TCHAR);                   \
 HASH_ADD(hh,head,strfield,_hf_u_keylen,add);                                    \
} while (0)
#endif // HASH_ADD_TSTR
// } // uthash

typedef struct _appid_list {
	UT_hash_handle hh;
	int nValue;
	WCHAR szKey[];
} APPID_LIST;

static BOOL ListAdd(APPID_LIST **ppList, WCHAR *pStr, int nValue);
static APPID_LIST *ListFind(APPID_LIST *pList, WCHAR *pStr);
static BOOL ListRemove(APPID_LIST **ppList, WCHAR *pStr);
static UINT ListCount(APPID_LIST *ppList);
static void ListFree(APPID_LIST **ppList);

static const WCHAR *szSectionNames[AILISTS_COUNT] = {
	L"Labeling",
	L"Grouping",
	L"Pinned grouping",
	L"Combining",
};
static APPID_LIST *appid_lists[AILISTS_COUNT];
static SRWLOCK SRWLock;

BOOL LoadAppidLists(void)
{
	PS_SECTION section;
	PS_FIND find;
	WCHAR szValueName[MAX_PATH];
	UINT uValueNameSize;
	int nValue;
	LSTATUS error;
	int i;

	for(i = 0; i < AILISTS_COUNT; i++)
	{
		error = PSOpenSection(szSectionNames[i], FALSE, &section);
		if(error == ERROR_SUCCESS)
		{
			error = PSFindInit(&section, &find);
			if(error == ERROR_SUCCESS)
			{
				do
				{
					uValueNameSize = MAX_PATH;

					error = PSFindNextInt(&section, &find, szValueName, &uValueNameSize, &nValue);
					if(error == ERROR_SUCCESS)
					{
						if(!ListAdd(&appid_lists[i], szValueName, nValue))
							error = ERROR_OUTOFMEMORY;
					}
					else if(error == ERROR_MORE_DATA)
						error = PSFindSkip(&section, &find);
				}
				while(error == ERROR_SUCCESS);

				if(error == ERROR_NO_MORE_ITEMS)
					error = ERROR_SUCCESS;

				PSFindClose(&section, &find);
			}

			PSCloseSection(&section);
		}

		if(error != ERROR_SUCCESS)
		{
			do
			{
				ListFree(&appid_lists[i]);
			}
			while(i--);

			return FALSE;
		}
	}

	InitializeSRWLock(&SRWLock);
	return TRUE;
}

void FreeAppidLists(void)
{
	int i;

	for(i = 0; i < AILISTS_COUNT; i++)
		ListFree(&appid_lists[i]);
}

BOOL AddAppidToList(int list, WCHAR *pStr, int nValue, BOOL bMemoryOnly)
{
	LSTATUS error;
	BOOL bSuccess;

	AcquireSRWLockExclusive(&SRWLock);

	if(ListAdd(&appid_lists[list], pStr, nValue))
	{
		if(!bMemoryOnly)
		{
			error = PSSetSingleInt(szSectionNames[list], pStr, nValue);
			bSuccess = (error == ERROR_SUCCESS);
		}
		else
			bSuccess = TRUE;
	}
	else
		bSuccess = FALSE;

	ReleaseSRWLockExclusive(&SRWLock);

	return bSuccess;
}

BOOL RemoveAppidFromList(int list, WCHAR *pStr, BOOL bMemoryOnly)
{
	LSTATUS error;
	BOOL bSuccess;

	AcquireSRWLockExclusive(&SRWLock);

	ListRemove(&appid_lists[list], pStr);

	if(!bMemoryOnly)
	{
		error = PSRemoveSingle(szSectionNames[list], pStr);
		bSuccess = (error == ERROR_SUCCESS || error == ERROR_FILE_NOT_FOUND);
	}
	else
		bSuccess = TRUE;

	ReleaseSRWLockExclusive(&SRWLock);

	return bSuccess;
}

BOOL GetAppidListValue(int list, WCHAR *pStr, int *pnValue)
{
	APPID_LIST *pListItem;
	BOOL bSuccess;

	AcquireSRWLockShared(&SRWLock);

	pListItem = ListFind(appid_lists[list], pStr);
	if(pListItem)
	{
		*pnValue = pListItem->nValue;
		bSuccess = TRUE;
	}
	else
		bSuccess = FALSE;

	ReleaseSRWLockShared(&SRWLock);

	return bSuccess;
}

BOOL IsAppidListEmpty(int list)
{
	BOOL bIsEmpty;

	AcquireSRWLockShared(&SRWLock);

	bIsEmpty = ListCount(appid_lists[list]) == 0;

	ReleaseSRWLockShared(&SRWLock);

	return bIsEmpty;
}

void ClearAppidLists(void)
{
	int i;

	AcquireSRWLockShared(&SRWLock);

	for(i = 0; i < AILISTS_COUNT; i++)
		ListFree(&appid_lists[i]);

	ReleaseSRWLockShared(&SRWLock);
}

static BOOL ListAdd(APPID_LIST **ppList, WCHAR *pStr, int nValue)
{
	APPID_LIST *pListItem;
	int len;

	HASH_FIND_TSTR(*ppList, pStr, pListItem);
	if(!pListItem)
	{
		len = lstrlen(pStr);

		pListItem = (APPID_LIST *)HeapAlloc(GetProcessHeap(), 0, sizeof(APPID_LIST) + (len + 1) * sizeof(WCHAR));
		if(!pListItem)
			return FALSE;

		lstrcpy(pListItem->szKey, pStr);
		pListItem->nValue = nValue;

		//HASH_ADD_TSTR(*ppList, szKey, pListItem);
		HASH_ADD(hh, *ppList, szKey, len * sizeof(WCHAR), pListItem); // we already know the length
	}
	else
		pListItem->nValue = nValue;

	return TRUE;
}

static APPID_LIST *ListFind(APPID_LIST *pList, WCHAR *pStr)
{
	APPID_LIST *pListItem;

	HASH_FIND_TSTR(pList, pStr, pListItem);

	return pListItem;
}

static BOOL ListRemove(APPID_LIST **ppList, WCHAR *pStr)
{
	APPID_LIST *pListItem;

	HASH_FIND_TSTR(*ppList, pStr, pListItem);
	if(!pListItem)
		return FALSE;

	HASH_DEL(*ppList, pListItem);
	HeapFree(GetProcessHeap(), 0, pListItem);

	return TRUE;
}

static UINT ListCount(APPID_LIST *ppList)
{
	return HASH_COUNT(ppList);
}

static void ListFree(APPID_LIST **ppList)
{
	APPID_LIST *pListItem, *pTemp;

	HASH_ITER(hh, *ppList, pListItem, pTemp)
	{
		HASH_DEL(*ppList, pListItem);
		HeapFree(GetProcessHeap(), 0, pListItem);
	}
}
