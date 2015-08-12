/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Configuration/UrlProviderTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UrlProviderTests.h"

#include "../Connect/MockLocalState.h"
#include "../Connect/StubLocalState.h"
#include "MockBuddiClient.h"
#include <WebServices/Configuration/UrlProvider.h>
#include <Bentley/BeDebugLog.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS
USING_NAMESPACE_BENTLEY_MOBILEDGN

#ifdef USE_GTEST
TEST_F(UrlProviderTests, GetPunchlistWsgUrl_NoCachedURL_GetsURLFromBuddiWritesToLocalState)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    MockLocalState localState;

    EXPECT_CALL(localState, GetValue(_, _))
        .WillOnce(Return(Json::Value::null))
        .WillOnce(Return(""));
    EXPECT_CALL(localState, SaveValue(_, _, _)).Times(2);
    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);

    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_UrlIsCached_GetsUrlFromLocalState)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    StubLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);

    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    UrlProvider::CleanUpUrlCache();
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_UrlIsCached_GetsURL)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    MockLocalState localState;

    EXPECT_CALL(localState, GetValue(_, _))
        .WillOnce(Return(Json::Value::null))
        .WillOnce(Return(url));
    EXPECT_CALL(*client, GetUrl(_, _)).Times(0);

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);

    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_NoCachedAndNoBuddiUrl_ReturnsDefaultUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    StubLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(""))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);
    EXPECT_TRUE(!UrlProvider::GetPunchlistWsgUrl().empty());
    }

TEST_F(UrlProviderTests, GetUrl_ValidateAllGetters)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    StubLocalState localState;

    bset<Utf8String> urlNames;
    EXPECT_CALL(*client, GetUrl(_, _)).Times(6).WillRepeatedly(Invoke([&] (Utf8StringCR urlName, int regionId)
        {
        EXPECT_TRUE(urlNames.find(urlName) == urlNames.end());
        urlNames.insert(urlName);
        return CreateCompletedAsyncTask(BuddiUrlResult::Success(url));
        }));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);

    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectEulaUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetImsStsAuthUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetUsageTrackingUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetPassportUrl().c_str());

    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectEulaUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetImsStsAuthUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetUsageTrackingUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetPassportUrl().c_str());

    UrlProvider::CleanUpUrlCache();
    }

TEST_F(UrlProviderTests, CleanUpCache_UrlsWereCached_RemovesUrlsFromLocalState)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    StubLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, _))
        .Times(6)
        .WillRepeatedly(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);

    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectEulaUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetImsStsAuthUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetUsageTrackingUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetPassportUrl().c_str());

    UrlProvider::CleanUpUrlCache();

    EXPECT_CALL(*client, GetUrl(_, _))
        .Times(6)
        .WillRepeatedly(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::GetPunchlistWsgUrl().c_str();
    UrlProvider::GetConnectWsgUrl().c_str();
    UrlProvider::GetConnectEulaUrl().c_str();
    UrlProvider::GetImsStsAuthUrl().c_str();
    UrlProvider::GetUsageTrackingUrl().c_str();
    UrlProvider::GetPassportUrl().c_str();
    }

TEST_F(UrlProviderTests, Initialize_CalledSecondTimeWithDifferentEnvironment_CleansUpCache)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String urlDev = "testUrl_Dev";
    Utf8String urlQa = "testUrl_Qa";
    StubLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, _))
    .WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(urlDev))))
    .WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(urlQa))))
    .WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(urlDev))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);
    EXPECT_STREQ(urlDev.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());

    UrlProvider::Initialize(UrlProvider::Environment::Qa, &localState, client);
    EXPECT_STREQ(urlQa.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);
    EXPECT_STREQ(urlDev.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    }

TEST_F(UrlProviderTests, Initialize_CalledSecondTimeWithSameEnvironment_DoesNotCleanUpCache)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    StubLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);
    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);
    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    }

TEST_F(UrlProviderTests, GetSecurityConfigurator_InitializedWithDev_DoesNotSetValidateCertificate)
    {
    StubLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState);
    auto configurator = UrlProvider::GetSecurityConfigurator(GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (HttpRequestCR request)
        {
        EXPECT_FALSE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    HttpRequest request("foo");
    configurator->PerformRequest(request)->Wait();
    }

TEST_F(UrlProviderTests, GetSecurityConfigurator_InitializedWithQa_DoesNotSetValidateCertificate)
    {
    StubLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Qa, &localState);
    auto configurator = UrlProvider::GetSecurityConfigurator(GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (HttpRequestCR request)
        {
        EXPECT_FALSE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    HttpRequest request("foo");
    configurator->PerformRequest(request)->Wait();
    }

TEST_F(UrlProviderTests, GetSecurityConfigurator_InitializedWithRelease_SetsValidateCertificate)
    {
    StubLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Release, &localState);
    auto configurator = UrlProvider::GetSecurityConfigurator(GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (HttpRequestCR request)
        {
        EXPECT_TRUE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    HttpRequest request("foo");
    configurator->PerformRequest(request)->Wait();
    }
#endif
