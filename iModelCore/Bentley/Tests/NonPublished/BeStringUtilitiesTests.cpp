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
//TEST(BeStringUtilitiesTest, ComputeNumLogicalChars)
//    {
//    WCharP input = L"a€sas+sas=d";
//    size_t received_size = BeStringUtilities::ComputeNumLogicalChars(input, 12);
//    size_t expected_size = 2;
//    EXPECT_EQ(expected_size, received_size);
//    }
//
//TEST(BeStringUtilitiesTest, ComputeByteOffsetOfLogicalChar)
//    {
//    Utf8P input = "ab€sas+sas=d";
//    Utf8P dummy = "abs as+sas=d";
//    size_t received_size = BeStringUtilities::ComputeByteOffsetOfLogicalChar(input, 4);
//    size_t expected_size = 2;
//    EXPECT_EQ(expected_size, received_size);
//    EXPECT_STREQ(input, dummy);
//    }