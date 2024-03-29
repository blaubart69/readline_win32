#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _BUFFERED_READER {
	HANDLE	fp;
	DWORD	size;
	DWORD	len;
	DWORD	readIdx;
	char*	buf;
} BUFFERED_READER;

BUFFERED_READER*	br_init(_In_ HANDLE fp, _In_ DWORD size);
void				br_free(_In_ BUFFERED_READER* br);

BOOL	br_fill_buffer(_Inout_ BUFFERED_READER* br);
BOOL	br_read(_Inout_ BUFFERED_READER* br, _Inout_ char* nextByte, _Out_ BOOL* eof);
//BOOL	br_peek(_In_ const BUFFERED_READER* br, _In_ const DWORD offset, _Inout_ char* byte);

#ifdef __cplusplus
}
#endif