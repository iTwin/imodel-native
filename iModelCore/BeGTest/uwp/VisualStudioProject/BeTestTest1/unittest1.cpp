
#include "CppUnitTest.h"
#include <Bentley/WString.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BeTestTest1
{
    TEST_CLASS(UnitTest1)
    {
    public:
        TEST_METHOD(TestMethod1)
        {
            WString str (L"abc");
            Assert::IsTrue (0==wcscmp(L"abc", str.c_str()));
        }
    };
}