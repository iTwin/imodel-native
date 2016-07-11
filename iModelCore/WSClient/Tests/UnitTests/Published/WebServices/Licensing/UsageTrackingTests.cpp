/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Licensing/UsageTrackingTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UsageTrackingTests.h"
#include <WebServices/Licensing/UsageTracking.h>
#include <WebServices/Licensing/UsageTrackingData.h>
#include <Bentley/Base64Utilities.h>

#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

void UsageTrackingTests::SetUp()
    {
    UsageTracking::Initialize(GetHandlerPtr());
    m_client = std::make_shared<StubBuddiClient>();
    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &m_localState, m_client);
    }

void UsageTrackingTests::TearDown()
    {
    UsageTracking::Uninitialize ();
    }

TEST_F(UsageTrackingTests, PostSingleUsage)
    {
    GetHandler().ForFirstRequest([&] (Http::RequestCR request)
        {
        auto bodyJson = request.GetRequestBody()->AsJson();
        EXPECT_EQ(1, (int) bodyJson.size());
        EXPECT_EQ("2321DDDD-B37B-49AC-B4A8-591C4EBC9062", bodyJson[0]["DeviceID"].asString());

        return StubHttpResponse(HttpStatus::OK);
        });

    DateTime dt(DateTime::GetCurrentTimeUtc());
    auto status = UsageTracking::RegisterUserUsages("2321DDDD-B37B-49AC-B4A8-591C4EBC9062",
                                                    "7409333D-EF73-4083-B377-22CFDF4ED4B7",
                                                    "1654",
                                                    "",
                                                    dt,
                                                    "5.3.1.11")->GetResult();

    EXPECT_EQ(UsageTracking::Status::Success, status);
    }


TEST_F(UsageTrackingTests, PostMultipleUsage)
    {
    GetHandler().ForFirstRequest([&] (Http::RequestCR request)
        {
        auto bodyJson = request.GetRequestBody()->AsJson();
        EXPECT_LT(1, (int) bodyJson.size());
        EXPECT_EQ("2321DDDD-B37B-49AC-B4A8-591C4EBC9062", bodyJson[1]["DeviceID"].asString());

        return StubHttpResponse(HttpStatus::OK);
        });

    DateTime dt(DateTime::GetCurrentTimeUtc());
    UsageTrackingData utd1 ("2321DDDD-B37B-49AC-B4A8-591C4EBC9062", "7409333D-EF73-4083-B377-22CFDF4ED4B7", "2545", "", dt, "5.3.1.11");
    UsageTrackingData utd2 ("2321DDDD-B37B-49AC-B4A8-591C4EBC9062", "7409333D-EF73-4083-B377-22CFDF4ED4B7", "2545", "", dt, "5.3.1.11");
    bvector<UsageTrackingData> usages;
    usages.push_back (utd1);
    usages.push_back (utd2);

    auto status = UsageTracking::RegisterUserUsages(usages)->GetResult();
    EXPECT_EQ(UsageTracking::Status::Success, status);
    }

TEST_F(UsageTrackingTests, PostEmptyUsage)
    {
    UsageTrackingData utd;
    auto status = UsageTracking::RegisterUserUsages (utd)->GetResult();
    EXPECT_EQ(UsageTracking::Status::NoUsages, status);
    }