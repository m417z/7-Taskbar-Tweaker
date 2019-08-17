#pragma once

#ifdef _WIN64
#define POINTER_REDIRECTION_ASM_COMMAND "\xFF\x25\xF2\xFF\xFF\xFF"
#else
#define POINTER_REDIRECTION_ASM_COMMAND "\xE8\x00\x00\x00\x00\x58\xFF\x60\xF7"
#endif

#define POINTER_REDIRECTION_SIGNATURE "ptr_redr"

typedef struct {
	void *pOriginalAddress;
	void *pRedirectionAddress;
	BYTE bAsmCommand[sizeof(POINTER_REDIRECTION_ASM_COMMAND) - 1];
	BYTE bSignature[sizeof(POINTER_REDIRECTION_SIGNATURE) - 1];
} POINTER_REDIRECTION;

#define POINTER_REDIRECTION_VAR(var) \
	__pragma(code_seg(push, stack1, ".text")) \
	__declspec(allocate(".text")) var = \
		{ DebugBreak, DebugBreak, POINTER_REDIRECTION_ASM_COMMAND, POINTER_REDIRECTION_SIGNATURE }; \
	__pragma(code_seg(pop, stack1))

void PointerRedirectionAdd(void **pp, void *pNew, POINTER_REDIRECTION *ppr);
void PointerRedirectionRemove(void **pp, POINTER_REDIRECTION *ppr);
void *PointerRedirectionGetOriginalPtr(void **pp);
