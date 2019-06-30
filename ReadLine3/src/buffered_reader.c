#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <windows.h>

#include "buffered_reader.h"

BUFFERED_READER* br_init(_In_ HANDLE fp, _In_ DWORD sizebytes)
{
	BUFFERED_READER* br = (BUFFERED_READER*)HeapAlloc(GetProcessHeap(), 0, sizeof(BUFFERED_READER));
	
	if (br != NULL) 
	{
		br->fp = fp;
		br->size = sizebytes;
		br->len = 0;
		br->readIdx = 0;
		br->buf = HeapAlloc(GetProcessHeap(), 0, sizebytes);
	}

	return br;
}
void br_free(_In_ BUFFERED_READER* br)
{
	HeapFree(GetProcessHeap(), 0, br->buf);
	HeapFree(GetProcessHeap(), 0, br);
}

BOOL br_fill_buffer(_Inout_ BUFFERED_READER* br)
{
	BOOL ok = ReadFile(
		br->fp
		, (LPVOID)br->buf
		, br->size
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

BOOL br_read(_Inout_ BUFFERED_READER* br, _Inout_ char* nextByte, _Out_ BOOL* eof)
{
	BOOL ok = ensure_buffer(br, eof);

	if ( ok && !*eof )
	{
		*nextByte = br->buf[br->readIdx];
		br->readIdx += 1;
	}

	return TRUE;
}

BOOL br_peek(_In_ const BUFFERED_READER* br, _In_ const DWORD offset, _Inout_ char* byte)
{
	BOOL ok;
	DWORD peekIdx = br->readIdx + offset;

	if (peekIdx < br->len)
	{
		*byte = br->buf[peekIdx];
		ok = TRUE;
	}
	else
	{
		ok = FALSE;
	}

	return ok;
}