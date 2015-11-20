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
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS
USING_NAMESPACE_BENTLEY_DGNCLIENTFX

#ifdef USE_GTEST
TEST_F(UrlProviderTests, GetPunchlistWsgUrl_NoCachedAndNoBuddiUrl_ReturnsDefaultUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    StubLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(""))));

    EXPECT_STRNE("", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_NoCachedAndNoConnectionError_ReturnsDefaultUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    StubLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Error(BuddiError::Status::ConnectionError))));

    EXPECT_STRNE("", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_CalledSecondTimeWhenUrlIsCached_GetsUrlFromLocalState)
    {
    auto client = std::make_shared<MockBuddiClient>();
    StubLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("TestUrl"))));

    EXPECT_STREQ("TestUrl", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    EXPECT_STREQ("TestUrl", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_TimeoutSetToZero_CallsBuddiToGetUrlEveryTime)
    {
    auto client = std::make_shared<MockBuddiClient>();
    StubLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, 0, &localState, client);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("A"))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("B"))));
    EXPECT_EQ("B", UrlProvider::Urls::ConnectWsgPunchList.Get());

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("C"))));
    EXPECT_EQ("C", UrlProvider::Urls::ConnectWsgPunchList.Get());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_CalledSecondTimeAfterTimeoutAndBuddiCannotConnect_ReturnsLastCachedUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    StubLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, 0, &localState, client);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("A"))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Error(BuddiError::Status::ConnectionError))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_TimeoutIsLessThanTimeElapsed_CallsBuddiToGetUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    StubLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, 5, &localState, client);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("A"))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());

    BeThreadUtilities::BeSleep(10);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("B"))));
    EXPECT_EQ("B", UrlProvider::Urls::ConnectWsgPunchList.Get());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_TimeoutIsMoreThanTimeElapsed_ReturnsCachedUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    StubLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, 3600 * 1000, &localState, client);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("A"))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());

    EXPECT_CALL(*client, GetUrl(_, _)).Times(0);
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_LocalStateHasOldUrlStoredAsString_IgnoresOldUrlAndCallsBuddi)
    {
    auto client = std::make_shared<MockBuddiClient>();
    MockLocalState localState;
    ON_CALL(localState, GetValue(_, _)).WillByDefault(Return(Json::Value::null));
    ON_CALL(localState, SaveValue(_, _, _)).WillByDefault(Return());
    ON_CALL(*client, GetUrl(_, _)).WillByDefault(Return(CreateCompletedAsyncTask(BuddiUrlResult())));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client);

    Json::Value newValue;

    EXPECT_CALL(localState, GetValue(_, _)).WillOnce(Return("OldUrl"));
    EXPECT_CALL(localState, SaveValue(_, _, _)).WillOnce(SaveArg<2>(&newValue));

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("NewUrl"))));
    EXPECT_EQ("NewUrl", UrlProvider::Urls::ConnectWsgPunchList.Get());

    EXPECT_CALL(localState, GetValue(_, _)).WillOnce(Return(newValue));
    EXPECT_EQ("NewUrl", UrlProvider::Urls::ConnectWsgPunchList.Get());
    }

TEST_F(UrlProviderTests, GetPunchlistWsgUrl_LocalStateHasOldUrlStoredAsStringAndBuddiConnectionError_ReturnsOldUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    MockLocalState localState;
    ON_CALL(localState, GetValue(_, _)).WillByDefault(Return(Json::Value::null));
    ON_CALL(localState, SaveValue(_, _, _)).WillByDefault(Return());

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client);

    EXPECT_CALL(localState, GetValue(_, _)).WillOnce(Return("OldUrl"));

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Error(BuddiError::Status::ConnectionError))));
    EXPECT_EQ("OldUrl", UrlProvider::Urls::ConnectWsgPunchList.Get());
    }

TEST_F(UrlProviderTests, GetUrl_ValidateAllGetters)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    StubLocalState localState;

    bset<Utf8String> urlNames;
    EXPECT_CALL(*client, GetUrl(_, _)).Times(9).WillRepeatedly(Invoke([&] (Utf8StringCR urlName, int regionId)
        {
        EXPECT_TRUE(urlNames.find(urlName) == urlNames.end());
        urlNames.insert(urlName);
        return CreateCompletedAsyncTask(BuddiUrlResult::Success(url));
        }));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client);

    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPersonalPublishing.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgProjectContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgGlobal.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgSharedContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectEula.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsStsAuth.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::UsageTracking.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::Passport.Get().c_str());

    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPersonalPublishing.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgProjectContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgGlobal.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgSharedContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectEula.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsStsAuth.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::UsageTracking.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::Passport.Get().c_str());

    UrlProvider::CleanUpUrlCache();
    }

TEST_F(UrlProviderTests, CleanUpCache_UrlsWereCached_RemovesUrlsFromLocalState)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "testUrl";
    StubLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, _))
        .Times(9)
        .WillRepeatedly(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client);

    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPersonalPublishing.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgProjectContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgGlobal.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgSharedContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectEula.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsStsAuth.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::UsageTracking.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::Passport.Get().c_str());

    UrlProvider::CleanUpUrlCache();

    EXPECT_CALL(*client, GetUrl(_, _))
        .Times(9)
        .WillRepeatedly(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::Urls::ConnectWsgPunchList.Get().c_str();
    UrlProvider::Urls::ConnectWsgPersonalPublishing.Get().c_str();
    UrlProvider::Urls::ConnectWsgProjectContent.Get().c_str();
    UrlProvider::Urls::ConnectWsgGlobal.Get().c_str();
    UrlProvider::Urls::ConnectWsgSharedContent.Get().c_str();
    UrlProvider::Urls::ConnectEula.Get().c_str();
    UrlProvider::Urls::ImsStsAuth.Get().c_str();
    UrlProvider::Urls::UsageTracking.Get().c_str();
    UrlProvider::Urls::Passport.Get().c_str();
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

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client);
    EXPECT_STREQ(urlDev.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());

    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, &localState, client);
    EXPECT_STREQ(urlQa.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client);
    EXPECT_STREQ(urlDev.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    }

TEST_F(UrlProviderTests, Initialize_CalledSecondTimeWithSameEnvironment_DoesNotCleanUpCache)
    {
    auto client = std::make_shared<MockBuddiClient>();
    StubLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("TestUrl"))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client);
    EXPECT_STREQ("TestUrl", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client);
    EXPECT_STREQ("TestUrl", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    }

TEST_F(UrlProviderTests, GetSecurityConfigurator_InitializedWithDev_DoesNotSetValidateCertificate)
    {
    StubLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState);
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

    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, &localState);
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

    UrlProvider::Initialize(UrlProvider::Environment::Release, UrlProvider::DefaultTimeout, &localState);
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
