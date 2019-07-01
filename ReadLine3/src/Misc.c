#pragma once

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <windows.h>

#include "Misc.h"

void tryDetectBOM(
	_In_	const BYTE* buf,
	_In_	DWORD bufLen,
	_Inout_ UINT* codepage,
	_Out_	BYTE* lenBOM,
	_Out_	BOOL* UTF16LEfound)
{
	*lenBOM = 0;
	*UTF16LEfound = FALSE;

	if (bufLen >= 2)
	{
		// UTF16 LE ... 0xFF 0xFE
		if (   buf[0] == 0xFF
			&& buf[1] == 0xFE)
		{
			*lenBOM = 2;
			*UTF16LEfound = TRUE;
		}
		else if (bufLen >= 3)
		{
			// UTF8 ... 0xEF,0xBB,0xBF
			if (   buf[0] == 0xEF
				&& buf[1] == 0xBB
				&& buf[2] == 0xBF)
			{
				*codepage = CP_UTF8;
				*lenBOM = 3;
			}
		}
	}
}

BOOLEAN WINAPI
_DllMainCRTStartup(IN HINSTANCE hDllHandle, IN DWORD nReason, IN LPVOID Reserved)
{
	return TRUE;
}
