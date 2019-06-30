//#include "readline/readline.h"
#include "readline.h"
//#include "../pch.h"

#undef RtlMoveMemory
__declspec(dllimport) void __stdcall RtlMoveMemory(void *dst, const void *src, size_t len);

READLINE* rl_new(_In_ const HANDLE handle, _In_ const DWORD buffersize)
{
	READLINE* rl = (READLINE*)HeapAlloc(GetProcessHeap(), 0, sizeof(READLINE));

	rl->handle = handle;
	rl->codepage = CP_ACP;
	rl->convertToCodepage = TRUE;
	rl->firstRead = TRUE;

	rl->bufSize = buffersize;
	rl->cchLenBuf = 0;

	rl->readBuffer = (char*)HeapAlloc(GetProcessHeap(), 0, rl->bufSize);
	rl->lineBuffer = NULL;
	
	rl->readIdx = 0;

	return rl;
}
void rl_delete(_Inout_ READLINE* rl)
{
	HeapFree(GetProcessHeap(), 0, rl->readBuffer);
	if (rl->lineBuffer != NULL)
	{
		HeapFree(GetProcessHeap(), 0, rl->lineBuffer);
	}
	HeapFree(GetProcessHeap(), 0, rl);
}

/*
#ifdef _DEBUG
void* memchr(const void *s, int c, size_t n)
{
	const unsigned char*  p = (const unsigned char*)s;
	const unsigned char*  end = p + n;
	for (;;) {
		if (p >= end || p[0] == c) break; p++;
		if (p >= end || p[0] == c) break; p++;
		if (p >= end || p[0] == c) break; p++;
		if (p >= end || p[0] == c) break; p++;
	}
	if (p >= end)
		return NULL;
	else
		return (void*)p;
}
#endif 
*/
static BOOL isEOF(_In_ const READLINE* rl)
{
	return rl->readIdx > (rl->cchLenBuf - 1);
}
static int fillBuffer(READLINE* rl, _Out_ DWORD* bytesRead)
{
	int rc;

	DWORD bytesToRead = rl->bufSize - rl->cchLenBuf;

	BOOL ok = ReadFile(
		rl->handle
		, rl->readBuffer + rl->cchLenBuf
		, bytesToRead
		, bytesRead
		, NULL);

	if (!ok)
	{
		rc = GetLastError();
	}
	else
	{
		rl->cchLenBuf += *bytesRead;
		rc = 0;
	}

	return rc;
}
static DWORD replaceNewlineWithZeroUTF16(_Inout_ WCHAR* buf, _In_ DWORD lastIdx)
{
	if ( buf[lastIdx] == L'\n' )
	{
		if (buf[lastIdx-1] == L'\r')
		{
			--lastIdx;		// set to	 \r
		}

		buf[lastIdx] = L'\0';
		--lastIdx;			// set to last character
	}
	else
	{
		buf[lastIdx + 1] = L'\0';
	}

	return lastIdx;
}
static DWORD calcMultibyteLenWithoutNewline(_In_ const char* buf, _In_ const DWORD startIdx, _In_ const DWORD endIdx)
{
	DWORD len;

	if (buf[endIdx] == '\n')
	{
		if ( buf[endIdx - 1] == '\r')
		{
			len = (endIdx - 1) - startIdx;
		}
		else
		{
			len = (endIdx - 0) - startIdx;
		}
	}
	else
	{
		    len = (endIdx + 1) - startIdx;
	}

	return len;
}
static DWORD reportLineFromTo(_Inout_ READLINE* rl, _In_ DWORD newlineIdx, _Out_ LPWSTR* line, _Out_ DWORD* cchLen)
{
	DWORD rc = 0;

	if (rl->convertToCodepage)
	{
		int widecharsWritten;
		*line = rl->lineBuffer;

		DWORD cbMultiByte = calcMultibyteLenWithoutNewline(rl->readBuffer, rl->readIdx, newlineIdx);

		if (cbMultiByte == 0)
		{
			widecharsWritten = 0;
		}
		else
		{
			if ((widecharsWritten = MultiByteToWideChar(
				rl->codepage						// CodePage 
				, 0									// dwFlags
				, &(rl->readBuffer[rl->readIdx])	// lpMultiByteStr
				, cbMultiByte						// cbMultiByte 
				, rl->lineBuffer					// lpWideCharStr
				, rl->bufSize						// cchWideChar 
			)) == 0)
			{
				rc = GetLastError();
				*cchLen = 0;
				//Log::Instance()->win32err(L"MultiByteToWideChar", L"reportLineFromTo()");
			}
		}
		if (rc == 0)
		{
			rl->lineBuffer[widecharsWritten] = L'\0';
			*cchLen = widecharsWritten;
		}
	}
	else
	{
		*line = &( ((WCHAR*)rl->readBuffer)[rl->readIdx] );
		DWORD lastcharIdx = replaceNewlineWithZeroUTF16(rl->readBuffer, newlineIdx);

		if (lastcharIdx < rl->readIdx)
		{
			*cchLen = 0;
		}
		else
		{
			*cchLen = lastcharIdx - rl->readIdx + 1;
		}
	}

	return rc;
}
static DWORD remainingBytesInBuffer(DWORD readIdx, DWORD bufLen)
{
	return bufLen - readIdx;
}
static void MoveRemainingDataToBeginOfBuffer(_Inout_ READLINE * rl)
{
	DWORD remainingByteLen = remainingBytesInBuffer(rl->readIdx, rl->cchLenBuf);

	RtlMoveMemory(
		rl->readBuffer
		, &(rl->readBuffer[rl->readIdx])
		, remainingByteLen);

	rl->readIdx = 0;
	rl->cchLenBuf = remainingByteLen;
}
static DWORD handleNoNewLine(_Inout_ READLINE* rl, _Out_ BOOL* searchNewline, _Out_ LPWSTR* line, _Out_ DWORD* cchLen)
{
	DWORD rc = 0;

	if (rl->cchLenBuf < rl->bufSize)
	{
		// buffer is not full and we have no \n --> must be the last line WITHOUT \n 
		DWORD lastIdx = rl->cchLenBuf - 1;
		rc = reportLineFromTo(rl, lastIdx, line, cchLen);
		rl->readIdx = rl->cchLenBuf;			// readIdx beyond buffer to signal EOF
		*searchNewline = FALSE;
	}
	else if (rl->cchLenBuf == rl->bufSize)
	{
		if (rl->readIdx > 0)   
		{
			MoveRemainingDataToBeginOfBuffer(rl);
			DWORD bytesRead;
			rc = fillBuffer(rl, &bytesRead);
			*searchNewline = TRUE;
		}
		else
		{
			// no newline char AND buffer is full
			*line = NULL;
			*cchLen = 0;
			rc = ERROR_INSUFFICIENT_BUFFER;
			*searchNewline = FALSE;
		}
	}
	else
	{
		// bufLen > bufSize ==> BAD!
		rc = 998;
	}

	return rc;
}
static DWORD findNewlineIdx(const char* buf, DWORD startIdx, DWORD bufLen, BOOL isUTF16)
{
	DWORD remainingBytes = remainingBytesInBuffer(startIdx, bufLen);

	char* newLineChar = memchr(
		&(buf[startIdx])
		, '\n'
		, remainingBytes);

	DWORD newlineIdx;
	if (newLineChar == NULL)
	{
		newlineIdx = 0;
	}
	else
	{
		newlineIdx = newLineChar - buf;

		if (isUTF16)
		{
			newlineIdx = newlineIdx / 2;
		}
	}

	return newlineIdx;
}
static DWORD reportLine(_Inout_ READLINE* rl, _Out_ LPWSTR* line, _Out_ DWORD* cchLen)
{
	DWORD rc = 0;
	*cchLen = 0;
	*line = NULL;
	BOOL searchNewLine = TRUE;

	while (rc == 0 && searchNewLine)
	{
		DWORD newlineIdx = findNewlineIdx(rl->readBuffer, rl->readIdx, rl->cchLenBuf, !rl->convertToCodepage);
		if (newlineIdx == 0)
		{
			rc = handleNoNewLine(rl, &searchNewLine, line, cchLen);
		}
		else
		{
			rc = reportLineFromTo(rl, newlineIdx, line, cchLen);
			rl->readIdx = newlineIdx + 1;
			if ( isEOF(rl) )
			{
				rl->readIdx = 0;
				rl->cchLenBuf = 0;
			}
			searchNewLine = FALSE;
		}
	} 

	return rc;
}

