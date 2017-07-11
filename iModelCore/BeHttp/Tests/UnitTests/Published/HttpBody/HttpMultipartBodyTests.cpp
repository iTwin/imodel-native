/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/HttpBody/HttpMultipartBodyTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "HttpMultipartBodyTests.h"

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                       06/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HttpMultipartBodyTests, SetPosition_ValuePassed_PositionSet)
    {
    auto body = HttpMultipartBody::Create ();
    body->AddPart (HttpStringBody::Create ("foo"), nullptr);
    ASSERT_EQ (SUCCESS, body->SetPosition (2));

    uint64_t pos = 0;
    body->GetPosition (pos);
    EXPECT_EQ (2, pos);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Vincas.Razma                       06/16
+---------------+---------------+---------------+---------------+---------------+------*/
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
