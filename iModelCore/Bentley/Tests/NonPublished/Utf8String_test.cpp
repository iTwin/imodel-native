/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>
#include <thread>
#include <map>
#include <vector>
#include <array>

#define VERIFY(X) ASSERT_TRUE(X)

// [17.6.5.9/3] A C++ standard library function shall not directly or indirectly modify objects (1.10) accessible by threads 
// other than the current thread unless the objects are accessed directly or indirectly via the function's non-const arguments, 
// including this.
// Bstdcxx::basic_string implements copy on write. A basic_string instance actually holds a pointer to a __string_ref object behind 
// the scenes. When you copy a basic_string, the copy points to the same __string_ref object that the original is using, and
// a side-effect of the copy is to increment the reference count on the shared object. 
// The following test will probably crash if the __string_ref functions for incrementing and decrementing the reference count are not atomic.

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest,ThreadSafeRead)
    {
    static Utf8String* sharedString;
    sharedString = new Utf8String("asdflslfjl3453453");     // Note: create explictly, as this test might be run repeatedly in the same process
    
    auto job = []
        {
        for (int i = 0; i < 2000000; ++i)
            {
            auto bar = *sharedString;       // increments ref count on shared __string_ref
            }                               // ~bar decrements ref count
        };
    
    int threadCount = std::thread::hardware_concurrency();
    if (threadCount == 0) // Unable to get hardware thread count use default.
        {
        threadCount = 2;
        }
        
    std::vector<std::thread> threads;
    
    for (int i = 0; i < threadCount; ++i)
        {
        threads.push_back(std::thread (job));
        }

    for (std::thread& t : threads)
        {
        t.join();
        }

    delete sharedString;                    // should remove last ref to shared __string_ref
    sharedString = nullptr;
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
static void initBeStringUtilities()
    {
    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDir);
    BeStringUtilities::Initialize(assetsDir);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, Utils)
    {
    // The first caller to convert strings in a process has to ensure BeStringUtilities::Initialize is called.
    initBeStringUtilities();
    
    Utf8String str (L" ThisIsATest!@#$%^&*()-= " );
    Utf8String cc (str);
    Utf8String trimmed (str);
    trimmed.Trim();
    VERIFY( str == cc);
    VERIFY( trimmed.Equals("ThisIsATest!@#$%^&*()-="));
    VERIFY( trimmed.length() == 23);

    Utf8String str2 (L"*** Test ***");
    Utf8String cc2 (str2);
    Utf8String trimmed2 (str2);
    trimmed2.Trim ("*");
    VERIFY (str2 == cc2);
    VERIFY (trimmed2.Equals (" Test "));
    VERIFY (trimmed2.length() == 6);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, EndsWith_EmptyStrings_False)
    {
    EXPECT_FALSE(Utf8String("").EndsWith(""));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, EndsWith_StringWithEmptyEnding_False)
    {
    EXPECT_FALSE(Utf8String("ABC").EndsWith(""));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, EndsWith_StringWithEnding_True)
    {
    EXPECT_TRUE(Utf8String("ABC").EndsWith("C"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, EndsWith_StringWithBegining_False)
    {
    EXPECT_FALSE(Utf8String("ABC").EndsWith("A"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, EndsWith_EqualStrings_True)
    {
    EXPECT_TRUE(Utf8String("ABC").EndsWith("ABC"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, EndsWith_DifferentCaseStrings)
    {
    EXPECT_FALSE(Utf8String("ABC").EndsWith("abc"));

    EXPECT_TRUE(Utf8String("ABC").EndsWithI("abc"));
    
    EXPECT_FALSE(Utf8String("ABC").EndsWithI(nullptr));

    EXPECT_FALSE(Utf8String("ABC").EndsWithI(""));

    EXPECT_FALSE(Utf8String("ABC").EndsWithI("abcd"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, EndsWith_Utf8String)
    {
    EXPECT_TRUE(Utf8String("ABC").EndsWith(Utf8String("")));

    EXPECT_TRUE(Utf8String("ABC").EndsWith(Utf8String("BC")));

    EXPECT_TRUE(Utf8String("ABC").EndsWith(Utf8String("ABC")));

    EXPECT_FALSE(Utf8String("ABC").EndsWith(Utf8String("ABCD")));

    EXPECT_FALSE(Utf8String("ABC").EndsWith(Utf8String("abc")));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, EndsWithIAscii)
    {
    EXPECT_FALSE(Utf8String("ABC").EndsWithIAscii(""));

    EXPECT_TRUE(Utf8String("ABC").EndsWithIAscii("BC"));

    EXPECT_TRUE(Utf8String("ABC").EndsWithIAscii("ABC"));

    EXPECT_FALSE(Utf8String("ABC").EndsWithIAscii("ABCD"));

    EXPECT_TRUE(Utf8String("ABC").EndsWithIAscii("abc"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, EndsWith_EndingLongerThanString_False)
    {
    EXPECT_FALSE(Utf8String("AAA").EndsWith("AAAA"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, TrimEnd_WhiteSpaceNotAtTheEnd_LeavesAsItIs)
    {
    EXPECT_STREQ("A B", Utf8String("A B").TrimEnd().c_str());
    EXPECT_STREQ(" A", Utf8String(" A").TrimEnd().c_str());
    EXPECT_STREQ("A\nB", Utf8String("A\nB").TrimEnd().c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, TrimEnd_WhiteSpaceAtTheEnd_TrimsWhiteSpace)
    {
    EXPECT_STREQ("A", Utf8String("A\n").TrimEnd().c_str());
    EXPECT_STREQ("B", Utf8String("B ").TrimEnd().c_str());
    EXPECT_STREQ("C", Utf8String("C \n  \t ").TrimEnd().c_str());
    EXPECT_STREQ("\n D", Utf8String("\n D \n  \t ").TrimEnd().c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, TrimEnd_ContainsOnlyWhiteSpace_LeavesEmptyString)
    {
    EXPECT_STREQ("", Utf8String("").TrimEnd().c_str());
    EXPECT_STREQ("", Utf8String(" ").TrimEnd().c_str());
    EXPECT_STREQ("", Utf8String("\r\n").TrimEnd().c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, Trim)
    {
    EXPECT_STREQ("A", Utf8String("A\n").Trim().c_str());
    EXPECT_STREQ("B", Utf8String("B ").Trim().c_str());
    EXPECT_STREQ("C", Utf8String("C \n  \t ").Trim().c_str());
    EXPECT_STREQ("D", Utf8String("\n D \n  \t ").Trim().c_str());

    // LeavesAsItIs
    EXPECT_STREQ("A B", Utf8String("A B").Trim().c_str());
    EXPECT_STREQ("A\t\n\rB", Utf8String("A\t\n\rB").Trim().c_str());
    EXPECT_STREQ("A\nB", Utf8String("A\nB").Trim().c_str());

    //LeavesEmptyString
    EXPECT_STREQ("", Utf8String("").Trim().c_str());
    EXPECT_STREQ("", Utf8String(" ").Trim().c_str());
    EXPECT_STREQ("", Utf8String("\r\n").Trim().c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, Trim_Characters)
    {
    EXPECT_STREQ("A\n", Utf8String("A\n").Trim("").c_str());
    EXPECT_STREQ("B ", Utf8String("B ").Trim("").c_str());
    EXPECT_STREQ("C \n  \t ", Utf8String("C \n  \t ").Trim("").c_str());
    EXPECT_STREQ("D", Utf8String("\n D \n  \t ").Trim("\t\n ").c_str());

    // LeavesAsItIs
    EXPECT_STREQ("A B", Utf8String("A B").Trim(" ").c_str());
    EXPECT_STREQ("A\t\n\rB", Utf8String("A\t\n\rB").Trim("\t\n\r").c_str());
    EXPECT_STREQ("A\nB", Utf8String("A\nB").Trim("\n").c_str());
    EXPECT_STREQ("ACB", Utf8String("ACB").Trim("C").c_str());
    EXPECT_STREQ("ACB", Utf8String("ACB").Trim("abc").c_str());

    //LeavesEmptyString
    EXPECT_STREQ("", Utf8String("").Trim("").c_str());
    EXPECT_STREQ("", Utf8String(" ").Trim(" ").c_str());
    EXPECT_STREQ("", Utf8String("\r\n").Trim("\r\n").c_str());
    EXPECT_STREQ("", Utf8String("ABCBA").Trim("ABC").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, StartsWith)
    {
    EXPECT_FALSE(Utf8String("").StartsWith(""));
    EXPECT_FALSE(Utf8String("ABC").StartsWith(""));
    EXPECT_TRUE(Utf8String("ABC").StartsWith("A"));
    EXPECT_FALSE(Utf8String("ABC").StartsWith("C"));
    EXPECT_TRUE(Utf8String("ABC").StartsWith("ABC"));
    EXPECT_FALSE(Utf8String("AAA").StartsWith("AAAA"));

    // Check Case
    EXPECT_FALSE(Utf8String("ABC").StartsWith("abc"));
    EXPECT_TRUE(Utf8String("ABC").StartsWithI("abc"));
    EXPECT_FALSE(Utf8String("ABC").StartsWithI("abcd"));
    EXPECT_FALSE(Utf8String("ABC").StartsWithI(""));

    // Check Ascii
    EXPECT_TRUE(Utf8String("ABC").StartsWithIAscii("abc"));
    EXPECT_FALSE(Utf8String("ABC").StartsWithIAscii("abcd"));
    EXPECT_FALSE(Utf8String("ABC").StartsWithIAscii(""));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, ReplaceAll)
    {
    Utf8String str("");
    EXPECT_EQ(0, str.ReplaceAll("", ""));
    EXPECT_STREQ("",str.c_str());

    str.assign("");
    EXPECT_EQ(0, str.ReplaceAll("", "A"));
    EXPECT_STREQ("", str.c_str());

    str.assign("ABC");
    EXPECT_EQ(1, str.ReplaceAll("A", ""));
    EXPECT_STREQ("BC", str.c_str());

    str.assign("ABC");
    EXPECT_EQ(1, str.ReplaceAll("A", "C"));
    EXPECT_STREQ("CBC", str.c_str());

    str.assign("ABCABC ");
    EXPECT_EQ(2, str.ReplaceAll("AB", ""));
    EXPECT_STREQ("CC ", str.c_str());

    str.assign("ABCABC");
    EXPECT_EQ(2, str.ReplaceAll("AB", "11223344"));
    EXPECT_STREQ("11223344C11223344C", str.c_str());

    str.assign("AAA");
    EXPECT_EQ(0, str.ReplaceAll("AAAA", ""));
    EXPECT_STREQ("AAA", str.c_str());

    // Check Case
    str.assign("ABC");
    EXPECT_EQ(0, str.ReplaceAll("abc", "def"));
    EXPECT_STREQ("ABC", str.c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, Contains)
    {
    // Check Utf8CP
    EXPECT_TRUE(Utf8String("").Contains(""));
    EXPECT_FALSE(Utf8String("").Contains("ABCDEF1232"));
    EXPECT_TRUE(Utf8String("ABC").Contains(""));
    EXPECT_TRUE(Utf8String("ABC").Contains("A"));
    EXPECT_FALSE(Utf8String("ABC").Contains("ABD"));
    EXPECT_TRUE(Utf8String("ABC").Contains("ABC"));
    EXPECT_FALSE(Utf8String("AAA").Contains("AAAA"));
    
    // Check Utf8String
    EXPECT_TRUE(Utf8String("").Contains(Utf8String("")));
    EXPECT_FALSE(Utf8String("").Contains(Utf8String("ABCDEF1232")));
    EXPECT_TRUE(Utf8String("ABC").Contains(Utf8String("")));
    EXPECT_TRUE(Utf8String("ABC").Contains(Utf8String("A")));
    EXPECT_FALSE(Utf8String("ABC").Contains(Utf8String("ABD")));
    EXPECT_TRUE(Utf8String("ABC").Contains(Utf8String("ABC")));
    EXPECT_FALSE(Utf8String("AAA").Contains(Utf8String("AAAA")));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, ContainsI)
    {
    // Check Utf8CP
    EXPECT_FALSE(Utf8String("ABC").Contains("abc"));
    EXPECT_TRUE(Utf8String("ABC").ContainsI("abc"));
    EXPECT_TRUE(Utf8String("ABC").ContainsI("ABC"));
        
    // Check Utf8String
    EXPECT_FALSE(Utf8String("ABC").Contains(Utf8String("abc")));
    EXPECT_TRUE(Utf8String("ABC").ContainsI(Utf8String("abc")));
    EXPECT_TRUE(Utf8String("ABC").ContainsI(Utf8String("ABC")));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, ToUpper)
    {
    EXPECT_STREQ("", Utf8String("").ToUpper().c_str());
    EXPECT_STREQ("ABC", Utf8String("abc").ToUpper().c_str());
    EXPECT_STREQ("ABC", Utf8String("Abc").ToUpper().c_str());
    EXPECT_STREQ("ABC", Utf8String("ABC").ToUpper().c_str());
    EXPECT_STREQ("123$%^ ABC", Utf8String("123$%^ AbC").ToUpper().c_str());

    // To Upper Non Ascii
    WCharCP nonasc = L"\u20AC"; // this is the Euro symbol
    //  Convert to UTF8 and lowercase it
    Utf8String nonasc_utf8(nonasc);    // s/ be E2 82 AC 00
    Utf8String nonasc_utf8_src(nonasc);    // s/ be E2 82 AC 00
    EXPECT_STREQ(nonasc_utf8_src.c_str(), nonasc_utf8.ToUpper().c_str()); // s/ be a nop
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, IsAscii)
    {
    EXPECT_TRUE(Utf8String("ABC").IsAscii());
    EXPECT_TRUE(Utf8String("abc").IsAscii());
    EXPECT_TRUE(Utf8String("~!@#$%^&*()123").IsAscii());

    uint8_t str[] = { 66, 101, 110, 116, 108, 121, 174, 0 };
    Utf8CP utf8Str = (Utf8CP)str;
    Utf8String inStr(utf8Str);
    EXPECT_FALSE(inStr.IsAscii());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
static void VerifyVSPrintf (Utf8CP expectedStr, Utf8CP fmt, ...)
    {
    va_list args;
    va_start(args, fmt);
    Utf8String str;
    str.VSprintf(fmt, args);
    va_end(args);
    EXPECT_TRUE(str.Equals(expectedStr))<<"Expected: "<<expectedStr<<"\nActual: "<<str.c_str();
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, VSprintf)
    {
    VerifyVSPrintf("Test", "Test");
    VerifyVSPrintf("Test no. 5", "%s no. %d", "Test", 5);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest, Compare)
    {
    Utf8String str(" abc ");
    EXPECT_FALSE(str.CompareTo(" ABC ") == 0);
    EXPECT_TRUE(str.CompareToI(" ABC ") == 0);
    EXPECT_TRUE(str.CompareToIAscii(" ABC ") == 0);
    EXPECT_TRUE(str.CompareToIAscii(Utf8String(" ABC ")) == 0);

    EXPECT_TRUE(str.Equals(" abc "));
    EXPECT_FALSE(str.Equals(" ABC "));
    EXPECT_TRUE(str.EqualsI(" abc "));
    EXPECT_TRUE(str.EqualsIAscii(" ABC "));
    EXPECT_TRUE(str.EqualsIAscii(Utf8String(" ABC ")));
    }


//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(Utf8StringTest,TrimUtf8) {

    #define NBSP u8"\u00a0"
    #define EMSPC u8"\u2003"
    static_assert(sizeof(EMSPC) - 1 == 3, "emspace char size was not 3");
    #define TESTSPC NBSP EMSPC " \n\r\v\t"

    struct TestCase { Utf8String pretrim, expectedPostTrim; };

    std::array<TestCase, 6> cases = {
      TestCase{
        "test" TESTSPC,
        "test"
      },
      TestCase{
        TESTSPC "test",
        "test"
      },
      TestCase{
        TESTSPC "te" TESTSPC "st" TESTSPC,
        "te" TESTSPC "st"
      },
      TestCase{
        TESTSPC "te" TESTSPC "st" TESTSPC TESTSPC TESTSPC,
        "te" TESTSPC  "st"
      },
      TestCase{
        "te " TESTSPC " st",
        "te " TESTSPC " st"
      },
      TestCase{
        TESTSPC "this is a longish string without any non-ascii characters" TESTSPC, 
        "this is a longish string without any non-ascii characters",
      },
    };

    for (auto& [pretrimMut, expected] : cases) {
        const auto pretrim = pretrimMut;
        pretrimMut.TrimUtf8();
        const auto& trimmed = pretrimMut;
        EXPECT_STREQ(trimmed.c_str(), expected.c_str());
    }
}