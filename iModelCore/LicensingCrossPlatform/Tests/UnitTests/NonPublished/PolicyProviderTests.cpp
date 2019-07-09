/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/PolicyProviderTests.cpp $
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
#include "../../../Licensing/Providers/PolicyProvider.h"

#include <BeHttp/HttpClient.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/L10N.h>
#include <fstream>
#include <Licensing/Utils/InMemoryJsonLocalState.h>
#include <Licensing/Utils/SCVWritter.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

#include "Mocks/BuddiProviderMock.h"
#include "Mocks/PolicyProviderMock.h"

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
    m_httpHandlerMock(std::make_shared<MockHttpHandler>()),
    m_buddiMock(std::make_shared<BuddiProviderMock>()),
    m_authProviderMock(std::make_shared<AuthHandlerProviderMock>())
    {
    auto clientInfo = std::make_shared<ClientInfo>("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", "2545");

    GetAuthHandlerProviderMock().MockGetAuthHandler(m_httpHandlerMock);

    m_policyProvider = std::make_shared<PolicyProvider>(m_buddiMock, clientInfo, m_httpHandlerMock, AuthType::SAML, m_authProviderMock);
    }

PolicyProvider& PolicyProviderTests::GetPolicyProvider() const
    {
    return *m_policyProvider;
    }

MockHttpHandler& PolicyProviderTests::GetMockHttpHandler() const
    {
    return *m_httpHandlerMock;
    }

std::shared_ptr<MockHttpHandler> PolicyProviderTests::GetMockHttpHandlerPtr() const
    {
    return m_httpHandlerMock;
    }

AuthHandlerProviderMock& PolicyProviderTests::GetAuthHandlerProviderMock() const
    {
    return *m_authProviderMock;
    }

std::shared_ptr<AuthHandlerProviderMock> PolicyProviderTests::GetAuthHandlerProviderMockPtr() const
    {
    return m_authProviderMock;
    }

void PolicyProviderTests::TearDown()
    {
    m_httpHandlerMock->ValidateAndClearExpectations();
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
    path.AppendToPath(L"TestAssets/sqlang/DgnClientFx_en.sqlang.db3");

    ASSERT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
    }

BuddiProviderMock& PolicyProviderTests::GetMockBuddi() const
    {
    return *m_buddiMock;
    } 

Utf8String PolicyProviderTests::MockEntitlementUrl()
    {
    Utf8String mockUrl("https://entitlementmockurl.bentley.com/");

    GetMockBuddi().MockEntitlementPolicyBaseUrl(mockUrl);

    return mockUrl;
    }

TEST_F(PolicyProviderTests, PerformGetPolicyRequest_Success)
    {
    const auto mockUrl = MockEntitlementUrl();
    Utf8String expectedUrl = mockUrl + "/GetPolicy";

    GetMockHttpHandler().ExpectRequests(1);
    GetMockHttpHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
        return MockHttpHandler::StubHttpResponse();
        });

    // assert our outgoing request matches expectation

    // assert that we return the result of the call on success
    const auto result = GetPolicyProvider().PerformGetPolicyRequest().get();
    EXPECT_EQ(MockHttpHandler::StubHttpResponse().GetBody().AsString(), result);
    }

TEST_F(PolicyProviderTests, PerformGetPolicyRequest_Failure)
    {
    // assert that we we fall into catch on non-success status code
    const auto mockUrl = MockEntitlementUrl();
    Utf8String expectedUrl = mockUrl + "/GetPolicy";

    GetMockHttpHandler().ExpectRequests(1);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
        return MockHttpHandler::StubHttpFailureResponse();
        });

    try
        {
        const auto result = GetPolicyProvider().PerformGetPolicyRequest().get();
        FAIL() << "Expected an execption to be thrown";
        }
    catch (HttpError error)
        {
        // expect connection and http status to be the same as StubHttpFailureResponse
        EXPECT_EQ(error.GetConnectionStatus(), MockHttpHandler::StubHttpFailureResponse().GetConnectionStatus());
        EXPECT_EQ(error.GetHttpStatus(), MockHttpHandler::StubHttpFailureResponse().GetHttpStatus());
        }
    catch (...)
        {
        FAIL() << "Expected exception to be an HttpError";
        }
    }

TEST_F(PolicyProviderTests, GetCertificate_Success)
    {
    // assert our outgoing request matches expectation
    const auto mockUrl = MockEntitlementUrl();
    Utf8String expectedUrl = mockUrl + "/PolicyTokenSigningCertificate";

    GetMockHttpHandler().ExpectRequests(1);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
        return MockHttpHandler::StubHttpResponse();
        });

    // assert that we return the result of the call on success
    const auto result = GetPolicyProvider().GetCertificate().get();
    EXPECT_EQ(MockHttpHandler::StubHttpResponse().GetBody().AsString(), result);
    }

TEST_F(PolicyProviderTests, GetCertificate_Failure)
    {
    // assert that we we fall into catch on non-success status code
    const auto mockUrl = MockEntitlementUrl();
    Utf8String expectedUrl = mockUrl + "/PolicyTokenSigningCertificate";

    GetMockHttpHandler().ExpectRequests(1);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
        return MockHttpHandler::StubHttpFailureResponse();
        });

    try
        {
        const auto result = GetPolicyProvider().GetCertificate().get();
        FAIL() << "Expected an execption to be thrown";
        }
    catch (HttpError error)
        {
        // expect connection and http status to be the same as StubHttpFailureResponse
        EXPECT_EQ(error.GetConnectionStatus(), MockHttpHandler::StubHttpFailureResponse().GetConnectionStatus());
        EXPECT_EQ(error.GetHttpStatus(), MockHttpHandler::StubHttpFailureResponse().GetHttpStatus());
        }
    catch (...)
        {
        FAIL() << "Expected exception to be an HttpError";
        }
    }

