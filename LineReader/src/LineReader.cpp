#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <Windows.h>

#include "streamtokenizer.h"
#include "LineReader.h"

LineReader::LineReader(HANDLE fp, int bufsize)
{
	tokenizer = new StreamTokenizer(fp, bufsize, '\n');
	firstRead = true;
	convertToWidechar = true;
	codepage = 0;
	bufW = nullptr;
}

LineReader::~LineReader()
{
	delete tokenizer;

	if (bufW != nullptr)
	{
		delete[] bufW;
	}
}

DWORD LineReader::tryDetectBOM()
{
	BOM bom;
	DWORD rc = tokenizer->readBOM(&bom);

	codepage = CP_ACP;
	convertToWidechar = true;

	if (rc == 0)
	{
		if (bom == BOM::UTF8)
		{
			codepage = CP_UTF8;
		}
		else if (bom == BOM::UTF16LE)
		{
			convertToWidechar = false;
		}
	}

	return rc;
}

DWORD LineReader::convertLineToWidechar(char* byteLine, DWORD cbLen, WCHAR** wLine, DWORD* cchLen)
{
	DWORD rc = 0;

	if (convertToWidechar)
	{
		
		if (cbLen == 0)
		{
			bufW[0] = L'\0';
			cchLen = 0;
		}
		else
		{
			int widecharsWritten;
			if ((widecharsWritten = MultiByteToWideChar(
				codepage					// CodePage 
				, 0							// dwFlags
				, byteLine					// lpMultiByteStr
				, cbLen						// cbMultiByte 
				, bufW   					// lpWideCharStr
				, tokenizer->getBufsize()	// cchWideChar 
			)) == 0)
			{
				rc = GetLastError();
				*cchLen = 0;
			}
			else
			{
				*cchLen = widecharsWritten;
				bufW[widecharsWritten] = L'\0';
			}
		}
		*wLine = bufW;
	}
	else
	{
		*wLine = (WCHAR*)byteLine;
		*cchLen = cbLen / 2;
	}

	if (*cchLen > 0)
	{
		DWORD lastIdx = *cchLen - 1;
		if ((*wLine)[lastIdx] == L'\r')
		{
			(*wLine)[lastIdx] = L'\0';
			*cchLen -= 1;
		}
	}

	return rc;
}

DWORD LineReader::readline(WCHAR** line, DWORD* cchLen)
{
	DWORD rc = 0;

	if (firstRead)
	{
		firstRead = false;
		rc = tryDetectBOM();
		if (convertToWidechar)
		{
			bufW = new WCHAR[tokenizer->getBufsize()];
		}
	}

	if (rc == 0)
	{
		char* byteLine;
		DWORD cbLen;
		rc = tokenizer->next(&byteLine, &cbLen);
		if (rc == 0)
		{
			if (byteLine == NULL)
			{
				*line = NULL;
				*cchLen = 0;
			}
			else
			{
				rc = convertLineToWidechar(byteLine, cbLen, line, cchLen);
			}
		}
	}

	return rc;
}
