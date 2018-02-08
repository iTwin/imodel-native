/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/WPrintfStringTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>

class WPrintfStringTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WPrintfStringTests, Ctor_EmptyStringPassed_Empty)
    {
    EXPECT_STREQ(L"", WPrintfString(L"").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WPrintfStringTests, Ctor_FormatAndArgumentPassed_FormattedString)
    {
    EXPECT_STREQ(L"Foo", WPrintfString(L"%ls", L"Foo").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WPrintfStringTests, Ctor_FormatAndMultipleArgumentsPassed_FormattedString)
    {
    EXPECT_STREQ(L"Foo Boo", WPrintfString(L"%ls %ls", L"Foo", L"Boo").c_str());
    EXPECT_STREQ(L"42 X", WPrintfString(L"%d %c", 42, 'X').c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WPrintfStringTests, Ctor_PositionalFormatWithStrings_FormattedString)
    {
    EXPECT_STREQ(L"A B C D", WPrintfString(L"A %1$ls C %2$ls", L"B", L"D").c_str());
    EXPECT_STREQ(L"A B C D", WPrintfString(L"A %2$ls C %1$ls", L"D", L"B").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WPrintfStringTests, Ctor_PositionalFormatWithMixedTypes_FormattedString)
    {
    EXPECT_STREQ(L"A 111 B Foo", WPrintfString(L"A %1$d B %2$ls", 111, L"Foo").c_str());
    EXPECT_STREQ(L"A Foo B 111", WPrintfString(L"A %2$ls B %1$d", 111, L"Foo").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined(BENTLEYCONFIG_OS_LINUX)
TEST_F(WPrintfStringTests, Sprintf_BadFormat_DoesNotCrash)
    {
    WCharCP a = L"a";
    WCharCP b = L"b";
    WString str;

    BeTest::SetFailOnInvalidParameterAssert(false);
    str.Sprintf(L"%ls foo % type", a, b);
    BeTest::SetFailOnInvalidParameterAssert(true);

    EXPECT_STREQ(L"", str.c_str());
    }
#endif
