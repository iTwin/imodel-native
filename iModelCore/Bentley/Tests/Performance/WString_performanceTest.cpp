/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/WString_performanceTest.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"
#include <Bentley/WString.h>

    static int s_niters = 100000;
    StopWatch timer;

TEST(PerformanceBentley, WStringCopying)
    {
    WString base = L"abcdef123456789";
    timer.Start();
    for (int i=0; i<s_niters*10; ++i)
        {
        WString str = base;
        ASSERT_TRUE (!str.empty ());
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "", s_niters);
    }

TEST(PerformanceBentley, WStringCtors)
    {
    // Empty string construction
    timer.Start();
    for (int i=0; i<s_niters*10; ++i)
        {
        WString str;
        ASSERT_TRUE (str.empty ());
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Empty string Construction", s_niters);

    // simple construction
    timer.Start();
    for (int i=0; i<s_niters*10; ++i)
        {
        WString str (1, L'0'+(i%10));
        ASSERT_TRUE (!str.empty ());
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "Simple Construction", s_niters);

    }

TEST(PerformanceBentley, WStringAppendStayShort)
    {
    // append+stay short
    timer.Start();
    for (int i=0; i<s_niters/10; ++i)
        {
        WString str;

        for (int j=0; j<10; ++j)
            {
            str.append (1, static_cast <wchar_t> (L'0'+(j%10)));
            ASSERT_EQ (str.length(), 1+j);
            }
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "", s_niters);
    }

TEST(PerformanceBentley, WStringAppendGrowLong)
    {
    // append+grow to long size
    timer.Start();
    for (int i=0; i<s_niters/10; ++i)
        {
        WString str;

        for (int j=0; j<s_niters/100; ++j)
            {
            str.append (1, static_cast <wchar_t> (L'0'+(j%10)));
            ASSERT_EQ (str.length(), 1+j);
            }
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "", s_niters);
    }

TEST(PerformanceBentley, WStringCompare)
    {
    // compare
    WString str1 (L"xyz");
    timer.Start();
    for (int i=0; i<s_niters*10; ++i)
        {
        WString str (1, L'0'+(i%10));
        ASSERT_TRUE (str != str1);
        ASSERT_TRUE (str1 == str1);
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "", s_niters);
    }

TEST(PerformanceBentley, StringConversions)
    {
    char const* asc = "Now is the time for all good men to come to the aid of the party.";
    timer.Start();
    for (int i=0; i<s_niters; ++i)
        {
        WString str (asc, BentleyCharEncoding::Utf8);
        ASSERT_EQ (str[0], asc[0]);
        ASSERT_EQ (str[1], asc[1]);
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "", s_niters);
    }

TEST(PerformanceBentley, Formatting)
    {
    timer.Start();
    for (int i=0; i<s_niters; ++i)
        {
        Utf8PrintfString str ("Iter [%d]", i);
        ASSERT_TRUE (!str.empty());
        }

    for (int i=0; i<s_niters; ++i)
        {
        Utf8PrintfString str ("Iter [%d] in a longer string that exceeds the min cap which is 23 bytes", i);
        ASSERT_TRUE (!str.empty());
        }

    for (int i=0; i<s_niters; ++i)
        {
        Utf8PrintfString str ("%ls", L"abc");
        ASSERT_TRUE (str.Equals ("abc"));
        }

    WString ulower = L"LOWER";
    for (int i=0; i<s_niters; ++i)
        {
        WString lower (ulower);
        lower.ToLower ();
        ASSERT_TRUE (lower == L"lower");
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "For all Formatting", s_niters);
    }

TEST(PerformanceBentley, LargeNumberOfWStrings)
    {
    bvector<WString> strings;
    timer.Start();
    for (int i=0; i<s_niters*10; ++i)
        {
        strings.push_back (WString (L"a pretty long WString"));
        }
    timer.Stop();
    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), "", s_niters);

    strings.clear();     // destruct them all
    }
