/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Licensing/UsageTrackingTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UsageTrackingTests.h"
#include <WebServices/Licensing/UsageTracking.h>
#include <WebServices/Licensing/MobileTracking.h>
#include <Bentley/Base64Utilities.h>

#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

void UsageTrackingTests::SetUp ()
    {
    UsageTracking::Initialize (GetHandlerPtr ());
    m_client = std::make_shared<StubBuddiClient>();
    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &m_localState, m_client);
    }

void UsageTrackingTests::TearDown ()
    {
    UsageTracking::Uninintialize ();
    }

TEST_F (UsageTrackingTests, PostSingleUsage)
    {
    GetHandler ().ForFirstRequest ([&] (HttpRequestCR request)
        {
        auto bodyJson = request.GetRequestBody ()->AsJson ();
        EXPECT_EQ (1, (int)bodyJson.size ());
        EXPECT_EQ ("2321DDDD-B37B-49AC-B4A8-591C4EBC9062", bodyJson[0]["DeviceID"].asString ());

        return StubHttpResponse (HttpStatus::OK);
        });

    DateTime dt (DateTime::GetCurrentTimeUtc ());
    auto status = UsageTracking::RegisterUserUsages ("2321DDDD-B37B-49AC-B4A8-591C4EBC9062",
           "7409333D-EF73-4083-B377-22CFDF4ED4B7", "1654", "", dt, "5.3.1.11");

    EXPECT_EQ (SUCCESS, status);
    }


TEST_F (UsageTrackingTests, PostMultipleUsage)
    {
    GetHandler ().ForFirstRequest ([&] (HttpRequestCR request)
        {
        auto bodyJson = request.GetRequestBody ()->AsJson ();
        EXPECT_LT (1, (int)bodyJson.size ());
        EXPECT_EQ ("2321DDDD-B37B-49AC-B4A8-591C4EBC9062", bodyJson[1]["DeviceID"].asString ());

        return StubHttpResponse (HttpStatus::OK);
        });

    DateTime dt (DateTime::GetCurrentTimeUtc ());
    MobileTracking mt1 ("2321DDDD-B37B-49AC-B4A8-591C4EBC9062", "7409333D-EF73-4083-B377-22CFDF4ED4B7", "1654", "", dt, "5.3.1.11");
    MobileTracking mt2 ("2321DDDD-B37B-49AC-B4A8-591C4EBC9062", "7409333D-EF73-4083-B377-22CFDF4ED4B7", "1654", "", dt, "5.3.1.11");
    bvector<MobileTracking> usages;
    usages.push_back (mt1);
    usages.push_back (mt2);

    auto status = UsageTracking::RegisterUserUsages (usages);
    EXPECT_EQ (SUCCESS, status);
    }

TEST_F (UsageTrackingTests, PostEmptyUsage)
    {
    MobileTracking mt;
    auto status = UsageTracking::RegisterUserUsages (mt);
    EXPECT_EQ (UsageTracking::USAGE_NO_USAGES, status);
    }