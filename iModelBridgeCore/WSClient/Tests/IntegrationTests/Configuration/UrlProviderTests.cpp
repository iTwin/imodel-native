/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Configuration/UrlProviderTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UrlProviderTests.h"

#include <WebServices/Client/WSClient.h>
#include "../../../ConnectC/WSLocalState.h"
#include <BeHttp/ProxyHttpHandler.h>

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
INSTANTIATE_TEST_CASE_P(,
                        UrlProviderTests,
                        ::testing::Combine(::testing::Values(UrlProvider::Environment::Dev, UrlProvider::Environment::Qa, UrlProvider::Environment::Release),
                                           ::testing::ValuesIn(UrlProvider::GerUrlRegistry())),
                        UrlProviderTests::PrintToStringParamName());

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(UrlProviderTests, VerifyClientConnection_Success)
    {
    auto ls = WSLocalState();
    UrlProvider::Environment enviroment = ::testing::get<0>(GetParam());
    UrlProvider::Initialize(enviroment, UrlProvider::DefaultTimeout, &ls);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    auto urlDescriptor = ::testing::get<1>(GetParam());
    Utf8StringCR serverUrl = urlDescriptor->Get();
    if (serverUrl.empty())
        return;

    auto client = WSClient::Create(serverUrl, StubValidClientInfo(), proxy);
    EXPECT_TRUE(client->VerifyConnection()->GetResult().IsSuccess()) << "Failed url: " + serverUrl;
    }