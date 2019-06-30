#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestRL3
{
	volatile LONG64 uniqueCounterRL;

	TEST_CLASS(TestReadline3)
	{
	private:

		InOutFile* hTmp;
		READLINE3* rl;

		DWORD LastRc = 0;

		//
		// Assert helper
		//
		void AssertReadline(LPCWSTR expectedLine)
		{
			LPWSTR line;
			DWORD cchLen;

			DWORD rc = rl3_next(rl, line, &cchLen);
			if (rc != 0)
			{
				WCHAR err[128];
				wsprintf(err, L"readline2->next() returned rc: %d", rc);
				Assert::Fail(err);
			}

			int expectedLen = expectedLine == NULL ? 0 : lstrlenW(expectedLine);

			Assert::AreEqual(expectedLine, line);
			Assert::IsTrue(expectedLen == cchLen);
		}

		void initReadline(DWORD bufSize)
		{
			//rl = new readline2(hTmp->getReadHandle(), bufSize);
			rl = rl3_init(hTmp->getReadHandle(), bufSize);
		}

	public:

		TEST_CLASS_INITIALIZE(classInit)
		{
			uniqueCounterRL = 0;
		}
		TEST_METHOD_INITIALIZE(initMeth)
		{
			InterlockedIncrement64(&uniqueCounterRL);
			hTmp = new InOutFile(
				(UINT)uniqueCounterRL
				, L"c:\\temp\\readline3"
				, L"rl2_");
		}

		TEST_METHOD_CLEANUP(cleanMeth)
		{
			if (LastRc == 0)
			{
				AssertReadline(NULL);
			}
			rl3_free(rl);
			delete hTmp;
			
		}
		//
		// wide
		//
		/*
		TEST_METHOD(OneLineWithCrLfW)
		{
			hTmp->WriteW(L"berni\r\n");
			initReadline(16);
			AssertReadline(L"berni");
		}
		TEST_METHOD(OneLineWithoutCrLfW)
		{
			hTmp->WriteW(L"berni");
			initReadline(32);
			AssertReadline(L"berni");
		}
		TEST_METHOD(TwoLinesWithinBufferW)
		{
			hTmp->WriteW(L"berni\r\nspindler\r\n");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
		TEST_METHOD(TwoLinesWithinBufferW_secondWithoutCrLf)
		{
			hTmp->WriteW(L"berni\r\nspindler");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
				TEST_METHOD(OneLineWithoutCrLfW_bufferToSmall)
		{
			hTmp->WriteW(L"berni");

			LPWSTR line;
			DWORD  cchLen;

			initReadline(8);
			//DWORD rc = rl_readline(rl, &line, &cchLen);
			DWORD rc = rl->next(line, cchLen);
			Assert::IsTrue(rc == ERROR_INSUFFICIENT_BUFFER);
			LastRc = rc;
			Assert::IsNull(line);
			Assert::IsTrue(0 == cchLen);
		}
		TEST_METHOD(SecondLineDoesNotFitInBuffer_W)
		{
			hTmp->WriteW(L"12345\r\nabcdefghijkl");

			initReadline(13 * 2);
			AssertReadline(L"12345");
			AssertReadline(L"abcdefghijkl");
		}
		TEST_METHOD(OneEmtpyLineW)
		{
			hTmp->WriteW(L"\r\n");
			initReadline(16);
			AssertReadline(L"");
		}
		TEST_METHOD(TwoEmtpyLineW)
		{
			hTmp->WriteW(L"\r\n\r\n");
			initReadline(16);
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(SixemptyLinesW)
		{
			hTmp->WriteW(L"\r\n\r\n\r\n\r\n\r\n\r\n");
			initReadline(128);
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(SixEmptyLinesWithTooSmallBufferW)
		{
			hTmp->WriteW(L"\r\n\r\n\r\n\r\n\r\n\r\n");
			initReadline(8);

			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(UnixOneEmptyLineW)
		{
			hTmp->WriteW(L"\n");
			initReadline(6);
			AssertReadline(L"");
		}
		TEST_METHOD(UnixTwoEmptyLineW)
		{
			hTmp->WriteW(L"\n\n");
			initReadline(6);
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(UnixOneLineWithLfW)
		{
			hTmp->WriteW(L"Berni\n");
			initReadline(16);
			AssertReadline(L"Berni");
		}

		*/
		//
		// one line with crlf
		//
		TEST_METHOD(OneLineWithCrLf)
		{
			hTmp->WriteA("Berni\r\n");
			initReadline(8);
			AssertReadline(L"Berni");
		}
		TEST_METHOD(OneLineWithCrLf_UTF8BOM)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteA("berni\r\n");
			initReadline(16);
			AssertReadline(L"berni");
		}
		//
		// one line without crlf
		//
		TEST_METHOD(OneLineWithoutCrLf)
		{
			hTmp->WriteA("berni");
			initReadline(8);
			AssertReadline(L"berni");
		}
		TEST_METHOD(OneLineWithoutCrLf_UTF8)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteA("berni");
			initReadline(16);
			AssertReadline(L"berni");
		}
		//
		// empty file, only BOMs
		//
		TEST_METHOD(EmptyFileA_no_BOM)
		{
			initReadline(8);
			//AssertReadline(NULL);
		}
		TEST_METHOD(EmptyFileA_UTF8BOM)
		{
			hTmp->WriteUTF8BOM();
			initReadline(8);
			//AssertReadline(NULL);
		}
		TEST_METHOD(EmptyFileA_UTF16BOM)
		{
			hTmp->WriteUTF16LEBOM();
			initReadline(8);
			AssertReadline(NULL);
		}
		TEST_METHOD(TwoLinesWithinBufferA)
		{
			hTmp->WriteA("berni\r\nspindler\r\n");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
		TEST_METHOD(TwoLinesWithinBufferUTF8)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteA("berni\r\nspindler\r\n");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
		TEST_METHOD(TwoLinesWithinBufferA_secondWithoutCrLf)
		{
			hTmp->WriteA("berni\r\nspindler");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
		TEST_METHOD(TwoLinesWithinBufferUTF8_secondWithoutCrLf)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteA("berni\r\nspindler");

			initReadline(64);
			AssertReadline(L"berni");
			AssertReadline(L"spindler");
		}
		TEST_METHOD(SecondLineDoesNotFitInBuffer_A)
		{
			hTmp->WriteA("12345\r\nabcdefghijkl");

			initReadline(16);
			AssertReadline(L"12345");
			AssertReadline(L"abcdefghijkl");
		}
		TEST_METHOD(ReadUmlautAsUTF8)
		{
			hTmp->WriteA("Börnie");

			initReadline(16);
			AssertReadline(L"Börnie");
		}
		//
		// emtpy line
		//
		TEST_METHOD(OneEmtpyLineA)
		{
			hTmp->WriteA("\r\n");
			initReadline(16);
			AssertReadline(L"");
		}
		TEST_METHOD(OneEmtpyLineUTF8)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteA("\r\n");
			initReadline(16);
			AssertReadline(L"");
		}
		//
		// emtpy lines
		//
		TEST_METHOD(TwoEmtpyLineA)
		{
			hTmp->WriteA("\r\n\r\n");
			initReadline(16);
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(TwoEmtpyLineUTF8)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteA("\r\n\r\n");
			initReadline(16);
			AssertReadline(L"");
			AssertReadline(L"");
		}
		//
		// buffer full with newlines 
		//
		TEST_METHOD(SixemptyLinesA)
		{
			hTmp->WriteA("\r\n\r\n\r\n\r\n\r\n\r\n");
			initReadline(16);
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(SixemptyLinesUTF8)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteA("\r\n\r\n\r\n\r\n\r\n\r\n");
			initReadline(16);
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(SixEmptyLinesWithTooSmallBufferA)
		{
			hTmp->WriteA("\r\n\r\n\r\n\r\n\r\n\r\n");
			initReadline(4);
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
		}
		//
		// UNIX ...
		//
		TEST_METHOD(UnixOneEmptyLineA)
		{
			hTmp->WriteA("\n");
			initReadline(6);
			AssertReadline(L"");
		}
		TEST_METHOD(UnixTwoEmptyLineA)
		{
			hTmp->WriteA("\n\n");
			initReadline(6);
			AssertReadline(L"");
			AssertReadline(L"");
		}
		TEST_METHOD(UnixOneLineWithLfA)
		{
			hTmp->WriteA("Berni\n");
			initReadline(6);
			AssertReadline(L"Berni");
		}
		TEST_METHOD(UnixOneLineWithLfUTF8_bufferWithBOMTooSmallButTextFits)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteA("Berni\n");
			initReadline(6);

			AssertReadline(L"Berni");
		}
		TEST_METHOD(LastLineHasOnlyOneCharA)
		{
			hTmp->WriteA("11111\r\n22222\r\n33333\r\n44444\r\nx");
			initReadline(21);
			AssertReadline(L"11111");
			AssertReadline(L"22222");
			AssertReadline(L"33333");
			AssertReadline(L"44444");
			AssertReadline(L"x");
		}
		TEST_METHOD(LastLineHasOnlyOneCharWithOnlyEmptyLinesBeforeA)
		{
			hTmp->WriteA("\n\n\n\n\n\nx");
			initReadline(21);
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"");
			AssertReadline(L"x");
		}
		// UTF8 codepoints
		TEST_METHOD(UTF8_chars)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteUTF8(L"Börnie\r\n");
			initReadline(32);
			AssertReadline(L"Börnie");
		}
		TEST_METHOD(UTF8_more_chars)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteUTF8(L"ẞßöäü€@\r\n");
			initReadline(32);
			AssertReadline(L"ẞßöäü€@");
		}
		TEST_METHOD(UTF8_more_chars_small_buffer)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteUTF8(L"ẞßöäü€@\r\n");
			initReadline(7);
			AssertReadline(L"ẞßöäü€@");
		}
		TEST_METHOD(UTF8_more_chars_small_buffer_noCrLf)
		{
			hTmp->WriteUTF8BOM();
			hTmp->WriteUTF8(L"ẞßöäü€@");
			initReadline(7);
			AssertReadline(L"ẞßöäü€@");
		}
	};
}