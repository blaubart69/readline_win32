#include "pch.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestBufReaderWithGivenMem
{
	TEST_CLASS(TestBufferedReaderEx)
	{

	public:

		TEST_METHOD(initEx)
		{
			char buf[32];
			br_initEx(NULL, buf, sizeof(buf));
		}
		TEST_METHOD(initExPlusSizeofBufferedReader)
		{
			DWORD sizeofbr = 32 + sizeof(BUFFERED_READER);
			char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, sizeofbr);
			br_initEx(NULL, buf, sizeofbr);

		}
	};
}