// can't tell which order folly::collectAll will call (seems to be different linux vs windows)
TEST_F(PolicyProviderTests, GetPolicy_Success)
    {
    // assert our outgoing request matches expectation
    const auto mockUrl = MockEntitlementUrl();
    Utf8String policyUrl = mockUrl + "/GetPolicy";
    Utf8String certUrl = mockUrl + "/PolicyTokenSigningCertificate";

    Utf8String certToken = "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";
    Utf8String policyToken = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsIng1dCI6ImVSUWFLMkhfZVVha2R5WjFxWVpDd1lmb0VlUSJ9.eyJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcy9wb2xpY3kiOiJleUpRYjJ4cFkzbEpaQ0k2SWpsa1ptSTBObU0yTFdKa00yRXROR1ZsWmkwNU5qaGtMV0prWkRkbE5XSXpaVEUzWVNJc0lsQnZiR2xqZVZabGNuTnBiMjRpT2pFdU1Dd2lVRzlzYVdONVEzSmxZWFJsWkU5dUlqb2lNakF4T1Mwd015MHhOVlF4T1Rvd05EbzBNaTQwTVRneE1ESTNXaUlzSWxCdmJHbGplVVY0Y0dseVpYTlBiaUk2SWpJd01Ua3RNRE10TURaVU1qRTZNVFE2TlRndU9EWTJXaUlzSWxKbGNYVmxjM1JFWVhSaElqcDdJazFoWTJocGJtVlRTVVFpT201MWJHd3NJa0ZqWTJWemMwdGxlU0k2Ym5Wc2JDd2lWWE5sY2tsa0lqb2lZMkV4WTJNMlkyRXRNbUZtTVMwMFpXWmtMVGc0TnpZdFptUTFPVEV3WVROaE4yWmhJaXdpUTJobFkydGxaRTkxZEVSaGRHVWlPbTUxYkd3c0lsSmxjWFZsYzNSbFpGTmxZM1Z5WVdKc1pYTWlPbHQ3SWxCeWIyUjFZM1JKWkNJNk1qVTBOU3dpUm1WaGRIVnlaVk4wY21sdVp5STZJaUlzSWxabGNuTnBiMjRpT2lJeExqQXVNQzR3SW4xZExDSk5ZV05vYVc1bFRtRnRaU0k2SWxSbGMzUkVaWFpwWTJWSlpDSXNJa05zYVdWdWRFUmhkR1ZVYVcxbElqb2lNakF4T1Mwd015MHhOVlF4T1Rvd05EbzBNQzR4TmpOYUlpd2lURzlqWVd4bElqb2laVzRpTENKQmNIQnNhV1Z6Vkc4aU9pSm9kSFJ3Y3pvdkwyVnVkR2wwYkdWdFpXNTBMWE5sWVhKamFDNWlaVzUwYkdWNUxtTnZiUzhpZlN3aVRXRmphR2x1WlZOcFoyNWhkSFZ5WlNJNkltNTNTMEp5WmpSc2JHUjZWblJhU1U5emRHNWFiVGxIT1VnM1l6MGlMQ0pCY0hCc2FXVnpWRzlWYzJWeVNXUWlPaUpqWVRGall6WmpZUzB5WVdZeExUUmxabVF0T0RnM05pMW1aRFU1TVRCaE0yRTNabUVpTENKQmNIQnNhV1Z6Vkc5VFpXTjFjbUZpYkdWSlpITWlPbHNpT0RRMFl6RXdZekl0TXpjMVpDMDBNVFV6TFdFd09HWXRPRGsyWkRKbE5qUmhNVE5tSWwwc0lrRkRUSE1pT2x0N0lsQnlhVzVqYVhCaGJFbGtJam9pWTJFeFkyTTJZMkV0TW1GbU1TMDBaV1prTFRnNE56WXRabVExT1RFd1lUTmhOMlpoSWl3aVUyVmpkWEpoWW14bFNXUWlPaUk0TkRSak1UQmpNaTB6TnpWa0xUUXhOVE10WVRBNFppMDRPVFprTW1VMk5HRXhNMllpTENKQlkyTmxjM05MYVc1a0lqbzBMQ0pGZUhCcGNtVnpUMjRpT2lJeU1ERTVMVEF6TFRBMlZESXhPakUwT2pVNExqZzJObG9pTENKUmRXRnNhV1pwWlhKUGRtVnljbWxrWlhNaU9sdDdJazVoYldVaU9pSlZjMkZuWlZSNWNHVWlMQ0pXWVd4MVpTSTZJbFJ5YVdGc0lpd2lWSGx3WlNJNkluTjBjbWx1WnlJc0lsQnliMjF3ZENJNklsUnlhV0ZzSUVWdWRHbDBiR1Z0Wlc1MEluMWRmVjBzSWxObFkzVnlZV0pzWlVSaGRHRWlPbHQ3SWxObFkzVnlZV0pzWlVsa0lqb2lPRFEwWXpFd1l6SXRNemMxWkMwME1UVXpMV0V3T0dZdE9EazJaREpsTmpSaE1UTm1JaXdpVUhKdlpIVmpkRWxrSWpveU5UUTFMQ0pRY205a2RXTjBUbUZ0WlNJNklrSmxiblJzWlhrZ1RtRjJhV2RoZEc5eUlpd2lSbVZoZEhWeVpWTjBjbWx1WnlJNklpSXNJbFpsY25OcGIyNGlPbTUxYkd3c0lsRjFZV3hwWm1sbGNrOTJaWEp5YVdSbGN5STZiblZzYkgxZExDSlZjMlZ5UkdGMFlTSTZleUpWYzJWeVNXUWlPaUpqWVRGall6WmpZUzB5WVdZeExUUmxabVF0T0RnM05pMW1aRFU1TVRCaE0yRTNabUVpTENKUGNtZGhibWw2WVhScGIyNUpaQ0k2SW1Fd1pHTTFZVE0xTFRkaE9ERXRORGRoTlMwNU16Z3dMVFZtTlRSaVpqQTBNR00wTXlJc0lsVnpZV2RsUTI5MWJuUnllVWxUVHlJNklsVlRJaXdpVld4MGFXMWhkR1ZUUVZCSlpDSTZJakV3TURVMk56TTJNRElpTENKVmJIUnBiV0YwWlVsa0lqb2lZVEkyWm1Sa01Ea3RNRGs0TkMwME1HTmlMV0V4T1RRdFpEbGhPREpoTXpVeE9UVTNJaXdpVld4MGFXMWhkR1ZEYjNWdWRISjVTV1FpT2lKallURmpZelpqWVMweVlXWXhMVFJsWm1RdE9EZzNOaTFtWkRVNU1UQmhNMkUzWm1FaUxDSkZiblJwZEd4bGJXVnVkRWR5YjNWd1NXUWlPaUl3TURBd01EQXdNQzB3TURBd0xUQXdNREF0TURBd01DMHdNREF3TURBd01EQXdNREFpZlN3aVRXVjBZVVJoZEdFaU9tNTFiR3dzSWtSbFptRjFiSFJSZFdGc2FXWnBaWEp6SWpwYmV5Sk9ZVzFsSWpvaVJuSmxjWFZsYm1ONVZHOVRaVzVrSWl3aVZtRnNkV1VpT2lJeU5DSXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJbFZ6WlVGc1pYSjBhVzVuVTJWeWRtbGpaU0lzSWxaaGJIVmxJam9pUkdsellXSnNaV1FpTENKVWVYQmxJam9pWlc1MWJTSXNJbEJ5YjIxd2RDSTZJbGx2ZFNCb1lYWmxJSEpsWVdOb1pXUWdZU0JzYVdObGJuTmxJSFJvY21WemFHOXNaQ0JrWldacGJtVmtJR0o1SUhsdmRYSWdiM0puWVc1cGVtRjBhVzl1SjNNZ1lXUnRhVzVwYzNSeVlYUnZjaTRnUTI5dWRHbHVkV2x1WnlCdFlYa2djbVZ6ZFd4MElHbHVJR0ZrWkdsMGFXOXVZV3dnWVhCd2JHbGpZWFJwYjI0Z2RYTmhaMlVnWTJoaGNtZGxjeTVjY2x4dVZHOGdjSEp2WTJWbFpDd2dlVzkxSUcxMWMzUWdYQ0pCWTJ0dWIzZHNaV1JuWlZ3aUxpSjlMSHNpVG1GdFpTSTZJa2x6UTJobFkydGxaRTkxZENJc0lsWmhiSFZsSWpvaVptRnNjMlVpTENKVWVYQmxJam9pWW05dmJDSXNJbEJ5YjIxd2RDSTZiblZzYkgwc2V5Sk9ZVzFsSWpvaVFXeHNiM2RQWm1ac2FXNWxWWE5oWjJVaUxDSldZV3gxWlNJNklsUlNWVVVpTENKVWVYQmxJam9pWW05dmJDSXNJbEJ5YjIxd2RDSTZJbGx2ZFNCdGRYTjBJRk5wWjI0Z1NXNGdkRzhnZFhObElIUm9hWE1nWVhCd2JHbGpZWFJwYjI0dUluMHNleUpPWVcxbElqb2lTRzkwY0dGMGFGSmxkSEo1UVhSMFpXMXdkSE1pTENKV1lXeDFaU0k2SWpRaUxDSlVlWEJsSWpvaWFXNTBJaXdpVUhKdmJYQjBJanB1ZFd4c2ZTeDdJazVoYldVaU9pSk5ZWGhNYjJkR2FXeGxSSFZ5WVhScGIyNUpiazFwYm5WMFpYTWlMQ0pXWVd4MVpTSTZJak0yTUNJc0lsUjVjR1VpT2lKcGJuUWlMQ0pRY205dGNIUWlPbTUxYkd4OUxIc2lUbUZ0WlNJNklrOW1abXhwYm1WRWRYSmhkR2x2YmlJc0lsWmhiSFZsSWpvaU55SXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJbFJwYldWVWIwdGxaWEJWYmxObGJuUk1iMmR6SWl3aVZtRnNkV1VpT2lJMk1DSXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJbFZ6WVdkbFZIbHdaU0lzSWxaaGJIVmxJam9pVUhKdlpIVmpkR2x2YmlJc0lsUjVjR1VpT2lKemRISnBibWNpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJa3h2WjFKbFkyOXlaSE5VYjFObGJtUkdWRElpTENKV1lXeDFaU0k2SWpjMU1EQXdJaXdpVkhsd1pTSTZJbWx1ZENJc0lsQnliMjF3ZENJNmJuVnNiSDBzZXlKT1lXMWxJam9pVUc5c2FXTjVTVzUwWlhKMllXd2lMQ0pXWVd4MVpTSTZJakkwTUNJc0lsUjVjR1VpT2lKcGJuUWlMQ0pRY205dGNIUWlPbTUxYkd4OUxIc2lUbUZ0WlNJNklreHZaMmx1UjNKaFkyVlFaWEpwYjJRaUxDSldZV3gxWlNJNklqY2lMQ0pVZVhCbElqb2lhVzUwSWl3aVVISnZiWEIwSWpwdWRXeHNmU3g3SWs1aGJXVWlPaUpJWldGeWRHSmxZWFJKYm5SbGNuWmhiQ0lzSWxaaGJIVmxJam9pTVNJc0lsUjVjR1VpT2lKcGJuUWlMQ0pRY205dGNIUWlPbTUxYkd4OUxIc2lUbUZ0WlNJNklsWmxjbk5wYjI0aUxDSldZV3gxWlNJNklqQXVNQzR3TGpBaUxDSlVlWEJsSWpvaWMzUnlhVzVuSWl3aVVISnZiWEIwSWpwdWRXeHNmVjE5IiwiaXNzIjoiaHR0cDovL3BvbGljeXNlcnZpY2UuYmVudGxleS5jb20iLCJhdWQiOiJodHRwczovL2VudGl0bGVtZW50LXNlYXJjaC5iZW50bGV5LmNvbS8iLCJleHAiOjE1NTMyODE0ODMsIm5iZiI6MTU1MjY3NjY4M30.NEdSIUWarGPv_sNvrvWj4WunKggA68Q_PQ7n2kBjoefpgs9bhSqzi9dzTjP5i2pBP0WHU-tTsRyEUiLI51Iz4P6LsBldfiFkF_Y4FFSmArR8LLqZpWgrruMAI7R9uwWYJyVnuVWhbcsZROVhOUTW4so9U8EPW_yGqj9vwlKjLy5PcjsV27_OupbWUSGE5eNzM7SpnXckoD-VM6DE0Yfe9p176sgXGukppqYwkNIZso9RUFvp6KGx_VYGi8i7L-tuf5jA78ENLZLL31_jGKaqNL8ZqHy3eA_7PzyhtlqrgQWVNSMxq-jmRv3U7EbgVRI9PeJAVLJvXqbPJGLt2ZiHSg";

    std::shared_ptr<Policy> expectedPolicy = Policy::Create(JWToken::Create(policyToken, certToken));

    // order of these calls should not matter, this doesn't check that each url is called
    GetMockHttpHandler().ExpectRequests(2);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        if (request.GetUrl() == policyUrl)
            return MockHttpHandler::StubHttpResponse(policyToken);
        else if (request.GetUrl() == certUrl)
            return MockHttpHandler::StubHttpResponse(certToken);
        else
            return MockHttpHandler::StubHttpFailureResponse();
        });
    GetMockHttpHandler().ForRequest(2, [=](Http::RequestCR request)
        {
        if (request.GetUrl() == policyUrl)
            return MockHttpHandler::StubHttpResponse(policyToken);
        else if (request.GetUrl() == certUrl)
            return MockHttpHandler::StubHttpResponse(certToken);
        else
            return MockHttpHandler::StubHttpFailureResponse();
        });

    // assert that we return a well formed policy
    const auto result = GetPolicyProvider().GetPolicy().get();
    auto resultJson = result->GetJson();
    auto expectedJson = expectedPolicy->GetJson();
    EXPECT_EQ(expectedJson, resultJson);
    }

