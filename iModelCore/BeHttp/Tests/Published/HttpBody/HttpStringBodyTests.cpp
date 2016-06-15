/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/HttpBody/HttpStringBodyTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "HttpStringBodyTests.h"

USING_NAMESPACE_BENTLEY_UNIT_TESTS

TEST_F (HttpStringBodyTests, AsString_Empty_ReturnsEmptyString)
    {
    auto body = HttpStringBody::Create ();
    EXPECT_EQ ("", body->AsString ());
    }

TEST_F (HttpStringBodyTests, AsString_ConstructedWithString_ReturnsEqualString)
    {
    auto body = HttpStringBody::Create ("TestContent");
    EXPECT_EQ ("TestContent", body->AsString ());
    }

TEST_F(HttpStringBodyTests, AsString_CreatedWithNullPtr_ReturnsEmptyString)
    {
    auto body = HttpStringBody::Create(std::shared_ptr<Utf8String>());
    EXPECT_EQ("", body->AsString());
    }

TEST_F(HttpStringBodyTests, AsString_CreatedWithSharedString_ReturnsString)
    {
    auto body = HttpStringBody::Create(std::make_shared<Utf8String>("TestContent"));
    EXPECT_EQ("TestContent", body->AsString());
    }

TEST_F (HttpStringBodyTests, SetPosition_ValuePassed_PositionSet)
    {
    auto body = HttpStringBody::Create ("foo");
    body->SetPosition (2);

    uint64_t pos = 0;
    body->GetPosition (pos);
    EXPECT_EQ (2, pos);
    }

TEST_F (HttpStringBodyTests, Reset_FilledBody_ClearsContents)
    {
    auto body = HttpStringBody::Create ("foo");
    body->Reset ();
    EXPECT_EQ ("", body->AsString ());
    }

TEST_F (HttpStringBodyTests, Reset_PositionSet_ClearsPosition)
    {
    auto body = HttpStringBody::Create ();
    body->SetPosition (10);

    body->Reset ();

    uint64_t pos = 0;
    body->GetPosition (pos);
    EXPECT_EQ (0, pos);
    }
