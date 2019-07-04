#pragma once

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <windows.h>

#include "buffered_reader.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _READLINE3 
{
	BUFFERED_READER*	br;
	char*				readbuf;
	WCHAR*				linebuf;
	DWORD				bufsize;
	UINT				codepage;
	BOOL				first;
} READLINE3;

__declspec(dllexport) READLINE3*	rl3_init(HANDLE fp, DWORD bufsize);
__declspec(dllexport) void			rl3_free(READLINE3* rl);
__declspec(dllexport) BOOL			rl3_next(_Inout_ READLINE3* rl, _Out_ LPWSTR* line, _Out_ DWORD* length);

#ifdef __cplusplus
}
#endif

