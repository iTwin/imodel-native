/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>

class Utf8PrintfStringTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Utf8PrintfStringTests, Ctor_EmptyStringPassed_Empty)
    {
    EXPECT_EQ("", Utf8PrintfString(""));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Utf8PrintfStringTests, Ctor_FormatAndArgumentPassed_FormattedString)
    {
    EXPECT_EQ("Foo", Utf8PrintfString("%s", "Foo"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Utf8PrintfStringTests, Ctor_FormatAndMultipleArgumentsPassed_FormattedString)
    {
    EXPECT_EQ("Foo Boo", Utf8PrintfString("%s %s", "Foo", "Boo"));
    EXPECT_EQ("42 X", Utf8PrintfString("%d %c", 42, 'X'));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void Utf8PrintfStringArgs(Utf8CP expectedStr, Utf8CP fmt, ...)
    {
    va_list args;
    va_start(args, fmt);
    Utf8PrintfString str =  Utf8PrintfString::CreateFromVaList(fmt, args);
    va_end(args);
    EXPECT_TRUE(str.Equals(expectedStr)) << "Expected: " << expectedStr << "\nActual: " << str.c_str();// << str.size();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(Utf8PrintfStringTests, Utf8PrintfStringTest)
    {
    Utf8PrintfStringArgs("No. 5", "No. %d", 5);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Utf8PrintfStringTests, Ctor_PositionalFormatWithStrings_FormattedString)
    {
    EXPECT_STREQ("A B C D", Utf8PrintfString("A %1$s C %2$s", "B", "D").c_str());
    EXPECT_STREQ("A B C D", Utf8PrintfString("A %2$s C %1$s", "D", "B").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Utf8PrintfStringTests, Ctor_PositionalFormatWithMixedTypes_FormattedString)
    {
    EXPECT_STREQ("A 111 B Foo", Utf8PrintfString("A %1$d B %2$s", 111, "Foo").c_str());
    EXPECT_STREQ("A Foo B 111", Utf8PrintfString("A %2$s B %1$d", 111, "Foo").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined(BENTLEYCONFIG_OS_ANDROID) // TFS#894638
TEST_F(Utf8PrintfStringTests, Sprintf_BadFormat_DoesNotCrash)
    {
    Utf8CP a = "a";
    Utf8CP b = "b";
    Utf8String str;

    BeTest::SetFailOnInvalidParameterAssert(false);
    str.Sprintf("%s foo % type", a, b);
    BeTest::SetFailOnInvalidParameterAssert(true);

    // Invalid format string/arg combinations result in "undefined" behavior across C libraries.
    // Therefore, I think we need unique result tests for each of these configurations.
#if defined(BENTLEYCONFIG_OS_APPLE)
    EXPECT_STREQ("a foo ype", str.c_str());
#elif defined(BENTLEYCONFIG_OS_UNIX)
    EXPECT_STREQ("a foo % ype", str.c_str());
#else
    EXPECT_STREQ("", str.c_str());
#endif
    }
#endif
