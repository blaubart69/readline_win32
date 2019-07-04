#include "readline3.h"
#include "Misc.h"

READLINE3* rl3_init(HANDLE fp, DWORD bufsize)
{
	READLINE3* rl = (READLINE3*)HeapAlloc(GetProcessHeap(), 0, sizeof(READLINE3));

	if (rl != NULL)
	{
		rl->br = br_init(fp, bufsize);
		rl->readbuf = HeapAlloc(GetProcessHeap(), 0, bufsize);
		rl->linebuf = HeapAlloc(GetProcessHeap(), 0, ((SIZE_T)bufsize + 1) * sizeof(WCHAR) );
		rl->first = TRUE;
		rl->codepage = CP_ACP;
		rl->bufsize = bufsize;
	}

	return rl;
}
void rl3_free(READLINE3* rl)
{
	br_free(rl->br);
	HeapFree(GetProcessHeap(), 0, rl->readbuf);
	HeapFree(GetProcessHeap(), 0, rl->linebuf);
}

BOOL handlePossibleBOM(_Inout_ READLINE3* rl)
{
	br_fill_buffer(rl->br);

	BYTE lenBOM = 0;
	BOOL UTF16LEfound = FALSE;

	tryDetectBOM(rl->br->buf, rl->br->len, &rl->codepage, &lenBOM, &UTF16LEfound);

	BOOL ok = TRUE;

	for (int i = 0; i < lenBOM; ++i)
	{
		char dummy;
		BOOL eof;
		ok = br_read(rl->br, &dummy, &eof);
		if (!ok)
		{
			break;
		}
	}

	return ok;
}
_Success_(return)
BOOL fill_read_buffer_with_line(
	  _Inout_					BUFFERED_READER* br
	, __inout_bcount(bufsize)	char*	buf
	, _In_ const				DWORD	bufsize
	, _Outptr_					DWORD*	bytesWritten
	, _Outptr_					BOOL*	eof)
{
	BOOL ok;
	
	*eof = FALSE;
	*bytesWritten = 0;
	DWORD writeIdx = 0;

	for(;;)
	{
		char data;
		ok = br_read(br, &data, eof);

		if (!ok)
		{
			break;
		}
		if (*eof)
		{
			*eof = TRUE;
			break;
		}
		if (data == '\r')
		{
			continue;
		}
		if (data == '\n')
		{
			break;
		}
		if (writeIdx >= bufsize)
		{
			ok = FALSE;
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
			break;
		}
		buf[writeIdx++] = data;
	}

	*bytesWritten = writeIdx;

	return ok;
}
BOOL convert_readbuf_to_linebuf(_Inout_ READLINE3* rl, _In_ const DWORD bytesInReadBuffer, _Out_ int* widecharsWritten)
{
	*widecharsWritten = MultiByteToWideChar(
		rl->codepage					// CodePage 
		, MB_ERR_INVALID_CHARS			// dwFlags
		, rl->readbuf					// lpMultiByteStr
		, bytesInReadBuffer				// cbMultiByte 
		, rl->linebuf					// lpWideCharStr
		, rl->bufsize					// cchWideChar 
	);

	if (*widecharsWritten == 0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

_Success_(return)
BOOL rl3_next(_Inout_ READLINE3* rl, _Outptr_ LPWSTR* line, _Outptr_ DWORD* length)
{
	*line = rl->linebuf;
	//
	// detect BOM. set codepage
	//
	if (rl->first)
	{
		rl->first = FALSE;
		if (!handlePossibleBOM(rl))
		{
			return FALSE;
		}
	}
	//
	// read whole line to buffer. SKIP '\n', '\r'
	//
	BOOL eof;
	DWORD bytesInReadBuffer;
	if (!fill_read_buffer_with_line(rl->br, rl->readbuf, rl->bufsize, &bytesInReadBuffer, &eof))
	{
		return FALSE;
	}
	//
	// if no more bytes we reached EOF
	//
	if (bytesInReadBuffer == 0 && eof)
	{
		*line = NULL;
		*length = 0;
		return TRUE;
	}
	//
	// convert the line to WCHAR
	//
	int wideCharsWritten = 0;
	if (bytesInReadBuffer > 0)
	{
		if (!convert_readbuf_to_linebuf(rl, bytesInReadBuffer, &wideCharsWritten))
		{
			return FALSE;
		}
	}
	//
	// set trailing zero
	//
	*length = wideCharsWritten;
	rl->linebuf[wideCharsWritten] = L'\0';

	return TRUE;
}