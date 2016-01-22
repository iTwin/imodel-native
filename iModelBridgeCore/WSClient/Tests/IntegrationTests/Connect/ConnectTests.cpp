/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Connect/ConnectTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ConnectTests.h"

#include <WebServices/Connect/Connect.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <DgnClientFx/Utils/Http/ProxyHttpHandler.h>
#include <curl/curl.h>

TEST_F(ConnectTests, GetStsToken_ProdUrls_RetrievesToken)
    {
    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    Connect::Initialize(StubClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    SamlToken token;

    EXPECT_EQ(0, Connect::Login(credentials, token));
    EXPECT_TRUE(token.IsSupported());

    auto tokenAuthStr = token.ToAuthorizationString();
    BeDebugLog(tokenAuthStr.c_str());
    }

TEST_F(ConnectTests, Login_QaUrls_RetrievesToken)
    {
    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    Connect::Initialize(StubClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");
    SamlToken token;

    EXPECT_EQ(0, Connect::Login(credentials, token));
    EXPECT_TRUE(token.IsSupported());

    auto tokenAuthStr = token.ToAuthorizationString();
    BeDebugLog(tokenAuthStr.c_str());
    }

TEST_F(ConnectTests, Login_QaImsStsWithOldAppliesTo_RetrievesValidTokensForValidRPUris)
    {
    Credentials credentials("bentleyvilnius@gmail.com", "Q!w2e3r4t5");

    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_DGNCLIENTFX_UTILS_HTTP, BentleyApi::NativeLogging::LOG_TRACE);

    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    Connect::Initialize(StubClientInfo(), proxy);

    SamlToken token;
    Utf8CP stsUrl = "https://qa-ims.bentley.com/rest/ActiveSTSService/json/IssueEx";

    // Legacy Graphite apps
    token = SamlToken();
    EXPECT_EQ(SUCCESS, Connect::Login(credentials, token, "https://dev-wsg20-eus.cloudapp.net", stsUrl));
    EXPECT_TRUE(token.IsSupported());

    token = SamlToken();
    EXPECT_EQ(SUCCESS, Connect::Login(credentials, token, "https://qa-wsg20-eus.cloudapp.net", stsUrl));
    EXPECT_TRUE(token.IsSupported());

    token = SamlToken();
    EXPECT_EQ(SUCCESS, Connect::Login(credentials, token, "https://connect-wsg20.bentley.com", stsUrl));
    EXPECT_TRUE(token.IsSupported());

    // Navigator CONNECT Edition
    token = SamlToken();
    EXPECT_EQ(SUCCESS, Connect::Login(credentials, token, "sso://wsfed_desktop/1654", stsUrl));
    EXPECT_TRUE(token.IsSupported());

    // Navigator Mobile
    token = SamlToken();
    EXPECT_EQ(SUCCESS, Connect::Login(credentials, token, "sso://wsfed_mobile/2223", stsUrl));
    EXPECT_TRUE(token.IsSupported());

    // ProjectWise Mobile
    token = SamlToken();
    EXPECT_EQ(SUCCESS, Connect::Login(credentials, token, "sso://wsfed_mobile/2530", stsUrl));
    EXPECT_TRUE(token.IsSupported());

    // ProjectWise Worksite
    token = SamlToken();
    EXPECT_EQ(SUCCESS, Connect::Login(credentials, token, "sso://wsfed_mobile/2226", stsUrl));
    EXPECT_TRUE(token.IsSupported());

    // Structural navigator
    token = SamlToken();
    EXPECT_EQ(SUCCESS, Connect::Login(credentials, token, "sso://wsfed_mobile/2067", stsUrl));
    EXPECT_TRUE(token.IsSupported());

    // Invalid RP URI
    BeTest::SetFailOnAssert(false);
    token = SamlToken();
    EXPECT_EQ(ERROR, Connect::Login(credentials, token, "https://zz-wsg20-eus.cloudapp.net", stsUrl));
    EXPECT_FALSE(token.IsSupported());
    BeTest::SetFailOnAssert(true);
    }
