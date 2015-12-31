/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeStringUtilities_tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/ScopedArray.h>
#include <map>

#define VERIFY(X) ASSERT_TRUE(X)
#define TESTDATA_String                     "ThisIsATest!@#$%^&*()-="
#define TESTDATA_StringW                    L"ThisIsATest!@#$%^&*()-="

#define TESTDATA_String_Lower                   "thisisatest!@#$%^&*()-="
#define TESTDATA_String_Upper                   "THISTISATEST!@#$%^&*()-="

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
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Utf16WChar method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilUtf16WChar)
    {
   uint16_t string1[]=  {48,0};
   WCharCP string2= L"0\0";
   Utf16CP string3= string1;

   int status= BeStringUtilities::CompareUtf16WChar(string3,string2);
   EXPECT_EQ(0,status);
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Utf16WCharHello method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilUtf16WCharHello)
    {
    uint16_t string1[]=  {72,69,76,76,79,0};
    WCharCP string2= L"HELLO\0";
    Utf16CP string3= string1;

    int status= BeStringUtilities::CompareUtf16WChar(string3,string2);
    EXPECT_EQ(0,status);
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of WCharToUtf16 method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilWCharToUtf16)
    {
    WCharCP string2= L"HELLO\0";
    Utf16Buffer utf16String ;
    Utf16BufferR stringUtf16= utf16String;
    size_t count= BeStringUtilities::AsManyAsPossible;

    BentleyStatus status= BeStringUtilities::WCharToUtf16(stringUtf16,string2, count);
    EXPECT_EQ(SUCCESS,status)<<status;
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of WCharToUtf16NotNull method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilWCharToUtf16NotNull)
    {
    WCharCP string2= L"HELLO";
    Utf16Buffer utf16String ;
    Utf16BufferR stringUtf16= utf16String;
    size_t count= BeStringUtilities::AsManyAsPossible;

    BentleyStatus status= BeStringUtilities::WCharToUtf16(stringUtf16,string2, count);
    EXPECT_EQ(SUCCESS,status)<<status;
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
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
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Strupr method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilStrupr)
    {
        {
        char string[] = "hello";
        char* outStr = BeStringUtilities::Strlwr(string);
        EXPECT_STREQ("HELLO", outStr) << outStr;
        }
        {
        char string[] = "HELLO 5! There?";
        char* outStr = BeStringUtilities::Strupr(string);
        EXPECT_STREQ("HELLO 5! THERE?", outStr) << outStr;
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of CopyUtf16 method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, CopyUtf16)
    {
    uint16_t string1[]=  {48,49,50,51,52,53,0};
    uint16_t string2[]= {27,28,29,30,31,0};
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
    uint16_t string4[10] = { 0 };
    Utf16P outStr3 = string4;
    BeStringUtilities::CopyUtf16(outStr3, 10, inStr);
    EXPECT_EQ(6, BeStringUtilities::Utf16Len(outStr3));
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
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
// @betest                                     Umar.Hayat                  12/15
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
// @betest                                     Umar.Hayat                  12/15
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, LexicographicCompare)
    {
    wchar_t string0[] = L"Hello";
    wchar_t string1[] = L"Helloo";
    wchar_t string2[] = L"Hello9";
    wchar_t string3[] = L"Hello10";

    int status = BeStringUtilities::LexicographicCompare(string0, string0);
    EXPECT_EQ(0, status) << status;

    status = BeStringUtilities::LexicographicCompare(string0, string1);
    EXPECT_GT(0, status) << status;

    status = BeStringUtilities::LexicographicCompare(string1, string0);
    EXPECT_LT(0, status) << status;

    status = BeStringUtilities::LexicographicCompare(string2, string3);
    EXPECT_GT(0, status) << status;

    status = BeStringUtilities::LexicographicCompare(string3, string2);
    EXPECT_LT(0, status) << status;
    }
//---------------------------------------------------------------------------------------
// @betest                                     Umar.Hayat                  12/15
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
// @betest                                     Umar.Hayat                  12/15
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, Duplicate)
    {
        {
        char * inStr = TESTDATA_String;
        char * outStr = BeStringUtilities::Strdup(inStr);
        EXPECT_TRUE(inStr != outStr);
        EXPECT_STREQ(inStr, outStr) ;
        delete outStr;
        }

        {
        wchar_t * inStr = TESTDATA_StringW;
        wchar_t * outStr = BeStringUtilities::Wcsdup(inStr);
        EXPECT_TRUE(inStr != outStr);
        EXPECT_STREQ(inStr, outStr);
        delete outStr;
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                     Umar.Hayat                  12/15
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
// @betest                                     Umar.Hayat                  12/15
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
// @betest                                     Umar.Hayat                  12/15
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
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Wmemcpy method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilWmemcpy)
    {
    wchar_t dest[]= L"123456";
    const wchar_t src[]= L"DELETE";
 
    BeStringUtilities::Wmemcpy(dest, _countof(dest), src,  _countof(src));
    EXPECT_EQ(SUCCESS,BeStringUtilities::Wmemcpy(dest, _countof(dest), src,  _countof(src)));
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
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
// @betest                                     Hassan.Arshad                  10/13
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
// @betest                                     Hassan.Arshad                  10/13
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringCurrentLocaleToWCharStatusCheck)
    {
    char inChar[]= "Hello";
    WString outWCharInitial= L"Hello";
    
    BentleyStatus status= BeStringUtilities::CurrentLocaleCharToWChar(outWCharInitial,inChar );
    EXPECT_EQ(SUCCESS, status);
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
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
    EXPECT_TRUE(BeStringUtilities::IsUriEncoded(encodedStr.c_str()));
    EXPECT_FALSE(BeStringUtilities::IsUriEncoded(inStr));

    //Decode
    Utf8String decodedStr = BeStringUtilities::UriDecode(encodedStr.c_str());
    EXPECT_STREQ("\"This a Test, OK?\"", decodedStr.c_str());
    
    // Decode SubURI
    Utf8String decodeSubStr = BeStringUtilities::UriDecode(decodedStr.c_str(), decodedStr.c_str() + 13);
    EXPECT_STREQ("\"This a Test,", decodeSubStr.c_str());    
    }

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                      03/14
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, TestWcsstr)
    {
    wchar_t wcs[] = L"This is a simple string";
    auto pwc = wcsstr (wcs,L"simple");
    ASSERT_TRUE (NULL != pwc);
    EXPECT_TRUE (0==wcsncmp (pwc, L"simple", 6));

    pwc = wcsstr (wcs, L"sample");
    EXPECT_TRUE (NULL == pwc);

    pwc = wcsstr (wcs, wcs);
    ASSERT_TRUE (NULL != pwc);
    EXPECT_TRUE (pwc == wcs);

    pwc = wcsstr (wcs, L"g");
    ASSERT_TRUE (NULL != pwc);
    EXPECT_TRUE (0==wcscmp (pwc, L"g"));    // pwc should be pointing to the g at the end of the string
    }

