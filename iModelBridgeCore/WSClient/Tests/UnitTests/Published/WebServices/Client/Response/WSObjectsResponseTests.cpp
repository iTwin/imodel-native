/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../../../Utils/WebServicesTestsHelper.h"
#include <WebServices/Client/Response/WSObjectsResponse.h>
#include <WebServices/Client/Response/WSObjectsReaderV2.h>

struct WSObjectsResponseTests : WSClientBaseTest {};

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

