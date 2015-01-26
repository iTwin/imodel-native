/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/WSClientTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/WSClient.h>
#include "WSClientTests.h"
#include "MockServerInfoListener.h"
#include "WebServicesTestsHelper.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

TEST_F (WSClientTests, SendGetInfoRequest_Called_SendsGetInfoUrl)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectOneRequest ().ForAnyRequest ([=] (HttpRequestCR request)
        {
        EXPECT_STREQ ("GET", request.GetMethod ().c_str ());
        EXPECT_STREQ ("https://srv.com/ws/v1.2/Info", request.GetUrl ().c_str ());
        return StubHttpResponse ();
        });

    client->SendGetInfoRequest ()->Wait ();
    }

TEST_F (WSClientTests, GetServerInfo_CalledFirstTime_SendsGetInfoUrl)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectOneRequest ().ForAnyRequest ([=] (HttpRequestCR request)
        {
        EXPECT_STREQ ("GET", request.GetMethod ().c_str ());
        EXPECT_STREQ ("https://srv.com/ws/v1.2/Info", request.GetUrl ().c_str ());
        return StubHttpResponse ();
        });

    client->GetServerInfo ()->Wait ();
    }

TEST_F (WSClientTests, GetServerInfo_FirstResponseReturnsNotFound_QueriesServerForAboutPage)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubHttpResponse (HttpStatus::NotFound));
    GetHandler ().ForRequest (2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ ("https://srv.com/ws/Pages/About.aspx", request.GetUrl ().c_str ());
        auto aboutPageStub = R"(Web Service Gateway for BentleyCONNECT ... any text here ... <span id="versionLabel">1.1.0.0</span>)";
        return StubHttpResponse (HttpStatus::OK, aboutPageStub, {{"Content-Type", "text/html"}});
        });

    client->GetServerInfo ()->Wait ();
    }

TEST_F (WSClientTests, GetServerInfo_FirstAndSecondResponsesReturnNotFound_QueriesServerForPlugins)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (3);
    GetHandler ().ForRequest (1, StubHttpResponse (HttpStatus::NotFound));
    GetHandler ().ForRequest (2, StubHttpResponse (HttpStatus::NotFound));
    GetHandler ().ForRequest (3, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ ("https://srv.com/ws/v2.0/Plugins", request.GetUrl ().c_str ());
        return StubHttpResponse ();
        });

    client->GetServerInfo ()->Wait ();
    }

TEST_F (WSClientTests, GetServerInfo_FirstResponseReturnsNotFound_UsesAboutPageToIdentifyWSGR1)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    auto aboutPageStub = R"(<span id="productNameLabel">Bentley Web Services Gateway 01.00</span>)";

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubHttpResponse (HttpStatus::NotFound));
    GetHandler ().ForRequest (2, StubHttpResponse (HttpStatus::OK, aboutPageStub, {{"Content-Type", "text/html"}}));

    auto info = client->GetServerInfo ()->GetResult ();
    EXPECT_EQ (BeVersion (1, 0), info.GetValue ().GetVersion ());
    }

TEST_F (WSClientTests, GetServerInfo_FirstResponseReturnsNotFound_UsesAboutPageToIdentifyBentleyConnectWSGR1)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    auto aboutPageStub = R"(Web Service Gateway for BentleyCONNECT ... any text here ... <span id="versionLabel">1.1.0.0</span>)";

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubHttpResponse (HttpStatus::NotFound));
    GetHandler ().ForRequest (2, StubHttpResponse (HttpStatus::OK, aboutPageStub, {{"Content-Type", "text/html"}}));

    auto info = client->GetServerInfo ()->GetResult ();
    EXPECT_EQ (BeVersion (1, 0), info.GetValue ().GetVersion ());
    EXPECT_EQ (BeVersion (1, 1), info.GetValue ().GetWebApiVersion ());
    }