#include <cwctype>
//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                      03/14
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, StrLwr)
    {
    char asc[] = "ASCII";
    BeStringUtilities::Strlwr (asc);
    EXPECT_STREQ( asc, "ascii" );

    // Start with a non-ascii string. In this case, it has no lower-case version.
    WCharCP nonasc = L"\u20AC"; // this is the Euro symbol
    //  Convert to UTF8 and lowercase it
    Utf8String nonasc_utf8 (nonasc);    // s/ be E2 82 AC 00
    EXPECT_FALSE( nonasc_utf8.IsAscii() );
    nonasc_utf8.ToLower();
    WString nonascAfter (nonasc_utf8.c_str(), BentleyCharEncoding::Utf8);
    EXPECT_STREQ (nonascAfter.c_str(), nonasc); // s/ be a nop
    // Do the same thing using a char array
    ScopedArray<char> nonascbuf (nonasc_utf8.size() + 1);
    strcpy (nonascbuf.GetData(), nonasc_utf8.c_str());
    BeStringUtilities::Strlwr (nonascbuf.GetData());
    WString nonascAfterBuf (nonascbuf.GetData(), BentleyCharEncoding::Utf8);
    EXPECT_STREQ (nonascAfterBuf.c_str(), nonasc);

#ifdef COMMENT_OUT // *** does not work. depends on locale
    //  Now make sure that tolower and comparei agree. Use a non-ascii string that has a different lowercase version.
    wchar_t Ntilde[] = L"\u00D1";
    wchar_t ntilde[] = L"\u00F1";

    EXPECT_TRUE( std::towlower (*Ntilde) == *ntilde );

    EXPECT_TRUE(                    wcscmp  (Ntilde, ntilde) != 0 );
    EXPECT_TRUE( BeStringUtilities::Wcsicmp (Ntilde, ntilde) == 0 );
    wchar_t wbuf[_countof(Ntilde)];
    wcscpy (wbuf, Ntilde);
    BeStringUtilities::Wcslwr (wbuf);
    EXPECT_TRUE( wcscmp (ntilde, wbuf) == 0 );
    EXPECT_TRUE( wcscmp (Ntilde, wbuf) != 0 );
    EXPECT_TRUE( BeStringUtilities::Wcsicmp (Ntilde, wbuf) == 0 );

    WCharCP nonascwithcase = L"\u00D1"; // Ntilde
    WString nonascwithcaseWString (nonascwithcase);
    WString nonascwithcaseWStringLower (nonascwithcaseWString);
    nonascwithcaseWStringLower.ToLower(); // s/b \u00F1 ntilde
    EXPECT_TRUE( nonascwithcaseWString.CompareTo  (nonascwithcaseWStringLower) != 0 );
    EXPECT_TRUE( nonascwithcaseWString.CompareToI (nonascwithcaseWStringLower) == 0 );

    Utf8String nonascwithcaseUtf8String (nonascwithcase); // s/be 0xC3 0x91 
    Utf8String nonascwithcaseUtf8StringLower (nonascwithcaseUtf8String);
    nonascwithcaseUtf8StringLower.ToLower(); // s/be 0xC3 0xB1
    EXPECT_TRUE( nonascwithcaseUtf8String.CompareTo (nonascwithcaseUtf8StringLower) != 0 );
    EXPECT_TRUE( nonascwithcaseUtf8String.CompareToI(nonascwithcaseUtf8StringLower) == 0 );
#endif
    }

