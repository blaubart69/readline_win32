#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void tryDetectBOM(
	_In_	const BYTE* buf,
	_In_	DWORD bufLen,
	_Inout_ UINT* codepage,
	_Out_	BYTE* lenBOM,
	_Out_	BOOL* UTF16LEfound);


#ifdef __cplusplus
}
#endif