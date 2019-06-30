#include "readline2.h"
//
// ----------------------------------------------------------------------------
//
void tryDetectBOM(
	_In_	const unsigned char* buf,
	_In_	DWORD bufLen,
	_Inout_ UINT* codepage,
	_Out_	BYTE* lenBOM,
	_Out_	BOOL* UTF16LEfound)
{
	*lenBOM = 0;
	*UTF16LEfound = false;

	if (bufLen >= 2)
	{
		// UTF16 LE ... 0xFF 0xFE
		if (buf[0] == 0xFF
			&& buf[1] == 0xFE)
		{
			*lenBOM = 2;
			*UTF16LEfound = TRUE;
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

#undef RtlMoveMemory
extern "C" __declspec(dllimport) void __stdcall RtlMoveMemory(void *dst, const void *src, size_t len);
//
// ----------------------------------------------------------------------------
//
readline2::readline2(const HANDLE fp, const DWORD bufsize)
	: _fp(fp), _bufsize(bufsize)
{
	_read_buffer = new char[bufsize];
	_conv_buffer = new WCHAR[bufsize + 1]; // +1 for trailing zero if needed

	_firstRead = true;
	_eof = false;
	_bytes_in_read_buffer = 0;
	_conv_buf_len = 0;
	_conv_start_idx = 0;
}
readline2::~readline2()
{
	delete[] _read_buffer;
	delete[] _conv_buffer;
}
DWORD readline2::fill_read_buffer()
{
	DWORD rc;
	DWORD bytesToRead = _bufsize - _bytes_in_read_buffer;
	DWORD bytesRead;

	if ( ReadFile(
		_fp
		, _read_buffer + _bytes_in_read_buffer
		, bytesToRead
		, &bytesRead
		, NULL))
	{
		rc = 0;
		_bytes_in_read_buffer += bytesRead;
		if (bytesRead == 0)
		{
			_eof = true;
		}
	}
	else
	{
		rc = GetLastError();
	}

	return rc;
}
DWORD readline2::first_read_and_convert()
{
	DWORD rc;
	if ( (rc = fill_read_buffer()) != 0 ) // startIdx == 0
	{
		return rc;
	}

	_codepage = CP_ACP;
	BYTE lenBOM;
	BOOL isUTF16LE;
	tryDetectBOM((unsigned char*)_read_buffer, _bytes_in_read_buffer, &_codepage, &lenBOM, &isUTF16LE);

	if (_bytes_in_read_buffer > lenBOM)
	{
		if (!isUTF16LE)
		{
			rc = convert(lenBOM, 0);
		}
	}
	else
	{
		// read again to see if we're really out of data
		_bytes_in_read_buffer = 0;
		rc = fill_read_buffer();
	}

	return rc;
}
void readline2::move_conv_buffer_down()
{
	DWORD wcharsToMove = _conv_buf_len - _conv_start_idx;
	DWORD convBytesToMove = wcharsToMove * 2;

	RtlMoveMemory(
		_conv_buffer,						// dest
		_conv_buffer + _conv_start_idx,		// src
		convBytesToMove);

	_conv_start_idx = 0;
	_conv_buf_len = wcharsToMove;
}
DWORD readline2::convert(_In_ int readBufStartIdx, _In_ int convBufStartIdx)
{
	DWORD rc;
	DWORD bytesNotConverted = 0;

	do
	{
		int bytesToConvert = _bytes_in_read_buffer - readBufStartIdx - bytesNotConverted;

		if (bytesToConvert == 0)
		{
			rc = 0;
		}
		else
		{
			int widecharsWritten;
			if ((widecharsWritten = MultiByteToWideChar(
				_codepage							// CodePage 
				, MB_ERR_INVALID_CHARS				// dwFlags
				, _read_buffer + readBufStartIdx	// lpMultiByteStr
				, bytesToConvert					// cbMultiByte 
				, _conv_buffer + convBufStartIdx	// lpWideCharStr
				, _bufsize  - convBufStartIdx       // cchWideChar 
			)) == 0)
			{
				rc = GetLastError();
				if (rc == ERROR_NO_UNICODE_TRANSLATION)
				{
					bytesNotConverted += 1;
				}
			}
			else
			{
				rc = 0;
				_conv_buf_len += widecharsWritten;
				//
				// if not every byte in the read_buffer was converted
				//	--> move down the bytes to the begin of the buffer
				//
				// example: a 3-byte UTF8 codepoint was not fully written to the read_buffer.
				// So the UTF8 character can not be decoded. We have to wait for the next read.
				//
				if (bytesNotConverted > 0)
				{
					RtlMoveMemory(
						_read_buffer													//	dest
						, &(_read_buffer[_bytes_in_read_buffer - bytesNotConverted])	// source
						, bytesNotConverted);
				}

				_bytes_in_read_buffer = bytesNotConverted;
			}
		}
	} while (rc == ERROR_NO_UNICODE_TRANSLATION && bytesNotConverted < 4);

	return rc;
}
DWORD readline2::clearCrLf(_In_ DWORD newLineIdx)
{
	int idxToSetZero = newLineIdx;

	if (newLineIdx > 0 && _conv_buffer[newLineIdx - 1] == L'\r')
	{
		--idxToSetZero;
	}
	_conv_buffer[idxToSetZero] = L'\0';

	return idxToSetZero;
}
bool readline2::find_newline(_Out_ DWORD* newLineIdx)
{
	DWORD rc = 0;

	bool newLineFound = false;
	*newLineIdx = _conv_start_idx;

	while ( *newLineIdx < _conv_buf_len )
	{
		if (_conv_buffer[*newLineIdx] == L'\n')
		{
			newLineFound = true;
			break;
		}
		++(*newLineIdx);
	}

	return newLineFound;
}
DWORD readline2::next(_Out_ LPWSTR & line, _Out_ DWORD & cchLen)
{
	DWORD rc = 0;

	line = nullptr;
	cchLen = 0;

	if (_firstRead)
	{
		_firstRead = false;
		rc = first_read_and_convert();
	}

	if (_eof)
	{
		return 0;
	}

	for (;;)
	{
		if (rc != 0)
		{
			break;
		}

		DWORD newLineIdx;
		if ( find_newline(&newLineIdx) )
		{
			DWORD zeroIdx = clearCrLf(newLineIdx);
			line = &(_conv_buffer[_conv_start_idx]);
			cchLen = zeroIdx - _conv_start_idx;
			_conv_start_idx = newLineIdx + 1;
			break;
		}
		else
		{
			if (_conv_start_idx == 0 && _conv_buf_len == _bufsize)
			{
				rc = ERROR_INSUFFICIENT_BUFFER;
				break;
			}

			if (0 != (rc = fill_read_buffer()))
			{
				break;
			}

			if (_eof)
			{
				cchLen = _conv_buf_len - _conv_start_idx;
				if (cchLen == 0)
				{
					line = nullptr;
				}
				else
				{
					// report the last characters as line
					_conv_buffer[_conv_buf_len] = L'\0';
					line = &(_conv_buffer[_conv_start_idx]);
				}
				break;
			}
			else
			{
				// no \n but more data in read buffer --> convert()
				move_conv_buffer_down();
				rc = convert(0, _conv_buf_len);
			}
		}
	}

	return rc;
}
/*
DWORD readline2::read_and_convert()
{
	DWORD rc;

					rc = fill_read_buffer();
	if (rc == 0) {	rc = convert(0, _conv_start_idx);	}

	return rc;
}
*/