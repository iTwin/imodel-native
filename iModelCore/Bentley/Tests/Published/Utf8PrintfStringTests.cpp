/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/Utf8PrintfStringTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>

class Utf8PrintfStringTests : public ::testing::Test {};

TEST_F(Utf8PrintfStringTests, Ctor_EmptyStringPassed_Empty)
    {
    EXPECT_EQ("", Utf8PrintfString(""));
    }

TEST_F(Utf8PrintfStringTests, Ctor_FormatAndArgumentPassed_FormattedString)
    {
    EXPECT_EQ("Foo", Utf8PrintfString("%s", "Foo"));
    }

TEST_F(Utf8PrintfStringTests, Ctor_FormatAndMultipleArgumentsPassed_FormattedString)
    {
    EXPECT_EQ("Foo Boo", Utf8PrintfString("%s %s", "Foo", "Boo"));
    EXPECT_EQ("42 X", Utf8PrintfString("%d %c", 42, 'X'));
    }