TEST_F (WSClientTests, GetServerInfo_FirstResponseReturnsNotFound_QueriesServerForAboutPageToIdentifyIfWSG2)
    {
    // TODO: this is temporary workaround for figuring out WSG 2.0 version until appropriate data is sent
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    auto aboutPageStub = R"(<span id="productNameLabel">Bentley Web Services Gateway 02.00</span>)";

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubHttpResponse (HttpStatus::NotFound));
    GetHandler ().ForRequest (2, StubHttpResponse (HttpStatus::OK, aboutPageStub, {{"Content-Type", "text/html"}}));

    auto info = client->GetServerInfo ()->GetResult ();
    EXPECT_EQ (BeVersion (2, 0), info.GetValue ().GetVersion ());
    }

TEST_F (WSClientTests, GetServerInfo_FirstResponseReturnsNotFoundSecondResponseHasUnknownBodyThirdResponseDoesNotHaveServerHeader_ReturnsNotSupported)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (3);
    GetHandler ().ForRequest (1, StubHttpResponse (HttpStatus::NotFound));
    GetHandler ().ForRequest (2, StubHttpResponse (HttpStatus::OK, "some other html", {{"Content-Type", "text/html"}}));
    GetHandler ().ForRequest (3, StubHttpResponse (HttpStatus::OK));

    auto info = client->GetServerInfo ()->GetResult ();
    EXPECT_EQ (WSError::Status::ServerNotSupported, info.GetError ().GetStatus ());
    }

TEST_F (WSClientTests, GetServerInfo_FirstResponseReturnsNotFoundSecondResponseHasUnknownBodyThirdResponseHasProperServerHeaderIdentifyingWSG2)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (3);
    GetHandler ().ForRequest (1, StubHttpResponse (HttpStatus::NotFound));
    GetHandler ().ForRequest (2, StubHttpResponse (HttpStatus::OK, "no version"));
    GetHandler ().ForRequest (3, StubHttpResponse (HttpStatus::OK, "", {{"Server", "Bentley-WebAPI/2.0"}}));

    auto info = client->GetServerInfo ()->GetResult ();
    EXPECT_EQ (BeVersion (2, 0), info.GetValue ().GetVersion ());
    }

TEST_F (WSClientTests, GetServerInfo_CalledTwiceWithSuccessfullConnection_QueriesServerOnce)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectOneRequest ().ForAnyRequest (StubHttpResponse (HttpStatus::OK, R"({"serverVersion":"01.02.00.00"})"));

    auto info1 = client->GetServerInfo ()->GetResult ();
    auto info2 = client->GetServerInfo ()->GetResult ();

    EXPECT_TRUE (info1.IsSuccess ());
    EXPECT_TRUE (info2.IsSuccess ());
    EXPECT_EQ (info1.GetValue ().GetVersion (), info2.GetValue ().GetVersion ());
    }

TEST_F (WSClientTests, GetServerInfo_SendInfoRequestCalledFirst_UsesCachedInfo)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectOneRequest ().ForAnyRequest (StubHttpResponse (HttpStatus::OK, R"({"serverVersion":"01.02.00.00"})"));

    auto info1 = client->SendGetInfoRequest ()->GetResult ();
    auto info2 = client->GetServerInfo ()->GetResult ();

    EXPECT_TRUE (info1.IsSuccess ());
    EXPECT_TRUE (info2.IsSuccess ());
    EXPECT_EQ (info1.GetValue ().GetVersion (), info2.GetValue ().GetVersion ());
    }

TEST_F (WSClientTests, GetServerInfo_PrevioulslyReceivedServerNotSupported_PreviouslyCachedInfoIsInvalidatedAndNewRequested)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (3);
    GetHandler ().ForRequest (1, StubHttpResponse (HttpStatus::OK, R"({"serverVersion":"01.02.00.00"})"));
    GetHandler ().ForRequest (2, StubHttpResponse (HttpStatus::OK, R"(this is not data source response)"));
    GetHandler ().ForRequest (3, StubHttpResponse (HttpStatus::OK, R"({"serverVersion":"01.02.00.00"})"));

    auto response = client->SendGetRepositoriesRequest ()->GetResult ();
    auto info = client->GetServerInfo ()->GetResult ();

    EXPECT_EQ (WSError::Status::ServerNotSupported, response.GetError ().GetStatus ());
    EXPECT_TRUE (info.IsSuccess ());
    }

