/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/Client/WSClient.h>
#include "WSClientTests.h"
#include "MockServerInfoListener.h"
#include "../../../../../Client/ServerInfoProvider.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

void WSClientTests::SetUp() 
    {
    BaseMockHttpHandlerTest::SetUp();
    BackDoor::DgnClientFx_Device::Initialize();
    ServerInfoProvider::InvalidateAllInfo();
    }

void WSClientTests::TearDown()
    {
    BaseMockHttpHandlerTest::TearDown();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, SendGetInfoRequest_Called_SendsGetPluginsUrl)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Plugins", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetInfoRequest()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, VerifyConnetction_Called_SendsGetRequestToMainUrl)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws", request.GetUrl().c_str());
        return StubWSErrorHttpResponse(HttpStatus::OK);
        });

    client->VerifyConnection()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, VerifyConnetction_ConnectionOK_Succeeds)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSErrorHttpResponse(HttpStatus::OK));

    EXPECT_TRUE(client->VerifyConnection()->GetResult().IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, VerifyConnetction_CouldNotConnect_Fails)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSErrorHttpResponse(HttpStatus::None));

    EXPECT_FALSE(client->VerifyConnection()->GetResult().IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_CalledFirstTime_SendsGetPluginsUrl)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Plugins", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->GetServerInfo()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_WithEmptyServiceVersionCalledFirstTime_SendsGetPluginsUrlUsingWebApiVersion)
    {
    auto client = WSClient::Create("https://srv.com/ws", {0, 0}, StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Plugins", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->GetServerInfo()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_WithServiceVersionCalledFirstTime_SendsGetPluginsUrlUsingServiceVersion)
    {
    auto client = WSClient::Create("https://srv.com/ws", {4, 2}, StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/sv4.2/Plugins", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->GetServerInfo()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_SecondResponseReturnsNotFound_ReturnsError)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::NotFound));

    auto result = client->GetServerInfo()->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Status::ServerNotSupported, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_WithServiceVersionFirstResponsesReturnNotFound_ReturnsError)
    {
    auto client = WSClient::Create("https://srv.com/ws", {4, 2}, StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::NotFound));

    auto result = client->GetServerInfo()->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Status::ServerNotSupported, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_FirstResponsesReturnsBadRequestAndNoWSGError_ReturnsError)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::BadRequest, "Foo Error Message"));

    auto result = client->GetServerInfo()->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Status::ServerNotSupported, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_FirstResponsesReturnsNotNotFoundAndWSGError_ReturnsError)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());
   
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSErrorHttpResponse(HttpStatus::BadRequest, "Foo", "Boo", "Goo"));

    auto result = client->GetServerInfo()->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ("Boo", result.GetError().GetMessage());
    EXPECT_EQ("Goo", result.GetError().GetDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_FirstResponseDoesNotHaveServerHeader_ReturnsError)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler ().ForRequest (1, StubHttpResponse (HttpStatus::OK, "some other html", {{"Content-Type", REQUESTHEADER_ContentType_TextHtml}}));

    auto result = client->GetServerInfo()->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Status::ServerNotSupported, result.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_FirstResponseHasProperServerHeader_IdentifyingWSG2)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::OK, "", {{"Server", "Bentley-WebAPI/2.1,Bentley-WSG/2.3"}}));

    auto info = client->GetServerInfo()->GetResult();
    EXPECT_EQ(BeVersion(2, 1), info.GetValue().GetWebApiVersion());
    EXPECT_EQ(BeVersion(2, 3), info.GetValue().GetVersion());
    EXPECT_EQ(BeVersion(), info.GetValue().GetServiceVersion());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_WithServiceVersionFirstResponseHasProperServerHeader_IdentifyingWSG2)
    {
    auto client = WSClient::Create("https://srv.com/ws", {4, 2}, StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::OK, "", {{"Server", "Bentley-WebAPI/2.1,Bentley-WSG/2.3"}}));

    auto info = client->GetServerInfo()->GetResult();
    EXPECT_EQ(BeVersion(2, 1), info.GetValue().GetWebApiVersion());
    EXPECT_EQ(BeVersion(2, 3), info.GetValue().GetVersion());
    EXPECT_EQ(BeVersion(4, 2), info.GetValue().GetServiceVersion());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_WithServiceVersionFirstResponseIdentifiesServiceWithLowerWebApiVersionThanSupported_ReturnsError)
    {
    auto client = WSClient::Create("https://srv.com/ws", {4, 2}, StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::OK, "", {{"Server", "Bentley-WebAPI/1.9,Bentley-WSG/2.3"}}));

    auto info = client->GetServerInfo()->GetResult();
    ASSERT_FALSE(info.IsSuccess());
    EXPECT_EQ(WSError::Status::ServerNotSupported, info.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_WithServiceVersionFirstResponseIdentifiesServiceWithHigherWebApiVersionThanSupported_ReturnsError)
    {
    auto client = WSClient::Create("https://srv.com/ws", {4, 2}, StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::OK, "", {{"Server", "Bentley-WebAPI/2.9,Bentley-WSG/2.3"}}));

    auto info = client->GetServerInfo()->GetResult();
    ASSERT_FALSE(info.IsSuccess());
    EXPECT_EQ(WSError::Status::ServerNotSupported, info.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                julius.cepukenas                     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_FirstResponseHasProperMasServerHeader_IdentifyingWSG)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::OK, "", {{"Mas-Server", "Bentley-WebAPI/2.1,Bentley-WSG/2.3"}}));

    auto info = client->GetServerInfo()->GetResult();
    EXPECT_EQ(BeVersion(2, 1), info.GetValue().GetWebApiVersion());
    EXPECT_EQ(BeVersion(2, 3), info.GetValue().GetVersion());
    EXPECT_EQ(BeVersion(), info.GetValue().GetServiceVersion());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                julius.cepukenas                     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_FirstResponseHasMasServerAndServerHeader_IdentifyingCorrectWSG)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::OK, "", {
            {"Mas-Server", "Bentley-WebAPI/2.6,Bentley-WSG/2.6"},
            {"Server", "Bentley-WebAPI/2.0,Bentley-WSG/2.0"}}));

    auto info = client->GetServerInfo()->GetResult();
    EXPECT_EQ(BeVersion(2, 6), info.GetValue().GetWebApiVersion());
    EXPECT_EQ(BeVersion(2, 6), info.GetValue().GetVersion());
    EXPECT_EQ(BeVersion(), info.GetValue().GetServiceVersion());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_CalledTwiceWithSuccessfullConnection_QueriesServerOnce)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest(StubHttpResponse(HttpStatus::OK, "", {{"Server", "Bentley-WebAPI/2.0,Bentley-WSG/2.0"}}));

    auto info1 = client->GetServerInfo()->GetResult();
    auto info2 = client->GetServerInfo()->GetResult();

    EXPECT_TRUE(info1.IsSuccess());
    EXPECT_TRUE(info2.IsSuccess());
    EXPECT_EQ(info1.GetValue().GetVersion(), info2.GetValue().GetVersion());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_SendInfoRequestCalledFirst_UsesCachedInfo)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectOneRequest().ForAnyRequest(StubHttpResponse(HttpStatus::OK, "", {{"Server", "Bentley-WebAPI/2.0,Bentley-WSG/2.0"}}));

    auto info1 = client->SendGetInfoRequest()->GetResult();
    auto info2 = client->GetServerInfo()->GetResult();

    EXPECT_TRUE(info1.IsSuccess());
    EXPECT_TRUE(info2.IsSuccess());
    EXPECT_EQ(info1.GetValue().GetVersion(), info2.GetValue().GetVersion());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_PrevioulslyReceivedServerNotSupported_PreviouslyCachedInfoIsInvalidatedAndNewRequested)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::OK, "", {{"Server", "Bentley-WebAPI/2.2,Bentley-WSG/2.3"}}));
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK, R"(this is not data source response)"));
    GetHandler().ForRequest(3, StubHttpResponse(HttpStatus::OK, "", {{"Server", "Bentley-WebAPI/2.2,Bentley-WSG/2.3"}}));

    auto response = client->SendGetRepositoriesRequest()->GetResult();
    auto info = client->GetServerInfo()->GetResult();

    EXPECT_EQ(WSError::Status::ServerNotSupported, response.GetError().GetStatus());
    EXPECT_TRUE(info.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_TwoServersCalledTwoTimesEach_UsesCachedInfo)
    {
    auto client1 = WSClient::Create("https://one.com/ws", StubClientInfo(), GetHandlerPtr());
    auto client2 = WSClient::Create("https://two.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ForAnyRequest([=] (Http::RequestCR request)
        {
        Utf8String url = request.GetUrl();
        if (url == "https://one.com/ws/v2.0/Plugins")
            return StubHttpResponse(HttpStatus::OK, "", { { "Server", "Bentley-WebAPI/2.0,Bentley-WSG/2.0" } });
        else
            return StubHttpResponse(HttpStatus::OK, "", { { "Server", "Bentley-WebAPI/2.5,Bentley-WSG/2.5" } });
        });

    struct StubServerInfoListener : IWSClient::IServerInfoListener
        {
        int& m_listenerCallCount;
        StubServerInfoListener(int& listenerCallCount) : m_listenerCallCount(listenerCallCount) {}
        void OnServerInfoReceived(WSInfoCR info)
            {
            m_listenerCallCount++;
            }
        };

    int listenerCallCount1 = 0;
    auto listener1 = std::make_shared<StubServerInfoListener>(listenerCallCount1);
    client1->RegisterServerInfoListener(listener1);

    int listenerCallCount2 = 0;
    auto listener2 = std::make_shared<StubServerInfoListener>(listenerCallCount2);
    client1->RegisterServerInfoListener(listener2);

    auto info1 = client1->GetServerInfo()->GetResult();
    auto info2 = client2->GetServerInfo()->GetResult();

    info1 = client1->GetServerInfo()->GetResult();
    EXPECT_EQ(BeVersion(2, 0), info1.GetValue().GetVersion());
    EXPECT_EQ(1, listenerCallCount1);

    info2 = client2->GetServerInfo()->GetResult();
    EXPECT_EQ(BeVersion(2, 5), info2.GetValue().GetVersion());
    EXPECT_EQ(1, listenerCallCount2);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_FirstResponseIsUnauthorized_StopsAndReturnsLoginError)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubHttpResponse(HttpStatus::Unauthorized));

    auto result = client->GetServerInfo()->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Status::ReceivedError, result.GetError().GetStatus());
    EXPECT_EQ(WSError::Id::LoginFailed, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, GetServerInfo_AllResponsesWithServerError_ReturnsServerNotSupportedError)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ForAnyRequest(StubHttpResponse(HttpStatus::InternalServerError));

    auto result = client->GetServerInfo()->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Status::ServerNotSupported, result.GetError().GetStatus());
    }

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, RegisterServerInfoListener_AddedListener_ListenerNotifiedWithReceivedInfo)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());
    auto listener = std::make_shared<MockServerInfoListener>();

    GetHandler().ForAnyRequest([=] (Http::RequestCR request)
        {
        return StubWSInfoHttpResponseWebApi20();
        });

    client->RegisterServerInfoListener(listener);

    EXPECT_CALL(*listener, OnServerInfoReceived(_)).Times(1).WillOnce(Invoke([=] (WSInfoCR info)
        {
        EXPECT_EQ(BeVersion(2, 0), info.GetVersion());
        }));

    client->SendGetInfoRequest()->Wait();
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, RegisterServerInfoListener_AddedListenerDeleted_ListenerNotLeakedAndNotNotified)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    int listenerCallCount = 0;
    struct StubServerInfoListener : public IWSClient::IServerInfoListener
        {
        int& m_listenerCallCount;
        StubServerInfoListener(int& listenerCallCount) : m_listenerCallCount(listenerCallCount) {}
        void OnServerInfoReceived(WSInfoCR info)
            {
            m_listenerCallCount++;
            }
        };

    GetHandler().ForAnyRequest([=] (Http::RequestCR request)
        {
        return StubWSInfoHttpResponseWebApi20();
        });

    auto listener = std::make_shared<StubServerInfoListener>(listenerCallCount);
    client->RegisterServerInfoListener(listener);
    EXPECT_EQ(0, listenerCallCount);

    client->SendGetInfoRequest()->Wait();
    EXPECT_EQ(1, listenerCallCount);

    listener = nullptr;
    client->SendGetInfoRequest()->Wait();

    EXPECT_EQ(1, listenerCallCount);
    }

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, RegisterServerInfoListener_InfoNotReceivedDueToNetworkError_ListenerNotNotified)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());
    auto listener = std::make_shared<MockServerInfoListener>();

    EXPECT_CALL(*listener, OnServerInfoReceived(_)).Times(0);
    GetHandler().ForAnyRequest([=] (Http::RequestCR request)
        {
        return StubHttpResponse();
        });

    client->RegisterServerInfoListener(listener);
    client->SendGetInfoRequest()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, RegisterServerInfoListener_NotSupportedServer_ListenerNotNotified)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());
    auto listener = std::make_shared<MockServerInfoListener>();

    EXPECT_CALL(*listener, OnServerInfoReceived(_)).Times(0);
    GetHandler().ForAnyRequest([=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::OK, "{}");
        });

    client->RegisterServerInfoListener(listener);
    client->SendGetInfoRequest()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, UnregisterServerInfoListener_ExistingListener_ListenerNotNotified)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());
    auto listener = std::make_shared<MockServerInfoListener>();

    EXPECT_CALL(*listener, OnServerInfoReceived(_)).Times(0);
    GetHandler().ForAnyRequest([=] (Http::RequestCR request)
        {
        return StubWSInfoHttpResponseWebApi20();
        });

    client->RegisterServerInfoListener(listener);
    client->UnregisterServerInfoListener(listener);

    client->SendGetInfoRequest()->Wait();
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, SendGetRepositoriesRequest_WithServiceVersionAndWebApi20_CorrectUrl)
    {
    auto client = WSClient::Create("https://srv.com/ws", {4, 2}, StubClientInfo(), GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/sv4.2/Repositories/", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRepositoriesRequest()->Wait();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, SendGetRepositoriesRequest_WebApiV2_CorrectUrl)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRepositoriesRequest()->Wait();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, SendGetRepositoriesRequest_WebApiV21_CorrectUrl)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.1/Repositories/", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRepositoriesRequest()->Wait();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, SendGetRepositoriesRequest_WebApiV22_CorrectUrl)
    {
    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi22());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.2/Repositories/", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRepositoriesRequest()->Wait();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, SendGetRepositoriesRequest_WebApiV2Format_ReturnsParsedDataSources)
    {
    Utf8String dataSourcesResponse =
        R"({
        "instances":
            [{
            "instanceId": "testRepositoryId",
            "className": "RepositoryIdentifier",
            "schemaName": "Repositories",
            "properties":
                {
                "ECPluginID": "testPluginId",
                "Location": "testLocation",
                "DisplayLabel": "testLabel",
                "Description": "testDescription"
                }
            }]
        })";

    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubJsonHttpResponse(HttpStatus::OK, dataSourcesResponse));

    auto response = client->SendGetRepositoriesRequest()->GetResult();

    EXPECT_TRUE(response.IsSuccess());
    EXPECT_EQ(1, response.GetValue().size());

    auto dataSource = response.GetValue().front();

    EXPECT_EQ("testRepositoryId", dataSource.GetId());
    EXPECT_EQ("testLabel", dataSource.GetLabel());
    EXPECT_EQ("testDescription", dataSource.GetDescription());
    EXPECT_EQ("testLocation", dataSource.GetLocation());
    EXPECT_EQ("testPluginId", dataSource.GetPluginId());
    EXPECT_EQ("https://srv.com/ws", dataSource.GetServerUrl());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSClientTests, SendGetRepositoriesRequest_WebApiV2FormatWithWrongSchema_ReturnsErrorNotSupported)
    {
    Utf8String dataSourcesResponse =
        R"({
        "instances":
            [{
            "instanceId": "testRepositoryId",
            "className": "OtherClass",
            "schemaName": "OtherSchema",
            "properties":
                {
                "ECPluginID": "testPluginId",
                "Location": "testLocation",
                "DisplayLabel": "testLabel",
                "Description": "testDescription"
                }
            }]
        })";

    auto client = WSClient::Create("https://srv.com/ws", StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubJsonHttpResponse(HttpStatus::OK, dataSourcesResponse));

    auto response = client->SendGetRepositoriesRequest()->GetResult();

    EXPECT_EQ(WSError::Status::ServerNotSupported, response.GetError().GetStatus());
    }
