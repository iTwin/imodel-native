/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/OsEnvUnitTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeStringUtilities.h>
#include <Logging/bentleylogging.h>
#include <regex>
#if !defined (_WIN32)
#include <regex.h>
#endif

/* */
struct MyEmptyStruct {};
struct MyNonEmptyStruct { int val; };

/* */
struct MyBase1 
{
virtual ~MyBase1 () {}
virtual void v1 () = 0;
};

struct MyBase2 
{
virtual ~MyBase2 () {}
virtual void v2 () = 0;
};

struct MyDerived : MyBase1, MyBase2
{
virtual void v1 () override {};
virtual void v2 () override {};
};

struct MySub1 : MyBase1
{
virtual void v1 () override {};
};


//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (AssertTests, AssertTrue)
{
    assert (TRUE);
    SUCCEED ();
}


#if defined (NOT_NOW)
//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (AssertTests, AssertFalse)
{
#if defined (ANDROID)
    // Calling "assert" on Android will crash JUnit
    FAIL ();
#else
    assert (FALSE);
    FAIL ();
#endif
}
#endif

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                      07/13
//---------------------------------------------------------------------------------------
TEST(AssertTests,SetThrowOnAssertFalse)
    {
    BeTest::SetFailOnAssert (false);
    BeAssert(false);    // should not trigger a failure, because asserts are disabled.
    BeTest::SetFailOnAssert (true);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (AssertTests, BeTestAssertMacros)
{
    ASSERT_EQ (1, 1);
    ASSERT_NE (1, 2);
    ASSERT_LE (1, 1);
    ASSERT_LE (1, 2);
    ASSERT_LT (1, 2);
    ASSERT_GE (1, 1);
    ASSERT_GE (2, 1);
    ASSERT_GT (2, 1);
    ASSERT_TRUE (TRUE);
    ASSERT_FALSE (FALSE);

    EXPECT_EQ (1, 1);
    EXPECT_NE (1, 2);
    EXPECT_LE (1, 1);
    EXPECT_LE (1, 2);
    EXPECT_LT (1, 2);
    EXPECT_GE (1, 1);
    EXPECT_GE (2, 1);
    EXPECT_GT (2, 1);
    EXPECT_TRUE (TRUE);
    EXPECT_FALSE (FALSE);
    
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (BuildEnvTests, IsEmptyStruct)
{
    ASSERT_TRUE (STD_TR1::is_empty<MyEmptyStruct>::value);
    ASSERT_FALSE (STD_TR1::is_empty<MyNonEmptyStruct>::value);
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                                      John.Gooding    02/2012
//--------------+------------------------------------------------------------------------
TEST (MemoryTests, BeStackTest)
    {
    char    testBuffer [500000];
    memset (testBuffer, 0, sizeof (testBuffer));

    //  If it doesn't crash it passes.
    SUCCEED ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, SprintfS)
{
    char buffer[128];
    strcpy (buffer, "?");

    int numCharsPrinted = BeStringUtilities::Snprintf (buffer, "s=%hs", "s");

    ASSERT_TRUE (numCharsPrinted > 0);
    ASSERT_EQ (0, strcmp (buffer, "s=s"));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, SprintfLLD)
{
    char    buffer[128];
    int64_t num64 = -1;
    
    strcpy (buffer, "?");

    int numCharsPrinted = BeStringUtilities::Snprintf (buffer, "lld=%lld", num64);

    ASSERT_TRUE (numCharsPrinted > 0);
    ASSERT_EQ (0, strcmp (buffer, "lld=-1"));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, SprintfLLU)
{
    char    buffer[128];
    uint64_t num64 = 103;
    
    strcpy (buffer, "?");

    int numCharsPrinted = BeStringUtilities::Snprintf (buffer, "llu=%llu", num64);

    ASSERT_TRUE (numCharsPrinted > 0);
    ASSERT_EQ (0, strcmp (buffer, "llu=103"));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Snwprintf)
{
    wchar_t buffer[128];

    buffer[0] = '?';
    buffer[1] = 0;

    int numCharsPrinted = BeStringUtilities::Snwprintf (buffer, _countof (buffer), L"d=%d", 5);

    ASSERT_TRUE (numCharsPrinted > 0);
    ASSERT_EQ (0, wcscmp (buffer, L"d=5"));
    SUCCEED ();
}

#if defined (_MSC_VER)
#pragma warning (disable:4189)
#endif

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Casts)
{
    MyBase1*   b1  = new MyDerived ();
    MyBase2*   b2  = new MyDerived ();
    MyDerived* dc1 = (MyDerived*) b1;
    MyDerived* ds1 = static_cast<MyDerived*>(b1);
    MyDerived* dd1 = dynamic_cast<MyDerived*>(b1);
    MyDerived* dr1 = reinterpret_cast<MyDerived*>(b1);
    MyDerived* dc2 = (MyDerived*) b2;
    MyDerived* ds2 = static_cast<MyDerived*>(b2);
    MyDerived* dd2 = dynamic_cast<MyDerived*>(b2);
    MyDerived* dr2 = reinterpret_cast<MyDerived*>(b2);
    MyBase2*   b2c = dc1;
    MyBase1*   b12 = dynamic_cast<MyBase1*>(b2);
    MyBase2*   b21 = dynamic_cast<MyBase2*>(b1);
    MyBase1*   r12 = reinterpret_cast<MyBase1*>(b2);
    MyBase2*   r21 = reinterpret_cast<MyBase2*>(b1);
    MySub1*    s1  = new MySub1 ();
    MyBase2*   b2s = dynamic_cast<MyBase2*>(s1);

    ASSERT_TRUE (NULL != dd1);
    ASSERT_TRUE (NULL != dd2);
    ASSERT_TRUE (NULL != b12);
    ASSERT_TRUE (NULL != b21);
    ASSERT_TRUE (NULL == b2s);

    delete b1;
    delete b2;
    delete s1;

    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswalnum)
{
    ASSERT_TRUE (0 != iswalnum ('A'));
    ASSERT_TRUE (0 != iswalnum ('1'));
    ASSERT_TRUE (0 == iswalnum (WEOF));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswalpha)
{
    ASSERT_TRUE (0 != iswalpha ('A'));
    ASSERT_TRUE (0 == iswalpha (WEOF));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswcntrl)
{
    ASSERT_TRUE (0 != iswcntrl ('\n'));
    ASSERT_TRUE (0 == iswcntrl ('A'));
    ASSERT_TRUE (0 == iswcntrl ('1'));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswdigit)
{
    ASSERT_TRUE (0 != iswdigit ('1'));
    ASSERT_TRUE (0 == iswdigit ('A'));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswgraph)
{
    ASSERT_TRUE (0 != iswgraph ('A'));
    ASSERT_TRUE (0 == iswgraph (' '));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswlower)
{
    ASSERT_TRUE (0 != iswlower ('a'));
    ASSERT_TRUE (0 == iswlower ('A'));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswprint)
{
    ASSERT_TRUE (0 != iswprint ('A'));
    ASSERT_TRUE (0 == iswprint (WEOF));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswpunct)
{
    ASSERT_TRUE (0 != iswpunct (';'));
    ASSERT_TRUE (0 == iswpunct ('A'));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswspace)
{
    ASSERT_TRUE (0 != iswspace (' '));
    ASSERT_TRUE (0 == iswspace ('A'));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswupper)
{
    ASSERT_TRUE (0 != iswupper ('A'));
    ASSERT_TRUE (0 == iswupper ('a'));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswxdigit)
{
    ASSERT_TRUE (0 != iswxdigit ('A'));
    ASSERT_TRUE (0 == iswxdigit ('G'));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Iswctype)
{
#if defined (ANDROID)
    // not sure if iswctype is portable, so keep this test Android-specific for now
    ASSERT_TRUE (iswctype ('A', WC_TYPE_ALNUM));
    ASSERT_TRUE (iswctype ('A', WC_TYPE_ALPHA));
    ASSERT_TRUE (iswctype ('\n', WC_TYPE_CNTRL));
    ASSERT_TRUE (iswctype ('1', WC_TYPE_DIGIT));
    ASSERT_TRUE (iswctype ('1', WC_TYPE_GRAPH));
    ASSERT_TRUE (iswctype ('a', WC_TYPE_LOWER));
    ASSERT_TRUE (iswctype ('A', WC_TYPE_PRINT));
    ASSERT_TRUE (iswctype (';', WC_TYPE_PUNCT));
    ASSERT_TRUE (iswctype (' ', WC_TYPE_SPACE));
    ASSERT_TRUE (iswctype ('A', WC_TYPE_UPPER));
    ASSERT_TRUE (iswctype ('A', WC_TYPE_XDIGIT));
#endif
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Swscanf)
{
    int d = 0;
    BE_STRING_UTILITIES_SWSCANF (L"1252", L"%d", &d);
    ASSERT_TRUE (1252 == d);
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Wcstod)
{
    double d = BeStringUtilities::Wcstod (L"1252", NULL);
    ASSERT_TRUE (1252.0 == d);
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Wcstol)
{
    long l = BeStringUtilities::Wcstol (L"1252", NULL, 0);
    ASSERT_TRUE (1252 == l);
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Wcstoul)
{
    unsigned long ul = BeStringUtilities::Wcstoul (L"1252", NULL, 0);
    ASSERT_TRUE (1252 == ul);
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Towlower)
{
    int wc = towlower ('A');
    ASSERT_TRUE ('a' == wc);
    wc = towlower ('a');
    ASSERT_TRUE ('a' == wc);
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Towupper)
{
    int wc = towupper ('A');
    ASSERT_TRUE ('A' == wc);
    wc = towupper ('a');
    ASSERT_TRUE ('A' == wc);
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Wcscat)
{
    wchar_t buffer[128];

    buffer[0] = 0;
    wcscat (buffer, L"abc");
    wcscat (buffer, L"def");

    ASSERT_TRUE (0 == wcscmp (buffer, L"abcdef"));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Wcschr)
{
    WCharCP ws = L"xxx_A_xxx";
    wchar_t wc = 'A';

    ASSERT_TRUE (0 != wcschr (ws, wc));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    01/12
//---------------------------------------------------------------------------------------
TEST (CRuntimeTests, Wcsstr)
{
    WCharCP ws1 = L"xxx_FindMe_xxx";
    WCharCP ws2 = L"FindMe";

    ASSERT_TRUE (0 != wcsstr (ws1, ws2));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (BuildEnvTests, ReturnStatus)
{
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (BuildEnvTests, Sizeof)
{
#if defined (_WIN32)

    ASSERT_EQ (2, sizeof (wchar_t));

    #if defined (_WIN64)
        ASSERT_EQ (8, sizeof (void*));
    #else
        ASSERT_EQ (4, sizeof (void*));
    #endif

#elif defined (__unix__)

    ASSERT_EQ (4, sizeof (wchar_t));
    //ASSERT_EQ (4, sizeof (void*));    now we have 32- and 64-bit unix builds.

#endif

    SUCCEED ();
}

#if defined (_MSC_VER)
#pragma warning (disable:4189)
#endif

//---------------------------------------------------------------------------------------
// ensure Bentley typedefs are properly seen by the compiler - which implies header files 
//  were properly processed...
//
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (BuildEnvTests, BentleyTypedefs)
{
    StatusInt   status = 0;
    WCharCP     wcharCP = 0;
    WCharP      wcharP = 0;
    CharCP      charCP = 0;
    CharP       charP = 0;
    Utf8CP      utf8P = 0;
    int8_t      numI8 = 0;
    uint8_t     numUI8 = 0;
    int16_t     numI16 = 0;
    uint16_t    numUI16 = 0;
    int32_t     numI32 = 0;
    uint32_t    numUI32 = 0;
    int64_t     numI64 = 0;
    uint64_t    numUI64 = 0;
    int8_t      xI8 = 0;
    uint8_t     xUI8 = 0;
    int16_t     xI16 = 0;
    uint16_t    xUI16 = 0;
    int32_t     xI32 = 0;
    uint32_t    xUI32 = 0;
    int64_t     xI64 = 0;
    uint64_t    xUI64 = 0;
    unsigned short us = 0;
    unsigned long ul = 0;
    short s = 0;
    unsigned int ui = 0;
    unsigned char uc = 0;
    Byte        b1 = 0;
    Byte b2 = 0;
    int32_t    l32 = 0;
    uint32_t   ul32 = 0;
    
    ASSERT_NE (ERROR,            SUCCESS);
    ASSERT_NE (TRUE,             FALSE);
    ASSERT_NE (sizeof (*charCP), sizeof (*wcharCP));
    ASSERT_NE (sizeof (ui),      sizeof (uc));
    ASSERT_NE (sizeof (us),      sizeof (ul));

    ASSERT_EQ (sizeof (wcharP),  sizeof (void*));
    ASSERT_EQ (sizeof (status),  sizeof (int));
    ASSERT_EQ (sizeof (*charP),  sizeof (*utf8P));
    ASSERT_EQ (sizeof (numI8),   sizeof (numUI8));
    ASSERT_EQ (sizeof (numI16),  sizeof (numUI16));
    ASSERT_EQ (sizeof (numI32),  sizeof (numUI32));
    ASSERT_EQ (sizeof (numI64),  sizeof (numUI64));
    ASSERT_EQ (sizeof (xI8),     sizeof (xUI8));
    ASSERT_EQ (sizeof (xI16),    sizeof (xUI16));
    ASSERT_EQ (sizeof (xI32),    sizeof (xUI32));
    ASSERT_EQ (sizeof (xI64),    sizeof (xUI64));
    ASSERT_EQ (sizeof (s),       sizeof (us));
    ASSERT_EQ (sizeof (b1),      sizeof (b2));
    ASSERT_EQ (sizeof (l32),     sizeof (ul32));
    SUCCEED ();
}

#if defined (_MSC_VER)
#pragma warning (default:4189)
#endif

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (BuildEnvTests, PlatformMacroDefined)
{
#if defined (_WIN32)

    #if defined (__unix__)
        FAIL ();
    #else
        SUCCEED ();
    #endif

#elif defined (__unix__)

    #if defined (_WIN32)
        FAIL ();
    #else
        SUCCEED ();
    #endif

#else
    FAIL ();
#endif
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    10/11
//---------------------------------------------------------------------------------------
TEST (BuildEnvTests, AndroidMacroDefined)
{
#if defined (ANDROID)

    #if defined (__unix__)
        SUCCEED ();
    #else
        FAIL ();
    #endif

#else
    SUCCEED ();
#endif
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    10/11
//---------------------------------------------------------------------------------------
TEST (BuildEnvTests, Win64MacroDefined)
{
#if defined (_WIN64)

    #if defined (_WIN32)
        SUCCEED ();
    #else
        FAIL ();
    #endif

#else
    SUCCEED ();
#endif
}

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                      03/2013
//---------------------------------------------------------------------------------------
#if !defined (_WIN32)
static void testPosixRegex (char const* regexPattern, char const* str, size_t nmatchesexpected)
    {
    regex_t regex;
    regmatch_t matches[32];
    memset (matches, 0, sizeof(matches));

    int status = regcomp (&regex, regexPattern, REG_EXTENDED|REG_ICASE);
    if (0 == status)
        status = regexec (&regex, str, _countof(matches), matches, 0);
    
    if (0 != status)
        {
        NativeLogging::LoggingManager::GetLogger(L"TestRunner")->tracev (L"%hs %hs", regexPattern, str);
        char errbuf[256];
        regerror(status, &regex, errbuf, sizeof(errbuf));
        regfree (&regex);
        FAIL() << errbuf;
        }

    size_t nfound=0;
    for (int i=0; i<_countof(matches); ++i)
        {
        int so = matches[i].rm_so;
        int eo = matches[i].rm_eo;
        if (so == -1)
            continue;
        nfound++;
        if (so < 0 || so >= strlen (str))
            NativeLogging::LoggingManager::GetLogger(L"TestRunner")->tracev (L"[%d] %d - out of range?!", i, so);
        else
            {
            if (nfound==1)
                NativeLogging::LoggingManager::GetLogger(L"TestRunner")->tracev (L"%hs matches %hs:", regexPattern, str);

            Utf8String substr (str+so, str+eo);
            NativeLogging::LoggingManager::GetLogger(L"TestRunner")->tracev (L"[%d] %hs", i, substr.c_str());
            }
        }

    regfree (&regex);

    ASSERT_EQ( nmatchesexpected, nfound ) << regexPattern << " - " << str << Utf8PrintfString (" - expected %d got %d matches", nmatchesexpected, nfound).c_str();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                      03/2013
//---------------------------------------------------------------------------------------
TEST(CRuntimeTests, posixregex)
    {
    testPosixRegex ("^a[[:alnum:]]",                                            "abc",          1+0);

    // from http://www.tutorialguide.net/introduction_to_regular_expressions_part_2_-_ere_posix_tutorial.html
    testPosixRegex ("([0-9]{4})-([0-9]{1,2})-([0-9]{1,2})",                     "1986-05-28",   1+3); // regexec matches the whole string and returns that as item 0. The matached subexpressions follow that.
    testPosixRegex ("([[:digit:]]{4})-([[:digit:]]{1,2})-([[:digit:]]{1,2})",   "1986-05-28",   1+3); 
    testPosixRegex ("([+-]?[[:digit:]]{4})-?(1[0-2]|0[1-9])-?(3[0-1]|0[1-9]|[1-2][[:digit:]])",   "1986-05-28",   1+3); 
    testPosixRegex ("([+-]?[[:digit:]]{4})-?(1[0-2]|0[1-9])-?(3[0-1]|0[1-9]|[1-2][[:digit:]])(T(2[0-3]|[0-1][[:digit:]]):?([0-5][[:digit:]])(:?([0-5][[:digit:]])(\\.[[:digit:]]+)?)?(Z|[+-](2[0-3]|[0-1][[:digit:]]):?[0-5][[:digit:]])?)?",
                                                                                                    "1986-05-28",   1+3); 
    testPosixRegex ("([+-]?[[:digit:]]{4})-?(1[0-2]|0[1-9])-?(3[0-1]|0[1-9]|[1-2][[:digit:]])(T(2[0-3]|[0-1][[:digit:]]):?([0-5][[:digit:]])(:?([0-5][[:digit:]])(\\.[[:digit:]]+)?)?(Z|[+-](2[0-3]|[0-1][[:digit:]]):?[0-5][[:digit:]])?)?",
                                                                                                    "2012-12-05T23:44:59.500Z",   1+10); 
    testPosixRegex ("([+-]?[[:digit:]]{4})-?(1[0-2]|0[1-9])-?(3[0-1]|0[1-9]|[1-2][[:digit:]])(T(2[0-3]|[0-1][[:digit:]]):?([0-5][[:digit:]])(:?([0-5][[:digit:]])(\\.[[:digit:]]+)?)?(Z|[+-](2[0-3]|[0-1][[:digit:]]):?[0-5][[:digit:]])?)?",
                                                                                                    "2012-12-05T13:11:00.1234567Z",   1+10); 
    }
#endif

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                      11/13
//---------------------------------------------------------------------------------------
TEST (CLanguageTests, SignedChar)
    {
    char c = EOF;
    ASSERT_EQ (c, EOF);
    c = -1;
    ASSERT_EQ (c, -1);
    
    // c = 0xff;  should not compile -- truncation
    }