#ifdef USE_GTEST
TEST_F (WSClientTests, RegisterServerInfoListener_AddedListener_ListenerNotifiedWithReceivedInfo)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());
    auto listener = std::make_shared<MockServerInfoListener> ();

    GetHandler ().ForAnyRequest ([=] (HttpRequestCR request)
        {
        return StubWSInfoHttpResponse (BeVersion (1, 2));
        });

    client->RegisterServerInfoListener (listener);

    EXPECT_CALL (*listener, OnServerInfoReceived (_)).Times (1).WillOnce (Invoke ([=] (WSInfoCR info)
        {
        EXPECT_EQ (BeVersion (1, 2), info.GetVersion ());
        }));

    client->SendGetInfoRequest ()->Wait ();
    }
#endif

TEST_F (WSClientTests, RegisterServerInfoListener_AddedListenerDeleted_ListenerNotLeakedAndNotNotified)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    static int listenerCallCount = 0;
    struct StubServerInfoListener : public IWSClient::IServerInfoListener
        {
        void OnServerInfoReceived (WSInfoCR info)
            {
            listenerCallCount ++;
            }
        };

    GetHandler ().ForAnyRequest ([=] (HttpRequestCR request)
        {
        return StubWSInfoHttpResponse (BeVersion (1, 2));
        });

    auto listener = std::make_shared<StubServerInfoListener> ();
    client->RegisterServerInfoListener (listener);
    EXPECT_EQ (0, listenerCallCount);

    client->SendGetInfoRequest ()->Wait ();
    EXPECT_EQ (1, listenerCallCount);

    listener = nullptr;
    client->SendGetInfoRequest ()->Wait ();

    EXPECT_EQ (1, listenerCallCount);
    }

#ifdef USE_GTEST
TEST_F (WSClientTests, RegisterServerInfoListener_InfoNotReceivedDueToNetworkError_ListenerNotNotified)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());
    auto listener = std::make_shared<MockServerInfoListener> ();

    EXPECT_CALL (*listener, OnServerInfoReceived (_)).Times (0);
    GetHandler ().ForAnyRequest ([=] (HttpRequestCR request)
        {
        return StubHttpResponse ();
        });

    client->RegisterServerInfoListener (listener);
    client->SendGetInfoRequest ()->Wait ();
    }

TEST_F (WSClientTests, RegisterServerInfoListener_NotSupportedServer_ListenerNotNotified)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());
    auto listener = std::make_shared<MockServerInfoListener> ();

    EXPECT_CALL (*listener, OnServerInfoReceived (_)).Times (0);
    GetHandler ().ForAnyRequest ([=] (HttpRequestCR request)
        {
        return StubHttpResponse (HttpStatus::OK, "{}");
        });

    client->RegisterServerInfoListener (listener);
    client->SendGetInfoRequest ()->Wait ();
    }

TEST_F (WSClientTests, UnregisterServerInfoListener_ExistingListener_ListenerNotNotified)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());
    auto listener = std::make_shared<MockServerInfoListener> ();

    EXPECT_CALL (*listener, OnServerInfoReceived (_)).Times (0);
    GetHandler ().ForAnyRequest ([=] (HttpRequestCR request)
        {
        return StubWSInfoHttpResponse (BeVersion (1, 2, 3, 0));
        });

    client->RegisterServerInfoListener (listener);
    client->UnregisterServerInfoListener (listener);

    client->SendGetInfoRequest ()->Wait ();
    }
#endif

