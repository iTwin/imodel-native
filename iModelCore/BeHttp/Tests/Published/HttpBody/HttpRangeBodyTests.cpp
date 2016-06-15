/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/HttpBody/HttpRangeBodyTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "HttpRangeBodyTests.h"

USING_NAMESPACE_BENTLEY_UNIT_TESTS

TEST_F (HttpRangeBodyTests, AsString_FullRange_ReturnsSameString)
    {
    HttpRangeBodyPtr body = HttpRangeBody::Create (HttpStringBody::Create ("abcde"), 0, 4);

    EXPECT_STREQ ("abcde", body->AsString ().c_str ());
    }

TEST_F (HttpRangeBodyTests, AsString_RangeClippedFromStart_ReturnsClipped)
    {
    HttpRangeBodyPtr body = HttpRangeBody::Create (HttpStringBody::Create ("abcde"), 2, 4);

    EXPECT_STREQ ("cde", body->AsString ().c_str ());
    }

TEST_F (HttpRangeBodyTests, AsString_RangeClippedFromEnd_ReturnsClipped)
    {
    HttpRangeBodyPtr body = HttpRangeBody::Create (HttpStringBody::Create ("abcde"), 0, 2);

    EXPECT_STREQ ("abc", body->AsString ().c_str ());
    }

TEST_F (HttpRangeBodyTests, AsString_RangeClippedFromBothEnds_ReturnsClipped)
    {
    HttpRangeBodyPtr body = HttpRangeBody::Create (HttpStringBody::Create ("abcde"), 1, 3);

    EXPECT_STREQ ("bcd", body->AsString ().c_str ());
    }

TEST_F (HttpRangeBodyTests, AsString_CalledAfterRead_StringifiesAllContent)
    {
    HttpRangeBodyPtr body = HttpRangeBody::Create (HttpStringBody::Create ("abcde"), 0, 4);

    char buffer[2];
    body->Read (buffer, 2);

    EXPECT_STREQ ("abcde", body->AsString ().c_str ());
    }

TEST_F (HttpRangeBodyTests, Read_AsStringCalledAfterRead_ReadingContinuesAsNormal)
    {
    HttpRangeBodyPtr body = HttpRangeBody::Create (HttpStringBody::Create ("abc"), 0, 4);

    char buffer[3] = {0, 0, 0};
    EXPECT_EQ (2, body->Read (buffer, 2));
    EXPECT_STREQ ("ab", buffer);

    body->AsString ();

    memset (buffer, 0, 3);
    EXPECT_EQ (1, body->Read (buffer, 2));
    EXPECT_STREQ ("c", buffer);
    }

TEST_F (HttpRangeBodyTests, SetPosition_ValuePassed_PositionSet)
    {
    auto body = HttpRangeBody::Create (HttpStringBody::Create ("foo"), 0, 2);
    body->SetPosition (2);

    uint64_t pos = 0;
    body->GetPosition (pos);
    EXPECT_EQ (2, pos);
    }

TEST_F (HttpRangeBodyTests, Reset_FilledBody_ClearsContentsInRange)
    {
    auto strBody = HttpStringBody::Create ("ABCD");
    auto body = HttpRangeBody::Create (strBody, 1, 2);
    body->Reset ();
    EXPECT_STREQ ("", body->AsString ().c_str ());

    auto str = strBody->AsString ();

    EXPECT_EQ ('A', str[0]);
    EXPECT_EQ ('\0', str[1]);
    EXPECT_EQ ('\0', str[2]);
    EXPECT_EQ ('D', str[3]);
    }

TEST_F (HttpRangeBodyTests, Reset_PositionSet_ClearsPosition)
    {
    auto body = HttpRangeBody::Create (HttpStringBody::Create ("foo"), 0, 2);
    body->SetPosition (2);

    body->Reset ();

    uint64_t pos = 0;
    body->GetPosition (pos);
    EXPECT_EQ (0, pos);
    }
