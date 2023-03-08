/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/CodePages.h>
#include <map>
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
#include <Windows.h>
#endif

#define VERIFY(X) ASSERT_TRUE(X)
#define TESTDATA_String                     "ThisIsATest!@#$%^&*()-="
#define TESTDATA_StringW                    L"ThisIsATest!@#$%^&*()-="

#define TESTDATA_String_Lower                   "thisisatest!@#$%^&*()-="
#define TESTDATA_String_Upper                   "THISTISATEST!@#$%^&*()-="

PUSH_DISABLE_DEPRECATION_WARNINGS

#if defined (NOT_USED)
struct AW
    {
    char const* m_asc;
    AW (char const* a) : m_asc(a) {;}
    operator char const*() const {return m_asc;}
    };
#endif

//int s_dummy;

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of Utf16WCharHello method.
//
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, CompareUtf16WChar)
    {
    uint16_t string1[]=  {72,69,76,76,79,0};
    WCharCP string2= L"HELLO\0";
    Utf16CP string3= string1;

    int status= BeStringUtilities::CompareUtf16WChar(string3,string2);
    EXPECT_EQ(0,status);
    }

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of WCharToUtf16 method.
//
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, WCharToUtf16)
    {
    WCharCP string2= L"HELLO\0";
    Utf16Buffer utf16String ;
    Utf16BufferR stringUtf16= utf16String;
    size_t count= BeStringUtilities::AsManyAsPossible;

    BentleyStatus status= BeStringUtilities::WCharToUtf16(stringUtf16,string2, count);
    EXPECT_EQ(SUCCESS,status)<<status;
    }

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of WCharToUtf16NotNull method.
//
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, WCharToUtf16NotNull)
    {
    WCharCP string2= L"HELLO";
    Utf16Buffer utf16String ;
    Utf16BufferR stringUtf16= utf16String;
    size_t count= BeStringUtilities::AsManyAsPossible;

    BentleyStatus status= BeStringUtilities::WCharToUtf16(stringUtf16,string2, count);
    EXPECT_EQ(SUCCESS,status)<<status;
    }

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of Strlwr method.
//
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, Strlwr)
    {
        {
        char string[] = "HELLO";
        char* outStr = BeStringUtilities::Strlwr(string);
        EXPECT_STREQ("hello", outStr) << outStr;
        }
        {
        char string[] = "HELLO 5! There?";
        char* outStr = BeStringUtilities::Strlwr(string);
        EXPECT_STREQ("hello 5! there?", outStr) << outStr;
        }
    }

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of Strupr method.
//
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, Strupr)
    {
        {
        char string[] = "hello";
        char* outStr = BeStringUtilities::Strupr(string);
        EXPECT_STREQ("HELLO", outStr) << outStr;
        }
        {
        char string[] = "HELLO 5! There?";
        char* outStr = BeStringUtilities::Strupr(string);
        EXPECT_STREQ("HELLO 5! THERE?", outStr) << outStr;
        }
        {
            wchar_t const* nonasc = L"\u20AC"; // this is the Euro symbol
            Utf8String nonasc_utf8(nonasc);    // s/ be E2 82 AC 00
            ScopedArray<char> nonascbuf(nonasc_utf8.size() + 1);
            strcpy(nonascbuf.GetData(), nonasc_utf8.c_str());
            char* outStr = BeStringUtilities::Strupr(nonascbuf.GetData());
            EXPECT_STREQ(nonasc_utf8.c_str(), outStr);
        }
    }

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of CopyUtf16 method.
//
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, CopyUtf16)
    {
    uint16_t string1[]=  {48,49,50,51,52,53,0};
    uint16_t string2[10] = {0 };
    Utf16CP inStr= string1;

    Utf16P outStr= string2;
    size_t outStrCapacity= BeStringUtilities::AsManyAsPossible;

    EXPECT_EQ(6, BeStringUtilities::Utf16Len(inStr));
    BeStringUtilities::CopyUtf16(outStr,outStrCapacity,inStr);
    EXPECT_EQ(6, BeStringUtilities::Utf16Len(outStr));

    // Out with less capacity
    uint16_t string3[3]={0};
    Utf16P outStr2 = string3;
    BeStringUtilities::CopyUtf16(outStr2, 3, inStr);
    EXPECT_EQ(2, BeStringUtilities::Utf16Len(outStr2));

    // Output with greater capacity that input
    uint16_t string4[10] = {0 };
    Utf16P outStr3 = string4;
    BeStringUtilities::CopyUtf16(outStr3, 10, inStr);
    EXPECT_EQ(6, BeStringUtilities::Utf16Len(outStr3));
    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of Stricmp method.
//
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, Stricmp)
    {
    char string0[] = "hello";
    char string1[] = "HeLLo";
    char string2[] = "hello2";
    char string3[] = "hello3";

    int status= BeStringUtilities::Stricmp(string0,string1);
    EXPECT_EQ(0,status)<<status;

    status = BeStringUtilities::Stricmp(string1, string2);
    EXPECT_GT(0, status) << status;

    status = BeStringUtilities::Stricmp(string3, string2);
    EXPECT_LT(0, status) << status;
    }
//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, NICompare)
    {
        {
        char string0[] = "hello";
        char string1[] = "HeLLo";
        char string2[] = "hello2";
        char string3[] = "Hello3";

        int status = BeStringUtilities::Strnicmp(string0, string1, 5);
        EXPECT_EQ(0, status) << status;

        status = BeStringUtilities::Strnicmp(string3, string2, 6);
        EXPECT_LT(0, status) << status;

        status = BeStringUtilities::Strnicmp(string2, string3, 6);
        EXPECT_GT(0, status) << status;

        status = BeStringUtilities::Strnicmp(string3, string2, 5);
        EXPECT_EQ(0, status) << status;
        }

        {
        wchar_t string0[] = L"hello";
        wchar_t string1[] = L"HeLLo";
        wchar_t string2[] = L"hello2";
        wchar_t string3[] = L"Hello3";

        int status = BeStringUtilities::Wcsnicmp(string0, string1, 5);
        EXPECT_EQ(0, status) << status;

        status = BeStringUtilities::Wcsnicmp(string3, string2, 6);
        EXPECT_LT(0, status) << status;

        status = BeStringUtilities::Wcsnicmp(string2, string3, 6);
        EXPECT_GT(0, status) << status;

        status = BeStringUtilities::Wcsnicmp(string3, string2, 5);
        EXPECT_EQ(0, status) << status;
        }
    }