TEST_F (WSClientTests, SendGetRepositoriesRequest_WebApiV1Format_ParsesDefaultFields)
    {
    Utf8String dataSourcesResponse =
        R"([{
            "id" : "D.testLocation",
            "label" : "A",
            "description" : "B",
            "type" : "C",
            "providerId" : "D" 
         }])";

    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubWSInfoHttpResponse (BeVersion (1, 2)));
    GetHandler ().ForRequest (2, StubJsonHttpResponse (HttpStatus::OK, dataSourcesResponse));

    auto response = client->SendGetRepositoriesRequest ()->GetResult ();
    auto dataSource = response.GetValue ().front ();

    EXPECT_EQ ("D.testLocation", dataSource.GetId ());
    EXPECT_EQ ("A", dataSource.GetLabel ());
    EXPECT_EQ ("B", dataSource.GetDescription ());
    EXPECT_EQ ("D", dataSource.GetPluginId ());
    }

TEST_F (WSClientTests, SendGetRepositoriesRequest_WebApiV1FormatWithType_ParsesLocationFromId)
    {
    Utf8String dataSourcesResponse =
        R"([{
            "id" : "testProvider.testLocation",
            "label" : "A",
            "description" : "B",
            "type" : "test.type",
            "providerId" : "testProvider" 
         }])";

    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubWSInfoHttpResponse (BeVersion (1, 2)));
    GetHandler ().ForRequest (2, StubJsonHttpResponse (HttpStatus::OK, dataSourcesResponse));

    auto response = client->SendGetRepositoriesRequest ()->GetResult ();
    auto dataSource = response.GetValue ().front ();

    EXPECT_EQ ("testLocation", dataSource.GetLocation ());
    }

TEST_F (WSClientTests, SendGetRepositoriesRequest_WebApiV1FormatWithNoType_ParsesLocationFromId)
    {
    Utf8String dataSourcesResponse =
        R"([{
            "id" : "testProvider.testLocation",
            "label" : "A",
            "description" : "B",
            "type" : null,
            "providerId" : "testProvider" 
         }])";

    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubWSInfoHttpResponse (BeVersion (1, 2)));
    GetHandler ().ForRequest (2, StubJsonHttpResponse (HttpStatus::OK, dataSourcesResponse));

    auto response = client->SendGetRepositoriesRequest ()->GetResult ();
    auto dataSource = response.GetValue ().front ();

    EXPECT_EQ ("testLocation", dataSource.GetLocation ());
    }

TEST_F (WSClientTests, SendGetRepositoriesRequest_WebApiV1FormatWithProviderIdECAndType_ParsesLocationFromId)
    {
    Utf8String dataSourcesResponse =
        R"([{
            "id" : "ec.test.type--testLocation",
            "label" : "A",
            "description" : "B",
            "type" : "test.type",
            "providerId" : "ec" 
         }])";

    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubWSInfoHttpResponse (BeVersion (1, 2)));
    GetHandler ().ForRequest (2, StubJsonHttpResponse (HttpStatus::OK, dataSourcesResponse));

    auto response = client->SendGetRepositoriesRequest ()->GetResult ();
    auto dataSource = response.GetValue ().front ();

    EXPECT_EQ ("testLocation", dataSource.GetLocation ());
    }

TEST_F (WSClientTests, SendGetRepositoriesRequest_WebApiV1AndIdIsNotKnownFormat_ReturnsEmptyLocation)
    {
    Utf8String dataSourcesResponse =
        R"([{                   
            "id" : "someOtherId",
            "label" : "A",
            "description" : "B",
            "type" : "C",
            "providerId" : "D" 
         }])";

    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubWSInfoHttpResponse (BeVersion (1, 2)));
    GetHandler ().ForRequest (2, StubJsonHttpResponse (HttpStatus::OK, dataSourcesResponse));

    auto response = client->SendGetRepositoriesRequest ()->GetResult ();

    ASSERT_TRUE (response.IsSuccess ());

    auto dataSource = response.GetValue ().front ();

    EXPECT_EQ ("someOtherId", dataSource.GetId ());
    EXPECT_EQ ("A", dataSource.GetLabel ());
    EXPECT_EQ ("B", dataSource.GetDescription ());
    EXPECT_EQ ("D", dataSource.GetPluginId ());

    EXPECT_EQ ("", dataSource.GetLocation ());
    }

