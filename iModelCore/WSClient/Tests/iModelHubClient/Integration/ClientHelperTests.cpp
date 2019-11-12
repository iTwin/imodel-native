/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "Helpers.h"
#include <WebServices/iModelHub/Client/ClientHelper.h>
#include <Bentley/BeTest.h>
#include <BeHttp/ProxyHttpHandler.h>

USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_IMODELHUB

using namespace ::testing;
using namespace ::std;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClientHelperTests : public Test
    {
    static ClientHelper* s_helper;
    static std::shared_ptr<MockHttpHandler> s_handler;

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              06/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        s_handler = std::make_shared<MockHttpHandler>();
        ClientHelper::Initialize(IntegrationTestsSettings::Instance().GetClientInfo(), StubLocalState::Instance(), s_handler);
        s_helper = ClientHelper::GetInstance();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              06/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        s_helper->SetUrl(IntegrationTestsSettings::Instance().GetServerUrl());
        ClientHelper::Initialize(IntegrationTestsSettings::Instance().GetClientInfo(), StubLocalState::Instance(), ProxyHttpHandler::GetFiddlerProxyIfReachable());
        }
    };

ClientHelper* ClientHelperTests::s_helper = nullptr;
std::shared_ptr<MockHttpHandler> ClientHelperTests::s_handler = nullptr;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClientHelperTests, SetsUrl)
    {
    Utf8String testUrl = "https://test.foo";
    s_helper->SetUrl(testUrl);
    ClientPtr client = s_helper->SignInWithStaticHeader("TestHeader");
    EXPECT_EQ(testUrl, client->GetServerUrl());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClientHelperTests, SetsStaticAuthorizationHeader)
    {
    Utf8String testUrl = "https://test.foo";
    Utf8String testHeader = "TestHeader";
    s_helper->SetUrl(testUrl);
    ClientPtr client = s_helper->SignInWithStaticHeader(testHeader);

    Http::Request request(testUrl);
    s_handler->ExpectRequest([&](Http::RequestCR req) {
        return Http::Response(HttpStatus::OK, testUrl, "Mas-Server:Bentley-WSG/02.06.00.00,Bentley-WebAPI/2.6", "");
    });
    s_handler->ExpectRequest([&](Http::RequestCR req) {
        request = req;
        return Http::Response(HttpStatus::OK, testUrl, "", "{}");
    });

    client->GetiModels(IntegrationTestsSettings::Instance().GetProjectId())->GetResult();
    EXPECT_EQ(testHeader, request.GetHeaders().GetAuthorization());
    }