// can't tell which order folly::collectAll will call (seems to be different linux vs windows)
TEST_F(PolicyProviderTests, GetPolicy_GetPolicyFailure)
    {
    // assert that we we fall into catch on failure getting the policy
    const auto mockUrl = MockEntitlementUrl();
    Utf8String policyUrl = mockUrl + "/GetPolicy";
    Utf8String certUrl = mockUrl + "/PolicyTokenSigningCertificate";

    Utf8String certToken = "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";

    // order of these calls should not matter, this doesn't check that each url is called
    GetMockHttpHandler().ExpectRequests(2);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        if (request.GetUrl() == policyUrl)
            return MockHttpHandler::StubHttpFailureResponse();
        else if (request.GetUrl() == certUrl)
            return MockHttpHandler::StubHttpResponse(certToken);
        else
            return MockHttpHandler::StubHttpResponse();
        });
    GetMockHttpHandler().ForRequest(2, [=](Http::RequestCR request)
        {
        if (request.GetUrl() == policyUrl)
            return MockHttpHandler::StubHttpFailureResponse();
        else if (request.GetUrl() == certUrl)
            return MockHttpHandler::StubHttpResponse(certToken);
        else
            return MockHttpHandler::StubHttpResponse();
        });

    try
        {
        const auto result = GetPolicyProvider().GetPolicy().get();
        FAIL() << "Expected an execption to be thrown";
        }
    catch (HttpError error)
        {
        // expect connection and http status to be the same as StubHttpFailureResponse
        EXPECT_EQ(error.GetConnectionStatus(), MockHttpHandler::StubHttpFailureResponse().GetConnectionStatus());
        EXPECT_EQ(error.GetHttpStatus(), MockHttpHandler::StubHttpFailureResponse().GetHttpStatus());
        }
    catch (...)
        {
        FAIL() << "Expected exception to be an HttpError";
        }
    }

