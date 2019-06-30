#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used 
#define STRICT
#include <Windows.h>

#include "streamtokenizer.h"

#undef RtlMoveMemory
extern "C" __declspec(dllimport) void __stdcall RtlMoveMemory(void *dst, const void *src, size_t len);

int StreamTokenizer::fillBuffer(DWORD offset)
{
	int rc;

	DWORD bytesRead;
	DWORD bytesToRead = this->bufsize - offset;

	BOOL ok = ReadFile(
		this->fp
		, this->buf + offset
		, bytesToRead
		, &bytesRead
		, NULL);

	if (ok)
	{
		rc = 0;

		if (bytesRead == 0)
		{
			setEOF();
		}
		else
		{
			this->buflen += bytesRead;
		}
	}
	else
	{
		rc = GetLastError();
	}

	return rc;
}

StreamTokenizer::StreamTokenizer(HANDLE fp, int bufsize, char token)
	: bufsize(bufsize)
	, fp(fp)
	, separator(token)
{
	this->buf = new char[bufsize];
	this->buflen = 0;
	this->readIdx = 0;
}

StreamTokenizer::~StreamTokenizer()
{
	delete[] this->buf;
}

DWORD StreamTokenizer::readBOM(BOM * bom)
{
	*bom = BOM::NO;

	DWORD rc = ensureBuffer();

	unsigned char* uBuf = (unsigned char*)buf;

	if (rc == 0)
	{
		if (buflen >= 2)
		{
			// UTF16 LE ... 0xFF 0xFE
			if (   uBuf[0] == 0xFF
				&& uBuf[1] == 0xFE)
			{
				*bom = BOM::UTF16LE;
				this->readIdx = 2;
			}
			else if (buflen >= 3)
			{
				// UTF8 ... 0xEF,0xBB,0xBF
				if (   uBuf[0] == 0xEF
					&& uBuf[1] == 0xBB
					&& uBuf[2] == 0xBF)
				{
					*bom = BOM::UTF8;
					this->readIdx = 3;
				}
			}
		}
	}

	return rc;
}

void StreamTokenizer::setEOF()
{
	readIdx = buflen;
}

bool StreamTokenizer::endOfStream() const
{
	return readIdx >= buflen;
}

bool StreamTokenizer::hasData() const
{
	return buflen > 0 && !endOfStream();
}


DWORD StreamTokenizer::ensureBuffer()
{
	DWORD rc;

	if (!hasData())
	{
		rc = fillBuffer(0);
	}
	else
	{
		rc = 0;
	}

	return rc;
}

DWORD StreamTokenizer::bytesLeftToRead() const
{
	return buflen - readIdx + 1;
}

DWORD StreamTokenizer::noSeparatorFound(bool *searchSeparator, DWORD* cbLen)
{
	DWORD rc = 0;

	*searchSeparator = false;

	if (buflen < bufsize)
	{
		// last token without separator
		//*token = buf + readIdx;
		//*cbLen = buflen - readIdx;
		buf[buflen] = '\0';
		*cbLen = buflen - readIdx;
	}
	else if (buflen == bufsize)
	{
		if (readIdx > 0)
		{
			DWORD bytesToMove = this->bytesLeftToRead();
			RtlMoveMemory(buf, &buf[readIdx], bytesToMove);
			buflen = bytesToMove;
			readIdx = 0;
			rc = fillBuffer(bytesToMove);
			*searchSeparator = true;
		}
		else
		{
			rc = ERROR_INSUFFICIENT_BUFFER;
		}
	}
	else
	{
		rc = 999;
	}

	return rc;
}

DWORD StreamTokenizer::readToken(char ** token, DWORD* cbLen)
{
	DWORD rc = 0;
	bool searchSeparator = true;

	while (rc == 0 && searchSeparator)
	{
		char* foundSeparator = (char*)memchr(&buf[readIdx], this->separator, bytesLeftToRead());
		if (foundSeparator == NULL)
		{
			rc = noSeparatorFound(&searchSeparator, cbLen);
		}
		else
		{
			searchSeparator = false;
			*foundSeparator = '\0';
			*cbLen = foundSeparator - &(buf[readIdx]);
		}
	}

	if (rc == 0)
	{
		*token = &(buf[readIdx]);
		readIdx += *cbLen + 1;
	}

	return rc;
}

DWORD StreamTokenizer::next(char ** token, DWORD* cbLen)
{
	DWORD rc;

	if (endOfStream())
	{
		*token = nullptr;
		*cbLen = 0;
		rc = 0;
	}
	else
	{
		rc = ensureBuffer();
		if (rc == 0 && !endOfStream())
		{
			rc = readToken(token, cbLen);
		}
	}

	return rc;
}


