//**********************************************************************
// File: buffer.h
//
// Custom Buffer-Manipulation Routines
//
// By RaMMicHaeL, 12.10.2009
//**********************************************************************

#ifndef _BUFFER_H_
#define _BUFFER_H_

#ifdef FillMemory
#undef FillMemory
#endif
#ifdef ZeroMemory
#undef ZeroMemory
#endif
#ifdef CopyMemory
#undef CopyMemory
#endif
#ifdef MoveMemory
#undef MoveMemory
#endif

__forceinline
void FillMemory(PVOID Destination, SIZE_T Length, BYTE Fill)
{
	volatile SIZE_T *sizet_p;
	volatile unsigned char *uchar_p;
	SIZE_T sizet_fill;
	SIZE_T count;

	sizet_p = (SIZE_T *)Destination;

	count = Length/sizeof(SIZE_T);
	if(count)
	{
		sizet_fill = Fill;
		sizet_fill |= sizet_fill << 8;
		sizet_fill |= sizet_fill << 16;

#ifdef _WIN64
		sizet_fill |= sizet_fill << 32;
#endif

		do {
			*(sizet_p++) = sizet_fill;
		} while(--count);
	}

	uchar_p = (unsigned char *)sizet_p;

	count = Length & (sizeof(SIZE_T)-1);
	if(count)
	{
		do
			*(uchar_p++) = Fill;
		while(--count);
	}
}

#define ZeroMemory(Destination, Length) FillMemory(Destination, Length, 0)

__forceinline
void CopyMemory(PVOID Destination, const VOID *Source, SIZE_T Length)
{
	volatile SIZE_T *sizet_p_dest, *sizet_p_src;
	volatile unsigned char *uchar_p_dest, *uchar_p_src;
	SIZE_T count;

	sizet_p_dest = (SIZE_T *)Destination;
	sizet_p_src = (SIZE_T *)Source;

	count = Length/sizeof(SIZE_T);
	if(count)
	{
		do {
			*(sizet_p_dest++) = *(sizet_p_src++);
		} while(--count);
	}

	uchar_p_dest = (unsigned char *)sizet_p_dest;
	uchar_p_src = (unsigned char *)sizet_p_src;

	count = Length & (sizeof(SIZE_T)-1);
	if(count)
	{
		do {
			*(uchar_p_dest++) = *(uchar_p_src++);
		} while(--count);
	}
}

__forceinline
void MoveMemory(PVOID Destination, const VOID *Source, SIZE_T Length)
{
	volatile SIZE_T *sizet_p_dest, *sizet_p_src;
	volatile unsigned char *uchar_p_dest, *uchar_p_src;
	SIZE_T count;

	if(Destination > Source)
	{
		// Copy in reverse

		uchar_p_dest = (unsigned char *)Destination+Length;
		uchar_p_src = (unsigned char *)Source+Length;

		count = Length & (sizeof(SIZE_T)-1);
		if(count)
		{
			do {
				*(--uchar_p_dest) = *(--uchar_p_src);
			} while(--count);
		}

		sizet_p_dest = (SIZE_T *)uchar_p_dest;
		sizet_p_src = (SIZE_T *)uchar_p_src;

		count = Length/sizeof(SIZE_T);
		if(count)
		{
			do {
				*(--sizet_p_dest) = *(--sizet_p_src);
			} while(--count);
		}
	}
	else
		CopyMemory(Destination, Source, Length);
}

#endif  // _BUFFER_H_