// can't tell which order folly::collectAll will call (seems to be different linux vs windows)
TEST_F(PolicyProviderTests, GetPolicy_GetCertificateFailure)
    {
    // assert that we we fall into catch on failure getting the cert
    const auto mockUrl = MockEntitlementUrl();
    Utf8String policyUrl = mockUrl + "/GetPolicy";
    Utf8String certUrl = mockUrl + "/PolicyTokenSigningCertificate";

    Utf8String policyToken = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsIng1dCI6ImVSUWFLMkhfZVVha2R5WjFxWVpDd1lmb0VlUSJ9.eyJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcy9wb2xpY3kiOiJleUpRYjJ4cFkzbEpaQ0k2SWpsa1ptSTBObU0yTFdKa00yRXROR1ZsWmkwNU5qaGtMV0prWkRkbE5XSXpaVEUzWVNJc0lsQnZiR2xqZVZabGNuTnBiMjRpT2pFdU1Dd2lVRzlzYVdONVEzSmxZWFJsWkU5dUlqb2lNakF4T1Mwd015MHhOVlF4T1Rvd05EbzBNaTQwTVRneE1ESTNXaUlzSWxCdmJHbGplVVY0Y0dseVpYTlBiaUk2SWpJd01Ua3RNRE10TURaVU1qRTZNVFE2TlRndU9EWTJXaUlzSWxKbGNYVmxjM1JFWVhSaElqcDdJazFoWTJocGJtVlRTVVFpT201MWJHd3NJa0ZqWTJWemMwdGxlU0k2Ym5Wc2JDd2lWWE5sY2tsa0lqb2lZMkV4WTJNMlkyRXRNbUZtTVMwMFpXWmtMVGc0TnpZdFptUTFPVEV3WVROaE4yWmhJaXdpUTJobFkydGxaRTkxZEVSaGRHVWlPbTUxYkd3c0lsSmxjWFZsYzNSbFpGTmxZM1Z5WVdKc1pYTWlPbHQ3SWxCeWIyUjFZM1JKWkNJNk1qVTBOU3dpUm1WaGRIVnlaVk4wY21sdVp5STZJaUlzSWxabGNuTnBiMjRpT2lJeExqQXVNQzR3SW4xZExDSk5ZV05vYVc1bFRtRnRaU0k2SWxSbGMzUkVaWFpwWTJWSlpDSXNJa05zYVdWdWRFUmhkR1ZVYVcxbElqb2lNakF4T1Mwd015MHhOVlF4T1Rvd05EbzBNQzR4TmpOYUlpd2lURzlqWVd4bElqb2laVzRpTENKQmNIQnNhV1Z6Vkc4aU9pSm9kSFJ3Y3pvdkwyVnVkR2wwYkdWdFpXNTBMWE5sWVhKamFDNWlaVzUwYkdWNUxtTnZiUzhpZlN3aVRXRmphR2x1WlZOcFoyNWhkSFZ5WlNJNkltNTNTMEp5WmpSc2JHUjZWblJhU1U5emRHNWFiVGxIT1VnM1l6MGlMQ0pCY0hCc2FXVnpWRzlWYzJWeVNXUWlPaUpqWVRGall6WmpZUzB5WVdZeExUUmxabVF0T0RnM05pMW1aRFU1TVRCaE0yRTNabUVpTENKQmNIQnNhV1Z6Vkc5VFpXTjFjbUZpYkdWSlpITWlPbHNpT0RRMFl6RXdZekl0TXpjMVpDMDBNVFV6TFdFd09HWXRPRGsyWkRKbE5qUmhNVE5tSWwwc0lrRkRUSE1pT2x0N0lsQnlhVzVqYVhCaGJFbGtJam9pWTJFeFkyTTJZMkV0TW1GbU1TMDBaV1prTFRnNE56WXRabVExT1RFd1lUTmhOMlpoSWl3aVUyVmpkWEpoWW14bFNXUWlPaUk0TkRSak1UQmpNaTB6TnpWa0xUUXhOVE10WVRBNFppMDRPVFprTW1VMk5HRXhNMllpTENKQlkyTmxjM05MYVc1a0lqbzBMQ0pGZUhCcGNtVnpUMjRpT2lJeU1ERTVMVEF6TFRBMlZESXhPakUwT2pVNExqZzJObG9pTENKUmRXRnNhV1pwWlhKUGRtVnljbWxrWlhNaU9sdDdJazVoYldVaU9pSlZjMkZuWlZSNWNHVWlMQ0pXWVd4MVpTSTZJbFJ5YVdGc0lpd2lWSGx3WlNJNkluTjBjbWx1WnlJc0lsQnliMjF3ZENJNklsUnlhV0ZzSUVWdWRHbDBiR1Z0Wlc1MEluMWRmVjBzSWxObFkzVnlZV0pzWlVSaGRHRWlPbHQ3SWxObFkzVnlZV0pzWlVsa0lqb2lPRFEwWXpFd1l6SXRNemMxWkMwME1UVXpMV0V3T0dZdE9EazJaREpsTmpSaE1UTm1JaXdpVUhKdlpIVmpkRWxrSWpveU5UUTFMQ0pRY205a2RXTjBUbUZ0WlNJNklrSmxiblJzWlhrZ1RtRjJhV2RoZEc5eUlpd2lSbVZoZEhWeVpWTjBjbWx1WnlJNklpSXNJbFpsY25OcGIyNGlPbTUxYkd3c0lsRjFZV3hwWm1sbGNrOTJaWEp5YVdSbGN5STZiblZzYkgxZExDSlZjMlZ5UkdGMFlTSTZleUpWYzJWeVNXUWlPaUpqWVRGall6WmpZUzB5WVdZeExUUmxabVF0T0RnM05pMW1aRFU1TVRCaE0yRTNabUVpTENKUGNtZGhibWw2WVhScGIyNUpaQ0k2SW1Fd1pHTTFZVE0xTFRkaE9ERXRORGRoTlMwNU16Z3dMVFZtTlRSaVpqQTBNR00wTXlJc0lsVnpZV2RsUTI5MWJuUnllVWxUVHlJNklsVlRJaXdpVld4MGFXMWhkR1ZUUVZCSlpDSTZJakV3TURVMk56TTJNRElpTENKVmJIUnBiV0YwWlVsa0lqb2lZVEkyWm1Sa01Ea3RNRGs0TkMwME1HTmlMV0V4T1RRdFpEbGhPREpoTXpVeE9UVTNJaXdpVld4MGFXMWhkR1ZEYjNWdWRISjVTV1FpT2lKallURmpZelpqWVMweVlXWXhMVFJsWm1RdE9EZzNOaTFtWkRVNU1UQmhNMkUzWm1FaUxDSkZiblJwZEd4bGJXVnVkRWR5YjNWd1NXUWlPaUl3TURBd01EQXdNQzB3TURBd0xUQXdNREF0TURBd01DMHdNREF3TURBd01EQXdNREFpZlN3aVRXVjBZVVJoZEdFaU9tNTFiR3dzSWtSbFptRjFiSFJSZFdGc2FXWnBaWEp6SWpwYmV5Sk9ZVzFsSWpvaVJuSmxjWFZsYm1ONVZHOVRaVzVrSWl3aVZtRnNkV1VpT2lJeU5DSXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJbFZ6WlVGc1pYSjBhVzVuVTJWeWRtbGpaU0lzSWxaaGJIVmxJam9pUkdsellXSnNaV1FpTENKVWVYQmxJam9pWlc1MWJTSXNJbEJ5YjIxd2RDSTZJbGx2ZFNCb1lYWmxJSEpsWVdOb1pXUWdZU0JzYVdObGJuTmxJSFJvY21WemFHOXNaQ0JrWldacGJtVmtJR0o1SUhsdmRYSWdiM0puWVc1cGVtRjBhVzl1SjNNZ1lXUnRhVzVwYzNSeVlYUnZjaTRnUTI5dWRHbHVkV2x1WnlCdFlYa2djbVZ6ZFd4MElHbHVJR0ZrWkdsMGFXOXVZV3dnWVhCd2JHbGpZWFJwYjI0Z2RYTmhaMlVnWTJoaGNtZGxjeTVjY2x4dVZHOGdjSEp2WTJWbFpDd2dlVzkxSUcxMWMzUWdYQ0pCWTJ0dWIzZHNaV1JuWlZ3aUxpSjlMSHNpVG1GdFpTSTZJa2x6UTJobFkydGxaRTkxZENJc0lsWmhiSFZsSWpvaVptRnNjMlVpTENKVWVYQmxJam9pWW05dmJDSXNJbEJ5YjIxd2RDSTZiblZzYkgwc2V5Sk9ZVzFsSWpvaVFXeHNiM2RQWm1ac2FXNWxWWE5oWjJVaUxDSldZV3gxWlNJNklsUlNWVVVpTENKVWVYQmxJam9pWW05dmJDSXNJbEJ5YjIxd2RDSTZJbGx2ZFNCdGRYTjBJRk5wWjI0Z1NXNGdkRzhnZFhObElIUm9hWE1nWVhCd2JHbGpZWFJwYjI0dUluMHNleUpPWVcxbElqb2lTRzkwY0dGMGFGSmxkSEo1UVhSMFpXMXdkSE1pTENKV1lXeDFaU0k2SWpRaUxDSlVlWEJsSWpvaWFXNTBJaXdpVUhKdmJYQjBJanB1ZFd4c2ZTeDdJazVoYldVaU9pSk5ZWGhNYjJkR2FXeGxSSFZ5WVhScGIyNUpiazFwYm5WMFpYTWlMQ0pXWVd4MVpTSTZJak0yTUNJc0lsUjVjR1VpT2lKcGJuUWlMQ0pRY205dGNIUWlPbTUxYkd4OUxIc2lUbUZ0WlNJNklrOW1abXhwYm1WRWRYSmhkR2x2YmlJc0lsWmhiSFZsSWpvaU55SXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJbFJwYldWVWIwdGxaWEJWYmxObGJuUk1iMmR6SWl3aVZtRnNkV1VpT2lJMk1DSXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJbFZ6WVdkbFZIbHdaU0lzSWxaaGJIVmxJam9pVUhKdlpIVmpkR2x2YmlJc0lsUjVjR1VpT2lKemRISnBibWNpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJa3h2WjFKbFkyOXlaSE5VYjFObGJtUkdWRElpTENKV1lXeDFaU0k2SWpjMU1EQXdJaXdpVkhsd1pTSTZJbWx1ZENJc0lsQnliMjF3ZENJNmJuVnNiSDBzZXlKT1lXMWxJam9pVUc5c2FXTjVTVzUwWlhKMllXd2lMQ0pXWVd4MVpTSTZJakkwTUNJc0lsUjVjR1VpT2lKcGJuUWlMQ0pRY205dGNIUWlPbTUxYkd4OUxIc2lUbUZ0WlNJNklreHZaMmx1UjNKaFkyVlFaWEpwYjJRaUxDSldZV3gxWlNJNklqY2lMQ0pVZVhCbElqb2lhVzUwSWl3aVVISnZiWEIwSWpwdWRXeHNmU3g3SWs1aGJXVWlPaUpJWldGeWRHSmxZWFJKYm5SbGNuWmhiQ0lzSWxaaGJIVmxJam9pTVNJc0lsUjVjR1VpT2lKcGJuUWlMQ0pRY205dGNIUWlPbTUxYkd4OUxIc2lUbUZ0WlNJNklsWmxjbk5wYjI0aUxDSldZV3gxWlNJNklqQXVNQzR3TGpBaUxDSlVlWEJsSWpvaWMzUnlhVzVuSWl3aVVISnZiWEIwSWpwdWRXeHNmVjE5IiwiaXNzIjoiaHR0cDovL3BvbGljeXNlcnZpY2UuYmVudGxleS5jb20iLCJhdWQiOiJodHRwczovL2VudGl0bGVtZW50LXNlYXJjaC5iZW50bGV5LmNvbS8iLCJleHAiOjE1NTMyODE0ODMsIm5iZiI6MTU1MjY3NjY4M30.NEdSIUWarGPv_sNvrvWj4WunKggA68Q_PQ7n2kBjoefpgs9bhSqzi9dzTjP5i2pBP0WHU-tTsRyEUiLI51Iz4P6LsBldfiFkF_Y4FFSmArR8LLqZpWgrruMAI7R9uwWYJyVnuVWhbcsZROVhOUTW4so9U8EPW_yGqj9vwlKjLy5PcjsV27_OupbWUSGE5eNzM7SpnXckoD-VM6DE0Yfe9p176sgXGukppqYwkNIZso9RUFvp6KGx_VYGi8i7L-tuf5jA78ENLZLL31_jGKaqNL8ZqHy3eA_7PzyhtlqrgQWVNSMxq-jmRv3U7EbgVRI9PeJAVLJvXqbPJGLt2ZiHSg";

    // order of these calls should not matter, this doesn't check that each url is called
    GetMockHttpHandler().ExpectRequests(2);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        if (request.GetUrl() == policyUrl)
            return MockHttpHandler::StubHttpResponse(policyToken);
        else if (request.GetUrl() == certUrl)
            return MockHttpHandler::StubHttpFailureResponse();
        else
            return MockHttpHandler::StubHttpResponse();
        });
    GetMockHttpHandler().ForRequest(2, [=](Http::RequestCR request)
        {
        if (request.GetUrl() == policyUrl)
            return MockHttpHandler::StubHttpResponse(policyToken);
        else if (request.GetUrl() == certUrl)
            return MockHttpHandler::StubHttpFailureResponse();
        else
            return MockHttpHandler::StubHttpResponse();
        });

    try
        {
        const auto result = GetPolicyProvider().GetPolicy().get();
        FAIL() << "Expected an execption to be thrown";
        }
    catch (HttpError error)
        {
        // expect connection and http status to be the same as StubHttpFailureResponse
        EXPECT_EQ(error.GetConnectionStatus(), MockHttpHandler::StubHttpFailureResponse().GetConnectionStatus());
        EXPECT_EQ(error.GetHttpStatus(), MockHttpHandler::StubHttpFailureResponse().GetHttpStatus());
        }
    catch (...)
        {
        FAIL() << "Expected exception to be an HttpError";
        }
    }

