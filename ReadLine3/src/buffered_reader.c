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

//BOOL fill_buffer(_In_ HANDLE fp, _Inout_ char* buf, _In_ DWORD size, _Out_ DWORD* len)
BOOL fill_buffer(_Inout_ BUFFERED_READER* br)
{
	BOOL ok = ReadFile(
		br->fp
		, (LPVOID)br->buf
		, br->size
		, &br->len
		, NULL);

	if ( ok && br->len > 0 )
	{
		br->readIdx = 0;
	}

	return ok;
}

BOOL br_read(_Inout_ BUFFERED_READER* br, _Out_ char* nextByte, _Out_ BOOL* eof)
{
	*eof = FALSE;

	if (br->readIdx == br->len)
	{
		if (!fill_buffer(br))
		{
			return FALSE;
		}

		if (br->len == 0)
		{
			*eof = TRUE;
		}
	}

	if ( !*eof )
	{
		*nextByte = br->buf[br->readIdx];
		br->readIdx += 1;
	}

	return TRUE;
}
