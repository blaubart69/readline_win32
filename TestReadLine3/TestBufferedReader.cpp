#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestReadLine3
{
	volatile LONG64 uniqueCounter;

	TEST_CLASS(TestReadLine3)
	{
	private:

		InOutFile* _hTestfile;
		BUFFERED_READER* _br;
		DWORD LastRc;

		void AssertNextChar(char expectedChar)
		{
			char nextChar;
			BOOL eof;

			BOOL ok = br_read(_br, &nextChar, &eof);

			Assert::IsTrue(ok == TRUE);
			Assert::AreEqual<char>(expectedChar, nextChar);
			Assert::IsTrue(eof == FALSE);
		}

		void initBr(DWORD bufsize)
		{
			_br = br_init(_hTestfile->getReadHandle(), bufsize);
		}

	public:

		TEST_METHOD(ThreeChars)
		{
			_hTestfile->WriteA("ber");
			initBr(32);
			AssertNextChar('b');
			AssertNextChar('e');
			AssertNextChar('r');
		}
		TEST_METHOD(AdditionalReadAfterEnd)
		{
			_hTestfile->WriteA("be");
			initBr(32);

			AssertNextChar('b');
			AssertNextChar('e');

			char nextChar;
			BOOL eof;
			BOOL ok = br_read(_br, &nextChar, &eof);
			Assert::IsTrue(ok == TRUE);
			Assert::IsTrue(eof == TRUE);

		}

		TEST_METHOD(EmptyFile)
		{
			initBr(32);
		}
		TEST_METHOD(BufferTooSmall)
		{
			_hTestfile->WriteA("1234567890");
			initBr(5);
			AssertNextChar('1');
			AssertNextChar('2');
			AssertNextChar('3');
			AssertNextChar('4');
			AssertNextChar('5');
			AssertNextChar('6');
			AssertNextChar('7');
			AssertNextChar('8');
			AssertNextChar('9');
			AssertNextChar('0');
		}
		TEST_METHOD(VerySmallBuffer)
		{
			_hTestfile->WriteA("1234567890");
			initBr(1);
			AssertNextChar('1');
			AssertNextChar('2');
			AssertNextChar('3');
			AssertNextChar('4');
			AssertNextChar('5');
			AssertNextChar('6');
			AssertNextChar('7');
			AssertNextChar('8');
			AssertNextChar('9');
			AssertNextChar('0');
		}
		TEST_METHOD(ExactBufferSize)
		{
			_hTestfile->WriteA("1234567890");
			initBr(10);
			AssertNextChar('1');
			AssertNextChar('2');
			AssertNextChar('3');
			AssertNextChar('4');
			AssertNextChar('5');
			AssertNextChar('6');
			AssertNextChar('7');
			AssertNextChar('8');
			AssertNextChar('9');
			AssertNextChar('0');
		}

		TEST_CLASS_INITIALIZE(classInit)
		{
			uniqueCounter = 0;
			CreateDirectoryW(L"c:\\temp\\BufferedReader", NULL);
		}
		TEST_METHOD_INITIALIZE(initMeth)
		{
			InterlockedIncrement64(&uniqueCounter);

			_hTestfile = new InOutFile(
				(UINT)uniqueCounter
				, L"c:\\temp\\BufferedReader"
				, L"br_");

			LastRc = 0;
		}

		TEST_METHOD_CLEANUP(cleanMeth)
		{
			if (LastRc == 0)
			{
				char nextChar;
				BOOL eof;

				BOOL ok = br_read(_br, &nextChar, &eof);

				Assert::IsTrue(ok == TRUE);
				Assert::IsTrue(eof == TRUE);
			}

			br_free(_br);
			delete _hTestfile;
		}
	};
}
