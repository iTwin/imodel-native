/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "WSUploadResponseTests.h"

#include <WebServices/Client/Response/WSObjectsResponse.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSUploadResponseTests, Ctor_NoParams_DefaultValues)
    {
    EXPECT_EQ(nullptr, WSUploadResponse().GetBody().get());
    EXPECT_EQ("", WSUploadResponse().GetFileETag());

    Json::Value jsonValue;
    EXPECT_EQ(ERROR, WSUploadResponse().GetJson(jsonValue));
    EXPECT_TRUE(jsonValue.isNull());

    rapidjson::Document jsonDocument;
    EXPECT_EQ(ERROR, WSUploadResponse().GetJson(jsonDocument));
    EXPECT_TRUE(jsonDocument.IsNull());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSUploadResponseTests, Ctor_JsonBodyPassed_BodyCanBeRead)
    {
    auto jsonStr = R"({ "Foo" : "Boo" })";
    WSUploadResponse response(HttpStringBody::Create(jsonStr));
    EXPECT_EQ("", response.GetFileETag());
    ASSERT_NE(nullptr, response.GetBody().get());
    EXPECT_EQ(jsonStr, response.GetBody()->AsString());

    Json::Value jsonValue;
    EXPECT_EQ(SUCCESS, response.GetJson(jsonValue));
    EXPECT_STREQ("Boo", jsonValue["Foo"].asString().c_str());

    rapidjson::Document jsonDocument;
    EXPECT_EQ(SUCCESS, response.GetJson(jsonDocument));
    EXPECT_STREQ("Boo", jsonDocument["Foo"].GetString());
    }
