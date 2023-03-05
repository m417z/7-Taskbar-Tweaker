#include "stdafx.h"
#include "prevent_explorer_appid_changes.h"
#include "pointer_redirection.h"
#include "functions.h"

static MODULEINFO ExplorerFrameInfo;
static MODULEINFO Shell32Info;
static volatile LONG nHookCallCounter;

static void **ppPropertyStoreSetValue;
POINTER_REDIRECTION_VAR(static POINTER_REDIRECTION prPropertyStoreSetValue);

static HRESULT STDMETHODCALLTYPE PropertyStoreSetValueHook(IPropertyStore *This, REFPROPERTYKEY key, REFPROPVARIANT propvar);

BOOL PreventExplorerAppidChanges_Init(void)
{
	if(!GetModuleInformation(GetCurrentProcess(), GetModuleHandle(L"explorerframe.dll"), &ExplorerFrameInfo, sizeof(MODULEINFO)))
		return FALSE;

	if(!GetModuleInformation(GetCurrentProcess(), GetModuleHandle(L"shell32.dll"), &Shell32Info, sizeof(MODULEINFO)))
		return FALSE;

	IPropertyStore *pps;
	HRESULT hr = SHGetPropertyStoreForWindow(GetDesktopWindow(), &IID_IPropertyStore, (void **)&pps);
	if(FAILED(hr))
		return FALSE;

	ppPropertyStoreSetValue = (void *)&pps->lpVtbl->SetValue;

	PointerRedirectionAdd(ppPropertyStoreSetValue, PropertyStoreSetValueHook, &prPropertyStoreSetValue);

	pps->lpVtbl->Release(pps);

	return TRUE;
}

void PreventExplorerAppidChanges_Exit(void)
{
	if(ppPropertyStoreSetValue)
	{
		PointerRedirectionRemove(ppPropertyStoreSetValue, &prPropertyStoreSetValue);
	}
}

void PreventExplorerAppidChanges_WaitTillDone(void)
{
	while(nHookCallCounter > 0)
		Sleep(10);
}

static HRESULT STDMETHODCALLTYPE PropertyStoreSetValueHook(IPropertyStore *This, REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
	BOOL bPreventChange = FALSE;
	HRESULT hrRet;

	InterlockedIncrement(&nHookCallCounter);

	void *pRetAddr = _ReturnAddress();
	if(
		(
			(ULONG_PTR)pRetAddr >= (ULONG_PTR)ExplorerFrameInfo.lpBaseOfDll &&
			(ULONG_PTR)pRetAddr < (ULONG_PTR)ExplorerFrameInfo.lpBaseOfDll + ExplorerFrameInfo.SizeOfImage
		) ||
		(
			(ULONG_PTR)pRetAddr >= (ULONG_PTR)Shell32Info.lpBaseOfDll &&
			(ULONG_PTR)pRetAddr < (ULONG_PTR)Shell32Info.lpBaseOfDll + Shell32Info.SizeOfImage
		)
	)
	{
		PROPVARIANT pv;
		HRESULT hr = This->lpVtbl->GetValue(This, &PKEY_AppUserModel_ID, &pv);
		if(SUCCEEDED(hr))
		{
			if(pv.vt == VT_LPWSTR)
			{
				if(IsAppIdARandomGroup(pv.pwszVal))
					bPreventChange = TRUE;
			}

			PropVariantClear(&pv);
		}
	}

	if(!bPreventChange)
	{
		hrRet = ((HRESULT(STDMETHODCALLTYPE *)(IPropertyStore *This, REFPROPERTYKEY key, REFPROPVARIANT propvar))prPropertyStoreSetValue.pOriginalAddress)(This, key, propvar);
	}
	else
	{
		hrRet = S_OK;
	}

	InterlockedDecrement(&nHookCallCounter);

	return hrRet;
}