// can't tell which order folly::collectAll will call (seems to be different linux vs windows)
TEST_F(PolicyProviderTests, GetPolicyWithKey_Success)
    {
    // assert our outgoing request matches expectation
    const auto mockUrl = MockEntitlementUrl();
    Utf8String policyUrl = mockUrl + "/GetPolicyWithAccessKey";
    Utf8String certUrl = mockUrl + "/PolicyTokenSigningCertificate";

    Utf8String certToken = "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";
    Utf8String policyToken = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsIng1dCI6ImVSUWFLMkhfZVVha2R5WjFxWVpDd1lmb0VlUSJ9.eyJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcy9wb2xpY3kiOiJleUpRYjJ4cFkzbEpaQ0k2SWpsa1ptSTBObU0yTFdKa00yRXROR1ZsWmkwNU5qaGtMV0prWkRkbE5XSXpaVEUzWVNJc0lsQnZiR2xqZVZabGNuTnBiMjRpT2pFdU1Dd2lVRzlzYVdONVEzSmxZWFJsWkU5dUlqb2lNakF4T1Mwd015MHhOVlF4T1Rvd05EbzBNaTQwTVRneE1ESTNXaUlzSWxCdmJHbGplVVY0Y0dseVpYTlBiaUk2SWpJd01Ua3RNRE10TURaVU1qRTZNVFE2TlRndU9EWTJXaUlzSWxKbGNYVmxjM1JFWVhSaElqcDdJazFoWTJocGJtVlRTVVFpT201MWJHd3NJa0ZqWTJWemMwdGxlU0k2Ym5Wc2JDd2lWWE5sY2tsa0lqb2lZMkV4WTJNMlkyRXRNbUZtTVMwMFpXWmtMVGc0TnpZdFptUTFPVEV3WVROaE4yWmhJaXdpUTJobFkydGxaRTkxZEVSaGRHVWlPbTUxYkd3c0lsSmxjWFZsYzNSbFpGTmxZM1Z5WVdKc1pYTWlPbHQ3SWxCeWIyUjFZM1JKWkNJNk1qVTBOU3dpUm1WaGRIVnlaVk4wY21sdVp5STZJaUlzSWxabGNuTnBiMjRpT2lJeExqQXVNQzR3SW4xZExDSk5ZV05vYVc1bFRtRnRaU0k2SWxSbGMzUkVaWFpwWTJWSlpDSXNJa05zYVdWdWRFUmhkR1ZVYVcxbElqb2lNakF4T1Mwd015MHhOVlF4T1Rvd05EbzBNQzR4TmpOYUlpd2lURzlqWVd4bElqb2laVzRpTENKQmNIQnNhV1Z6Vkc4aU9pSm9kSFJ3Y3pvdkwyVnVkR2wwYkdWdFpXNTBMWE5sWVhKamFDNWlaVzUwYkdWNUxtTnZiUzhpZlN3aVRXRmphR2x1WlZOcFoyNWhkSFZ5WlNJNkltNTNTMEp5WmpSc2JHUjZWblJhU1U5emRHNWFiVGxIT1VnM1l6MGlMQ0pCY0hCc2FXVnpWRzlWYzJWeVNXUWlPaUpqWVRGall6WmpZUzB5WVdZeExUUmxabVF0T0RnM05pMW1aRFU1TVRCaE0yRTNabUVpTENKQmNIQnNhV1Z6Vkc5VFpXTjFjbUZpYkdWSlpITWlPbHNpT0RRMFl6RXdZekl0TXpjMVpDMDBNVFV6TFdFd09HWXRPRGsyWkRKbE5qUmhNVE5tSWwwc0lrRkRUSE1pT2x0N0lsQnlhVzVqYVhCaGJFbGtJam9pWTJFeFkyTTJZMkV0TW1GbU1TMDBaV1prTFRnNE56WXRabVExT1RFd1lUTmhOMlpoSWl3aVUyVmpkWEpoWW14bFNXUWlPaUk0TkRSak1UQmpNaTB6TnpWa0xUUXhOVE10WVRBNFppMDRPVFprTW1VMk5HRXhNMllpTENKQlkyTmxjM05MYVc1a0lqbzBMQ0pGZUhCcGNtVnpUMjRpT2lJeU1ERTVMVEF6TFRBMlZESXhPakUwT2pVNExqZzJObG9pTENKUmRXRnNhV1pwWlhKUGRtVnljbWxrWlhNaU9sdDdJazVoYldVaU9pSlZjMkZuWlZSNWNHVWlMQ0pXWVd4MVpTSTZJbFJ5YVdGc0lpd2lWSGx3WlNJNkluTjBjbWx1WnlJc0lsQnliMjF3ZENJNklsUnlhV0ZzSUVWdWRHbDBiR1Z0Wlc1MEluMWRmVjBzSWxObFkzVnlZV0pzWlVSaGRHRWlPbHQ3SWxObFkzVnlZV0pzWlVsa0lqb2lPRFEwWXpFd1l6SXRNemMxWkMwME1UVXpMV0V3T0dZdE9EazJaREpsTmpSaE1UTm1JaXdpVUhKdlpIVmpkRWxrSWpveU5UUTFMQ0pRY205a2RXTjBUbUZ0WlNJNklrSmxiblJzWlhrZ1RtRjJhV2RoZEc5eUlpd2lSbVZoZEhWeVpWTjBjbWx1WnlJNklpSXNJbFpsY25OcGIyNGlPbTUxYkd3c0lsRjFZV3hwWm1sbGNrOTJaWEp5YVdSbGN5STZiblZzYkgxZExDSlZjMlZ5UkdGMFlTSTZleUpWYzJWeVNXUWlPaUpqWVRGall6WmpZUzB5WVdZeExUUmxabVF0T0RnM05pMW1aRFU1TVRCaE0yRTNabUVpTENKUGNtZGhibWw2WVhScGIyNUpaQ0k2SW1Fd1pHTTFZVE0xTFRkaE9ERXRORGRoTlMwNU16Z3dMVFZtTlRSaVpqQTBNR00wTXlJc0lsVnpZV2RsUTI5MWJuUnllVWxUVHlJNklsVlRJaXdpVld4MGFXMWhkR1ZUUVZCSlpDSTZJakV3TURVMk56TTJNRElpTENKVmJIUnBiV0YwWlVsa0lqb2lZVEkyWm1Sa01Ea3RNRGs0TkMwME1HTmlMV0V4T1RRdFpEbGhPREpoTXpVeE9UVTNJaXdpVld4MGFXMWhkR1ZEYjNWdWRISjVTV1FpT2lKallURmpZelpqWVMweVlXWXhMVFJsWm1RdE9EZzNOaTFtWkRVNU1UQmhNMkUzWm1FaUxDSkZiblJwZEd4bGJXVnVkRWR5YjNWd1NXUWlPaUl3TURBd01EQXdNQzB3TURBd0xUQXdNREF0TURBd01DMHdNREF3TURBd01EQXdNREFpZlN3aVRXVjBZVVJoZEdFaU9tNTFiR3dzSWtSbFptRjFiSFJSZFdGc2FXWnBaWEp6SWpwYmV5Sk9ZVzFsSWpvaVJuSmxjWFZsYm1ONVZHOVRaVzVrSWl3aVZtRnNkV1VpT2lJeU5DSXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJbFZ6WlVGc1pYSjBhVzVuVTJWeWRtbGpaU0lzSWxaaGJIVmxJam9pUkdsellXSnNaV1FpTENKVWVYQmxJam9pWlc1MWJTSXNJbEJ5YjIxd2RDSTZJbGx2ZFNCb1lYWmxJSEpsWVdOb1pXUWdZU0JzYVdObGJuTmxJSFJvY21WemFHOXNaQ0JrWldacGJtVmtJR0o1SUhsdmRYSWdiM0puWVc1cGVtRjBhVzl1SjNNZ1lXUnRhVzVwYzNSeVlYUnZjaTRnUTI5dWRHbHVkV2x1WnlCdFlYa2djbVZ6ZFd4MElHbHVJR0ZrWkdsMGFXOXVZV3dnWVhCd2JHbGpZWFJwYjI0Z2RYTmhaMlVnWTJoaGNtZGxjeTVjY2x4dVZHOGdjSEp2WTJWbFpDd2dlVzkxSUcxMWMzUWdYQ0pCWTJ0dWIzZHNaV1JuWlZ3aUxpSjlMSHNpVG1GdFpTSTZJa2x6UTJobFkydGxaRTkxZENJc0lsWmhiSFZsSWpvaVptRnNjMlVpTENKVWVYQmxJam9pWW05dmJDSXNJbEJ5YjIxd2RDSTZiblZzYkgwc2V5Sk9ZVzFsSWpvaVFXeHNiM2RQWm1ac2FXNWxWWE5oWjJVaUxDSldZV3gxWlNJNklsUlNWVVVpTENKVWVYQmxJam9pWW05dmJDSXNJbEJ5YjIxd2RDSTZJbGx2ZFNCdGRYTjBJRk5wWjI0Z1NXNGdkRzhnZFhObElIUm9hWE1nWVhCd2JHbGpZWFJwYjI0dUluMHNleUpPWVcxbElqb2lTRzkwY0dGMGFGSmxkSEo1UVhSMFpXMXdkSE1pTENKV1lXeDFaU0k2SWpRaUxDSlVlWEJsSWpvaWFXNTBJaXdpVUhKdmJYQjBJanB1ZFd4c2ZTeDdJazVoYldVaU9pSk5ZWGhNYjJkR2FXeGxSSFZ5WVhScGIyNUpiazFwYm5WMFpYTWlMQ0pXWVd4MVpTSTZJak0yTUNJc0lsUjVjR1VpT2lKcGJuUWlMQ0pRY205dGNIUWlPbTUxYkd4OUxIc2lUbUZ0WlNJNklrOW1abXhwYm1WRWRYSmhkR2x2YmlJc0lsWmhiSFZsSWpvaU55SXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJbFJwYldWVWIwdGxaWEJWYmxObGJuUk1iMmR6SWl3aVZtRnNkV1VpT2lJMk1DSXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJbFZ6WVdkbFZIbHdaU0lzSWxaaGJIVmxJam9pVUhKdlpIVmpkR2x2YmlJc0lsUjVjR1VpT2lKemRISnBibWNpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJa3h2WjFKbFkyOXlaSE5VYjFObGJtUkdWRElpTENKV1lXeDFaU0k2SWpjMU1EQXdJaXdpVkhsd1pTSTZJbWx1ZENJc0lsQnliMjF3ZENJNmJuVnNiSDBzZXlKT1lXMWxJam9pVUc5c2FXTjVTVzUwWlhKMllXd2lMQ0pXWVd4MVpTSTZJakkwTUNJc0lsUjVjR1VpT2lKcGJuUWlMQ0pRY205dGNIUWlPbTUxYkd4OUxIc2lUbUZ0WlNJNklreHZaMmx1UjNKaFkyVlFaWEpwYjJRaUxDSldZV3gxWlNJNklqY2lMQ0pVZVhCbElqb2lhVzUwSWl3aVVISnZiWEIwSWpwdWRXeHNmU3g3SWs1aGJXVWlPaUpJWldGeWRHSmxZWFJKYm5SbGNuWmhiQ0lzSWxaaGJIVmxJam9pTVNJc0lsUjVjR1VpT2lKcGJuUWlMQ0pRY205dGNIUWlPbTUxYkd4OUxIc2lUbUZ0WlNJNklsWmxjbk5wYjI0aUxDSldZV3gxWlNJNklqQXVNQzR3TGpBaUxDSlVlWEJsSWpvaWMzUnlhVzVuSWl3aVVISnZiWEIwSWpwdWRXeHNmVjE5IiwiaXNzIjoiaHR0cDovL3BvbGljeXNlcnZpY2UuYmVudGxleS5jb20iLCJhdWQiOiJodHRwczovL2VudGl0bGVtZW50LXNlYXJjaC5iZW50bGV5LmNvbS8iLCJleHAiOjE1NTMyODE0ODMsIm5iZiI6MTU1MjY3NjY4M30.NEdSIUWarGPv_sNvrvWj4WunKggA68Q_PQ7n2kBjoefpgs9bhSqzi9dzTjP5i2pBP0WHU-tTsRyEUiLI51Iz4P6LsBldfiFkF_Y4FFSmArR8LLqZpWgrruMAI7R9uwWYJyVnuVWhbcsZROVhOUTW4so9U8EPW_yGqj9vwlKjLy5PcjsV27_OupbWUSGE5eNzM7SpnXckoD-VM6DE0Yfe9p176sgXGukppqYwkNIZso9RUFvp6KGx_VYGi8i7L-tuf5jA78ENLZLL31_jGKaqNL8ZqHy3eA_7PzyhtlqrgQWVNSMxq-jmRv3U7EbgVRI9PeJAVLJvXqbPJGLt2ZiHSg";

    Utf8String accessKey = "TestAccessKey";

    std::shared_ptr<Policy> expectedPolicy = Policy::Create(JWToken::Create(policyToken, certToken));

    // order of these calls should not matter, this doesn't check that each url is called
    GetMockHttpHandler().ExpectRequests(2);
    GetMockHttpHandler().ForRequest(1, [=](Http::RequestCR request)
        {
        if (request.GetUrl() == policyUrl)
            return MockHttpHandler::StubHttpResponse(policyToken);
        else if (request.GetUrl() == certUrl)
            return MockHttpHandler::StubHttpResponse(certToken);
        else
            return MockHttpHandler::StubHttpFailureResponse();
        });
    GetMockHttpHandler().ForRequest(2, [=](Http::RequestCR request)
        {
        if (request.GetUrl() == policyUrl)
            return MockHttpHandler::StubHttpResponse(policyToken);
        else if (request.GetUrl() == certUrl)
            return MockHttpHandler::StubHttpResponse(certToken);
        else
            return MockHttpHandler::StubHttpFailureResponse();
        });

    // assert that we return a well formed policy
    const auto result = GetPolicyProvider().GetPolicyWithKey(accessKey, "").get();
    auto resultJson = result->GetJson();
    auto expectedJson = expectedPolicy->GetJson();
    EXPECT_EQ(expectedJson, resultJson);
    }
