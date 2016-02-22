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
#include <MobileDgn/Utils/Http/ProxyHttpHandler.h>
#include <curl/curl.h>

TEST_F(ConnectTests, GetStsToken_ProdUrls_RetrievesToken)
    {
    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    Connect::Initialize(StubClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = Connect::Login(credentials)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    auto tokenAuthStr = token->ToAuthorizationString();
    BeDebugLog(tokenAuthStr.c_str());
    }

TEST_F(ConnectTests, Login_QaUrls_RetrievesToken)
    {
    StubLocalState localState;
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &localState);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    Connect::Initialize(StubClientInfo(), proxy);

    Credentials credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    auto result = Connect::Login(credentials)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    SamlTokenPtr token = result.GetValue();
    auto tokenAuthStr = token->ToAuthorizationString();
    BeDebugLog(tokenAuthStr.c_str());
    }
