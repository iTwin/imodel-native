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

    EXPECT_CALL(localState, GetValue(_, _)).WillOnce(Return(""));
    EXPECT_CALL(localState, SaveValue(_, _, _)).Times(1);
    EXPECT_CALL(*client, GetUrl(_, 0)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);

    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_UrlIsCached_GetsUrlFromLocalState)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    StubLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, 0)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

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

    EXPECT_CALL(localState, GetValue(_, _)).WillOnce(Return(url));
    EXPECT_CALL(*client, GetUrl(_, 0)).Times(0);

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);

    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_NoCachedAndNoBuddiUrl_ReturnsDefaultUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    StubLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, 0)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(""))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);
    EXPECT_TRUE(!UrlProvider::GetPunchlistWsgUrl().empty());
    }

TEST_F(UrlProviderTests, GetUrl_ValidateAllGetters)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    StubLocalState localState;

    bset<Utf8String> urlNames;
    EXPECT_CALL(*client, GetUrl(_, 0)).Times(6).WillRepeatedly(Invoke([&] (Utf8StringCR urlName, int regionId)
        {
        EXPECT_TRUE(urlNames.find(urlName) == urlNames.end());
        urlNames.insert(urlName);
        return CreateCompletedAsyncTask(BuddiUrlResult::Success(url));
        }));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);

    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectEulaUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectLearnStsAuthUri().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetUsageTrackingUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetPassportUrl().c_str());

    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectEulaUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectLearnStsAuthUri().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetUsageTrackingUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetPassportUrl().c_str());

    UrlProvider::CleanUpUrlCache();
    }

TEST_F(UrlProviderTests, DISABLED_CleanUpCache_UrlsWereCached_RemovesUrlsFromLocalState)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    StubLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, 0))
        .Times(6)
        .WillRepeatedly(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, &localState, client);

    EXPECT_STREQ(url.c_str(), UrlProvider::GetPunchlistWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectWsgUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectEulaUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetConnectLearnStsAuthUri().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetUsageTrackingUrl().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::GetPassportUrl().c_str());

    UrlProvider::CleanUpUrlCache();

    EXPECT_CALL(*client, GetUrl(_, 0))
        .Times(6)
        .WillRepeatedly(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::GetPunchlistWsgUrl().c_str();
    UrlProvider::GetConnectWsgUrl().c_str();
    UrlProvider::GetConnectEulaUrl().c_str();
    UrlProvider::GetConnectLearnStsAuthUri().c_str();
    UrlProvider::GetUsageTrackingUrl().c_str();
    UrlProvider::GetPassportUrl().c_str();
    }
#endif
