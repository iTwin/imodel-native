/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../../Utils/WebServicesTestsHelper.h"
#include "../Connect/MockLocalState.h"
#include "MockBuddiClient.h"
#include <WebServices/Configuration/UrlProvider.h>
#include <Bentley/BeDebugLog.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

#define URL_COUNT 17

struct UrlProviderTests : BaseMockHttpHandlerTest
    {
    static WorkerThreadPtr s_thread;

    void SetUp() override
        {
        BaseMockHttpHandlerTest::SetUp();
        if (nullptr == s_thread)
            s_thread = WorkerThread::Create("UrlProviderTests");
        s_thread->OnEmpty()->Wait();
        };

    void TearDown() override
        {
        s_thread->OnEmpty()->Wait();
        BaseMockHttpHandlerTest::TearDown();
        };
    };

WorkerThreadPtr UrlProviderTests::s_thread;

struct UrlDescriptorTests : ::testing::Test {};

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetPunchlistWsgUrl_NoCachedAndNoBuddiUrl_ReturnsDefaultUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    RuntimeJsonLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client, nullptr, s_thread);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(""))));

    EXPECT_STRNE("", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetPunchlistWsgUrl_NoCachedAndNoConnectionError_ReturnsDefaultUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    RuntimeJsonLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client, nullptr, s_thread);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Error(BuddiError::Status::ConnectionError))));

    EXPECT_STRNE("", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetPunchlistWsgUrl_CalledSecondTimeWhenUrlIsCached_GetsUrlFromLocalState)
    {
    auto client = std::make_shared<MockBuddiClient>();
    RuntimeJsonLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client, nullptr, s_thread);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("https://test/foo"))));

    EXPECT_STREQ("https://test/foo", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    EXPECT_STREQ("https://test/foo", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetPunchlistWsgUrl_TimeoutSetToZero_ReturnsCachedUrlButAlsoCallBuddiGet)
    {
    auto client = std::make_shared<MockBuddiClient>();
    RuntimeJsonLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, 0, &localState, client, nullptr, s_thread);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("A"))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());
    s_thread->OnEmpty()->Wait();

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("B"))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());
    s_thread->OnEmpty()->Wait();

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("C"))));
    EXPECT_EQ("B", UrlProvider::Urls::ConnectWsgPunchList.Get());
    s_thread->OnEmpty()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetPunchlistWsgUrl_CalledSecondTimeAfterTimeoutAndBuddiCannotConnect_ReturnsLastCachedUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    RuntimeJsonLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, 0, &localState, client, nullptr, s_thread);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("A"))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Error(BuddiError::Status::ConnectionError))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetPunchlistWsgUrl_TimeoutIsLessThanTimeElapsed_CallsBuddiToGetUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    RuntimeJsonLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, 5, &localState, client, nullptr, s_thread);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("A"))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());
    s_thread->OnEmpty()->Wait();

    BeThreadUtilities::BeSleep(10);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("B"))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());
    s_thread->OnEmpty()->Wait();

    ON_CALL(*client, GetUrl(_, _)).WillByDefault(Return(CreateCompletedAsyncTask(BuddiUrlResult::Error({}))));
    EXPECT_EQ("B", UrlProvider::Urls::ConnectWsgPunchList.Get());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetPunchlistWsgUrl_TimeoutIsMoreThanTimeElapsed_ReturnsCachedUrl)
    {
    auto client = std::make_shared<MockBuddiClient>();
    RuntimeJsonLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, 3600 * 1000, &localState, client, nullptr, s_thread);

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("A"))));
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());

    EXPECT_CALL(*client, GetUrl(_, _)).Times(0);
    EXPECT_EQ("A", UrlProvider::Urls::ConnectWsgPunchList.Get());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetPunchlistWsgUrl_LocalStateHasOldUrlStoredAsString_IgnoresOldUrlAndCallsBuddi)
    {
    auto client = std::make_shared<MockBuddiClient>();
    MockLocalState localState;
    ON_CALL(localState, _GetValue(_, _)).WillByDefault(Return("null"));
    ON_CALL(localState, _SaveValue(_, _, _)).WillByDefault(Return());
    ON_CALL(*client, GetUrl(_, _)).WillByDefault(Return(CreateCompletedAsyncTask(BuddiUrlResult())));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client, nullptr, s_thread);

    Utf8String newValue;

    EXPECT_CALL(localState, _GetValue(_, _)).WillOnce(Return("OldUrl"));
    EXPECT_CALL(localState, _SaveValue(_, _, _)).WillOnce(SaveArg<2>(&newValue));

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("NewUrl"))));
    EXPECT_EQ("NewUrl", UrlProvider::Urls::ConnectWsgPunchList.Get());

    EXPECT_CALL(localState, _GetValue(_, _)).WillOnce(Return(newValue));
    EXPECT_EQ("NewUrl", UrlProvider::Urls::ConnectWsgPunchList.Get());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetUrl_ValidateAllGetters)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "https://test/foo";
    RuntimeJsonLocalState localState;

    bset<Utf8String> urlNames;
    EXPECT_CALL(*client, GetUrl(_, _)).Times(URL_COUNT).WillRepeatedly(Invoke([&] (Utf8StringCR urlName, int regionId)
        {
        EXPECT_TRUE(urlNames.find(urlName) == urlNames.end());
        urlNames.insert(urlName);
        return CreateCompletedAsyncTask(BuddiUrlResult::Success(url));
        }));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client, nullptr, s_thread);

    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectEula.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectTermsOfServiceUrl.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectProjectUrl.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgGlobal.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPersonalPublishing.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgProjectContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgProjectShare.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgClashIssues.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgSharedContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgRepositoryFederation.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectForms.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsStsAuth.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsActiveStsDelegationService.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsFederatedAuth.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::Passport.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::UsageTracking.Get().c_str());

    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectEula.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectTermsOfServiceUrl.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectProjectUrl.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgGlobal.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPersonalPublishing.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgProjectContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgProjectShare.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgClashIssues.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgSharedContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgRepositoryFederation.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectForms.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsStsAuth.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsActiveStsDelegationService.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsFederatedAuth.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::Passport.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::UsageTracking.Get().c_str());

    UrlProvider::CleanUpUrlCache();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, CleanUpCache_UrlsWereCached_RemovesUrlsFromLocalState)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String url = "https://test/foo";
    RuntimeJsonLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, _))
        .Times(URL_COUNT)
        .WillRepeatedly(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client, nullptr, s_thread);

    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectEula.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectTermsOfServiceUrl.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectProjectUrl.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgGlobal.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPersonalPublishing.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgProjectContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgProjectShare.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgClashIssues.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgSharedContent.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgRepositoryFederation.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectForms.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsStsAuth.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsActiveStsDelegationService.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ImsFederatedAuth.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::Passport.Get().c_str());
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::UsageTracking.Get().c_str());

    UrlProvider::CleanUpUrlCache();

    EXPECT_CALL(*client, GetUrl(_, _))
        .Times(URL_COUNT)
        .WillRepeatedly(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(url))));

    UrlProvider::Urls::ConnectEula.Get().c_str();
    UrlProvider::Urls::ConnectTermsOfServiceUrl.Get().c_str();
    UrlProvider::Urls::ConnectProjectUrl.Get().c_str();
    UrlProvider::Urls::ConnectWsgGlobal.Get().c_str();
    UrlProvider::Urls::ConnectWsgPersonalPublishing.Get().c_str();
    UrlProvider::Urls::ConnectWsgProjectContent.Get().c_str();
    UrlProvider::Urls::ConnectWsgProjectShare.Get().c_str();
    UrlProvider::Urls::ConnectWsgPunchList.Get().c_str();
    UrlProvider::Urls::ConnectWsgClashIssues.Get().c_str();
    UrlProvider::Urls::ConnectWsgSharedContent.Get().c_str();
    EXPECT_STREQ(url.c_str(), UrlProvider::Urls::ConnectWsgRepositoryFederation.Get().c_str());
    UrlProvider::Urls::ConnectForms.Get().c_str();
    UrlProvider::Urls::ImsStsAuth.Get().c_str();
    UrlProvider::Urls::ImsActiveStsDelegationService.Get().c_str();
    UrlProvider::Urls::ImsFederatedAuth.Get().c_str();
    UrlProvider::Urls::Passport.Get().c_str();
    UrlProvider::Urls::UsageTracking.Get().c_str();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, Initialize_CalledSecondTimeWithDifferentEnvironment_CleansUpCache)
    {
    auto client = std::make_shared<MockBuddiClient>();
    Utf8String urlDev = "http://test/dev";
    Utf8String urlQa = "http://test/qa";
    RuntimeJsonLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, _))
        .WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(urlDev))))
        .WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(urlQa))))
        .WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success(urlDev))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client, nullptr, s_thread);
    EXPECT_STREQ(urlDev.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());

    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, &localState, client, nullptr, s_thread);
    EXPECT_STREQ(urlQa.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client, nullptr, s_thread);
    EXPECT_STREQ(urlDev.c_str(), UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, Initialize_CalledSecondTimeWithSameEnvironment_DoesNotCleanUpCache)
    {
    auto client = std::make_shared<MockBuddiClient>();
    RuntimeJsonLocalState localState;

    EXPECT_CALL(*client, GetUrl(_, _)).WillOnce(Return(CreateCompletedAsyncTask(BuddiUrlResult::Success("https://test/foo"))));

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client, nullptr, s_thread);
    EXPECT_STREQ("https://test/foo", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, client, nullptr, s_thread);
    EXPECT_STREQ("https://test/foo", UrlProvider::Urls::ConnectWsgPunchList.Get().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetSecurityConfigurator_InitializedWithDev_DoesNotSetValidateCertificate)
    {
    RuntimeJsonLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &localState, nullptr, nullptr, s_thread);
    auto configurator = UrlProvider::GetSecurityConfigurator(GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (Http::RequestCR request)
        {
        EXPECT_FALSE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    Http::Request request("https://test/foo", "GET", configurator);
    request.SetValidateCertificate(false);
    request.Perform().wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetSecurityConfigurator_InitializedWithQa_DoesNotSetValidateCertificate)
    {
    RuntimeJsonLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, &localState, nullptr, nullptr, s_thread);
    auto configurator = UrlProvider::GetSecurityConfigurator(GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (Http::RequestCR request)
        {
        EXPECT_FALSE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    Http::Request request("https://test/foo", "GET", configurator);
    request.SetValidateCertificate(false);
    request.Perform().wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetSecurityConfigurator_InitializedWithRelease_SetsValidateCertificate)
    {
    RuntimeJsonLocalState localState;

    UrlProvider::Initialize(UrlProvider::Environment::Release, UrlProvider::DefaultTimeout, &localState, nullptr, nullptr, s_thread);
    auto configurator = UrlProvider::GetSecurityConfigurator(GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (Http::RequestCR request)
        {
        EXPECT_TRUE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    Http::Request request("https://test/foo", "GET", configurator);
    request.SetValidateCertificate(false);
    request.Perform().wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Daumantas.Kojelis   08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetSecurityConfigurator_InitializedWithReleaseAndSetToQa_DoesNotSetValidateCertificate)
    {
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Environment::Release, UrlProvider::DefaultTimeout, &localState, nullptr, nullptr, s_thread);
    UrlProvider::SetEnvironment(UrlProvider::Environment::Qa);
    ASSERT_EQ(UrlProvider::Environment::Qa, UrlProvider::GetEnvironment());

    auto configurator = UrlProvider::GetSecurityConfigurator(GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=](Http::RequestCR request)
        {
        EXPECT_FALSE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    Http::Request request("https://test/foo", "GET", configurator);
    request.SetValidateCertificate(false);
    request.Perform().wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Daumantas.Kojelis   08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetSecurityConfigurator_InitializedWithQaAndSetToRelease_SetsValidateCertificate)
    {
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, &localState, nullptr, nullptr, s_thread);
    UrlProvider::SetEnvironment(UrlProvider::Environment::Release);
    ASSERT_EQ(UrlProvider::Environment::Release, UrlProvider::GetEnvironment());

    auto configurator = UrlProvider::GetSecurityConfigurator(GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=](Http::RequestCR request)
        {
        EXPECT_TRUE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    Http::Request request("https://test/foo", "GET", configurator);
    request.SetValidateCertificate(false);
    request.Perform().wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, ResolveUrlDescriptor_EmptyURI_Null)
    {
    EXPECT_EQ(nullptr, UrlProvider::ResolveUrlDescriptor(""));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, ResolveUrlDescriptor_InvalidScheme_Null)
    {
    EXPECT_EQ(nullptr, UrlProvider::ResolveUrlDescriptor("test://resolve/" + UrlProvider::Urls::ConnectWsgGlobal.GetName()));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, ResolveUrlDescriptor_InvalidHost_Null)
    {
    EXPECT_EQ(nullptr, UrlProvider::ResolveUrlDescriptor("buddi://foo/" + UrlProvider::Urls::ConnectWsgGlobal.GetName()));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, ResolveUrlDescriptor_EmptyPath_Null)
    {
    EXPECT_EQ(nullptr, UrlProvider::ResolveUrlDescriptor("buddi://resolve" + UrlProvider::Urls::ConnectWsgGlobal.GetName()));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, ResolveUrlDescriptor_UnknownDescriptorName_Null)
    {
    EXPECT_EQ(nullptr, UrlProvider::ResolveUrlDescriptor("buddi://resolve/foobar" + UrlProvider::Urls::ConnectWsgGlobal.GetName()));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, ResolveUrlDescriptor_KnownDescriptorName_ExpectedDescriptor)
    {
    const UrlProvider::UrlDescriptor* descriptor = UrlProvider::ResolveUrlDescriptor
        (
        "buddi://resolve/" + UrlProvider::Urls::ConnectWsgGlobal.GetName()
        );
    ASSERT_NE(nullptr, descriptor);
    EXPECT_STREQ(UrlProvider::Urls::ConnectWsgGlobal.GetName().c_str(), descriptor->GetName().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlDescriptorTests, GetBuddiUrl_UrlDescriptorWithNonEmptyName_ExpectedUriString)
    {
    UrlProvider::UrlDescriptor::Registry registry;
    UrlProvider::UrlDescriptor descriptor("foobar", "", "", "", "", &registry);
    EXPECT_STREQ("buddi://resolve/foobar", descriptor.GetBuddiUri().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                  Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlDescriptorTests, GetBuddiUrl_UrlDescriptorWithEmptyName_EmptyString)
    {
    UrlProvider::UrlDescriptor::Registry registry;
    UrlProvider::UrlDescriptor descriptor("", "", "", "", "", &registry);
    EXPECT_STREQ("", descriptor.GetBuddiUri().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetStoredEnvironment_FreshLocalState_ReturnsFalse)
    {
    RuntimeJsonLocalState localState;
    UrlProvider::Environment env = (UrlProvider::Environment)123;
    EXPECT_FALSE(UrlProvider::GetStoredEnvironment(localState, env));
    EXPECT_EQ(123, env);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, GetStoredEnvironment_InitializedLocalState_ReturnsTrueAndEnvironment)
    {
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, &localState, nullptr, nullptr, s_thread);
    UrlProvider::Uninitialize();

    UrlProvider::Environment env = (UrlProvider::Environment)123;
    EXPECT_TRUE(UrlProvider::GetStoredEnvironment(localState, env));
    EXPECT_EQ(UrlProvider::Environment::Qa, env);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, ToEnvironmentString_ValidValues_ReturnsStringRepresentations)
    {
    EXPECT_EQ(UrlProvider::ToEnvironmentString(UrlProvider::Environment::Dev), "DEV");
    EXPECT_EQ(UrlProvider::ToEnvironmentString(UrlProvider::Environment::Qa), "QA");
    EXPECT_EQ(UrlProvider::ToEnvironmentString(UrlProvider::Environment::Release), "PROD");
    EXPECT_EQ(UrlProvider::ToEnvironmentString(UrlProvider::Environment::Perf), "PERF");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, ToEnvironmentString_InvalidValue_ReturnsEmpty)
    {
    EXPECT_EQ(UrlProvider::ToEnvironmentString((UrlProvider::Environment) 42), "");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, FromEnvironmentString_ValidValues_ReturnsEnumRepresentations)
    {
    auto env = UrlProvider::Environment::Qa;
    EXPECT_EQ(UrlProvider::FromEnvironmentString("DEV", env), SUCCESS);
    EXPECT_EQ(env, UrlProvider::Environment::Dev);
    EXPECT_EQ(UrlProvider::FromEnvironmentString("QA", env), SUCCESS);
    EXPECT_EQ(env, UrlProvider::Environment::Qa);
    EXPECT_EQ(UrlProvider::FromEnvironmentString("PROD", env), SUCCESS);
    EXPECT_EQ(env, UrlProvider::Environment::Release);
    EXPECT_EQ(UrlProvider::FromEnvironmentString("PERF", env), SUCCESS);
    EXPECT_EQ(env, UrlProvider::Environment::Perf);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, FromEnvironmentString_InvalidValue_ReturnsError)
    {
    auto env = UrlProvider::Environment::Qa;
    EXPECT_EQ(UrlProvider::FromEnvironmentString("FOO", env), ERROR);
    EXPECT_EQ(env, UrlProvider::Environment::Qa);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                                      Vincas.Razma    06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UrlProviderTests, FromEnvironmentString_NullValue_ReturnsError)
    {
    auto env = UrlProvider::Environment::Qa;
    EXPECT_EQ(UrlProvider::FromEnvironmentString(nullptr, env), ERROR);
    EXPECT_EQ(env, UrlProvider::Environment::Qa);
    }


#endif
