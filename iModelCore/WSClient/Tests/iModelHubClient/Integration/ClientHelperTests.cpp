/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/ClientHelperTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "Helpers.h"
#include <WebServices/iModelHub/Client/ClientHelper.h>
#include <Bentley/BeTest.h>
#include <BeHttp/ProxyHttpHandler.h>

USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_IMODELHUB

using namespace ::testing;
using namespace ::std;

struct ClientHelperTests : public Test
    {
    static ClientHelper* s_helper;
    static std::shared_ptr<MockHttpHandler> s_handler;

    static void SetUpTestCase()
        {
        s_handler = std::make_shared<MockHttpHandler>();
        ClientHelper::Initialize(IntegrationTestsSettings::Instance().GetClientInfo(), StubLocalState::Instance(), s_handler);
        s_helper = ClientHelper::GetInstance();
        }

    static void TearDownTestCase()
        {
        s_helper->SetUrl("");
        ClientHelper::Initialize(IntegrationTestsSettings::Instance().GetClientInfo(), StubLocalState::Instance(), ProxyHttpHandler::GetFiddlerProxyIfReachable());
        }
    };

ClientHelper* ClientHelperTests::s_helper = nullptr;
std::shared_ptr<MockHttpHandler> ClientHelperTests::s_handler = nullptr;

TEST_F(ClientHelperTests, SetsUrl)
    {
    Utf8String testUrl = "TestUrl";
    s_helper->SetUrl(testUrl);
    ClientPtr client = s_helper->SignInWithStaticHeader("TestHeader");
    EXPECT_EQ(testUrl, client->GetServerUrl());
    }

TEST_F(ClientHelperTests, SetsStaticAuthorizationHeader)
    {
    Utf8String testUrl = "TestUrl";
    Utf8String testHeader = "TestHeader";
    s_helper->SetUrl(testUrl);
    ClientPtr client = s_helper->SignInWithStaticHeader(testHeader);

    Http::Request request("");
    s_handler->ExpectRequest([&](Http::RequestCR req) {
        return Http::Response(HttpStatus::OK, "TestUrl", "Mas-Server:Bentley-WSG/02.06.00.00,Bentley-WebAPI/2.6", "");
    });
    s_handler->ExpectRequest([&](Http::RequestCR req) {
        request = req;
        return Http::Response(HttpStatus::OK, "TestUrl", "", "{}");
    });

    client->GetiModels(IntegrationTestsSettings::Instance().GetProjectId())->GetResult();
    EXPECT_EQ(testHeader, request.GetHeaders().GetAuthorization());
    }
