/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/PolicyProviderTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "TestsHelper.h"
#include "PolicyProviderTests.h"
#include "Utils/MockHttpHandler.h"
#include "DummyPolicyHelper.h"

#include <Licensing/Client.h>
#include <Licensing/SaasClient.h>
#include <Licensing/Utils/DateHelper.h>
#include <Licensing/AuthType.h>
#include "../../Licensing/Providers/PolicyProvider.h"
#include "../../PublicAPI/Licensing/Utils/SCVWritter.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <fstream>
#include <Licensing/Utils/InMemoryJsonLocalState.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

#include "Mocks/BuddiProviderMock.h"
#include "Mocks/PolicyProviderMock.h"

using ::testing::AtLeast;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::ByMove;
using ::testing::_;

#define TEST_PRODUCT_ID     "2545"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_SQLITE

// ClientImplPtr CreateTestClient(bool signIn)
//     {
//     return CreateTestClient(signIn, 1000, TimeRetriever::Get(), DelayedExecutor::Get(), UrlProvider::Environment::Qa, TEST_PRODUCT_ID, nullptr, nullptr);
//     }

PolicyProviderTests::PolicyProviderTests() :
    m_handlerMock(std::make_shared<MockHttpHandler>()),
    m_buddiMock(std::make_shared<BuddiProviderMock>()),
    m_authMock(std::make_shared<AuthHandlerProviderMock>())
    {
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", "2545");

    EXPECT_CALL(*m_authMock, GetAuthHandler(_, _))
        .WillRepeatedly(Return(m_handlerMock));
    m_policyProvider = std::make_shared<PolicyProvider>(m_buddiMock, clientInfo, m_handlerMock, AuthType::SAML, m_authMock);
    }

PolicyProvider& PolicyProviderTests::GetPolicyProvider() const {
    return *m_policyProvider;
}

MockHttpHandler& PolicyProviderTests::GetMockHttp() const
    {
    return *m_handlerMock;
    }


std::shared_ptr<MockHttpHandler> PolicyProviderTests::GetHandlerPtr() const
    {
    return m_handlerMock;
    }


void PolicyProviderTests::TearDown()
    {
    m_handlerMock->ValidateAndClearExpectations();
    }

void PolicyProviderTests::SetUpTestCase()
    {
    // This is only an example of how to set logging severity and see info logs. Usually should be set more globally than in TestCase SetUp
    // NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_LICENSING, BentleyApi::NativeLogging::LOG_INFO);

    BeFileName asssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(asssetsDir);
    HttpClient::Initialize(asssetsDir);

    BeFileName tmpDir;
    BeTest::GetHost().GetTempDir(tmpDir);
    BeSQLiteLib::Initialize(tmpDir);

    BeFileName path;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
    path.AppendToPath(L"sqlang/DgnClientFx_en.sqlang.db3");

    ASSERT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
    }

BuddiProviderMock& PolicyProviderTests::GetMockBuddi() const {
    return *m_buddiMock;
} 

Utf8String PolicyProviderTests::MockEntitlementUrl() {
    Utf8String mockUrl("https://entitlementmockurl.bentley.com/");
    EXPECT_CALL(GetMockBuddi(), EntitlementPolicyBaseUrl())
        .Times(1)
        .WillRepeatedly(Return(mockUrl));

    return mockUrl;
}

TEST_F(PolicyProviderTests, PerformGetPolicyRequest_Success)
    {
    const auto mockUrl = MockEntitlementUrl();
    Utf8String expectedUrl = mockUrl + "/GetPolicy";

    GetMockHttp().ExpectRequests(1);
    GetMockHttp().ForRequest(1, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
        return MockHttpHandler::StubHttpResponse();
        });

    // assert our outgoing request matches expectation

    // assert that we return the result of the call on success
    const auto result = GetPolicyProvider().PerformGetPolicyRequest().get();
    }

TEST_F(PolicyProviderTests, PerformGetPolicyRequest_Failure)
    {
    // assert that we we fall into catch on non-success status code
    }

TEST_F(PolicyProviderTests, GetCertificate_Success)
    {
    // assert our outgoing request matches expectation

    // assert that we return the result of the call on success
    }

TEST_F(PolicyProviderTests, GetCertificate_Failure)
    {
    // assert that we we fall into catch on non-success status code
    }


TEST_F(PolicyProviderTests, GetPolicy_Success)
    {
    // assert our outgoing request matches expectation

    // assert that we return a well formed policy
    }

TEST_F(PolicyProviderTests, GetPolicy_FetchPolicyFailure)
    {
    // assert that we we fall into catch on failure getting the policy
    }

TEST_F(PolicyProviderTests, GetPolicy_getCertificateFailure)
    {
    // assert that we we fall into catch on failure getting the cert
    }