//---------------------------------------------------------------------------------------
// @betest                                      Jeff.Marker                    11/15
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, IsInvalidUtf8Sequence)
    {
    static const Utf8CP TEST_URI = "pw://VILTEST2-5.bentley.com:PW_Ora/Documents/Ega/variuos_names/%AD%E4%CE%F3n%E5,%FD%85%DD%FC%BF%EB%B5%A4%C8gY.pdf";
    ASSERT_FALSE(BeStringUtilities::IsInvalidUtf8Sequence(TEST_URI));
    Utf8String decodedUri = BeStringUtilities::UriDecode(TEST_URI);
    ASSERT_TRUE(BeStringUtilities::IsInvalidUtf8Sequence(decodedUri.c_str()));
    }

//---------------------------------------------------------------------------------------
//                                       Jeff.Marker                 10/14
//---------------------------------------------------------------------------------------
static void initBeStringUtilities()
    {
    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDir);
    BeStringUtilities::Initialize(assetsDir);
    }


//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
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
    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
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
    SUCCEED();
    }

//---------------------------------------------------------------------------------------
// @betest                                      
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, Itow)
    {
    wchar_t buf[10];
    ASSERT_TRUE( BeStringUtilities::Itow (buf, 1, _countof(buf), 10) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp (buf, L"1") );

    ASSERT_TRUE( BeStringUtilities::Itow (buf, 1, _countof(buf), 16) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp (buf, L"1") );

    ASSERT_TRUE( BeStringUtilities::Itow (buf, 1, _countof(buf), 2) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp (buf, L"1") );

#if defined (WIP_UNDERSIZED_BUFFER_IS_FATAL_ERROR_ON_WINDOWS)
    ASSERT_TRUE( BeStringUtilities::Itow (buf, 1, 0, 10) != SUCCESS );
    ASSERT_TRUE( BeStringUtilities::Itow (buf, 1, 1, 10) != SUCCESS );
#endif
    ASSERT_TRUE( BeStringUtilities::Itow (buf, 1, 2, 10) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp (buf, L"1") );

    ASSERT_TRUE( BeStringUtilities::Itow (buf, 16, _countof(buf), 10) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp (buf, L"16") );

    ASSERT_TRUE( BeStringUtilities::Itow (buf, 16, _countof(buf), 16) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp (buf, L"10") );

    ASSERT_TRUE( BeStringUtilities::Itow (buf, 16, _countof(buf), 2) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp (buf, L"10000") );

#if defined (WIP_UNDERSIZED_BUFFER_IS_FATAL_ERROR_ON_WINDOWS)
    ASSERT_TRUE( BeStringUtilities::Itow (buf, 16, 0, 10) != SUCCESS );
    ASSERT_TRUE( BeStringUtilities::Itow (buf, 16, 1, 10) != SUCCESS );
    ASSERT_TRUE( BeStringUtilities::Itow (buf, 16, 2, 10) != SUCCESS );
#endif
    ASSERT_TRUE( BeStringUtilities::Itow (buf, 16, 3, 10) == SUCCESS );
    ASSERT_TRUE( 0==wcscmp (buf, L"16") );

    for (int i=0; i<1000; ++i)
        {
        ASSERT_TRUE( SUCCESS == BeStringUtilities::Itow (buf, i, _countof(buf), 10) );
        ASSERT_TRUE( WPrintfString(L"%d", i) == buf );

        ASSERT_TRUE( SUCCESS == BeStringUtilities::Itow (buf, i, _countof(buf), 16) );
        ASSERT_TRUE( WPrintfString(L"%x", i) == buf );

        ASSERT_TRUE( SUCCESS == BeStringUtilities::Itow (buf, i, _countof(buf), 8) );
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
    BeStringUtilities::Utf8ToUtf16 (utf16, const_ascii);    // utf8->utf16
    WString wstr;
    BeStringUtilities::Utf16ToWChar (wstr, utf16.data());   // utf16->wchar
    ASSERT_TRUE( wstr.compare (const_asciiW) == 0 );
    Utf16Buffer utf162;
    BeStringUtilities::WCharToUtf16 (utf162, wstr.c_str(), BeStringUtilities::AsManyAsPossible);    // utf16<-wchar
    ASSERT_TRUE (BeStringUtilities::CompareUtf16 (utf162.data(), utf16.data()) == 0);
    char asc[256];
    BeStringUtilities::WCharToCurrentLocaleChar (asc, wstr.c_str(), _countof(asc)); // locale <- wchar
    ASSERT_TRUE (0 == strcmp (asc, const_ascii) );
    }

//---------------------------------------------------------------------------------------
// @betest                                      
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTests, FmtLongLong)
    {
    wchar_t buf[64];

    uint64_t i64_minus_one = -1;
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%llx", i64_minus_one);
    ASSERT_STREQ( buf, L"ffffffffffffffff");
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%lld", i64_minus_one);
    ASSERT_STREQ( buf, L"-1");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_minus_one);
    //ASSERT_STREQ( buf, L"ffffffffffffffff");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_minus_one);
    //ASSERT_STREQ( buf, L"-1");

    uint64_t i64_uint64_max = UINT64_MAX;
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%llx", i64_uint64_max);
    ASSERT_STREQ( buf, L"ffffffffffffffff");
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%lld", i64_uint64_max);
    ASSERT_STREQ( buf, L"-1");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_uint64_max);
    //ASSERT_STREQ( buf, L"ffffffffffffffff");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_uint64_max);
    //ASSERT_STREQ( buf, L"-1");

    uint64_t i64_int64_max = INT64_MAX;
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%llx", i64_int64_max);
    ASSERT_STREQ( buf, L"7fffffffffffffff");
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%lld", i64_int64_max);
    ASSERT_STREQ( buf, L"9223372036854775807");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_int64_max);
    //ASSERT_STREQ( buf, L"7fffffffffffffff");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_int64_max);
    //ASSERT_STREQ( buf, L"9223372036854775807");

    uint64_t i64_one = 1;
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%llx", i64_one);
    ASSERT_STREQ( buf, L"1");
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%lld", i64_one);
    ASSERT_STREQ( buf, L"1");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_one);
    //ASSERT_STREQ( buf, L"1");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_one);
    //ASSERT_STREQ( buf, L"1");

    uint64_t i64_zero = 0;
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%llx", i64_zero);
    ASSERT_STREQ( buf, L"0");
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%lld", i64_zero);
    ASSERT_STREQ( buf, L"0");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_zero);
    //ASSERT_STREQ( buf, L"0");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_zero);
    //ASSERT_STREQ( buf, L"0");

    uint64_t i64_max_int = (uint64_t)INT32_MAX;
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%llx", i64_max_int);
    ASSERT_STREQ( buf, L"7fffffff");
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%lld", i64_max_int);
    ASSERT_STREQ( buf, L"2147483647");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_max_int);
    //ASSERT_STREQ( buf, L"7fffffff");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_max_int);
    //ASSERT_STREQ( buf, L"2147483647");

    uint64_t i64_uint32_max = (uint64_t)UINT32_MAX;
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%llx", i64_uint32_max);
    ASSERT_STREQ( buf, L"ffffffff");
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%lld", i64_uint32_max);
    ASSERT_STREQ( buf, L"4294967295");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_uint32_max);
    //ASSERT_STREQ( buf, L"ffffffff");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_uint32_max);
    //ASSERT_STREQ( buf, L"4294967295");

    uint64_t i64_uint32_max_plus_one = (uint64_t)UINT32_MAX + 1;
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%llx", i64_uint32_max_plus_one);
    ASSERT_STREQ( buf, L"100000000");
    *buf = 0;
    BeStringUtilities::Snwprintf (buf, L"%lld", i64_uint32_max_plus_one);
    ASSERT_STREQ( buf, L"4294967296");
    *buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64x", i64_uint32_max_plus_one);
    //ASSERT_STREQ( buf, L"100000000");
    //*buf = 0;
    //BeStringUtilities::Snwprintf (buf, L"%I64d", i64_uint32_max_plus_one);
    //ASSERT_STREQ( buf, L"4294967296");    
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    09/13
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, FormatUInt64)
    {
        {
        uint64_t number = 0ULL;
        WString str;
        str.reserve (2); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((WCharP) str.data(), number);
        ASSERT_STREQ (L"0", str.c_str ());
        }

        {
        uint64_t number = 43123ULL;
        WString str;
        str.reserve (6); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((WCharP) str.data(), number);
        ASSERT_STREQ (L"43123", str.c_str ());
        }

        {
        uint64_t number = 14235263432521323ULL;
        WString str;
        str.reserve (21); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((WCharP) str.data(), number);
        ASSERT_STREQ (L"14235263432521323", str.c_str ());
        }
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.hayat                          12/15
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, FormatUInt64FromUtf8)
    {
        {
        uint64_t number = 0ULL;
        Utf8String str;
        str.reserve (2); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((Utf8P)str.data(), number);
        ASSERT_STREQ ("0", str.c_str ());
        }

        {
        uint64_t number = 43123ULL;
        Utf8String str;
        str.reserve (6); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((Utf8P)str.data(), number);
        ASSERT_STREQ ("43123", str.c_str ());
        }

        {
        uint64_t number = 14235263432521323ULL;
        Utf8String str;
        str.reserve (21); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((Utf8P)str.data(), number);
        ASSERT_STREQ ("14235263432521323", str.c_str ());
        }
}
//---------------------------------------------------------------------------------------
// @betest                                      Umar.hayat                          12/15
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, HexFormatOptions)
    {
        {
        uint64_t number = 65297ULL;
        WString str;
        str.reserve (10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((WCharP) str.data(), 10,number,HexFormatOptions::None);
        EXPECT_STREQ (L"ff11", str.c_str ());
        }
        
        {
        uint64_t number = 65297ULL;
        WString str;
        str.reserve (10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((WCharP) str.data(), 10,number,HexFormatOptions::None,6);
        EXPECT_STREQ (L"  ff11", str.c_str ());
        }
        
        {
        uint64_t number = 65297ULL;
        WString str;
        str.reserve (10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((WCharP) str.data(), 10,number,HexFormatOptions::LeadingZeros,6);
        EXPECT_STREQ (L"00ff11", str.c_str ());
        }

        {
        uint64_t number = 65297ULL;
        WString str;
        str.reserve (10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((WCharP) str.data(), 10,number,HexFormatOptions::UsePrecision,6);
        EXPECT_STREQ (L"  ff11", str.c_str ());
        }
        
        {
        uint64_t number = 65297ULL;
        WString str;
        str.reserve (10); //max length in hex format for an UInt64 (incl. trailing \0)
        int opts = (int)HexFormatOptions::LeftJustify | (int)HexFormatOptions::Uppercase | (int)HexFormatOptions::UsePrecision;
        BeStringUtilities::FormatUInt64((WCharP)str.data(), 10, number, (HexFormatOptions)opts, 8, 6);
        EXPECT_STREQ (L"00FF11  ", str.c_str ());
        }
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.hayat                          12/15
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, HexFormatOptionsUtf8)
    {
        {
        uint64_t number = 65297ULL;
        Utf8String str;
        str.reserve (10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((Utf8P) str.data(), 10,number,HexFormatOptions::None);
        EXPECT_STREQ ("ff11", str.c_str ());
        }
        
        {
        uint64_t number = 65297ULL;
        Utf8String str;
        str.reserve (10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((Utf8P) str.data(), 10,number,HexFormatOptions::None,6);
        EXPECT_STREQ ("  ff11", str.c_str ());
        }
        
        {
        uint64_t number = 65297ULL;
        Utf8String str;
        str.reserve (10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((Utf8P) str.data(), 10,number,HexFormatOptions::LeadingZeros,6);
        EXPECT_STREQ ("00ff11", str.c_str ());
        }

        {
        uint64_t number = 65297ULL;
        Utf8String str;
        str.reserve (10); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatUInt64 ((Utf8P) str.data(), 10,number,HexFormatOptions::UsePrecision,6);
        EXPECT_STREQ ("  ff11", str.c_str ());
        }
        
        {
        uint64_t number = 65297ULL;
        Utf8String str;
        str.reserve (10); //max length in hex format for an UInt64 (incl. trailing \0)
        int opts = (int)HexFormatOptions::LeftJustify | (int)HexFormatOptions::Uppercase | (int)HexFormatOptions::UsePrecision;
        BeStringUtilities::FormatUInt64 ((Utf8P)str.data(), 10, number, (HexFormatOptions)opts, 8, 6);
        EXPECT_STREQ ("00FF11  ", str.c_str ());
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    09/13
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, FormatHexUInt64)
    {
        {
        uint64_t number = 0ULL;
        WString str;
        str.reserve (17); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatHexUInt64 ((WCharP) str.data(), number);
        ASSERT_STREQ (L"0", str.c_str ());
        }

        {
        uint64_t number = 15ULL;
        WString str;
        str.reserve (17); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatHexUInt64 ((WCharP) str.data(), number);
        ASSERT_STREQ (L"f", str.c_str ());
        }

        {
        uint64_t number = 14235263432521323ULL;
        WString str;
        str.reserve (17); //max length in hex format for an UInt64 (incl. trailing \0)
        BeStringUtilities::FormatHexUInt64 ((WCharP) str.data(), number);
        ASSERT_STREQ (L"3292e58c2df66b", str.c_str ());
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    09/13
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, ParseUInt64)
    {
    WCharCP str = L"14235263432521323";
    uint64_t number = 0ULL;
    BentleyStatus stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (SUCCESS, stat);
    ASSERT_EQ (14235263432521323ULL, number);

    str = L"00014235263432521323";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (SUCCESS, stat);
    ASSERT_EQ (14235263432521323ULL, number);

    str = L"-1423526343";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (ERROR, stat);

    str = L"0xff";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (ERROR, stat) << "Number in hexadecimal format is not expected to be supported by BeStringUtilities::ParseUInt64";

    str = L"ff";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (ERROR, stat) << "Number in hexadecimal format is not expected to be supported by BeStringUtilities::ParseUInt64";

    str = L"1234aa54345";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (ERROR, stat) << "Number in hexadecimal format is not expected to be supported by BeStringUtilities::ParseUInt64";

    str = L"blabla";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (ERROR, stat);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    12/13
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, ParseUInt64FromUtf8)
    {
    Utf8CP str = "14235263432521323";
    uint64_t number = 0ULL;
    BentleyStatus stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (SUCCESS, stat);
    ASSERT_EQ (14235263432521323ULL, number);

    str = "00014235263432521323";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (SUCCESS, stat);
    ASSERT_EQ (14235263432521323ULL, number);

    str = "-1423526343";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (ERROR, stat);

    str = "0xff";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (ERROR, stat) << "Number in hexadecimal format is not expected to be supported by BeStringUtilities::ParseUInt64";

    str = "ff";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (ERROR, stat) << "Number in hexadecimal format is not expected to be supported by BeStringUtilities::ParseUInt64";

    str = "1234aa54345";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (ERROR, stat) << "Number in hexadecimal format is not expected to be supported by BeStringUtilities::ParseUInt64";

    str = "blabla";
    number = 0ULL;
    stat = BeStringUtilities::ParseUInt64 (number, str);
    ASSERT_EQ (ERROR, stat);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                    12/15
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, ParseHexUInt64)
    {
    WCharCP str = L"3292E58C2DF66B";
    uint64_t number = 0ULL;
    BentleyStatus stat = BeStringUtilities::ParseHexUInt64(number, str);
    ASSERT_EQ (SUCCESS, stat);
    ASSERT_EQ (14235263432521323ULL, number);

    str = L"003292E58C2DF66B";
    number = 0ULL;
    stat = BeStringUtilities::ParseHexUInt64(number, str);
    ASSERT_EQ (SUCCESS, stat);
    ASSERT_EQ (14235263432521323ULL, number);
    
    str = L"0x3292E58C2DF66B";
    number = 0ULL;
    stat = BeStringUtilities::ParseHexUInt64(number, str);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(14235263432521323ULL, number);
    
    str = L"0X3292E58C2DF66B";
    number = 0ULL;
    stat = BeStringUtilities::ParseHexUInt64(number, str);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(14235263432521323ULL, number);

    str = L"0x003292E58C2DF66B";
    number = 0ULL;
    stat = BeStringUtilities::ParseHexUInt64(number, str);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(14235263432521323ULL, number);

    str = L"0X1";
    number = 0ULL;
    stat = BeStringUtilities::ParseHexUInt64(number, str);
    ASSERT_EQ(SUCCESS, stat);
    ASSERT_EQ(1ULL, number);

    str = L"-1423526343";
    number = 0ULL;
    stat = BeStringUtilities::ParseHexUInt64(number, str);
    ASSERT_EQ (ERROR, stat);
    
    str = L"blabla";
    number = 0ULL;
    stat = BeStringUtilities::ParseHexUInt64(number, str);
    ASSERT_EQ (ERROR, stat);
    }

