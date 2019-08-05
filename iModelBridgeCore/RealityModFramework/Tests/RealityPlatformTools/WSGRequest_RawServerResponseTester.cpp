/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <RealityPlatformTools/WSGServices.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <ostream>
#include <curl/curl.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(RawServerResponseTestBase, DefaultConstructor)
    {
    auto serverResponseToTest = RawServerResponse();
    EXPECT_TRUE(serverResponseToTest.responseCode == -1);
    EXPECT_TRUE(serverResponseToTest.toolCode == ServerType::WSG);
    EXPECT_TRUE(serverResponseToTest.status == RequestStatus::UNSENT);
    EXPECT_STREQ(serverResponseToTest.header.c_str(), "");
    EXPECT_STREQ(serverResponseToTest.body.c_str(), "");
    }

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(RawServerResponseTestBase, OperatorOverload)
    {
    auto serverResponseToTest = RawServerResponse();
    serverResponseToTest.status = ::OK;

    EXPECT_TRUE(serverResponseToTest == ::OK);
    EXPECT_TRUE(serverResponseToTest != ::BADREQ);

    EXPECT_TRUE(::OK == serverResponseToTest);
    EXPECT_TRUE(::BADREQ != serverResponseToTest);
    }

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST(RawServerResponseTestBase, ClearMethod)
    {
    auto serverResponseToTest = RawServerResponse();
    serverResponseToTest.header = "header";
    serverResponseToTest.body = "body";
    serverResponseToTest.responseCode = 5454545;
    serverResponseToTest.toolCode = ServerType::Azure;
    serverResponseToTest.status = RequestStatus::LASTPAGE;

    serverResponseToTest.clear();

    EXPECT_TRUE(serverResponseToTest.responseCode == -1);
    EXPECT_TRUE(serverResponseToTest.toolCode == ServerType::WSG);
    EXPECT_TRUE(serverResponseToTest.status == RequestStatus::UNSENT);
    EXPECT_STREQ(serverResponseToTest.header.c_str(), "");
    EXPECT_STREQ(serverResponseToTest.body.c_str(), "");
    }

//=====================================================================================
//! @bsiclass                         Remi.Charbonneau                        05/2017
//=====================================================================================
struct response_state
    {
    int toolCode;
    long responseCode;
    RequestStatus result;
    Utf8String body;

    friend std::ostream& operator<<(std::ostream& os, const response_state& obj)
        {
        return os
            << "toolCode: " << obj.toolCode
            << " responseCode: " << obj.responseCode
            << " result: " << obj.result
            << " body: " << obj.body;
        }
    };

//=====================================================================================
//! @bsiclass                         Remi.Charbonneau                        05/2017
//=====================================================================================
struct RawServerResponseTest : testing::Test
    {
    RawServerResponse* rawServerResponse;
    RawServerResponseTest()
        {
        rawServerResponse = new RawServerResponse;
        }

    ~RawServerResponseTest()
        {
        delete rawServerResponse;
        }

    };

//=====================================================================================
//! @bsiclass                         Remi.Charbonneau                        05/2017
//=====================================================================================
struct RawServerValidationResponseTest : RawServerResponseTest, testing::WithParamInterface<response_state>
    {
    RawServerValidationResponseTest()
        {
        rawServerResponse->toolCode = GetParam().toolCode;
        rawServerResponse->responseCode = GetParam().responseCode;
        }
    };

//=====================================================================================
//! @bsiclass                         Remi.Charbonneau                        05/2017
//=====================================================================================
struct RawServerJSONValidationResponseTest : RawServerResponseTest, testing::WithParamInterface<response_state>
    {
    RawServerJSONValidationResponseTest()
        {
        rawServerResponse->toolCode = GetParam().toolCode;
        rawServerResponse->responseCode = GetParam().responseCode;
        rawServerResponse->body = GetParam().body;
        }
    };

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST_P(RawServerValidationResponseTest, ResponseRequestValidation)
    {
    EXPECT_TRUE(rawServerResponse->ValidateResponse() == GetParam().result);
    }

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
INSTANTIATE_TEST_CASE_P(Default, RawServerValidationResponseTest, testing::Values(
    response_state {CURLE_OK, 200, RequestStatus::OK, ""},
    response_state {CURLE_LOGIN_DENIED, 403, RequestStatus::BADREQ, ""},
    response_state {CURLE_LOGIN_DENIED, 200, RequestStatus::BADREQ, ""},
    response_state {CURLE_OK, 403, RequestStatus::BADREQ, ""}
));

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
TEST_P(RawServerJSONValidationResponseTest, JSONResponseRequestValidation)
    {
    auto instance = Json::Value();
    auto result = rawServerResponse->ValidateJSONResponse(instance, "memberPresent");
    EXPECT_TRUE(result == GetParam().result);
    }

//=====================================================================================
//! @bsimethod                          Remi.Charbonneau                        05/2017
//=====================================================================================
INSTANTIATE_TEST_CASE_P(Default, RawServerJSONValidationResponseTest, testing::Values(
    response_state {CURLE_OK, 200, RequestStatus::BADREQ, ""},
    response_state {CURLE_LOGIN_DENIED, 403, RequestStatus::BADREQ, ""},
    response_state {CURLE_OK, 403, RequestStatus::BADREQ, ""},
    response_state {CURLE_LOGIN_DENIED, 200, RequestStatus::BADREQ, ""},
    response_state {CURLE_OK, 200, RequestStatus::OK, "{ \"memberPresent\": \"Yes\" }"},
    response_state {CURLE_OK, 200, RequestStatus::BADREQ, "{ \"errorMessage\": \"ERROR\" }"}
));