//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, LexicographicCompare)
    {
    wchar_t string0[] = L"Hello";
    wchar_t string1[] = L"Helloo";
    wchar_t string2[] = L"Hello9";
    wchar_t string3[] = L"Hello10";
    wchar_t string4[] = L"001";
    wchar_t string5[] = L"0001";
    wchar_t string6[] = L"000000000000000000";

    int status = BeStringUtilities::LexicographicCompare(string0, string0);
    EXPECT_TRUE(0 == status) << status;

    status = BeStringUtilities::LexicographicCompare(string0, string1);
    EXPECT_TRUE(status < 0) << status;

    status = BeStringUtilities::LexicographicCompare(string1, string0);
    EXPECT_TRUE(status > 0) << status;

    status = BeStringUtilities::LexicographicCompare(string2, string3);
    EXPECT_TRUE(status < 0) << status;

    status = BeStringUtilities::LexicographicCompare(string3, string2);
    EXPECT_TRUE(status > 0) << status;

    status = BeStringUtilities::LexicographicCompare(string4, string5);
    EXPECT_TRUE(0 == status) << status;

    status = BeStringUtilities::LexicographicCompare(string5, string6);
    EXPECT_TRUE(status > 0) << status;
    }
//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, ReverseStr)
    {
    char  inStr[]  = TESTDATA_String;
    char * outStr = BeStringUtilities::Strrev(inStr);
    EXPECT_STREQ("=-)(*&^%$#@!tseTAsIsihT", outStr) << outStr;

    wchar_t  inStrW[] = TESTDATA_StringW;
    wchar_t * outStrW = BeStringUtilities::Wcsrev(inStrW);
    EXPECT_STREQ(L"=-)(*&^%$#@!tseTAsIsihT", outStrW) << outStrW;
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, Duplicate)
    {
        {
        const char* inStr = TESTDATA_String;
        char* outStr = BeStringUtilities::Strdup(inStr);
        EXPECT_TRUE(inStr != outStr);
        EXPECT_STREQ(inStr, outStr) ;
        free(outStr);
        }

        {
        const wchar_t* inStr = TESTDATA_StringW;
        wchar_t* outStr = BeStringUtilities::Wcsdup(inStr);
        EXPECT_TRUE(inStr != outStr);
        EXPECT_STREQ(inStr, outStr);
        free(outStr);
        }
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, Join)
    {
        {
        bvector<Utf8String> list;
        list.push_back("Hello");
        list.push_back("World");
        list.push_back("!");
        list.push_back("");

        Utf8String outStr = BeStringUtilities::Join(list, ",");
        EXPECT_STREQ("Hello,World,!,", outStr.c_str()) << outStr;
        }

        {
        bvector<Utf8CP> list;
        list.push_back("Hello");
        list.push_back("World");
        list.push_back("!");
        list.push_back("");

        Utf8String outStr = BeStringUtilities::Join(list, ",");
        EXPECT_STREQ("Hello,World,!,", outStr.c_str()) << outStr;
        }
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, Split)
    {
        {
        WCharCP str = L"One,Two 3G4";
        bvector<WString> tokens;

        BeStringUtilities::Split(str, L", G", nullptr, tokens);
        ASSERT_EQ(4, tokens.size());
        EXPECT_STREQ(L"One", tokens[0].c_str());
        EXPECT_STREQ(L"Two", tokens[1].c_str());
        EXPECT_STREQ(L"3", tokens[2].c_str());
        EXPECT_STREQ(L"4", tokens[3].c_str());
        }

        {
        Utf8CP str = "One,Two 3G4";
        bvector<Utf8String> tokens;

        BeStringUtilities::Split(str, ", G", nullptr, tokens);
        ASSERT_EQ(4, tokens.size());
        EXPECT_STREQ("One", tokens[0].c_str());
        EXPECT_STREQ("Two", tokens[1].c_str());
        EXPECT_STREQ("3", tokens[2].c_str());
        EXPECT_STREQ("4", tokens[3].c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, ParseArguments)
    {
        {
        WCharCP str = L"One Two 3G4,\"Fourty Two\"";
        bvector<WString> tokens;

        BeStringUtilities::ParseArguments(tokens, str,L",");
        ASSERT_EQ(4, tokens.size());
        EXPECT_STREQ(L"One", tokens[0].c_str());
        EXPECT_STREQ(L"Two", tokens[1].c_str());
        EXPECT_STREQ(L"3G4", tokens[2].c_str());
        EXPECT_STREQ(L"Fourty Two", tokens[3].c_str());
        }

        {
        WCharCP str = L"1 \"2 3\"";
        WString tokens[3];

        uint32_t argFound = BeStringUtilities::ParseArguments(str, 3, &tokens[0], &tokens[1], &tokens[2]);
        ASSERT_EQ(2, argFound);
        EXPECT_STREQ(L"1", tokens[0].c_str());
        EXPECT_STREQ(L"2 3", tokens[1].c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of Wmemcpy method.
//
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilWmemcpy)
    {
    wchar_t dest[]= L"123456";
    const wchar_t src[]= L"DELETE";

    BeStringUtilities::Wmemcpy(dest, _countof(dest), src,  _countof(src));
    EXPECT_EQ(SUCCESS,BeStringUtilities::Wmemcpy(dest, _countof(dest), src,  _countof(src)));

    EXPECT_STREQ(src, dest);
    }

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of Wmemcpy method.
//
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilMemcpy)
    {
    const wchar_t src[]= L"DELETE";
    wchar_t dest[]= L"123456";

    VERIFY(BentleyStatus::SUCCESS == BeStringUtilities::Memcpy(dest, sizeof(dest), src, 3 * sizeof(wchar_t)));
    ASSERT_STREQ(L"DEL456", dest);
    VERIFY(BentleyStatus::SUCCESS == BeStringUtilities::Memcpy(dest, sizeof(dest), src, 0));
    ASSERT_STREQ(L"DEL456", dest);
    }

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of Utf16Len method.
//
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilUtf16Len)
    {
    uint16_t string1[]=  {72,69,76,76,79,0};
    Utf16CP string3= string1;

    size_t status= BeStringUtilities::Utf16Len(string3);
    EXPECT_EQ(5,status);
    }

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of CurrentLocaleToWChar method.
//
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringCurrentLocaleToWChar)
    {
    char inChar[]= "Hello";
    WCharCP outWCharTest= L"Hello";
    wchar_t outWCharInitial[]= L"Hello";

    EXPECT_TRUE(0==wcscmp(outWCharTest,BeStringUtilities::CurrentLocaleCharToWChar(outWCharInitial,inChar,_countof(inChar) )));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringCurrentLocaleToWCharStatusCheck)
    {
    char inChar[]= "Hello";
    WString outWCharInitial= L"Hello";

    BentleyStatus status= BeStringUtilities::CurrentLocaleCharToWChar(outWCharInitial,inChar );
    EXPECT_EQ(SUCCESS, status);
    }

//---------------------------------------------------------------------------------------
// @betest
// Desc: Testing of IsUri8Encoded method.
//
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, UriEncoding)
    {
    char test[] = "\"This a Test, OK?\"";
    Utf8CP inStr = test;

    // Encode
    Utf8String encodedStr = BeStringUtilities::UriEncode(inStr);
    ASSERT_STREQ("%22This%20a%20Test%2C%20OK%3F%22", encodedStr.c_str());

    //Check
    //EXPECT_TRUE(BeStringUtilities::IsUriEncoded(encodedStr.c_str()));
    //EXPECT_FALSE(BeStringUtilities::IsUriEncoded(inStr));

    //Decode
    Utf8String decodedStr = BeStringUtilities::UriDecode(encodedStr.c_str());
    EXPECT_STREQ("\"This a Test, OK?\"", decodedStr.c_str());

    // Decode SubURI
    Utf8String decodeSubStr = BeStringUtilities::UriDecode(decodedStr.c_str(), decodedStr.c_str() + 13);
    EXPECT_STREQ("\"This a Test,", decodeSubStr.c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, TestWcsstr)
    {
    wchar_t wcs[] = L"This is a simple string";
    auto pwc = wcsstr(wcs,L"simple");
    ASSERT_TRUE (NULL != pwc);
    EXPECT_TRUE (0==wcsncmp(pwc, L"simple", 6));

    pwc = wcsstr(wcs, L"sample");
    EXPECT_TRUE (NULL == pwc);

    pwc = wcsstr(wcs, wcs);
    ASSERT_TRUE (NULL != pwc);
    EXPECT_TRUE (pwc == wcs);

    pwc = wcsstr(wcs, L"g");
    ASSERT_TRUE (NULL != pwc);
    EXPECT_TRUE (0==wcscmp(pwc, L"g"));    // pwc should be pointing to the g at the end of the string
    }

#include <cwctype>
//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, StrLwr)
    {
    char asc[] = "ASCII";
    BeStringUtilities::Strlwr(asc);
    EXPECT_STREQ( asc, "ascii" );

    // Start with a non-ascii string. In this case, it has no lower-case version.
    WCharCP nonasc = L"\u20AC"; // this is the Euro symbol
    //  Convert to UTF8 and lowercase it
    Utf8String nonasc_utf8(nonasc);    // s/ be E2 82 AC 00
    EXPECT_FALSE( nonasc_utf8.IsAscii() );
    nonasc_utf8.ToLower();
    WString nonascAfter(nonasc_utf8.c_str(), BentleyCharEncoding::Utf8);
    EXPECT_STREQ (nonascAfter.c_str(), nonasc); // s/ be a nop
    // Do the same thing using a char array
    ScopedArray<char> nonascbuf(nonasc_utf8.size() + 1);
    strcpy(nonascbuf.GetData(), nonasc_utf8.c_str());
    BeStringUtilities::Strlwr(nonascbuf.GetData());
    WString nonascAfterBuf(nonascbuf.GetData(), BentleyCharEncoding::Utf8);
    EXPECT_STREQ (nonascAfterBuf.c_str(), nonasc);

    EXPECT_TRUE(0 == BeStringUtilities::StricmpAscii("abc", "abc"));
    EXPECT_TRUE(0 == BeStringUtilities::StricmpAscii("abC", "abc"));
    EXPECT_TRUE(0 == BeStringUtilities::StricmpAscii("abc", "aBc"));
    EXPECT_TRUE(0 == BeStringUtilities::StricmpAscii("abc", "ABC"));
    EXPECT_TRUE(0 == BeStringUtilities::StricmpAscii("VAV", "vav"));
    EXPECT_TRUE(0 == BeStringUtilities::StricmpAscii("a", "A"));
    EXPECT_TRUE(0 == BeStringUtilities::StricmpAscii("", ""));
    EXPECT_TRUE(0 == BeStringUtilities::StricmpAscii("a#e", "a#E"));
    EXPECT_TRUE(0 == BeStringUtilities::StricmpAscii("123A@e", "123a@E"));
    EXPECT_TRUE(-('d') == BeStringUtilities::StricmpAscii("abc", "aBcd"));
    EXPECT_TRUE(('d') == BeStringUtilities::StricmpAscii("abcd", "aBc"));
    EXPECT_TRUE(('d'-'b') == BeStringUtilities::StricmpAscii("ad", "aBc"));
    EXPECT_TRUE(('c'-'b') == BeStringUtilities::StricmpAscii("abcd", "AbBc"));
    EXPECT_TRUE(('c'-'b') == BeStringUtilities::StricmpAscii("ABCD", "abbc"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, IsInvalidUtf8Sequence)
    {
    static const Utf8CP TEST_URI = "pw://VILTEST2-5.bentley.com:PW_Ora/Documents/Ega/variuos_names/%AD%E4%CE%F3n%E5,%FD%85%DD%FC%BF%EB%B5%A4%C8gY.pdf";
    ASSERT_FALSE(BeStringUtilities::IsInvalidUtf8Sequence(TEST_URI));
    Utf8String decodedUri = BeStringUtilities::UriDecode(TEST_URI);
    ASSERT_TRUE(BeStringUtilities::IsInvalidUtf8Sequence(decodedUri.c_str()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
TEST(BeStringUtilitiesTests, RoundtripUtf8)
    {
    // The first caller to convert strings in a process has to ensure BeStringUtilities::Initialize is called.
    initBeStringUtilities();

    Utf8String strUtf8;
    BeStringUtilities::WCharToUtf8(strUtf8, TESTDATA_StringW);

    WString strWString;
    BeStringUtilities::Utf8ToWChar(strWString, strUtf8.c_str());

    ASSERT_EQ(0, wcscmp(TESTDATA_StringW, strWString.c_str()));

    WChar strW[30];
    BeStringUtilities::Utf8ToWChar(strW, strUtf8.c_str(), 30);
    ASSERT_STREQ(TESTDATA_StringW, strW);

    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, RoundtripUtf16)
    {
    // The first caller to convert strings in a process has to ensure BeStringUtilities::Initialize is called.
    initBeStringUtilities();

    Utf16Buffer strUtf16;
    BeStringUtilities::WCharToUtf16(strUtf16, TESTDATA_StringW);

    WString strWString;
    BeStringUtilities::Utf16ToWChar(strWString, strUtf16.data());

    ASSERT_EQ(0, wcscmp(TESTDATA_StringW, strWString.c_str()));

    // Empty Utf16 String
    BeStringUtilities::WCharToUtf16(strUtf16, L"");

    BeStringUtilities::Utf16ToWChar(strWString, strUtf16.data());

    ASSERT_EQ(0, wcscmp(L"", strWString.c_str()));

    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, Utf16ToUtf8)
    {
    // The first caller to convert strings in a process has to ensure BeStringUtilities::Initialize is called.
    initBeStringUtilities();

    Utf16Buffer strUtf16;
    BeStringUtilities::WCharToUtf16(strUtf16, TESTDATA_StringW);

    Utf8String strWString;
    BeStringUtilities::Utf16ToUtf8(strWString, strUtf16.data());

    EXPECT_STREQ( TESTDATA_String, strWString.c_str());

    // Empty Utf16 String
    BeStringUtilities::WCharToUtf16(strUtf16, L"");

    BeStringUtilities::Utf16ToUtf8(strWString, strUtf16.data());

    EXPECT_STREQ("", strWString.c_str());

    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, Itow)
    {
    wchar_t buf[10];
    ASSERT_TRUE( BeStringUtilities::Itow(buf, 1, _countof(buf), 10) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp(buf, L"1") );

    ASSERT_TRUE( BeStringUtilities::Itow(buf, 1, _countof(buf), 16) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp(buf, L"1") );

    ASSERT_TRUE( BeStringUtilities::Itow(buf, 1, _countof(buf), 2) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp(buf, L"1") );

#if defined (WIP_UNDERSIZED_BUFFER_IS_FATAL_ERROR_ON_WINDOWS)
    ASSERT_TRUE( BeStringUtilities::Itow(buf, 1, 0, 10) != SUCCESS );
    ASSERT_TRUE( BeStringUtilities::Itow(buf, 1, 1, 10) != SUCCESS );
#endif
    ASSERT_TRUE( BeStringUtilities::Itow(buf, 1, 2, 10) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp(buf, L"1") );

    ASSERT_TRUE( BeStringUtilities::Itow(buf, 16, _countof(buf), 10) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp(buf, L"16") );

    ASSERT_TRUE( BeStringUtilities::Itow(buf, 16, _countof(buf), 16) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp(buf, L"10") );

    ASSERT_TRUE( BeStringUtilities::Itow(buf, 16, _countof(buf), 2) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp(buf, L"10000") );

#if defined (WIP_UNDERSIZED_BUFFER_IS_FATAL_ERROR_ON_WINDOWS)
    ASSERT_TRUE( BeStringUtilities::Itow(buf, 16, 0, 10) != SUCCESS );
    ASSERT_TRUE( BeStringUtilities::Itow(buf, 16, 1, 10) != SUCCESS );
    ASSERT_TRUE( BeStringUtilities::Itow(buf, 16, 2, 10) != SUCCESS );
#endif
    ASSERT_TRUE( BeStringUtilities::Itow(buf, 16, 3, 10) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp(buf, L"16") );

    for (int i=0; i<1000; ++i)
        {
        ASSERT_TRUE( SUCCESS == BeStringUtilities::Itow(buf, i, _countof(buf), 10) );
        ASSERT_TRUE( WPrintfString(L"%d", i) == buf );

        ASSERT_TRUE( SUCCESS == BeStringUtilities::Itow(buf, i, _countof(buf), 16) );
        ASSERT_TRUE( WPrintfString(L"%x", i) == buf );

        ASSERT_TRUE( SUCCESS == BeStringUtilities::Itow(buf, i, _countof(buf), 8) );
        ASSERT_TRUE( WPrintfString(L"%o", i) == buf );
        }
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, Utf16ToWCharTest1)
    {
    // The first caller to convert strings in a process has to ensure BeStringUtilities::Initialize is called.
    initBeStringUtilities();

    CharCP  const_ascii  = "ascii";
    WCharCP const_asciiW = L"ascii";
    Utf16Buffer utf16;
    BeStringUtilities::Utf8ToUtf16(utf16, const_ascii);    // utf8->utf16
    WString wstr;
    BeStringUtilities::Utf16ToWChar(wstr, utf16.data());   // utf16->wchar
    ASSERT_TRUE( wstr.compare(const_asciiW) == 0 );
    Utf16Buffer utf162;
    BeStringUtilities::WCharToUtf16(utf162, wstr.c_str(), BeStringUtilities::AsManyAsPossible);    // utf16<-wchar
    ASSERT_TRUE (BeStringUtilities::CompareUtf16(utf162.data(), utf16.data()) == 0);
    char asc[256];
    BeStringUtilities::WCharToCurrentLocaleChar(asc, wstr.c_str(), _countof(asc)); // locale <- wchar
    ASSERT_TRUE (0 == strcmp(asc, const_ascii) );
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, FmtLongLong)
    {
    wchar_t buf[64];

    uint64_t i64_minus_one = -1;
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%llx", i64_minus_one);
    ASSERT_STREQ( buf, L"ffffffffffffffff");
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%lld", i64_minus_one);
    ASSERT_STREQ( buf, L"-1");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_minus_one);
    //ASSERT_STREQ( buf, L"ffffffffffffffff");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_minus_one);
    //ASSERT_STREQ( buf, L"-1");

    uint64_t i64_uint64_max = UINT64_MAX;
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%llx", i64_uint64_max);
    ASSERT_STREQ( buf, L"ffffffffffffffff");
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%lld", i64_uint64_max);
    ASSERT_STREQ( buf, L"-1");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_uint64_max);
    //ASSERT_STREQ( buf, L"ffffffffffffffff");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_uint64_max);
    //ASSERT_STREQ( buf, L"-1");

    uint64_t i64_int64_max = INT64_MAX;
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%llx", i64_int64_max);
    ASSERT_STREQ( buf, L"7fffffffffffffff");
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%lld", i64_int64_max);
    ASSERT_STREQ( buf, L"9223372036854775807");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_int64_max);
    //ASSERT_STREQ( buf, L"7fffffffffffffff");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_int64_max);
    //ASSERT_STREQ( buf, L"9223372036854775807");

    uint64_t i64_one = 1;
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%llx", i64_one);
    ASSERT_STREQ( buf, L"1");
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%lld", i64_one);
    ASSERT_STREQ( buf, L"1");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_one);
    //ASSERT_STREQ( buf, L"1");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_one);
    //ASSERT_STREQ( buf, L"1");

    uint64_t i64_zero = 0;
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%llx", i64_zero);
    ASSERT_STREQ( buf, L"0");
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%lld", i64_zero);
    ASSERT_STREQ( buf, L"0");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_zero);
    //ASSERT_STREQ( buf, L"0");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_zero);
    //ASSERT_STREQ( buf, L"0");

    uint64_t i64_max_int = (uint64_t)INT32_MAX;
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%llx", i64_max_int);
    ASSERT_STREQ( buf, L"7fffffff");
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%lld", i64_max_int);
    ASSERT_STREQ( buf, L"2147483647");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_max_int);
    //ASSERT_STREQ( buf, L"7fffffff");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_max_int);
    //ASSERT_STREQ( buf, L"2147483647");

    uint64_t i64_uint32_max = (uint64_t)UINT32_MAX;
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%llx", i64_uint32_max);
    ASSERT_STREQ( buf, L"ffffffff");
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%lld", i64_uint32_max);
    ASSERT_STREQ( buf, L"4294967295");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_uint32_max);
    //ASSERT_STREQ( buf, L"ffffffff");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_uint32_max);
    //ASSERT_STREQ( buf, L"4294967295");

    uint64_t i64_uint32_max_plus_one = (uint64_t)UINT32_MAX + 1;
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%llx", i64_uint32_max_plus_one);
    ASSERT_STREQ( buf, L"100000000");
    *buf = 0;
    BeStringUtilities::Snwprintf(buf, L"%lld", i64_uint32_max_plus_one);
    ASSERT_STREQ( buf, L"4294967296");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_uint32_max_plus_one);
    //ASSERT_STREQ( buf, L"100000000");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_uint32_max_plus_one);
    //ASSERT_STREQ( buf, L"4294967296");
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, FormatUInt64FromUtf8)
    {
        {
        uint64_t number = 0ULL;
        Utf8String str;
        str.reserve(2); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64((Utf8P)str.data(), number);
        ASSERT_STREQ ("0", str.c_str());
        }

        {
        uint64_t number = 43123ULL;
        Utf8String str;
        str.reserve(6); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64((Utf8P)str.data(), number);
        ASSERT_STREQ ("43123", str.c_str());
        }

        {
        uint64_t number = 14235263432521323ULL;
        Utf8String str;
        str.reserve(21); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64((Utf8P)str.data(), number);
        ASSERT_STREQ ("14235263432521323", str.c_str());
        }
}
//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, HexFormatOptions)
    {
        {
        uint64_t number = 65297ULL;
        WString str;
        str.reserve(10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64((WCharP) str.data(), 10,number,HexFormatOptions::None);
        EXPECT_STREQ (L"ff11", str.c_str());
        }

        {
        uint64_t number = 65297ULL;
        WString str;
        str.reserve(10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64((WCharP) str.data(), 10,number,HexFormatOptions::None,6);
        EXPECT_STREQ (L"  ff11", str.c_str());
        }

        {
        uint64_t number = 65297ULL;
        WString str;
        str.reserve(10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64((WCharP) str.data(), 10,number,HexFormatOptions::LeadingZeros,6);
        EXPECT_STREQ (L"00ff11", str.c_str());
        }

        {
        uint64_t number = 65297ULL;
        WString str;
        str.reserve(10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64((WCharP) str.data(), 10,number,HexFormatOptions::UsePrecision,6);
        EXPECT_STREQ (L"  ff11", str.c_str());
        }

        {
        uint64_t number = 65297ULL;
        WString str;
        str.reserve(12); //max length in hex format for an UInt64 (incl. trailing \0)
        int opts = (int)HexFormatOptions::LeftJustify | (int)HexFormatOptions::Uppercase | (int)HexFormatOptions::UsePrecision | (int)HexFormatOptions::IncludePrefix;
        BeStringUtilities::FormatUInt64((WCharP)str.data(), 12, number, (HexFormatOptions)opts, 10, 6);
        EXPECT_STREQ (L"0X00FF11  ", str.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, HexFormatOptionsUtf8)
    {
        {
        uint64_t number = 65297ULL;
        Utf8String str;
        str.reserve(10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64((Utf8P) str.data(), 10,number,HexFormatOptions::None);
        EXPECT_STREQ ("ff11", str.c_str());
        }

        {
        uint64_t number = 65297ULL;
        Utf8String str;
        str.reserve(10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64((Utf8P) str.data(), 10,number,HexFormatOptions::None,6);
        EXPECT_STREQ ("  ff11", str.c_str());
        }

        {
        uint64_t number = 65297ULL;
        Utf8String str;
        str.reserve(10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64((Utf8P) str.data(), 10,number,HexFormatOptions::LeadingZeros,6);
        EXPECT_STREQ ("00ff11", str.c_str());
        }

        {
        uint64_t number = 65297ULL;
        Utf8String str;
        str.reserve(10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64((Utf8P) str.data(), 10,number,HexFormatOptions::UsePrecision,6);
        EXPECT_STREQ ("  ff11", str.c_str());
        }

        {
        uint64_t number = 65297ULL;
        Utf8String str;
        str.reserve(12); //max length in hex format for an UInt64 (incl. trailing \0)
        int opts = (int)HexFormatOptions::LeftJustify | (int)HexFormatOptions::Uppercase | (int)HexFormatOptions::UsePrecision | (int)HexFormatOptions::IncludePrefix;
        BeStringUtilities::FormatUInt64((Utf8P)str.data(), 12, number, (HexFormatOptions)opts, 10, 6);
        EXPECT_STREQ ("0X00FF11  ", str.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, FormatHexUInt64)
    {
        {
        uint64_t number = 0ULL;
        Utf8String str;
        str.reserve(17); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatHexUInt64((Utf8P) str.data(), number);
        ASSERT_STREQ ("0", str.c_str());
        }

        {
        uint64_t number = 15ULL;
        Utf8String str;
        str.reserve(17); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatHexUInt64((Utf8P)str.data(), number);
        ASSERT_STREQ ("f", str.c_str());
        }

        {
        uint64_t number = 14235263432521323ULL;
        Utf8String str;
        str.reserve(17); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatHexUInt64((Utf8P)str.data(), number);
        ASSERT_STREQ ("3292e58c2df66b", str.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, ParseUInt64)
    {
    BentleyStatus stat;

    uint64_t number = BeStringUtilities::ParseUInt64("14235263432521323", &stat);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(14235263432521323ULL, number);

    number = BeStringUtilities::ParseUInt64("00014235263432521323", &stat);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(14235263432521323ULL, number);

    number = BeStringUtilities::ParseUInt64("-1423526343", &stat);
    ASSERT_EQ(ERROR, stat);
    ASSERT_EQ(0ULL, number);

    number = BeStringUtilities::ParseUInt64("0xff", &stat);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(0xff, number);

    number = BeStringUtilities::ParseUInt64("0X003292E58C2DF66B", &stat);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(14235263432521323ULL, number);

    number = BeStringUtilities::ParseUInt64("0xffnonsense", &stat);
    ASSERT_EQ(ERROR, stat);
    ASSERT_EQ(0ULL, number);

    number = BeStringUtilities::ParseUInt64("ff", &stat);
    ASSERT_EQ(ERROR, stat);
    ASSERT_EQ(0ULL, number);

    number = BeStringUtilities::ParseUInt64("1234aa54345", &stat);
    ASSERT_EQ(ERROR, stat);
    ASSERT_EQ(0ULL, number);

    number = BeStringUtilities::ParseUInt64("blabla", &stat);
    ASSERT_EQ(ERROR, stat);
    ASSERT_EQ(0ULL, number);

    number = BeStringUtilities::ParseHex("0x3292E58C2DF66B", &stat);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(14235263432521323ULL, number);

    number = BeStringUtilities::ParseHex("0X003292E58C2DF66B", &stat);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(14235263432521323ULL, number);

    number = BeStringUtilities::ParseHex("3292E58C2DF66B", &stat);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(14235263432521323ULL, number);

    number = BeStringUtilities::ParseHex("0X3292E58C2DF66B", &stat);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(14235263432521323ULL, number);

    number = BeStringUtilities::ParseHex("0x003292E58C2DF66B", &stat);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(14235263432521323ULL, number);

    number = BeStringUtilities::ParseHex("0X1", &stat);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(1ULL, number);

    number = BeStringUtilities::ParseHex("-1423526343", &stat);
    ASSERT_EQ(ERROR, stat);
    ASSERT_EQ(0ULL, number);

    number = BeStringUtilities::ParseHex("blabla", &stat);
    ASSERT_EQ(0ULL, number);
    ASSERT_EQ(ERROR, stat);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, Wtof)
    {
    EXPECT_DOUBLE_EQ(0.0, BeStringUtilities::Wtof(L"0"));
    EXPECT_DOUBLE_EQ(1.0, BeStringUtilities::Wtof(L"1"));
    EXPECT_DOUBLE_EQ(0.0, BeStringUtilities::Wtof(L"0.0"));
    EXPECT_DOUBLE_EQ(1.0, BeStringUtilities::Wtof(L"1.0"));
    EXPECT_NEAR(1.10,   BeStringUtilities::Wtof(L"1.10"), 0.0001);
    EXPECT_NEAR(1.01,   BeStringUtilities::Wtof(L"1.01"), 0.01);
    EXPECT_NEAR(1.001,  BeStringUtilities::Wtof(L"1.001"), 0.001);
    EXPECT_NEAR(1.0001, BeStringUtilities::Wtof(L"1.0001"), 0.0001);
    EXPECT_NEAR(1.00001,    BeStringUtilities::Wtof(L"1.00001"), 0.00001);
    EXPECT_NEAR(1.000001,   BeStringUtilities::Wtof(L"1.000001"), 0.000001);

    EXPECT_NEAR(-1.0, BeStringUtilities::Wtof(L"-1.0"),0.0001);
    EXPECT_DOUBLE_EQ(0.0, BeStringUtilities::Wtof(L"-0"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, Wtoi)
    {
    EXPECT_EQ(0,    BeStringUtilities::Wtoi(L"0"));
    EXPECT_EQ(1,    BeStringUtilities::Wtoi(L"1"));
    EXPECT_EQ(0,    BeStringUtilities::Wtoi(L"0.0"));
    EXPECT_EQ(100,    BeStringUtilities::Wtoi(L"100"));
    EXPECT_EQ(1,    BeStringUtilities::Wtoi(L"01"));
    EXPECT_EQ(999999, BeStringUtilities::Wtoi(L"999999"));

    EXPECT_EQ(-10,    BeStringUtilities::Wtoi(L"-10"));
    EXPECT_EQ(0, BeStringUtilities::Wtoi(L"-0"));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, MemMove)
    {
    char str[] = "foo-bar";

    BeStringUtilities::Memmove(&str[3],4, &str[4], 4);

    EXPECT_STREQ("foobar", str);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, WMemMove)
    {
    wchar_t str[] = L"foo-bar";

    BeStringUtilities::Wmemmove(&str[3],4, &str[4], 4);

    EXPECT_STREQ(L"foobar", str);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, Wcslwr)
    {
    wchar_t str1[] = L"ASCII";
    EXPECT_STREQ(L"ascii",  BeStringUtilities::Wcslwr(str1));
    wchar_t str2[] = L"Ascii";
    EXPECT_STREQ(L"ascii", BeStringUtilities::Wcslwr(str2));
    wchar_t str3[] = L"Dgn V8";
    EXPECT_STREQ(L"dgn v8", BeStringUtilities::Wcslwr(str3));

    // Start with a non-ascii string. In this case, it has no lower-case version. So, lwr should do nothing.
    WCharCP nonasc = L"\u20AC"; // this is the Euro symbol
    WChar nonasc_buf[2] = {L'\u20AC', L'\0'};
    EXPECT_STREQ(nonasc, BeStringUtilities::Wcslwr(nonasc_buf));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, Wcsupr)
    {
    wchar_t str1[] = L"ascii";
    EXPECT_STREQ(L"ASCII", BeStringUtilities::Wcsupr(str1));
    wchar_t str2[] = L"Ascii";
    EXPECT_STREQ(L"ASCII", BeStringUtilities::Wcsupr(str2));
    wchar_t str3[] = L"Dgn v8";
    EXPECT_STREQ(L"DGN V8", BeStringUtilities::Wcsupr(str3));

    // Start with a non-ascii string. In this case, it has no upper-case version. So, upr should do nothing.
    WCharCP nonasc = L"\u20AC"; // this is the Euro symbol
    WChar nonasc_buf[2] = {L'\u20AC', L'\0'};
    EXPECT_STREQ(nonasc, BeStringUtilities::Wcsupr(nonasc_buf));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, ParseDelimitedString)
    {
    WCharCP str = L"One Two 3G4,\t\"Fourty Two\"";
    bvector<WString> tokens;

    BeStringUtilities::ParseDelimitedString(tokens, str, L", ");
    wprintf(L"%ls\n%ls\n", tokens[0].c_str(), tokens[1].c_str());
    ASSERT_EQ(4, tokens.size());
    EXPECT_STREQ(L"One", tokens[0].c_str());
    EXPECT_STREQ(L"Two", tokens[1].c_str());
    EXPECT_STREQ(L"3G4", tokens[2].c_str());
    EXPECT_STREQ(L"Fourty Two", tokens[3].c_str());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, WCharUtf16Roundtrip)
    {
    // The first caller to convert strings in a process has to ensure BeStringUtilities::Initialize is called.
    initBeStringUtilities();

    WString expected(TESTDATA_StringW);
    Utf16P strUtf16 = new Utf16Char[expected.size() + 1];
    EXPECT_TRUE(SUCCESS == BeStringUtilities::WCharToUtf16(strUtf16, expected.size() + 1, expected.c_str()));

    WString strWString;
    strWString.reserve(expected.size() + 1);

    BeStringUtilities::Utf16ToWChar((WCharP)strWString.data(), expected.size() + 1, strUtf16);

    EXPECT_EQ(0, wcscmp(expected.c_str(), strWString.c_str()));
    delete[] strUtf16;
    }

#define EXPECT_SPRINTF_SSCANF(number,stringifiedNumber,Type,printfFormat,scanfFormat)\
        {\
        Type num = (Type) number;\
        Utf8String actualStr;\
        actualStr.Sprintf(printfFormat, num);\
        actualStr.ToLower();\
        EXPECT_STREQ(stringifiedNumber, actualStr.c_str()) << " Utf8String::Sprintf with " << #Type;\
        Type actualNumber = (Type) 0;\
        BE_STRING_UTILITIES_UTF8_SSCANF(stringifiedNumber, scanfFormat, &actualNumber);\
        EXPECT_EQ(number, actualNumber) << " BE_STRING_UTILITIES_UTF8_SSCANF with " << #Type << " actualNumber=" << actualNumber;\
        }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, SprintfSscanf)
    {
    #ifdef FAILS_ON_ANDROID
    EXPECT_SPRINTF_SSCANF(-8, "-8", int8_t, "%" PRId8, "%" SCNd8);
    EXPECT_SPRINTF_SSCANF(8, "8", uint8_t, "%" PRIu8, "%" SCNu8);
    EXPECT_SPRINTF_SSCANF(8, "8", uint8_t, "%" PRIx8, "%" SCNx8);
    #endif

    EXPECT_SPRINTF_SSCANF(-1616, "-1616", int16_t, "%" PRId16, "%" SCNd16);
    EXPECT_SPRINTF_SSCANF(1616, "1616", uint16_t, "%" PRIu16, "%" SCNu16);
    EXPECT_SPRINTF_SSCANF(1616, "650", uint16_t, "%" PRIx16, "%" SCNx16);

    EXPECT_SPRINTF_SSCANF(-323232, "-323232", int32_t, "%" PRId32, "%" SCNd32);
    EXPECT_SPRINTF_SSCANF(323232, "323232", uint32_t, "%" PRIu32, "%" SCNu32);
    EXPECT_SPRINTF_SSCANF(323232,"4eea0", uint32_t, "%" PRIx32, "%" SCNx32);

    EXPECT_SPRINTF_SSCANF(-64646464, "-64646464", int64_t, "%" PRId64, "%" SCNd64);
    EXPECT_SPRINTF_SSCANF(64646464, "64646464", uint64_t, "%" PRIu64, "%" SCNu64);
    EXPECT_SPRINTF_SSCANF(64646464, "3da6d40", uint64_t, "%" PRIx64, "%" SCNx64);

    EXPECT_SPRINTF_SSCANF(-12345, "-12345", int, "%d", "%d");
    EXPECT_SPRINTF_SSCANF(123, "123", unsigned, "%u", "%u");
    EXPECT_SPRINTF_SSCANF(123, "7b", unsigned, "%x", "%x");

    // "long" varies in widths across platforms/compilers, and shouldn't be used -- not worth testing.
    // Also fails on AndroidArm64 because of how long long is resolved to int64_t etc.
    // EXPECT_SPRINTF_SSCANF(-12345, "-12345", long long, "%lld", "%lld");
    // EXPECT_SPRINTF_SSCANF(12345, "12345", unsigned long long, "%llu", "%llu");
    // EXPECT_SPRINTF_SSCANF(12345, "3039", unsigned long long, "%llx", "%llx");
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, CharToPointer)
    {
    if (sizeof(void*) == 8)
        {
        void* ptr = BeStringUtilities::WCharToPointer(L"0x1122334455667788");
        EXPECT_EQ(0x1122334455667788, (uintptr_t)ptr);

        ptr = BeStringUtilities::Utf8ToPointer("0x1122334455667788");
        EXPECT_EQ(0x1122334455667788, (uintptr_t)ptr);
        }
    else if (sizeof(void*) == 4)
        {
        void* ptr = BeStringUtilities::WCharToPointer(L"0x11223344");
        EXPECT_EQ(0x11223344, (uintptr_t)ptr) << Utf8PrintfString("ptr value=%x", (unsigned)(uintptr_t)ptr).c_str();

        ptr = BeStringUtilities::Utf8ToPointer("0x11223344");
        EXPECT_EQ(0x11223344, (uintptr_t)ptr) << Utf8PrintfString("ptr value=%x", (unsigned)(uintptr_t)ptr).c_str();
        }
    else
        {
        FAIL() << "Unknown pointer size";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, ParseArgumentsUtf8)
    {
    //with aux delimiter
    Utf8CP input_str = "One Two 3G4,\"Fourty Two\"";
    bvector<Utf8String> tokens;

    BeStringUtilities::ParseArguments(tokens, input_str, ",");
    ASSERT_EQ(4, tokens.size());
    EXPECT_STREQ("One", tokens[0].c_str());
    EXPECT_STREQ("Two", tokens[1].c_str());
    EXPECT_STREQ("3G4", tokens[2].c_str());
    EXPECT_STREQ("Fourty Two", tokens[3].c_str());
    tokens.clear();
    input_str = "Coding is%followed by,good programming-practices";

    // without aux delimiter
    BeStringUtilities::ParseArguments(tokens, input_str);
    ASSERT_EQ(4, tokens.size());
    EXPECT_STREQ("Coding", tokens[0].c_str());
    EXPECT_STREQ("is%followed", tokens[1].c_str());
    EXPECT_STREQ("by,good", tokens[2].c_str());
    EXPECT_STREQ("programming-practices", tokens[3].c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, IsValidCodePage)
    {
    EXPECT_EQ(true, BeStringUtilities::IsValidCodePage(LangCodePage::Arabic));
    EXPECT_EQ(true, BeStringUtilities::IsValidCodePage(LangCodePage::Greek));
    EXPECT_EQ(false, BeStringUtilities::IsValidCodePage(LangCodePage::Unknown));

    //for local code page
#if defined (_WIN32) && !defined (BENTLEY_WINRT)
    UINT current_code_page = ::GetACP(); //::GetACP() gives code page of local machine
    LangCodePage codePage_retrieved;
    ASSERT_EQ(BentleyStatus::SUCCESS, BeStringUtilities::GetCurrentCodePage(codePage_retrieved));
    ASSERT_EQ(current_code_page, (UINT)codePage_retrieved);
    EXPECT_EQ(true, BeStringUtilities::IsValidCodePage((LangCodePage)current_code_page));
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, GetDecimalSeparator)
    {
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    wchar_t formatted[20];
    GetNumberFormatEx(LOCALE_NAME_USER_DEFAULT, 0, L"67.89", 0, formatted, 20);
    WString refrenced_str;
    WStringR decimal_separator = refrenced_str;
    ASSERT_EQ(BentleyStatus::SUCCESS, BeStringUtilities::GetDecimalSeparator(decimal_separator));
    Utf8String utf8decimal(decimal_separator);
    Utf8String utf8str(formatted);
    ASSERT_TRUE( utf8str.Contains(utf8decimal));


#elif defined (__unix__)
    WString refrenced_str;
    WStringR decimal_separator = refrenced_str;
    ASSERT_EQ(BentleyStatus::SUCCESS, BeStringUtilities::GetDecimalSeparator(decimal_separator));
    WString expected_decimal_str = L".";
    EXPECT_STREQ(expected_decimal_str.c_str(), decimal_separator.c_str());
#endif
    }
POP_DISABLE_DEPRECATION_WARNINGS
