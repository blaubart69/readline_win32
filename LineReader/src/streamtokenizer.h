#pragma once

enum BOM 
{ 
	NO
	, UTF8
	, UTF16LE
};

class StreamTokenizer
{
public:
	
	StreamTokenizer(HANDLE fp, int bufsize, char separator);
	~StreamTokenizer();

	DWORD next(char ** token, DWORD* cbLen);
	DWORD readBOM(BOM* bom);
	bool endOfStream() const;
	DWORD getBufsize() const { return bufsize; };

private:

	const DWORD bufsize;
	const HANDLE fp;
	const char separator;

	char* buf;
	DWORD buflen;
	DWORD readIdx;

	int fillBuffer(DWORD offset);
	bool hasData() const;
	void setEOF();
	DWORD ensureBuffer();
	DWORD bytesLeftToRead() const;
	DWORD noSeparatorFound(bool *searchSeparator, DWORD* endIdx);
	DWORD readToken(char ** token, DWORD* cbLen);

};