TEST_F (WSClientTests, SendGetRepositoriesRequest_WebApiV1FormatWithProviderIdEC_ReturnsPluginIdFromProviderTypeField)
    {
    Utf8String dataSourcesResponse =
        R"([{                   
            "id" : "ec.test.pluginId--test",
            "label" : "",
            "description" : "",
            "type" : "test.pluginId",
            "providerId" : "ec" 
         }])";

    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubWSInfoHttpResponse (BeVersion (1, 2)));
    GetHandler ().ForRequest (2, StubJsonHttpResponse (HttpStatus::OK, dataSourcesResponse));

    auto response = client->SendGetRepositoriesRequest ()->GetResult ();

    auto dataSource = response.GetValue ().front ();
    EXPECT_EQ ("test.pluginId", dataSource.GetPluginId ());
    }

TEST_F (WSClientTests, SendGetRepositoriesRequest_WebApiV1_CorrectUrl)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubWSInfoHttpResponse (BeVersion (1, 2))); // WebApi 1.3
    GetHandler ().ForRequest (2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ ("https://srv.com/ws/v1.1/DataSources", request.GetUrl ().c_str ());
        return StubHttpResponse ();
        });

    client->SendGetRepositoriesRequest ()->Wait ();
    }

TEST_F (WSClientTests, SendGetRepositoriesRequest_WebApiV11_UrlWithoutWebApiVersion)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubWSInfoHttpResponse (BeVersion (1, 0))); // WebApi 1.1
    GetHandler ().ForRequest (2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ ("https://srv.com/ws/DataSources", request.GetUrl ().c_str ());
        return StubHttpResponse ();
        });

    client->SendGetRepositoriesRequest ()->Wait ();
    }

TEST_F (WSClientTests, SendGetRepositoriesRequest_WebApiV2_CorrectUrl)
    {
    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubWSInfoHttpResponse (BeVersion (2, 0)));
    GetHandler ().ForRequest (2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ ("https://srv.com/ws/v2.0/Repositories/", request.GetUrl ().c_str ());
        return StubHttpResponse ();
        });

    client->SendGetRepositoriesRequest ()->Wait ();
    }

TEST_F (WSClientTests, SendGetRepositoriesRequest_WebApiV2Format_ReturnsParsedDataSources)
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

    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubWSInfoHttpResponse (BeVersion (2, 0)));
    GetHandler ().ForRequest (2, StubJsonHttpResponse (HttpStatus::OK, dataSourcesResponse));

    auto response = client->SendGetRepositoriesRequest ()->GetResult ();

    EXPECT_TRUE (response.IsSuccess ());
    EXPECT_EQ (1, response.GetValue ().size ());

    auto dataSource = response.GetValue ().front ();

    EXPECT_EQ ("testRepositoryId", dataSource.GetId ());
    EXPECT_EQ ("testLabel", dataSource.GetLabel ());
    EXPECT_EQ ("testDescription", dataSource.GetDescription ());
    EXPECT_EQ ("testLocation", dataSource.GetLocation ());
    EXPECT_EQ ("testPluginId", dataSource.GetPluginId ());
    }

TEST_F (WSClientTests, SendGetRepositoriesRequest_WebApiV2FormatWithWrongSchema_ReturnsErrorNotSupported)
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

    auto client = WSClient::Create ("https://srv.com/ws", {}, GetHandlerPtr ());

    GetHandler ().ExpectRequests (2);
    GetHandler ().ForRequest (1, StubWSInfoHttpResponse (BeVersion (2, 0)));
    GetHandler ().ForRequest (2, StubJsonHttpResponse (HttpStatus::OK, dataSourcesResponse));

    auto response = client->SendGetRepositoriesRequest ()->GetResult ();

    EXPECT_EQ (WSError::Status::ServerNotSupported, response.GetError ().GetStatus ());
    }
