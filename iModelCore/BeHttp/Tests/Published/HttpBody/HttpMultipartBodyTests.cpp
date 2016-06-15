/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/HttpBody/HttpMultipartBodyTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "HttpMultipartBodyTests.h"

USING_NAMESPACE_BENTLEY_UNIT_TESTS

TEST_F (HttpMultipartBodyTests, SetPosition_ValuePassed_PositionSet)
    {
    auto body = HttpMultipartBody::Create ();
    body->AddPart (HttpStringBody::Create ("foo"), nullptr);
    ASSERT_EQ (SUCCESS, body->SetPosition (2));

    uint64_t pos = 0;
    body->GetPosition (pos);
    EXPECT_EQ (2, pos);
    }

TEST_F (HttpMultipartBodyTests, Reset_PositionSet_ClearsPosition)
    {
    auto body = HttpMultipartBody::Create ();
    body->AddPart (HttpStringBody::Create ("foo"), nullptr);
    ASSERT_EQ (SUCCESS, body->SetPosition (2));

    body->Reset ();

    uint64_t pos = 0;
    body->GetPosition (pos);
    EXPECT_EQ (0, pos);
    }
