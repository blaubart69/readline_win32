#pragma once

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _readline
{
	HANDLE handle;
	UINT codepage;
	BOOL convertToCodepage;

	DWORD bufSize;
	DWORD cchLenBuf;

	char* readBuffer;
	DWORD readIdx;
	
	BOOL   firstRead;
	WCHAR* lineBuffer;
	//BYTE   charSize;

} READLINE;

READLINE* rl_new     (_In_ const HANDLE handle, _In_ const DWORD buffersize);
void      rl_delete  (_In_ READLINE* rl);
DWORD     rl_readline(_Inout_ READLINE * rl, _Out_ LPWSTR * line, _Out_ DWORD* cchLen);

//static void MoveRemainingDataToBeginOfBuffer(READLINE * rl);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class Readline {
public:
	Readline(HANDLE handle, DWORD buffersize)
	{
		rl = rl_new(handle, buffersize);
		line = NULL;
		cchLen = 0;
	}
	~Readline()
	{

		rl_delete(rl);
	}
	DWORD     nextLine()
	{
		return rl_readline(
			this->rl
			, &this->line
			, &this->cchLen);
	}

	LPWSTR line;
	DWORD  cchLen;

private:
	READLINE* rl;
};
#endif
