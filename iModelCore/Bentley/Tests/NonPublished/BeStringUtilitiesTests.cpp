/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/BeStringUtilitiesTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                  11/16
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTest, ComputeNumLogicalChars)
    {
    //can be tested with other multibyte characters, depending upon the current codepage
    WCharP input = L"awesas+s a=d";
    size_t receivedLogicalChars = BeStringUtilities::ComputeNumLogicalChars(input, 12);
    size_t expectedLogicalChars = 12;
    EXPECT_EQ(expectedLogicalChars, receivedLogicalChars);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                  11/16
//---------------------------------------------------------------------------------------
TEST(BeStringUtilitiesTest, ComputeByteOffsetOfLogicalChar)
    {
    //can be tested with other multibyte characters, depending upon the current codepage
    Utf8P input = "assas+sas=d";
    size_t receivedByteOffset = BeStringUtilities::ComputeByteOffsetOfLogicalChar(input, 8);
    size_t expectedByteOffset = 8;
    EXPECT_EQ(expectedByteOffset, receivedByteOffset);
    }