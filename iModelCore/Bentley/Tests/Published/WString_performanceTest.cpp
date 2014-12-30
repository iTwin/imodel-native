/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/WString_performanceTest.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/WString.h>

    static int s_niters = 100000;

TEST(PerformanceBentley, WStringCopying)
    {
    WString base = L"abcdef123456789";
    for (int i=0; i<s_niters*10; ++i)
        {
        WString str = base;
        ASSERT_TRUE (!str.empty ());
        }
    }

TEST(PerformanceBentley, WStringCtors)
    {
    // Empty string construction
    for (int i=0; i<s_niters*10; ++i)
        {
        WString str;
        ASSERT_TRUE (str.empty ());
        }

    // simple construction
    for (int i=0; i<s_niters*10; ++i)
        {
        WString str (1, L'0'+(i%10));
        ASSERT_TRUE (!str.empty ());
        }
    }

TEST(PerformanceBentley, WStringAppendStayShort)
    {
    // append+stay short
    for (int i=0; i<s_niters/10; ++i)
        {
        WString str;

        for (int j=0; j<10; ++j)
            {
            str.append (1, static_cast <wchar_t> (L'0'+(j%10)));
            ASSERT_EQ (str.length(), 1+j);
            }
        }
    }

TEST(PerformanceBentley, WStringAppendGrowLong)
    {
    // append+grow to long size
    for (int i=0; i<s_niters/10; ++i)
        {
        WString str;

        for (int j=0; j<s_niters/100; ++j)
            {
            str.append (1, static_cast <wchar_t> (L'0'+(j%10)));
            ASSERT_EQ (str.length(), 1+j);
            }
        }
    }

TEST(PerformanceBentley, WStringCompare)
    {
    // compare
    WString str1 (L"xyz");
    for (int i=0; i<s_niters*10; ++i)
        {
        WString str (1, L'0'+(i%10));
        ASSERT_TRUE (str != str1);
        ASSERT_TRUE (str1 == str1);
        }
    }

TEST(PerformanceBentley, StringConversions)
    {
    char const* asc = "Now is the time for all good men to come to the aid of the party.";
    for (int i=0; i<s_niters; ++i)
        {
        WString str (asc, BentleyCharEncoding::Utf8);
        ASSERT_EQ (str[0], asc[0]);
        ASSERT_EQ (str[1], asc[1]);
        }
    }

TEST(PerformanceBentley, Formatting)
    {
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
    }

TEST(PerformanceBentley, LargeNumberOfWStrings)
    {
    bvector<WString> strings;
    for (int i=0; i<s_niters*10; ++i)
        {
        strings.push_back (WString (L"a pretty long WString"));
        }

    strings.clear();     // destruct them all
    }
