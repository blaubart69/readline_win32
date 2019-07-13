#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <windows.h>

#include "buffered_reader.h"

BUFFERED_READER* br_init(_In_ HANDLE fp, _In_ DWORD sizebytes)
{
	SIZE_T toAlloc =
		sizeof(BUFFERED_READER)
		+ sizebytes - 1;	// cause struct has one byte "char buf[1]"

	BUFFERED_READER* br = (BUFFERED_READER*)HeapAlloc(GetProcessHeap(), 0, toAlloc);
	
	if (br != NULL) 
	{
		br->fp = fp;
		br->capacity = sizebytes;
		br->len = 0;
		br->readIdx = 0;
	}

	return br;
}
BUFFERED_READER* br_initEx(_In_ HANDLE fp, _In_ LPVOID mem, _In_ DWORD memLen)
{
	DWORD sizeStruct = sizeof(BUFFERED_READER);

	if (memLen < sizeStruct)
	{
		return NULL;
	}

	BUFFERED_READER* br = mem;

	if (br != NULL)
	{
		br->fp = fp;
		br->len = 0;
		br->readIdx = 0;
		br->capacity = memLen - sizeStruct + 1;
	}

	return br;
}
void br_free(_In_ BUFFERED_READER* br)
{
	HeapFree(GetProcessHeap(), 0, br);
}

BOOL br_fill_buffer(_Inout_ BUFFERED_READER* br)
{
	BOOL ok = ReadFile(
		br->fp
		, (LPVOID)br->buf
		, br->capacity
		, &br->len
		, NULL);

	if ( ok )
	{
		br->readIdx = 0;
	}

	return ok;
}

static BOOL ensure_buffer(_Inout_ BUFFERED_READER* br, _Out_ BOOL* eof)
{
	*eof = FALSE;
	BOOL ok = TRUE;

	if (br->readIdx == br->len)
	{
		if (!br_fill_buffer(br))
		{
			ok = FALSE;
		}

		if (br->len == 0)
		{
			*eof = TRUE;
		}
	}

	return ok;
}

BOOL br_read(_Inout_ BUFFERED_READER* br, _Inout_ char* nextByte, _Outptr_ BOOL* eof)
{
	BOOL ok = ensure_buffer(br, eof);

	if ( ok && !*eof )
	{
		*nextByte = br->buf[br->readIdx];
		br->readIdx += 1;
	}

	return TRUE;
}
