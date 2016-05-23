/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/UnitTests/Published/WebServices/Client/Response/WSObjectsResponseTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSObjectsResponseTests.h"

#include <WebServices/Client/Response/WSObjectsResponse.h>
#include <WebServices/Client/Response/WSObjectsReaderV1.h>

TEST_F(WSObjectsResponseTests, Ctor_NoParams_IsModifiedTrueToDetectInvalidInstances)
    {
    EXPECT_TRUE(WSObjectsResponse().IsModified());
    }

TEST_F(WSObjectsResponseTests, GetInstances_DefaultCtor_ReturnsEmpty)
    {
    BeTest::SetFailOnAssert(false);
    EXPECT_TRUE(WSObjectsResponse().GetInstances().IsEmpty());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsResponseTests, GetJsonValue_DefaultCtor_ReturnsNull)
    {
    BeTest::SetFailOnAssert(false);
    EXPECT_TRUE(WSObjectsResponse().GetJsonValue().isNull());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsResponseTests, GetRapidJsonDocument_DefaultCtor_ReturnsNull)
    {
    BeTest::SetFailOnAssert(false);
    EXPECT_TRUE(WSObjectsResponse().GetRapidJsonDocument().IsNull());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSObjectsResponseTests, GetInstances_CtorWithReaderAndBody_ReturnsInstances)
    {
    auto reader = WSObjectsReaderV1::Create("Schema", "Class");
    WSObjectsResponse result(reader, HttpStringBody::Create(R"({"$id":"A"})"), HttpStatus::OK, "", "");

    EXPECT_FALSE(result.GetInstances().IsEmpty());
    EXPECT_EQ(ObjectId("Schema", "Class", "A"), (*result.GetInstances().begin()).GetObjectId());
    }

TEST_F(WSObjectsResponseTests, IsFinal_SkipTokenEmpty_True)
    {
    WSObjectsResponse response(nullptr, HttpStringBody::Create(), HttpStatus::OK, "", "");

    EXPECT_TRUE(response.IsFinal());
    EXPECT_EQ("", response.GetSkipToken());
    }

TEST_F(WSObjectsResponseTests, IsFinal_SkipTokenNotEmpty_False)
    {
    WSObjectsResponse response(nullptr, HttpStringBody::Create(), HttpStatus::OK, "", "SomeToken");

    EXPECT_FALSE(response.IsFinal());
    EXPECT_EQ("SomeToken", response.GetSkipToken());
    }

