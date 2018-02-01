/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/Utf8PrintfStringTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>

class Utf8PrintfStringTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Utf8PrintfStringTests, Ctor_EmptyStringPassed_Empty)
    {
    EXPECT_EQ("", Utf8PrintfString(""));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Utf8PrintfStringTests, Ctor_FormatAndArgumentPassed_FormattedString)
    {
    EXPECT_EQ("Foo", Utf8PrintfString("%s", "Foo"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Utf8PrintfStringTests, Ctor_FormatAndMultipleArgumentsPassed_FormattedString)
    {
    EXPECT_EQ("Foo Boo", Utf8PrintfString("%s %s", "Foo", "Boo"));
    EXPECT_EQ("42 X", Utf8PrintfString("%d %c", 42, 'X'));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                        12/16
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
// @bsimethod                                     Farhad.Kabir                        12/16
//---------------------------------------------------------------------------------------
TEST_F(Utf8PrintfStringTests, Utf8PrintfStringTest)
    {
    Utf8PrintfStringArgs("No. 5", "No. %d", 5);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Utf8PrintfStringTests, Ctor_PositionalFormatWithStrings_FormattedString)
    {
    EXPECT_STREQ("A B C D", Utf8PrintfString("A %1$s C %2$s", "B", "D").c_str());
    EXPECT_STREQ("A B C D", Utf8PrintfString("A %2$s C %1$s", "D", "B").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Utf8PrintfStringTests, Ctor_PositionalFormatWithMixedTypes_FormattedString)
    {
    EXPECT_STREQ("A 111 B Foo", Utf8PrintfString("A %1$d B %2$s", 111, "Foo").c_str());
    EXPECT_STREQ("A Foo B 111", Utf8PrintfString("A %2$s B %1$d", 111, "Foo").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(Utf8PrintfStringTests, Sprintf_BadFormat_DoesNotCrash)
    {
    Utf8CP a = "a";
    Utf8CP b = "b";
    Utf8String str;

    BeTest::SetFailOnInvalidParameterAssert(false);
    str.Sprintf("%s foo % type", a, b);
    BeTest::SetFailOnInvalidParameterAssert(true);

    EXPECT_STREQ("", str.c_str());
    }
