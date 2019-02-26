/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Connect/ImsClientTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServicesTestsHelper.h>

#include <WebServices/Connect/ImsClient.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <curl/curl.h>

struct ImsClientTests : public WSClientBaseTest {};

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, RequestToken_ProdUrls_RetrievesToken)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto client = ImsClient::Create(StubValidClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = client->RequestToken(credentials, ImsClient::GetLegacyRelyingPartyUri())->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    auto tokenAuthStr = token->ToAuthorizationString();
    BeDebugLog(tokenAuthStr.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, RequestToken_QaUrls_RetrievesToken)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto client = ImsClient::Create(StubValidClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = client->RequestToken(credentials, ImsClient::GetLegacyRelyingPartyUri())->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    auto tokenAuthStr = token->ToAuthorizationString();
    BeDebugLog(tokenAuthStr.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, RequestToken_NoLifetimeSpecified_RetrievesTokenWithDefaultLifetime)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto client = ImsClient::Create(StubValidClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = client->RequestToken(credentials, nullptr, 0)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    EXPECT_TRUE(token->IsSupported());
    EXPECT_TRUE(token->IsValidNow(2));
    EXPECT_NE(0, token->GetLifetime());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, RequestToken_LifetimeSpecified_RetrievesTokenWithSpecifiedLifetime)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto client = ImsClient::Create(StubValidClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = client->RequestToken(credentials, nullptr, 5)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    EXPECT_TRUE(token->IsSupported());
    EXPECT_TRUE(token->IsValidNow(2));
    EXPECT_EQ(5, token->GetLifetime());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, RequestToken_UsingParentTokenAndLifetimeSpecified_RetrievesTokenWithSpecifiedLifetime)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto client = ImsClient::Create(StubValidClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    auto parentTokenResult = client->RequestToken(credentials)->GetResult();
    ASSERT_TRUE(parentTokenResult.IsSuccess());

    auto result = client->RequestToken(*parentTokenResult.GetValue(), nullptr, 5)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    EXPECT_TRUE(token->IsSupported());
    EXPECT_TRUE(token->IsValidNow(2));
    EXPECT_EQ(5, token->GetLifetime());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/    
TEST_F(ImsClientTests, Login_QaImsStsWithOldAppliesTo_RetrievesValidTokensForValidRPUris)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, BentleyApi::NativeLogging::LOG_TRACE);

    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto client = ImsClient::Create(StubClientInfo(), proxy);

    SamlTokenResult result;

    // Legacy Graphite apps
    result = client->RequestToken(credentials, "https://dev-wsg20-eus.cloudapp.net")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    result = client->RequestToken(credentials, "https://qa-wsg20-eus.cloudapp.net")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    result = client->RequestToken(credentials, "https://connect-wsg20.bentley.com")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    // Navigator CONNECT Edition
    result = client->RequestToken(credentials, "sso://wsfed_desktop/2545")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    // Navigator Mobile
    result = client->RequestToken(credentials, "sso://wsfed_mobile/2223")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    // ProjectWise Mobile
    result = client->RequestToken(credentials, "sso://wsfed_mobile/2530")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    // ProjectWise Worksite
    result = client->RequestToken(credentials, "sso://wsfed_mobile/2226")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    // Structural navigator
    result = client->RequestToken(credentials, "sso://wsfed_mobile/2067")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    // Invalid RP URI
    BeTest::SetFailOnAssert(false);
    result = client->RequestToken(credentials, "https://zz-wsg20-eus.cloudapp.net")->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Tomas.Tamasauskas                       08/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, GetA2PUrl)
    {
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, BentleyApi::NativeLogging::LOG_TRACE);

    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto client = ImsClient::Create(StubClientInfo(), proxy);

    SamlTokenResult result;

    result = client->RequestToken(credentials, "https://connect-wsg20.bentley.com")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    SamlTokenPtr token = result.GetValue();

    //crafting the url should succeed for connect
    auto url = client->GetA2PUrl("https://qa-connect-webportal.bentley.com/", token, BeGuid(true).ToString());
    ASSERT_NE("https://qa-connect-webportal.bentley.com/", url);

    //crafting the url should fail for non valid url - thus returning the original url
    BeTest::SetFailOnAssert(false);
    url = client->GetA2PUrl("https://thisshouldfail.bentley.com/", token, BeGuid(true).ToString());
    ASSERT_EQ("https://thisshouldfail.bentley.com/", url);
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/  
TEST_F(ImsClientTests, Login_DevImsAppliesToTest_RetrievesValidToken)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, NativeLogging::LOG_TRACE);

    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Dev, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto client = ImsClient::Create(StubClientInfo(), proxy);

    SamlTokenResult result;

    // Tests
    result = client->RequestToken(credentials, "sso://wsfed_desktop/test")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    result = client->RequestToken(credentials, "sso://wsfed_mobile/test")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/  
TEST_F(ImsClientTests, Login_QaImsAppliesToTest_RetrievesValidToken)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, NativeLogging::LOG_TRACE);

    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto client = ImsClient::Create(StubClientInfo(), proxy);

    SamlTokenResult result;

    // Tests
    result = client->RequestToken(credentials, "sso://wsfed_desktop/test")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    result = client->RequestToken(credentials, "sso://wsfed_mobile/test")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/  
TEST_F(ImsClientTests, Login_ReleaseImsAppliesToTest_RetrievesValidToken)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();

    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, NativeLogging::LOG_TRACE);

    RuntimeJsonLocalState localState;
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &localState, nullptr, proxy);

    auto client = ImsClient::Create(StubClientInfo(), proxy);

    SamlTokenResult result;

    // Tests
    result = client->RequestToken(credentials, "sso://wsfed_desktop/test")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());

    result = client->RequestToken(credentials, "sso://wsfed_mobile/test")->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue()->IsSupported());
    }
