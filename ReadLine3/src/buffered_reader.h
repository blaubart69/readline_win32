#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _br {
	HANDLE	fp;
	DWORD	size;
	DWORD	len;
	DWORD	readIdx;
	char* buf;
} BUFFERED_READER;

BUFFERED_READER*	br_init(_In_ HANDLE fp, _In_ DWORD size);
void				br_free(_In_ BUFFERED_READER* br);

BOOL br_read(_Inout_ BUFFERED_READER* br, _Out_ char* nextByte, _Out_ BOOL* eof);

#ifdef __cplusplus
}
#endif