static void tryDetectBOM(_In_ const unsigned char* buf, _In_ DWORD bufLen, _Inout_ UINT* codepage, _Out_ BYTE* lenBOM, _Out_ BOOL* UTF16found)
{
	*lenBOM = 0;

	if (bufLen >= 2)
	{
		// UTF16 LE ... 0xFF 0xFE
		if (   buf[0] == 0xFF 
			&& buf[1] == 0xFE)
		{
			*lenBOM = 2;
			*UTF16found = TRUE;
		}
		else if (bufLen >= 3)
		{
			// UTF8 ... 0xEF,0xBB,0xBF
			if (buf[0] == 0xEF
				&& buf[1] == 0xBB
				&& buf[2] == 0xBF)
			{
				*codepage = CP_UTF8;
				*lenBOM = 3;
			}
		}
	}
}

static void handleFirstRead(_Inout_ READLINE* rl, _Out_ BYTE* lenBOM)
{
	BOOL UTF16found = FALSE;

	tryDetectBOM((unsigned char*)rl->readBuffer, rl->cchLenBuf, &rl->codepage, lenBOM, &UTF16found);
	rl->readIdx += *lenBOM;

	if (UTF16found)
	{
		rl->convertToCodepage = FALSE;
		//rl->charSize = 2;
		rl->readIdx = 1;
	}
	else
	{
		//rl->charSize = 1;
		rl->readIdx += *lenBOM;
		// create a buffer for line conversions
		rl->lineBuffer = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)rl->bufSize * 2);
	}
}
DWORD rl_readline(_Inout_ READLINE* rl, _Out_ LPWSTR* line, _Out_ DWORD* cchLen)
{
	DWORD rc = 0;

	if (rl->cchLenBuf > 0 && isEOF(rl) )
	{
		// we enter ReadLine and the readPointer is past the bufLen.
		// means there is no more data.
		// the last call to readLine should return NULL for line
		*line = NULL;
		*cchLen = 0;
	}
	else
	{
		if (rl->cchLenBuf == 0)
		{
			DWORD bytesRead;
			rc = fillBuffer(rl, &bytesRead);
			if (rc == 0)
			{
				if (rl->firstRead)
				{
					rl->firstRead = FALSE;
					BYTE lenBOM;
					handleFirstRead(rl, &lenBOM);
					if (bytesRead == lenBOM)
					{
						bytesRead = 0;	// set *line to NULL, ccLen to 0
						rl->cchLenBuf = 0; // to skip reportLine()
					}
				}

				if (bytesRead == 0)
				{
					// the previous fillBuffer filled the buffer exactly with the last bytes of the file
					// so the last call gives us 0 bytesRead
					*line = NULL;
					*cchLen = 0;
				}
			}
		}


		if (rc == 0 && rl->cchLenBuf > 0)
		{
			rc = reportLine(rl, line, cchLen);
		}
	}

	return rc;
}