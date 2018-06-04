/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Client/RequestOptionsTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Client/RequestOptions.h>

#include "../../Utils/WebServicesTestsHelper.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

class RequestOptionsTests : public WSClientBaseTest
    {};

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RequestOptionsTests, ToJson_DefaultCtor_CorrectJson)
    {
    RequestOptions options;

    options.SetFailureStrategy(RequestOptions::FailureStrategy::Stop);

    Json::Value json;
    options.ToJson(json);

    Json::Value expected;
    expected["FailureStrategy"] = "Stop";
    expected["ResponseContent"] = "FullInstance";
    expected["RefreshInstances"] = false;
    EXPECT_EQ(expected, json);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RequestOptionsTests, ToJson_FailureStrategyValues_CorrectJson)
    {
    Json::Value json;
    RequestOptions options;

    options.SetFailureStrategy(RequestOptions::FailureStrategy::Stop);
    options.ToJson(json);
    EXPECT_EQ("Stop", json["FailureStrategy"].asString());

    options.SetFailureStrategy(RequestOptions::FailureStrategy::Continue);
    options.ToJson(json);
    EXPECT_EQ("Continue", json["FailureStrategy"].asString());

    options.SetFailureStrategy(RequestOptions::FailureStrategy::Default);
    options.ToJson(json);
    EXPECT_EQ("Stop", json["FailureStrategy"].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RequestOptionsTests, ToJson_ResponseContentValues_CorrectJson)
    {
    Json::Value json;
    RequestOptions options;

    options.SetResponseContent(RequestOptions::ResponseContent::Empty);
    options.ToJson(json);
    EXPECT_EQ("Empty", json["ResponseContent"].asString());

    options.SetResponseContent(RequestOptions::ResponseContent::FullInstance);
    options.ToJson(json);
    EXPECT_EQ("FullInstance", json["ResponseContent"].asString());

    options.SetResponseContent(RequestOptions::ResponseContent::InstanceId);
    options.ToJson(json);
    EXPECT_EQ("InstanceId", json["ResponseContent"].asString());

    options.SetResponseContent(RequestOptions::ResponseContent::Default);
    options.ToJson(json);
    EXPECT_EQ("FullInstance", json["ResponseContent"].asString());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RequestOptionsTests, ToJson_ShouldRefreshInstancesValues_CorrectJson)
    {
    Json::Value json;
    RequestOptions options;

    options.SetShouldRefreshInstances(true);
    options.ToJson(json);
    EXPECT_EQ(true, json["RefreshInstances"].asBool());

    options.SetShouldRefreshInstances(false);
    options.ToJson(json);
    EXPECT_EQ(false, json["RefreshInstances"].asBool());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Julius.Senkus       04/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RequestOptionsTests, ToJson_CustomRequestOptionValues_CorrectJson)
    {
    Json::Value json;
    RequestOptions options;

    options.SetCustomRequestOption("Option", true);
    options.ToJson(json);
    ASSERT_FALSE(json["CustomOptions"].empty());
    EXPECT_TRUE(json["CustomOptions"]["Option"].asBool());

    options.SetCustomRequestOption("Option", false);
    options.ToJson(json);
    ASSERT_FALSE(json["CustomOptions"].empty());
    EXPECT_FALSE(json["CustomOptions"]["Option"].asBool());

    options.SetCustomRequestOption("Option", "{json:{}}");
    options.ToJson(json);
    ASSERT_FALSE(json["CustomOptions"].empty());
    EXPECT_EQ("{json:{}}", json["CustomOptions"]["Option"].asString());
    }