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
#include <MobileDgn/Utils/Http/ProxyHttpHandler.h>
#include <curl/curl.h>

TEST_F(ImsClientTests, GetToken_ProdUrls_RetrievesToken)
    {
    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto client = ImsClient::Create(StubClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = client->GetToken(credentials, ImsClient::GetLegacyRelyingPartyUri())->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    auto tokenAuthStr = token->ToAuthorizationString();
    BeDebugLog(tokenAuthStr.c_str());
    }

TEST_F(ImsClientTests, GetToken_QaUrls_RetrievesToken)
    {
    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto client = ImsClient::Create(StubClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = client->GetToken(credentials, ImsClient::GetLegacyRelyingPartyUri())->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    auto tokenAuthStr = token->ToAuthorizationString();
    BeDebugLog(tokenAuthStr.c_str());
    }
