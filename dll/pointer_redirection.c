#include "stdafx.h"
#include "pointer_redirection.h"
#include "functions.h"

void PointerRedirectionAdd(void **pp, void *pNew, POINTER_REDIRECTION *ppr)
{
	PatchPtr(&ppr->pOriginalAddress, *pp);
	PatchPtr(&ppr->pRedirectionAddress, pNew);

	PatchPtr(pp, &ppr->bAsmCommand);
}

void PointerRedirectionRemove(void **pp, POINTER_REDIRECTION *ppr)
{
	POINTER_REDIRECTION *pprTemp;

	if(*pp != ppr->bAsmCommand)
	{
		pprTemp = (POINTER_REDIRECTION *)((BYTE *)*pp - offsetof(POINTER_REDIRECTION, bAsmCommand));
		while(pprTemp->pOriginalAddress != ppr->bAsmCommand)
			pprTemp = (POINTER_REDIRECTION *)((BYTE *)pprTemp->pOriginalAddress - offsetof(POINTER_REDIRECTION, bAsmCommand));

		PatchPtr(&pprTemp->pOriginalAddress, ppr->pOriginalAddress);
	}
	else
		PatchPtr(pp, ppr->pOriginalAddress);
}

void *PointerRedirectionGetOriginalPtr(void **pp)
{
	void *p = *pp;

	for(;;)
	{
		const BYTE *pCompare = POINTER_REDIRECTION_ASM_COMMAND POINTER_REDIRECTION_SIGNATURE;
		int nCompareLen = sizeof(POINTER_REDIRECTION_ASM_COMMAND POINTER_REDIRECTION_SIGNATURE) - 1;
		BYTE *pByte = p;

		for(int i = 0; i < nCompareLen; i++)
		{
			if(pByte[i] != pCompare[i])
				return p;
		}

		POINTER_REDIRECTION *pprTemp = (POINTER_REDIRECTION *)(pByte - offsetof(POINTER_REDIRECTION, bAsmCommand));
		p = pprTemp->pOriginalAddress;
	}
}
