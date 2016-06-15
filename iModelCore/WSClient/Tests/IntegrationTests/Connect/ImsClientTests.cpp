/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Connect/ImsClientTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ImsClientTests.h"

#include <WebServices/Connect/ImsClient.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <curl/curl.h>

TEST_F(ImsClientTests, RequestToken_ProdUrls_RetrievesToken)
    {
    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto client = ImsClient::Create(StubValidClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = client->RequestToken(credentials, ImsClient::GetLegacyRelyingPartyUri())->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    auto tokenAuthStr = token->ToAuthorizationString();
    BeDebugLog(tokenAuthStr.c_str());
    }

TEST_F(ImsClientTests, RequestToken_QaUrls_RetrievesToken)
    {
    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto client = ImsClient::Create(StubValidClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = client->RequestToken(credentials, ImsClient::GetLegacyRelyingPartyUri())->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    auto tokenAuthStr = token->ToAuthorizationString();
    BeDebugLog(tokenAuthStr.c_str());
    }

TEST_F(ImsClientTests, RequestToken_NoLifetimeSpecified_RetrievesTokenWithDefaultLifetime)
    {
    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto client = ImsClient::Create(StubValidClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = client->RequestToken(credentials, nullptr, 0)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    EXPECT_TRUE(token->IsSupported());
    EXPECT_TRUE(token->IsValidNow(2));
    EXPECT_NE(0, token->GetLifetime());
    }

TEST_F(ImsClientTests, RequestToken_LifetimeSpecified_RetrievesTokenWithSpecifiedLifetime)
    {
    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto client = ImsClient::Create(StubValidClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = client->RequestToken(credentials, nullptr, 5)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    EXPECT_TRUE(token->IsSupported());
    EXPECT_TRUE(token->IsValidNow(2));
    EXPECT_EQ(5, token->GetLifetime());
    }

TEST_F(ImsClientTests, RequestToken_UsingParentTokenAndLifetimeSpecified_RetrievesTokenWithSpecifiedLifetime)
    {
    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
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
    
TEST_F(ImsClientTests, Login_QaImsStsWithOldAppliesTo_RetrievesValidTokensForValidRPUris)
    {
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_DGNCLIENTFX_UTILS_HTTP, BentleyApi::NativeLogging::LOG_TRACE);

    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto client = ImsClient::Create (StubClientInfo(), proxy);

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
    result = client->RequestToken(credentials, "sso://wsfed_desktop/1654")->GetResult();
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
