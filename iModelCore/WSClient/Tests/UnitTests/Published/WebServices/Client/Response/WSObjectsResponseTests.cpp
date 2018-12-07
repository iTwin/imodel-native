/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/UnitTests/Published/WebServices/Client/Response/WSObjectsResponseTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSObjectsResponseTests.h"

#include <WebServices/Client/Response/WSObjectsResponse.h>
#include <WebServices/Client/Response/WSObjectsReaderV2.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSObjectsResponseTests, Ctor_NoParams_IsModifiedTrueToDetectInvalidInstances)
    {
    EXPECT_TRUE(WSObjectsResponse().IsModified());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSObjectsResponseTests, GetInstances_DefaultCtor_ReturnsEmpty)
    {
    BeTest::SetFailOnAssert(false);
    EXPECT_TRUE(WSObjectsResponse().GetInstances().IsEmpty());
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSObjectsResponseTests, GetJsonValue_DefaultCtor_ReturnsNull)
    {
    BeTest::SetFailOnAssert(false);
    EXPECT_TRUE(WSObjectsResponse().GetJsonValue().isNull());
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSObjectsResponseTests, GetRapidJsonDocument_DefaultCtor_ReturnsNull)
    {
    BeTest::SetFailOnAssert(false);
    EXPECT_TRUE(WSObjectsResponse().GetRapidJsonDocument().IsNull());
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSObjectsResponseTests, IsFinal_SkipTokenEmpty_True)
    {
    WSObjectsResponse response(nullptr, HttpStringBody::Create(), HttpStatus::OK, "", "");

    EXPECT_TRUE(response.IsFinal());
    EXPECT_EQ("", response.GetSkipToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSObjectsResponseTests, IsFinal_SkipTokenNotEmpty_False)
    {
    WSObjectsResponse response(nullptr, HttpStringBody::Create(), HttpStatus::OK, "", "SomeToken");

    EXPECT_FALSE(response.IsFinal());
    EXPECT_EQ("SomeToken", response.GetSkipToken());
    }

