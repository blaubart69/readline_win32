#pragma once

class LineReader
{
public:
	LineReader(HANDLE fp, int bufsize);
	virtual ~LineReader();
	
	DWORD readline(WCHAR ** line, DWORD * cchLen);

private:
	
	bool firstRead;
	bool convertToWidechar;
	UINT codepage;
	WCHAR* bufW;
	StreamTokenizer* tokenizer;

	DWORD tryDetectBOM();
	DWORD convertLineToWidechar(char * byteLine, DWORD cbLen, WCHAR ** line, DWORD * cchLen);
};

