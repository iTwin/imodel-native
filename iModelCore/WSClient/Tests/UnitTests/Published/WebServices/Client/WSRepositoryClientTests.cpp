/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Client/WSRepositoryClientTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSRepositoryClientTests.h"

#include <fstream>
#include <iostream>

#include <Bentley/Base64Utilities.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/Client/WSChangeset.h>
#include <WebServices/Cache/Util/JsonUtil.h>
#include "../../../../../Client/ServerInfoProvider.h"

#include "MockWSSchemaProvider.h"
#include "MockRepositoryInfoListener.h"

using namespace ::testing;
using namespace ::std;
USING_NAMESPACE_BENTLEY_WEBSERVICES

#define HEADER_MasFileETag                 "Mas-File-ETag"
#define HEADER_MasConnectionInfo           "Mas-Connection-Info"
#define HEADER_MasFileAccessUrlType        "Mas-File-Access-Url-Type"
#define HEADER_MasRequestId                "Mas-Request-Id"
#define HEADER_MasUploadConfirmationId     "Mas-Upload-Confirmation-Id"
#define HEADER_Location                    "Location"

#define INSTANCE_PersistenceFileBackable   "Persistence.FileBackable"
#define INSTANCE_PersistenceStreamBackable "Persistence.StreamBackable"

enum class VersionType
    {
    WebApi,
    Service
    };

void WSRepositoryClientTests::SetUp()
    {
    BaseMockHttpHandlerTest::SetUp();
    ServerInfoProvider::InvalidateAllInfo();
    }

void WSRepositoryClientTests::TearDown()
    {
    ServerInfoProvider::InvalidateAllInfo();
    BaseMockHttpHandlerTest::TearDown();
    }

Utf8String StubGetMaxUploadSizeResponseBody(uint64_t maxUploadSize, Utf8String instanceId = INSTANCE_PersistenceFileBackable)
    {
    Utf8String maxUploadSizeProperty = maxUploadSize > 0 ? Utf8PrintfString(R"(,
        {
        "Name": "MaxUploadSize",
        "Value": "%i",
        "Type": 2
        })", maxUploadSize)
        : Utf8String();

    return Utf8PrintfString(R"({
        "instances": [
            {
            "instanceId": "%s",
            "schemaName": "Policies",
            "className": "PolicyAssertion",
            "properties" :
                {
                "Supported": true,
                "AdhocProperties": [
                    {
                    "Name": "SupportsWrite",
                    "Value": "True",
                    "Type": 5
                    }
                    %s
                ]
                }
            }]
        })", instanceId.c_str(), maxUploadSizeProperty.c_str());
    }

void AddMaxUploadSizeRequestUrlSubPath(Utf8StringR unescapedPath)
    {
    Utf8String subPath = "/Policies/PolicyAssertion?$filter=(Supported+eq+true)+and+$id+in+['Persistence.FileBackable','Persistence.StreamBackable']&$top=1";
    unescapedPath.append(subPath);

    BeUri escapedUri(unescapedPath);
    unescapedPath = escapedUri.ToString();
    }

void TestGetMaxUploadSize(shared_ptr<MockHttpHandler> handlerPtr, BeVersion webApiVersion, Utf8String responseBody, uint64_t expectedMaxUploadSize)
    {
    MockHttpHandler& handler = (*handlerPtr);

    auto client = WSRepositoryClient::Create("https://srv.com/ws", "testPluginId--locationId", StubClientInfo(), nullptr, handlerPtr);

    handler.ExpectRequests(2);
    handler.ForRequest(1, StubWSInfoHttpResponseWebApi(webApiVersion));
    handler.ForRequest(2, [=] (Http::RequestCR request)
        {
        Utf8String expectedUri = Utf8PrintfString("https://srv.com/ws/v%s/Repositories/testPluginId--locationId", webApiVersion.ToMajorMinorString().c_str());
        AddMaxUploadSizeRequestUrlSubPath(expectedUri);
        EXPECT_STRCASEEQ(expectedUri.c_str(), request.GetUrl().c_str());
        return StubHttpResponse(HttpStatus::OK, responseBody);
        });

    auto result = client->GetInfo()->GetResult();
    EXPECT_TRUE(result.IsSuccess());

    auto repository = result.GetValue();
    EXPECT_EQ(expectedMaxUploadSize, repository.GetMaxUploadSize());
    }

Json::Value StubWSObjectCreationJson()
    {
    return ToJson(
        R"( {
            "instance" :
                {
                "schemaName": "TestSchema",
                "className": "TestClass",
                "properties": {}
                }
            })");
    }

void Expect4_jSrS(MockHttpHandler& handler, HttpStatus status)
    {
    handler.ExpectRequests(4);
    handler.ForRequest(1, StubWSInfoHttpResponseWebApi27());
    std::map<Utf8String, Utf8String> headers {{"Operation-Location", "https://test/foo"}};
    handler.ForRequest(2, StubHttpResponse(HttpStatus::Accepted, "", headers));

    DateTime dateTime;
    auto scheduleTime = dateTime.GetCurrentTimeUtc().ToString();

    Utf8String body =
        Utf8PrintfString(R"({
            "instances" : [
                {
                "instanceId": "6b39f111-e38e-41b2-bf33-1c9b2a0965e1",
                "schemaName": "Jobs",
                "className": "Job",
                "properties" :
                    {
                    "ResponseStatusCode":%i,
                    "ResponseContent":"Good Content",
                    "ResponseHeaders":"",
                    "ScheduleTime": "%s",
                    "Status":"Succeeded"
                    }
                }]
            })", status, scheduleTime.c_str());

    handler.ForRequest(3, StubHttpResponse(HttpStatus::OK, body, headers));
    handler.ForRequest(4, StubHttpResponse(HttpStatus::OK));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WebApi20ResponseIsOkNoPluginVersion_ReturnsRepositoryInfoWithNoPluginVersion)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "testPluginId--locationId", StubClientInfo(), nullptr, GetHandlerPtr());

    std::map<Utf8String, Utf8String> headers {{"Mas-Server", "Bentley-WebAPI/2.6,Bentley-WSG/2.6"}};

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->GetInfo()->GetResult();
    EXPECT_TRUE(result.IsSuccess());

    auto repository = result.GetValue();
    EXPECT_EQ("https://srv.com/ws", repository.GetServerUrl());
    EXPECT_EQ("testPluginId--locationId", repository.GetId());
    EXPECT_EQ("locationId", repository.GetLocation());
    EXPECT_EQ("testPluginId", repository.GetPluginId());
    EXPECT_TRUE(repository.GetPluginVersion().IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WithServiceVersionAndWebApi20ResponseIsOkNoPluginVersion_ReturnsRepositoryInfoWithNoPluginVersion)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", {4, 2}, "testPluginId--locationId", StubClientInfo(), nullptr, GetHandlerPtr());

    std::map<Utf8String, Utf8String> headers {{"Mas-Server", "Bentley-WebAPI/2.6,Bentley-WSG/2.6"}};

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->GetInfo()->GetResult();
    EXPECT_TRUE(result.IsSuccess());

    auto repository = result.GetValue();
    EXPECT_EQ("https://srv.com/ws", repository.GetServerUrl());
    EXPECT_EQ("testPluginId--locationId", repository.GetId());
    EXPECT_EQ("locationId", repository.GetLocation());
    EXPECT_EQ("testPluginId", repository.GetPluginId());
    EXPECT_TRUE(repository.GetPluginVersion().IsEmpty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Mantas.Smicius    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WebApi20Response_ReturnsRepositoryInfoWithInvalidMaxUploadSize)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", {4, 2}, "testPluginId--locationId", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->GetInfo()->GetResult();
    EXPECT_TRUE(result.IsSuccess());

    auto repository = result.GetValue();
    EXPECT_EQ(0, repository.GetMaxUploadSize());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WebApi28ResponseIsOkWithPluginVersionAndMassServerHeader_ReturnsRepositoryInfoWithPluginVersion)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "testPluginId--locationId", StubClientInfo(), nullptr, GetHandlerPtr());

    std::map<Utf8String, Utf8String> headers {{"Mas-Server", "Bentley-WebAPI/2.6,Bentley-WSG/2.6,testPluginId/1.2"}};

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi28());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        Utf8String expectedUri = "https://srv.com/ws/v2.8/Repositories/testPluginId--locationId";
        AddMaxUploadSizeRequestUrlSubPath(expectedUri);
        EXPECT_STRCASEEQ(expectedUri.c_str(), request.GetUrl().c_str());
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        return StubHttpResponse(HttpStatus::OK, "", headers);
        });

    auto result = client->GetInfo()->GetResult();
    EXPECT_TRUE(result.IsSuccess());

    auto repository = result.GetValue();
    EXPECT_EQ("https://srv.com/ws", repository.GetServerUrl());
    EXPECT_EQ("testPluginId--locationId", repository.GetId());
    EXPECT_EQ("locationId", repository.GetLocation());
    EXPECT_EQ("testPluginId", repository.GetPluginId());
    EXPECT_EQ(BeVersion(1, 2), repository.GetPluginVersion());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Mantas.Smicius    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WebApi28ResponseIsOkWithFileBackableSupported_ReturnsRepositoryInfoWithMaxUploadSize)
    {
    uint64_t maxUploadSize = rand();
    TestGetMaxUploadSize(GetHandlerPtr(), {2, 8}, StubGetMaxUploadSizeResponseBody(maxUploadSize, INSTANCE_PersistenceFileBackable), maxUploadSize);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Mantas.Smicius    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WebApi28ResponseIsOkWithStreamBackableSupported_ReturnsRepositoryInfoWithMaxUploadSize)
    {
    uint64_t maxUploadSize = rand();
    TestGetMaxUploadSize(GetHandlerPtr(), {2, 8}, StubGetMaxUploadSizeResponseBody(maxUploadSize, INSTANCE_PersistenceStreamBackable), maxUploadSize);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Mantas.Smicius    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WebApi28ResponseIsOkWithFileUploadIsNotSupported_ReturnsRepositoryInfoWithInvalidMaxUploadSize)
    {
    Utf8String responseBody = Utf8PrintfString(R"({
            "instances": []
        })");
    TestGetMaxUploadSize(GetHandlerPtr(), {2, 8}, responseBody, 0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Mantas.Smicius    09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WebApi28ServerVersion2688ResponseIsOkWithoutMaxUploadSize_ReturnsRepositoryInfoWithInvalidMaxUploadSize)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "testPluginId--locationId", StubClientInfo(), nullptr, GetHandlerPtr());

    std::map<Utf8String, Utf8String> headers {{"Mas-Server", "Bentley-WebAPI/2.8,Bentley-WSG/2.6.8.8"}};

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, [=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::OK, "", headers);
        });
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        // WSG with server version 2.6.8.8 does not support MaxUploadSize, but this mock is only to make test failed if code does not check server version
        return StubHttpResponse(HttpStatus::OK, StubGetMaxUploadSizeResponseBody(rand(), INSTANCE_PersistenceStreamBackable));
        });

    auto result = client->GetInfo()->GetResult();
    EXPECT_TRUE(result.IsSuccess());

    auto repository = result.GetValue();
    EXPECT_EQ(0, repository.GetMaxUploadSize());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WithServiceVersionAndWebApi28ResponseIsOkWithPluginVersionAndMassServerHeader_ReturnsRepositoryInfoWithPluginVersion)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", {4, 2}, "testPluginId--locationId", StubClientInfo(), nullptr, GetHandlerPtr());

    std::map<Utf8String, Utf8String> headers {{"Mas-Server", "Bentley-WebAPI/2.6,Bentley-WSG/2.6,testPluginId/1.2"}};

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi28());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        Utf8String expectedUri = "https://srv.com/ws/sv4.2/Repositories/testPluginId--locationId";
        AddMaxUploadSizeRequestUrlSubPath(expectedUri);
        EXPECT_STRCASEEQ(expectedUri.c_str(), request.GetUrl().c_str());
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        return StubHttpResponse(HttpStatus::OK, "", headers);
        });

    auto result = client->GetInfo()->GetResult();
    EXPECT_TRUE(result.IsSuccess());

    auto repository = result.GetValue();
    EXPECT_EQ("https://srv.com/ws", repository.GetServerUrl());
    EXPECT_EQ("testPluginId--locationId", repository.GetId());
    EXPECT_EQ("locationId", repository.GetLocation());
    EXPECT_EQ("testPluginId", repository.GetPluginId());
    EXPECT_EQ(BeVersion(1, 2), repository.GetPluginVersion());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WebApi28ResponseServerError_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "testPluginId--locationId", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi28());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::InternalServerError));

    auto result = client->GetInfo()->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WebApi28ResponseNotServerError_Error)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "testPluginId--locationId", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi28());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Unauthorized));

    auto result = client->GetInfo()->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    auto error = result.GetError();
    EXPECT_EQ(WSError::Status::ReceivedError, error.GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WebApi20CalledTwice_ReturnsCachedRepositoryInfo)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo--plugin", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->GetInfo()->GetResult();
    EXPECT_TRUE(result.IsSuccess());

    auto secondResult = client->GetInfo()->GetResult();
    EXPECT_TRUE(secondResult.IsSuccess());

    EXPECT_EQ(result.GetValue().GetId(), secondResult.GetValue().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetInfo_WebApi28CalledTwiceFirstCallFailed_RequestsRepositoryInfoSecondTime)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "testPluginId--locationId", StubClientInfo(), nullptr, GetHandlerPtr());
    std::map<Utf8String, Utf8String> headers {{"Mas-Server", "Bentley-WebAPI/2.6,Bentley-WSG/2.6,testPluginId/1.2"}};

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi28());
    GetHandler().ForRequest(2, StubHttpResponse(ConnectionStatus::None));
    GetHandler().ForRequest(3, StubHttpResponse(HttpStatus::OK, "", headers));

    auto result = client->GetInfo()->GetResult();
    EXPECT_FALSE(result.IsSuccess());

    auto secondResult = client->GetInfo()->GetResult();
    EXPECT_TRUE(secondResult.IsSuccess());

    auto repository = secondResult.GetValue();
    EXPECT_EQ("https://srv.com/ws", repository.GetServerUrl());
    EXPECT_EQ("testPluginId--locationId", repository.GetId());
    EXPECT_EQ("locationId", repository.GetLocation());
    EXPECT_EQ("testPluginId", repository.GetPluginId());
    EXPECT_EQ(BeVersion(1, 2), repository.GetPluginVersion());
    }

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, RegisterInfoListener_AddedListener_ListenerNotifiedWithReceivedInfo)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "testPluginId--locationId", StubClientInfo(), nullptr, GetHandlerPtr());
    auto listener = std::make_shared<MockRepositoryInfoListener>();

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    client->RegisterRepositoryInfoListener(listener);

    EXPECT_CALL(*listener, OnInfoReceived(_)).Times(1).WillOnce(Invoke([=] (WSRepositoryCR info)
        {
        EXPECT_STREQ("testPluginId--locationId", info.GetId().c_str());
        }));

    client->GetInfo()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, RegisterInfoListener_AddedListenerDeleted_ListenerNotLeakedAndNotNotified)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo--plugin", StubClientInfo(), nullptr, GetHandlerPtr());

    int listenerCallCount = 0;
    struct StubRepositoryInfoListener : public IWSRepositoryClient::IRepositoryInfoListener
        {
        int& m_listenerCallCount;
        StubRepositoryInfoListener(int& listenerCallCount) : m_listenerCallCount(listenerCallCount) {}
        void OnInfoReceived(WSRepositoryCR info)
            {
            m_listenerCallCount++;
            }
        };

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto listener = std::make_shared<StubRepositoryInfoListener>(listenerCallCount);
    client->RegisterRepositoryInfoListener(listener);
    EXPECT_EQ(0, listenerCallCount);

    client->GetInfo()->Wait();
    EXPECT_EQ(1, listenerCallCount);

    listener = nullptr;
    client->GetInfo()->Wait();

    EXPECT_EQ(1, listenerCallCount);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, UnregisterInfoListener_ExistingListener_ListenerNotNotified)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo--plugin", StubClientInfo(), nullptr, GetHandlerPtr());
    auto listener = std::make_shared<MockRepositoryInfoListener>();

    EXPECT_CALL(*listener, OnInfoReceived(_)).Times(0);

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    client->RegisterRepositoryInfoListener(listener);
    client->UnregisterRepositoryInfoListener(listener);

    client->GetInfo()->Wait();
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, VerifyAccess_CredentialsPassed_SendsSameCredentials)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2)
        .ForRequest(1, StubWSInfoHttpResponseWebApi20())
        .ForRequest(2, [] (Http::RequestCR request)
        {
        EXPECT_EQ(Credentials("TestUser", "TestPassword"), request.GetCredentials());
        EXPECT_STREQ(nullptr, request.GetHeaders().GetValue(HEADER_MasConnectionInfo));
        return StubHttpResponse();
        });

    client->SetCredentials(Credentials("TestUser", "TestPassword"));
    client->VerifyAccess()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, VerifyAccess_CredentialsAndAutheTypeWindowsPassed_SendsSameCredentialsWithAuthType)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2)
        .ForRequest(1, StubWSInfoHttpResponseWebApi20())
        .ForRequest(2, [] (Http::RequestCR request)
        {
        EXPECT_EQ(Credentials("TestUser", "TestPassword"), request.GetCredentials());
        EXPECT_STREQ("CredentialType=Windows", request.GetHeaders().GetValue(HEADER_MasConnectionInfo));
        return StubHttpResponse();
        });

    client->SetCredentials(Credentials("TestUser", "TestPassword"), IWSRepositoryClient::AuthenticationType::Windows);
    client->VerifyAccess()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, VerifyAccess_ResponseWithClassNotFound_ReturnsSuccess)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2)
        .ForRequest(1, StubWSInfoHttpResponseWebApi20())
        .ForRequest(2, StubWSErrorHttpResponse(HttpStatus::NotFound, "ClassNotFound"));

    auto result = client->VerifyAccess()->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, VerifyAccess_ResponseWithSchemaNotFound_ReturnsSuccess)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2)
        .ForRequest(1, StubWSInfoHttpResponseWebApi20())
        .ForRequest(2, StubWSErrorHttpResponse(HttpStatus::NotFound, "SchemaNotFound"));

    auto result = client->VerifyAccess()->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, GetPersistenceProviderId_SetWithCustom_RetursCustom)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    client->Config().SetPersistenceProviderId("id");
    EXPECT_STREQ("id", client->Config().GetPersistenceProviderId().c_str());
    }

#ifdef USE_GTEST
struct WSRepositoryClientTests_RepositoryIds : TestWithParam<vector<Utf8CP>> {};
INSTANTIATE_TEST_CASE_P(WithProviderId, WSRepositoryClientTests_RepositoryIds, ValuesIn(vector<vector<Utf8CP>>{
    {"A--B", "A"},
    {"A--B--C", "A"},
    {"A--B-C-D-E", "A"}
}));
INSTANTIATE_TEST_CASE_P(NoProviderId, WSRepositoryClientTests_RepositoryIds, ValuesIn(vector<vector<Utf8CP>>{
    {"Foo", ""},
    {"Foo-Boo", ""}
}));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(WSRepositoryClientTests_RepositoryIds, GetPersistenceProviderId_RepositoryIdNotSet_RepositoryIdIsPluginId)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", GetParam()[0], StubClientInfo(), nullptr, nullptr);
    EXPECT_STREQ(GetParam()[1], client->Config().GetPersistenceProviderId().c_str());
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV2WithSchemaProviderAndQueryNavigationRoot_DoesNotCallSchemaProvider)
    {
    auto schemaProvider = std::make_shared<MockWSSchemaProvider>();
    EXPECT_CALL(*schemaProvider, GetSchema(_)).Times(0);

    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), schemaProvider, GetHandlerPtr());

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "TestId"});

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK, instances.ToJsonWebApiV2()));

    auto response = client->SendGetChildrenRequest(ObjectId())->GetResult();

    ASSERT_TRUE(response.IsSuccess());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "TestId"), (*response.GetValue().GetInstances().begin()).GetObjectId());
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV2AndNavigationRoot_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/Navigation/NavNode", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetChildrenRequest(ObjectId())->Wait();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV2AndPropertiesSpecifiedToSelect_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/Navigation/NavNode?$select=Boo,Foo", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    bset<Utf8String> properties;
    properties.insert("Boo");
    properties.insert("Foo");

    client->SendGetChildrenRequest(ObjectId(), properties)->Wait();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV2AndSpecificNavNode_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/Navigation/NavNode/Foo/NavNode", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetChildrenRequest(ObjectId("Navigation.NavNode", "Foo"))->Wait();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV2AndResponseContainsInstance_SucceedsAndParsesInstance)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    Http::Response response = StubHttpResponse(HttpStatus::OK, instances.ToJsonWebApiV2());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, response);

    auto result = client->SendGetChildrenRequest(ObjectId())->GetResult();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), (*result.GetValue().GetInstances().begin()).GetObjectId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApi25_SendsRequestsWithoutMasRequestId)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi25());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(nullptr, request.GetHeaders().GetValue(HEADER_MasRequestId));
        return StubHttpResponse(HttpStatus::OK);
        });

    client->SendGetChildrenRequest(ObjectId())->GetResult();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApi27_SendsRequestsWithMasRequestId)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi27());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        auto actualActivityId = request.GetHeaders().GetValue(HEADER_MasRequestId);
        EXPECT_FALSE(Utf8String::IsNullOrEmpty(actualActivityId));
        return StubHttpResponse(HttpStatus::OK);
        });

    client->SendGetChildrenRequest(ObjectId())->GetResult();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_PerformedTwiceWithWebApi27_SendsAllRequestsWithDifferentMasRequestId)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    set<Utf8CP> requestIds;

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi27());
    GetHandler().ForRequest(2, [=, &requestIds] (Http::RequestCR request)
        {
        auto actualActivityId = request.GetHeaders().GetValue(HEADER_MasRequestId);
        EXPECT_NE(nullptr, actualActivityId);
        requestIds.insert(actualActivityId);
        return StubHttpResponse(HttpStatus::OK);
        });

    GetHandler().ForRequest(3, [=, &requestIds] (Http::RequestCR request)
        {
        auto actualActivityId = request.GetHeaders().GetValue(HEADER_MasRequestId);
        EXPECT_NE(nullptr, actualActivityId);
        requestIds.insert(actualActivityId);
        return StubHttpResponse(HttpStatus::OK);
        });

    client->SendGetChildrenRequest(ObjectId())->GetResult();
    client->SendGetChildrenRequest(ObjectId())->GetResult();

    auto uniqueRequestIds = requestIds.size();
    EXPECT_EQ(2, uniqueRequestIds);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV2AndEmptyObjectId_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    BeFileName fileName = StubFilePath();
    auto result = client->SendGetFileRequest(ObjectId(), fileName)->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV2AndEmptyFilePath_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(0);

    auto result = client->SendGetFileRequest(StubObjectId(), BeFileName())->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV2_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass/TestId/$file", request.GetUrl().c_str());
        EXPECT_EQ(nullptr, request.GetHeaders().GetValue("Mas-Allow-Redirect"));
        WriteStringToHttpBody("TestResponseBody", request.GetResponseBody());
        return StubHttpResponse(HttpStatus::OK);
        });

    BeFileName filePath = StubFilePath();
    auto response = client->SendGetFileRequest({"TestSchema", "TestClass", "TestId"}, filePath)->GetResult();
    EXPECT_TRUE(response.IsSuccess());
    EXPECT_EQ("TestResponseBody", SimpleReadFile(filePath));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV24_SendsCorrectUrlAndAllowRedirectHeader)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.4/Repositories/foo/TestSchema/TestClass/TestId/$file", request.GetUrl().c_str());
        EXPECT_STREQ("true", request.GetHeaders().GetValue("Mas-Allow-Redirect"));
        WriteStringToHttpBody("TestResponseBody", request.GetResponseBody());
        return StubHttpResponse(HttpStatus::OK);
        });

    BeFileName filePath = StubFilePath();
    auto response = client->SendGetFileRequest({"TestSchema", "TestClass", "TestId"}, filePath)->GetResult();
    EXPECT_TRUE(response.IsSuccess());
    EXPECT_EQ("TestResponseBody", SimpleReadFile(filePath));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV24AndAzureRedirectReceived_DownloadsFileFromExternalLocation)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
                {HEADER_Location, "https://foo.com/boo"},
                {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"}});
        });
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://foo.com/boo", request.GetUrl().c_str());
        WriteStringToHttpBody("TestResponseBody", request.GetResponseBody());
        return StubHttpResponse(HttpStatus::OK);
        });

    BeFileName filePath = StubFilePath();
    auto response = client->SendGetFileRequest(StubObjectId(), filePath)->GetResult();
    EXPECT_TRUE(response.IsSuccess());
    EXPECT_EQ("TestResponseBody", SimpleReadFile(filePath));
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequestForStream_WebApiV24AndAzureRedirectReceived_DownloadsFileFromExternalLocation)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());
    auto responseBody = Http::HttpByteStreamBody::Create();

    EXPECT_REQUEST_COUNT(GetHandler(), 3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
                {HEADER_Location, "https://foo.com/boo"},
                                {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"}});
        });
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://foo.com/boo", request.GetUrl().c_str());
        WriteStringToHttpBody("TestResponseBody", request.GetResponseBody());
        return StubHttpResponse(HttpStatus::OK);
        });

    auto response = client->SendGetFileRequest(StubObjectId(), responseBody)->GetResult();
    EXPECT_TRUE(response.IsSuccess());

    EXPECT_STREQ("TestResponseBody", SimpleReadByteStream(responseBody->GetByteStream()).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV24AndUnknownRedirectReceived_DownloadsAnyway)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
                {HEADER_Location, "https://foo.com/boo"},
                {HEADER_MasFileAccessUrlType, "SomethingNotSupportedHere"}});
        });
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://foo.com/boo", request.GetUrl().c_str());
        WriteStringToHttpBody("TestResponseBody", request.GetResponseBody());
        return StubHttpResponse(HttpStatus::OK);
        });

    BeFileName filePath = StubFilePath();
    auto response = client->SendGetFileRequest(StubObjectId(), filePath)->GetResult();
    EXPECT_TRUE(response.IsSuccess());
    EXPECT_EQ("TestResponseBody", SimpleReadFile(filePath));
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequestForStream_WebApiV24_SendsCorrectUrlAndAllowRedirectHeader)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());
    auto responseBody = Http::HttpByteStreamBody::Create();

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.4/Repositories/foo/TestSchema/TestClass/TestId/$file", request.GetUrl().c_str());
        EXPECT_STREQ("true", request.GetHeaders().GetValue("Mas-Allow-Redirect"));
        WriteStringToHttpBody("TestResponseBody", request.GetResponseBody());
        return StubHttpResponse(HttpStatus::OK);
        });

    auto response = client->SendGetFileRequest({"TestSchema", "TestClass", "TestId"}, responseBody)->GetResult();
    EXPECT_TRUE(response.IsSuccess());

    EXPECT_STREQ("TestResponseBody", SimpleReadByteStream(responseBody->GetByteStream()).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequestForStream_WebApiV24AndNullptrResponseBody_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::OK);
        });

    auto response = client->SendGetFileRequest({"TestSchema", "TestClass", "TestId"}, nullptr)->GetResult();
    EXPECT_TRUE(response.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV24AndUnknownRedirectStatusReceived_Error)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::Found, "", {
                {HEADER_Location, "https://foo.com/boo"},
                {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"}});
        });

    BeFileName filePath = StubFilePath();
    auto response = client->SendGetFileRequest(StubObjectId(), filePath)->GetResult();
    EXPECT_FALSE(response.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV2ETagSet_SendsAndReceivesETag)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("RequestETag", request.GetHeaders().GetIfNoneMatch());
        return StubHttpResponse(HttpStatus::OK, "", {{"ETag", "ResponseETag"}});
        });

    auto result = client->SendGetFileRequest(StubObjectId(), StubFilePath(), "RequestETag")->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("ResponseETag", result.GetValue().GetETag());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV24ETagSetAndReceivedAzureRedirect_SendsAndReceivesETag)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("RequestETag", request.GetHeaders().GetIfNoneMatch());
        return StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
                {HEADER_Location, "https://foo.com/boo"},
                {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"}});
        });
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://foo.com/boo", request.GetUrl().c_str());
        EXPECT_STREQ("RequestETag", request.GetHeaders().GetIfNoneMatch());
        return StubHttpResponse(HttpStatus::OK, "", {{"ETag", "ResponseETag"}});
        });

    auto result = client->SendGetFileRequest(StubObjectId(), StubFilePath(), "RequestETag")->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_STREQ("ResponseETag", result.GetValue().GetETag().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV24AndReceivedAzureRedirectAndAzureReturnsError_ErrorIsParsedAndReturned)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
                {HEADER_Location, "https://foo.com/boo"},
                {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"}});
        });
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://foo.com/boo", request.GetUrl().c_str());
        return StubHttpResponse(HttpStatus::NotFound,
R"(<?xml version="1.0" encoding="utf-8"?><Error><Code>BlobNotFound</Code><Message>TestMessage</Message></Error>)",
{{"Content-Type", "application/xml"}}); 
        });

    auto result = client->SendGetFileRequest(StubObjectId(), StubFilePath())->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Id::FileNotFound, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F (WSRepositoryClientTests, SendGetFileRequest_WebApiV2ButNoSchemaInObjectId_ReturnsError)
//   {
//    auto client = WSRepositoryClient::Create ("https://srv.com/ws", "foo", StubClientInfo(), GetHandlerPtr ());
//
//    GetHandler ().ExpectRequests (1);
//    GetHandler ().ForRequest (1, StubWSInfoHttpResponseWebApi20 ());
//
//    auto response = client->SendGetFileRequest ({ "TestClass", "TestId" }, BeFileName ("testFile"))->GetResult ();
//    EXPECT_FALSE (response.IsSuccess ());
//    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetObjectRequest_WebApiV2AndInvalidObjectId_ReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->SendGetObjectRequest(ObjectId("", "Foo", ""))->GetResult();
    EXPECT_EQ(result.GetError().GetId(), WSError::Id::NotSupported);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetObjectRequest_WithEmptyServiceVersionAndWebApiV2_SendsWebApiVersionUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", {0, 0}, "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/testSchema/testClass/testId", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetObjectRequest({"testSchema", "testClass", "testId"})->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetObjectRequest_WithServiceVersionAndWebApiV2_SendsServiceVersionUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", {4, 2}, "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/sv4.2/Repositories/foo/testSchema/testClass/testId", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetObjectRequest({"testSchema", "testClass", "testId"})->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetObjectRequest_WebApiV2_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/testSchema/testClass/testId", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetObjectRequest({"testSchema", "testClass", "testId"})->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV2_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/testSchema/class1,class2?$select=testSelect", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    WSQuery query("testSchema", set<Utf8String> {"class1", "class2"});
    query.SetSelect("testSelect");

    client->SendQueryRequest(query)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV21_ParsesInstanceETagAndAddsQuotes)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        auto json = R"({"instances" :
                [{
                "instanceId" : "Foo",
                "className" : "Foo",
                "schemaName" : "Foo",
                "eTag" : "TestEtagA",
                "properties" : {},
                "relationshipInstances": [{
                    "instanceId" : "Foo",
                    "className" : "Foo",
                    "schemaName" : "Foo",
                    "eTag" : "TestEtagB",
                    "direction" : "forward",
                    "relatedInstance":
                        {
                        "instanceId" : "Foo",
                        "className" : "Foo",
                        "schemaName" : "Foo",
                        "properties" : {}
                        }
                    }]
                }]
            })";
        return StubHttpResponse(HttpStatus::OK, json);
        });

    auto result = client->SendQueryRequest(WSQuery("Foo", "Foo"))->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    auto& instances = result.GetValue().GetInstances();
    EXPECT_EQ("\"TestEtagA\"", (*instances.begin()).GetETag());
    EXPECT_EQ("\"TestEtagB\"", (*(*instances.begin()).GetRelationshipInstances().begin()).GetETag());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV22_ParsesInstanceETagDirectly)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi22());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        auto json = R"({"instances" :
                [{
                "instanceId" : "Foo",
                "className" : "Foo",
                "schemaName" : "Foo",
                "eTag" : "TestEtagA",
                "properties" : {},
                "relationshipInstances": [{
                    "instanceId" : "Foo",
                    "className" : "Foo",
                    "schemaName" : "Foo",
                    "eTag" : "TestEtagB",
                    "direction" : "forward",
                    "relatedInstance":
                        {
                        "instanceId" : "Foo",
                        "className" : "Foo",
                        "schemaName" : "Foo",
                        "properties" : {}
                        }
                    }]
                }]
            })";
        return StubHttpResponse(HttpStatus::OK, json);
        });

    auto result = client->SendQueryRequest(WSQuery("Foo", "Foo"))->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    auto& instances = result.GetValue().GetInstances();
    EXPECT_EQ("TestEtagA", (*instances.begin()).GetETag());
    EXPECT_EQ("TestEtagB", (*(*instances.begin()).GetRelationshipInstances().begin()).GetETag());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV24SkipTokenSuppliedAndSentBack_IgnoresSkipTokens)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ(nullptr, request.GetHeaders().GetValue("SkipToken"));
        return StubHttpResponse(HttpStatus::OK, StubInstances().ToJsonWebApiV2(),
        {{"SkipToken", "ServerSkipToken"}, {"Content-Type", "application/json"}});
        });

    auto result = client->SendQueryRequest(StubWSQuery(), nullptr, "SomeSkipToken")->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue().IsFinal());
    EXPECT_EQ("", result.GetValue().GetSkipToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV25AndHttpResponseHasSkipTokenButItWasNotSent_DoesNotAddSkipTokenToResponse)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi25());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK, StubInstances().ToJsonWebApiV2(), {{"SkipToken", "ServerSkipToken"}}));

    auto result = client->SendQueryRequest(StubWSQuery(), nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue().IsFinal());
    EXPECT_EQ("", result.GetValue().GetSkipToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApi25SkipTokenEmpty_DoesNotSendSkipToken)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi25());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(nullptr, request.GetHeaders().GetValue("SkipToken"));
        return StubHttpResponse();
        });

    client->SendQueryRequest(StubWSQuery(), nullptr, nullptr)->GetResult();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV25SkipTokenSupplied_SendsSkipToken)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi25());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("SomeSkipToken", request.GetHeaders().GetValue("SkipToken"));
        return StubHttpResponse();
        });

    client->SendQueryRequest(StubWSQuery(), nullptr, "SomeSkipToken")->GetResult();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV25AndHttpResponseHasSkipToken_AddsSkipTokenToResponse)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi25());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK, StubInstances().ToJsonWebApiV2(), {{"SkipToken", "ServerSkipToken"}}));

    auto result = client->SendQueryRequest(StubWSQuery(), nullptr, "SomeSkipToken")->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().IsFinal());
    EXPECT_EQ("ServerSkipToken", result.GetValue().GetSkipToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV23_CapsWebApiToV23)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 3}));
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v2.3/Repositories/foo/TestSchema/TestClass", request.GetUrl());
        return StubHttpResponse();
        });

    client->SendQueryRequest(StubWSQuery(), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV24_CapsWebApiToV24)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 4}));
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v2.4/Repositories/foo/TestSchema/TestClass", request.GetUrl());
        return StubHttpResponse();
        });

    client->SendQueryRequest(StubWSQuery(), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV25_CapsWebApiToV25)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 5}));
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v2.5/Repositories/foo/TestSchema/TestClass", request.GetUrl());
        return StubHttpResponse();
        });

    client->SendQueryRequest(StubWSQuery(), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV26_CapsWebApiToV26)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 6}));
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v2.6/Repositories/foo/TestSchema/TestClass", request.GetUrl());
        return StubHttpResponse();
        });

    client->SendQueryRequest(StubWSQuery(), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV27_CapsWebApiToV27)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 7}));
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v2.7/Repositories/foo/TestSchema/TestClass", request.GetUrl());
        return StubHttpResponse();
        });

    client->SendQueryRequest(StubWSQuery(), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV28_CapsWebApiToV28)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 8}));
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v2.8/Repositories/foo/TestSchema/TestClass", request.GetUrl());
        return StubHttpResponse();
        });

    client->SendQueryRequest(StubWSQuery(), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV29_CapsWebApiToV28)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 9}));
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v2.8/Repositories/foo/TestSchema/TestClass", request.GetUrl());
        return StubHttpResponse();
        });

    client->SendQueryRequest(StubWSQuery(), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV23MaxUrlLenghtNotExceeded_CorrectRequestCreated)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());
    Utf8String expectedUrl = "https://srv.com/ws/v2.3/Repositories/foo/TestSchema/TestClass?$filter=filter";

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 3}));
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
        EXPECT_EQ("GET", request.GetMethod());
        EXPECT_EQ(NULL, request.GetHeaders().GetContentType());
        EXPECT_TRUE(request.GetRequestBody().IsNull());
        return StubHttpResponse();
        });

    auto query = StubWSQuery();
    Utf8String filter = "filter";
    query.SetFilter(filter);
    
    size_t maxLenght = expectedUrl.length();
    client->Config().SetMaxUrlLength(maxLenght);

    client->SendQueryRequest(query, nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV23MaxUrlLenghtExceeded_CorrectRequestCreated)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());
    Utf8String expectedUrl = "https://srv.com/ws/v2.3/Repositories/foo/TestSchema/TestClass?$filter=filter";

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 3}));
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
        EXPECT_EQ("GET", request.GetMethod());
        EXPECT_EQ(NULL, request.GetHeaders().GetContentType());
        EXPECT_TRUE(request.GetRequestBody().IsNull());
        return StubHttpResponse();
        });

    auto query = StubWSQuery();
    Utf8String filter = "filter";
    query.SetFilter(filter);

    size_t maxLenght = expectedUrl.length() - 1;
    client->Config().SetMaxUrlLength(maxLenght);
    //Excpecting to have warning about url length
    BeTest::SetFailOnAssert(false);
    client->SendQueryRequest(query, nullptr, nullptr)->Wait();
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV24MaxUrlLenghtNotExceeded_CorrectRequestCreated)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());
    Utf8String expectedUrl = "https://srv.com/ws/v2.4/Repositories/foo/TestSchema/TestClass?$filter=filter";

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 4}));
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(expectedUrl, request.GetUrl());
        EXPECT_EQ("GET", request.GetMethod());
        EXPECT_EQ(NULL, request.GetHeaders().GetContentType());
        EXPECT_TRUE(request.GetRequestBody().IsNull());
        return StubHttpResponse();
        });

    auto query = StubWSQuery();
    Utf8String filter = "filter";
    query.SetFilter(filter);

    size_t maxLenght = expectedUrl.length();
    client->Config().SetMaxUrlLength(maxLenght);

    client->SendQueryRequest(query, nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV24MaxUrlLenghtExceeded_CorrectRequestCreated)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 4}));
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v2.4/Repositories/foo/TestSchema/TestClass/$query", request.GetUrl());
        EXPECT_EQ("POST", request.GetMethod());
        EXPECT_STREQ(REQUESTHEADER_ContentType_ApplicationJson, request.GetHeaders().GetContentType());
        EXPECT_STREQ("$filter=filter", request.GetRequestBody()->AsString().c_str());
        return StubHttpResponse();
        });

    auto query = StubWSQuery();
    Utf8String filter = "filter";
    query.SetFilter(filter);

    Utf8String expectedUrl = "https://srv.com/ws/v2.4/Repositories/foo/TestSchema/TestClass?$filter=filter";
    size_t maxLenght = expectedUrl.length() - 1;
    client->Config().SetMaxUrlLength(maxLenght);

    client->SendQueryRequest(query, nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2WithInvalidRelatedObjectIdAndCorrectJson_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "properties": {}
                }
            })");
    ObjectId relatedObject("TestSchema", "TestClass");

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    BeTest::SetFailOnAssert(false);
    auto result = client->SendCreateObjectRequest(relatedObject, objectCreationJson)->GetResult();
    BeTest::SetFailOnAssert(true);
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV25WithRelatedObjectId_UrlContainsRelatedObjectIdAndNewObjectSchemaAndClass)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName" : "TargetObjectSchema",
                "className" : "TargetObjectClass",
                "properties" : {}
                }
            })");

    auto relatedObject = ObjectId("RelatedObjectSchema", "RelatedObjectClass", "RelatedObjectId");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi25());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.5/Repositories/foo/RelatedObjectSchema/RelatedObjectClass/RelatedObjectId/TargetObjectSchema.TargetObjectClass", request.GetUrl().c_str());
        EXPECT_EQ(objectCreationJson, ToJson(request.GetRequestBody()->AsString()));
        return StubHttpResponse(ConnectionStatus::OK);
        });
    client->SendCreateObjectRequest(relatedObject, objectCreationJson)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV25WithRelatedObjectIdSameSchema_UrlContainsRelatedObjectIdAndNewObjectClass)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName" : "RelatedObjectSchema",
                "className" : "TargetObjectClass",
                "properties" : {}
                }
            })");

    auto relatedObject = ObjectId("RelatedObjectSchema", "RelatedObjectClass", "RelatedObjectId");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi25());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.5/Repositories/foo/RelatedObjectSchema/RelatedObjectClass/RelatedObjectId/TargetObjectClass", request.GetUrl().c_str());
        EXPECT_EQ(objectCreationJson, ToJson(request.GetRequestBody()->AsString()));
        return StubHttpResponse(HttpStatus::Created);
        });
    client->SendCreateObjectRequest(relatedObject, objectCreationJson)->Then([=] (WSCreateObjectResult result)
        {
        EXPECT_TRUE(result.IsSuccess());
        })->Wait();
    }
   
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2WithJsonWithSchemaAndClass_UsesSchemaAndClassInURLForPostRequestWithSameBody)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "properties": {}
                }
            })");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass", request.GetUrl().c_str());
        EXPECT_EQ(objectCreationJson, Json::Reader::DoParse(request.GetRequestBody()->AsString()));
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(objectCreationJson)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2AndRootInstanceContainsId_IdAddedToUrlToAllowRelatedInstanceModifications)
    {
    // TODO: unify WebApi 1 and 2 to have SendChangeRequest that would accept changeset. WebApi 1 would be limited to single instance changes.
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "instanceId" : "TestId",
                "properties": {}
                }
            })");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass/TestId", request.GetUrl().c_str());
        EXPECT_EQ(objectCreationJson, Json::Reader::DoParse(request.GetRequestBody()->AsString()));
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(objectCreationJson)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2AndRootInstanceContainsIdAndChangeStateNew_CreateRequestIsSentWithoutIdInUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "instanceId" : "TestId",
                "changeState" : "new",
                "properties": {}
                }
            })");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass", request.GetUrl().c_str());
        EXPECT_EQ(objectCreationJson, Json::Reader::DoParse(request.GetRequestBody()->AsString()));
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(objectCreationJson)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2AndRootInstanceContainsIdAndChangeStateNewButRelatedObjectExists_CreateRequestIsSentWithRelatedObjectId)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName" : "TestSchema",
                "className" : "TestClass",
                "instanceId" : "TestId",
                "changeState" : "new",
                "properties": {}
                }
            })");
    auto relatedObject = ObjectId("A", "B", "C");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/A/B/C/TestSchema.TestClass", request.GetUrl().c_str());
        EXPECT_EQ(objectCreationJson, Json::Reader::DoParse(request.GetRequestBody()->AsString()));
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(relatedObject, objectCreationJson)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2WithoutIdAndResponseStatusCreated_PassesResponseJsonAsObject)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value responseObject = ToJson(R"({ "testMember" : "testValue" })");
    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Created, responseObject.toStyledString()));

    Json::Value creationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName": "TestSchema",
                "className": "TestClass",
                "properties": {}
                }
            })");
    auto response = client->SendCreateObjectRequest(creationJson)->GetResult();

    ASSERT_TRUE(response.IsSuccess());
    Json::Value jsonBody;
    response.GetValue().GetJson(jsonBody);
    EXPECT_EQ(responseObject, jsonBody);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2WithoutIdAndResponseStatusOK_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value responseObject = ToJson(R"({ "testMember" : "testValue" })");
    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK, responseObject.toStyledString()));

    Json::Value creationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName": "TestSchema",
                "className": "TestClass",
                "properties": {}
                }
            })");
    auto response = client->SendCreateObjectRequest(creationJson)->GetResult();

    ASSERT_TRUE(response.IsSuccess());
    Json::Value jsonBody;
    response.GetValue().GetJson(jsonBody);
    EXPECT_EQ(responseObject, jsonBody);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2WithIdAndResponseStatusOK_PassesResponseJsonAsObject)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value responseObject = ToJson(R"({ "testMember" : "testValue" })");
    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK, responseObject.toStyledString()));

    Json::Value creationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName": "TestSchema",
                "className": "TestClass",
                "instanceId": "TestId",
                "properties": {}
                }
            })");
    auto response = client->SendCreateObjectRequest(creationJson)->GetResult();

    ASSERT_TRUE(response.IsSuccess());
    Json::Value jsonBody;
    response.GetValue().GetJson(jsonBody);
    EXPECT_EQ(responseObject, jsonBody);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2WithIdAndResponseStatusCreated_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value responseObject = ToJson(R"({ "testMember" : "testValue" })");
    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Created, responseObject.toStyledString()));

    Json::Value creationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName": "TestSchema",
                "className": "TestClass",
                "instanceId": "TestId",
                "properties": {}
                }
            })");
    auto response = client->SendCreateObjectRequest(creationJson)->GetResult();

    ASSERT_TRUE(response.IsSuccess());
    Json::Value jsonBody;
    response.GetValue().GetJson(jsonBody);
    EXPECT_EQ(responseObject, jsonBody);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2AndFileETagSentBack_ReturnsNewFileETag)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::ResumeIncomplete));
    GetHandler().ForRequest(3, StubHttpResponse(HttpStatus::Created, "", {{HEADER_MasFileETag, "NewTag"}}));

    auto result = client->SendCreateObjectRequest(StubWSObjectCreationJson(), StubFile())->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ("NewTag", result.GetValue().GetFileETag());
    }

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2WithFilePath_AddsFileNameToContentDisposition)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    auto filePath = StubFile();
    auto fileName = Utf8String(filePath.GetFileNameAndExtension());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_THAT(request.GetHeaders().GetContentDisposition(), HasSubstr(fileName.c_str()));
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(StubWSObjectCreationJson(), filePath)->Wait();
    }

#endif

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_EnableJobsWebApiV2JobSucceedsResponseSucceeds_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "schemaName" : "TargetObjectSchema",
                "className" : "TargetObjectClass",
                "properties" : {}
                }
            })");

    auto relatedObject = ObjectId("RelatedObjectSchema", "RelatedObjectClass", "RelatedObjectId");

    Expect4_jSrS(GetHandler(), HttpStatus::Created);

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendCreateObjectRequestWithOptions(relatedObject, objectCreationJson, BeFileName(), nullptr, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_WebApiV20_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->SendChangesetRequest(HttpStringBody::Create("TestChangeset"), nullptr, nullptr)->GetResult();
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_WebApiV21_SendsRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.1/Repositories/foo/$changeset", request.GetUrl().c_str());
        EXPECT_STREQ("TestChangeset", request.GetRequestBody()->AsString().c_str());
        return StubHttpResponse();
        });

    client->SendChangesetRequest(HttpStringBody::Create("TestChangeset"), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_CompressionIsNotEnabled_RequestCompressionDisabled)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_FALSE(request.GetCompressionOptions().IsRequestCompressionEnabled());
        return StubHttpResponse();
        });

    EXPECT_FALSE(client->Config().GetCompressionOptions().IsRequestCompressionEnabled());
    client->SendChangesetRequest(HttpStringBody::Create(""), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_EnableCompression_RequestCompressionEnabled)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_TRUE(request.GetCompressionOptions().IsRequestCompressionEnabled());
        EXPECT_EQ(1111, request.GetCompressionOptions().GetMinimumSizeToCompress());
        return StubHttpResponse();
        });

    EXPECT_FALSE(client->Config().GetCompressionOptions().IsRequestCompressionEnabled());
    CompressionOptions options;
    options.EnableRequestCompression(true, 1111);
    client->Config().SetCompressionOptions(options);
    client->SendChangesetRequest(HttpStringBody::Create(""), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_RequestOptionsNotIncluded_RequestTimeOutsAreDefaults)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(IWSRepositoryClient::Timeout::Connection::Default, request.GetConnectionTimeoutSeconds());
        EXPECT_EQ(IWSRepositoryClient::Timeout::Transfer::Upload, request.GetTransferTimeoutSeconds());
        return StubHttpResponse();
        });

    client->SendChangesetRequest(HttpStringBody::Create(""), nullptr, nullptr)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_TransferTimeOutSetViaRequestOptions_RequestTrasferTimeOutIsSet)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(IWSRepositoryClient::Timeout::Connection::Default, request.GetConnectionTimeoutSeconds());
        EXPECT_EQ(1111, request.GetTransferTimeoutSeconds());
        return StubHttpResponse();
        });

    auto options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->SetTransferTimeOut(1111);
    client->SendChangesetRequestWithOptions(HttpStringBody::Create(""), nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_TransferTimeOutSetViaDefaultRequestOptions_RequestTrasferTimeOutIsSet)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());
    auto options = std::make_shared<WSRepositoryClient::RequestOptions>();

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(IWSRepositoryClient::Timeout::Connection::Default, request.GetConnectionTimeoutSeconds());
        EXPECT_EQ(options->GetTransferTimeOut(), request.GetTransferTimeoutSeconds());
        EXPECT_EQ(IWSRepositoryClient::Timeout::Transfer::Default, options->GetTransferTimeOut());
        return StubHttpResponse();
        });

    client->SendChangesetRequestWithOptions(HttpStringBody::Create(""), nullptr, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_WebApiV21AndReceives201_Error)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Created, "{}"));

    auto result = client->SendChangesetRequest(HttpStringBody::Create("TestChangeset"), nullptr, nullptr)->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_WebApiV21AndReceives200_SuccessAndReturnsBody)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK, "TestChangesetResponse"));

    auto result = client->SendChangesetRequest(HttpStringBody::Create("TestChangeset"), nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("TestChangesetResponse", result.GetValue()->AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_EnableJobsWebApiV2JobSucceedsResponseSucceeds_SuccessAndReturnsBody)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Expect4_jSrS(GetHandler(), HttpStatus::OK);

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendChangesetRequestWithOptions(HttpStringBody::Create("TestChangeset"), nullptr, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("Good Content", result.GetValue()->AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV2AndInvalidObjectId_ReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->SendUpdateObjectRequest(ObjectId("", "Foo", ""), ToJson(R"({"TestProperty" : "TestValue" })"))->GetResult();
    EXPECT_EQ(result.GetError().GetId(), WSError::Id::NotSupported);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV2_SendsPostRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass/TestId", request.GetUrl().c_str());
        Json::Value expectedBodyJson = ToJson(
            R"( {
                "instance" :
                    {
                    "schemaName" : "TestSchema",
                    "className" : "TestClass",
                    "instanceId" : "TestId",
                    "properties" :
                        {
                        "TestProperty" : "TestValue"
                        }
                    }
                })");
        EXPECT_EQ(expectedBodyJson, Json::Reader::DoParse(request.GetRequestBody()->AsString()));
        return StubHttpResponse();
        });

    client->SendUpdateObjectRequest({"TestSchema.TestClass", "TestId"}, ToJson(R"({"TestProperty" : "TestValue" })"))->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV2AndETagPassed_SendsRequestWithoutIfMatch)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(nullptr, request.GetHeaders().GetIfMatch());
        return StubHttpResponse();
        });

    client->SendUpdateObjectRequest({"A.B", "C"}, Json::objectValue, "TestETag")->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV2ResponseIsOK_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK));

    auto result = client->SendUpdateObjectRequest(StubObjectId(), Json::objectValue)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV23WithFile_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi23());

    BeTest::SetFailOnAssert(false);
    auto result = client->SendUpdateObjectRequest(StubObjectId(), Json::objectValue, nullptr, StubFilePath())->GetResult();
    BeTest::SetFailOnAssert(true);

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV24WithFilePath_AddsFileNameToContentDisposition)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    auto filePath = StubFile();
    auto fileName = Utf8String(filePath.GetFileNameAndExtension());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_THAT(request.GetHeaders().GetContentDisposition(), HasSubstr(fileName.c_str()));
        return StubHttpResponse();
        });

    client->SendUpdateObjectRequest(StubObjectId(), Json::objectValue, nullptr, filePath)->Wait();
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV2AndFileETagSentBack_ReturnsNewFileETag)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::ResumeIncomplete));
    GetHandler().ForRequest(3, StubHttpResponse(HttpStatus::OK, "", {{HEADER_MasFileETag, "NewTag"}}));

    auto result = client->SendUpdateObjectRequest(StubObjectId(), Json::objectValue, nullptr, StubFile())->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ("NewTag", result.GetValue().GetFileETag());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_EnableJobsWebApiV2JobSucceedsResponseSucceeds_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Expect4_jSrS(GetHandler(), HttpStatus::OK);

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendUpdateObjectRequestWithOptions(StubObjectId(), Json::objectValue, nullptr, BeFileName(), nullptr, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_WebApiV2AndInvalidObjectId_ReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->SendDeleteObjectRequest(ObjectId("", "Foo", ""))->GetResult();
    EXPECT_EQ(result.GetError().GetId(), WSError::Id::NotSupported);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_WebApiV2_SendsDeleteRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("DELETE", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass/TestId", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendDeleteObjectRequest({"TestSchema.TestClass", "TestId"})->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_WebApiV2ResponseIsOK_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK));

    auto result = client->SendDeleteObjectRequest({"TestSchema.TestClass", "TestId"})->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV2AndInvalidObjectId_ReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->SendUpdateFileRequest(ObjectId("", "Foo", ""), StubFile())->GetResult();
    EXPECT_EQ(result.GetError().GetId(), WSError::Id::NotSupported);
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsWebApiV2JobNotSupported_SendsSimpleDeleteRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("DELETE", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass/TestId", request.GetUrl().c_str());
        return StubHttpResponse(ConnectionStatus::OK);
        });

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    client->SendDeleteObjectRequestWithOptions({"TestSchema.TestClass", "TestId"}, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsWebApiV2JobNotSupportedResponseIsOK_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK));

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendDeleteObjectRequestWithOptions({"TestSchema.TestClass", "TestId"}, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsWebApiV2ResponseSendsJobsSequence_SendsJobDeleteRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    auto jobUrl = "https://srv.com/ws/v2.6/Repositories/Job";
    DateTime dateTime;
    auto scheduleTime = dateTime.GetCurrentTimeUtc().ToString();

    EXPECT_REQUEST_COUNT(GetHandler(), 7);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi27());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("DELETE", request.GetMethod().c_str());
        EXPECT_STREQ("Allow", request.GetHeaders().GetValue("Mas-Async-Job"));
        EXPECT_STREQ("https://srv.com/ws/v2.7/Repositories/foo/TestSchema/TestClass/TestId", request.GetUrl().c_str());
        std::map<Utf8String, Utf8String> headers {{"Operation-Location", jobUrl}};

        return StubHttpResponse(HttpStatus::Accepted, "", headers);
        });
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ(jobUrl, request.GetUrl().c_str());
        Utf8String body =
            Utf8PrintfString(R"({
            "instances" : [
                {
                "instanceId": "6b39f111-e38e-41b2-bf33-1c9b2a0965e1",
                "schemaName": "Jobs",
                "className": "Job",
                "properties" :
                    {
                    "ScheduleTime": "%s",
                    "Status":"NotStarted"
                    }
                }]
            })", scheduleTime.c_str());

        std::map<Utf8String, Utf8String> headers;
        return StubHttpResponse(HttpStatus::OK, body, headers);
        });
    GetHandler().ForRequest(4, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ(jobUrl, request.GetUrl().c_str());
        Utf8String body =
            Utf8PrintfString(R"({
            "instances" : [
                {
                "instanceId": "6b39f111-e38e-41b2-bf33-1c9b2a0965e1",
                "schemaName": "Jobs",
                "className": "Job",
                "properties" :
                    {
                    "ScheduleTime": "%s",
                    "Status":"Running"
                    }
                }]
            })", scheduleTime.c_str());

        std::map<Utf8String, Utf8String> headers;
        return StubHttpResponse(HttpStatus::OK, body, headers);
        });
    GetHandler().ForRequest(5, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ(jobUrl, request.GetUrl().c_str());
        Utf8String body =
            Utf8PrintfString(R"({
            "instances" : [
                {
                "instanceId": "6b39f111-e38e-41b2-bf33-1c9b2a0965e1",
                "schemaName": "Jobs",
                "className": "Job",
                "properties" :
                    {
                    "ScheduleTime": "%s",
                    "Status":"Running"
                    }
                }]
            })", scheduleTime.c_str());

        std::map<Utf8String, Utf8String> headers;
        return StubHttpResponse(HttpStatus::OK, body, headers);
        });
    GetHandler().ForRequest(6, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_STREQ(jobUrl, request.GetUrl().c_str());
        Utf8String body =
            Utf8PrintfString(R"({
            "instances" : [
                {
                "instanceId": "6b39f111-e38e-41b2-bf33-1c9b2a0965e1",
                "schemaName": "Jobs",
                "className": "Job",
                "properties" :
                    {
                    "ScheduleTime": "%s",
                    "ResponseStatusCode":200,
                    "ResponseContent":"Good Content",
                    "ResponseHeaders":"",
                    "Status":"Succeeded"
                    }
                }]
            })", scheduleTime.c_str());

        std::map<Utf8String, Utf8String> headers;
        return StubHttpResponse(HttpStatus::OK, body, headers);
        });
    GetHandler().ForRequest(7, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("DELETE", request.GetMethod().c_str());
        EXPECT_STREQ(jobUrl, request.GetUrl().c_str());

        return StubHttpResponse(HttpStatus::OK);
        });

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    client->SendDeleteObjectRequestWithOptions({"TestSchema.TestClass", "TestId"}, options)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsWebApiV2JobRequestFailed_Fails)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi27());
    auto jobUrl = "https://srv.com/ws/v2.0/Repositories/Job";
    std::map<Utf8String, Utf8String> headers {{"Operation-Location", jobUrl}};
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::InternalServerError, "", headers));

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendDeleteObjectRequestWithOptions({"TestSchema.TestClass", "TestId"}, options)->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsWebApiV2JobSucceedsResponseSucceeds_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Expect4_jSrS(GetHandler(), HttpStatus::OK);

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendDeleteObjectRequestWithOptions({"TestSchema.TestClass", "TestId"}, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsWebApiV2JobSucceedsAfterLongDelayResponseSucceeds_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(4);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi27());
    auto jobUrl = "https://srv.com/ws/v2.0/Repositories/Job";
    std::map<Utf8String, Utf8String> headers {{"Operation-Location", jobUrl}};
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Accepted, "", headers));

    Utf8String body =
        Utf8PrintfString(R"({
            "instances" : [
                {
                "instanceId": "6b39f111-e38e-41b2-bf33-1c9b2a0965e1",
                "schemaName": "Jobs",
                "className": "Job",
                "properties" :
                    {
                    "ResponseStatusCode":200,
                    "ResponseContent":"Good Content",
                    "ResponseHeaders":"",
                    "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
                    "Status":"Succeeded"
                    }
                }]
            })");

    GetHandler().ForRequest(3, StubHttpResponse(HttpStatus::OK, body, headers));
    GetHandler().ForRequest(4, StubHttpResponse(HttpStatus::OK));

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendDeleteObjectRequestWithOptions({"TestSchema.TestClass", "TestId"}, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsWebApiV2JobsRunningAfterLongDelay_Fails)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(4);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi27());
    auto jobUrl = "https://srv.com/ws/v2.0/Repositories/Job";
    std::map<Utf8String, Utf8String> headers {{"Operation-Location", jobUrl}};
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Accepted, "", headers));

    Utf8String body =
        Utf8PrintfString(R"({
            "instances" : [
                {
                "instanceId": "6b39f111-e38e-41b2-bf33-1c9b2a0965e1",
                "schemaName": "Jobs",
                "className": "Job",
                "properties" :
                    {
                    "ScheduleTime":  "2000-01-01T13:43:57.3126099Z",
                    "Status":"Running"
                    }
                }]
            })");

    GetHandler().ForRequest(3, StubHttpResponse(HttpStatus::OK, body, headers));
    GetHandler().ForRequest(4, StubHttpResponse(HttpStatus::OK));

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendDeleteObjectRequestWithOptions({"TestSchema.TestClass", "TestId"}, options)->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsAndWebApiV2JobsSuceedsFailsToDeleteJob_Fails)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(4);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi27());
    auto jobUrl = "https://srv.com/ws/v2.0/Repositories/Job";
    std::map<Utf8String, Utf8String> headers {{"Operation-Location", jobUrl}};
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Accepted, "", headers));

    DateTime dateTime;
    auto scheduleTime = dateTime.GetCurrentTimeUtc().ToString();

    Utf8String body =
        Utf8PrintfString(R"({
            "instances" : [
                {
                "instanceId": "6b39f111-e38e-41b2-bf33-1c9b2a0965e1",
                "schemaName": "Jobs",
                "className": "Job",
                "properties" :
                    {
                    "ResponseStatusCode":420,
                    "ResponseContent":"Failed",
                    "ResponseHeaders":"",
                    "ScheduleTime": "%s",
                    "Status":"Succeeded"
                    }
                }]
            })", scheduleTime.c_str());

    GetHandler().ForRequest(3, StubHttpResponse(HttpStatus::OK, body, headers));
    GetHandler().ForRequest(4, StubHttpResponse(HttpStatus::OK));

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendDeleteObjectRequestWithOptions({"TestSchema.TestClass", "TestId"}, options)->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsWebApiV2JobFails_Fails)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(4);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi27());
    auto jobUrl = "https://srv.com/ws/v2.0/Repositories/Job";
    std::map<Utf8String, Utf8String> headers {{"Operation-Location", jobUrl}};
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Accepted, "", headers));

    DateTime dateTime;
    auto scheduleTime = dateTime.GetCurrentTimeUtc().ToString();

    Utf8String body =
        Utf8PrintfString(R"({
            "instances" : [
                {
                "instanceId": "6b39f111-e38e-41b2-bf33-1c9b2a0965e1",
                "schemaName": "Jobs",
                "className": "Job",
                "properties" :
                    {
                    "ScheduleTime": "%s",
                    "Status":"Failed"
                    }
                }]
            })", scheduleTime.c_str());

    GetHandler().ForRequest(3, StubHttpResponse(HttpStatus::OK, body, headers));
    GetHandler().ForRequest(4, StubHttpResponse(HttpStatus::OK));

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendDeleteObjectRequestWithOptions({"TestSchema.TestClass", "TestId"}, options)->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsWebApiV2JobsResponseIsUnauthorized_Fails)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(4);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi27());
    auto jobUrl = "https://srv.com/ws/v2.0/Repositories/Job";
    std::map<Utf8String, Utf8String> headers {{"Operation-Location", jobUrl}};
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Accepted, "", headers));
    GetHandler().ForRequest(3, StubHttpResponse(HttpStatus::Unauthorized));
    GetHandler().ForRequest(4, StubHttpResponse(HttpStatus::Unauthorized));

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendDeleteObjectRequestWithOptions({"TestSchema.TestClass", "TestId"}, options)->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV2_SendsPutRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass/TestId/$file", request.GetUrl().c_str());
        return StubHttpResponse(HttpStatus::ResumeIncomplete);
        });
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        EXPECT_EQ("TestContent", ReadHttpBody(request.GetRequestBody()));
        return StubHttpResponse();
        });

    client->SendUpdateFileRequest({"TestSchema.TestClass", "TestId"}, StubFile("TestContent"))->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApi27_ChunkUploadSendsAllRequestsWithSameMasRequestId)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    set<Utf8CP> requestIds;

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi27());
    GetHandler().ForRequest(2, [=, &requestIds] (Http::RequestCR request)
        {
        auto actualActivityId = request.GetHeaders().GetValue(HEADER_MasRequestId);
        EXPECT_NE(nullptr, actualActivityId);
        requestIds.insert(actualActivityId);
        return StubHttpResponse(HttpStatus::ResumeIncomplete);
        });

    GetHandler().ForRequest(3, [=, &requestIds] (Http::RequestCR request)
        {
        auto actualActivityId = request.GetHeaders().GetValue(HEADER_MasRequestId);
        EXPECT_NE(nullptr, actualActivityId);
        requestIds.insert(actualActivityId);
        return StubHttpResponse(HttpStatus::ResumeIncomplete);
        });

    GetHandler().ForRequest(4, [=, &requestIds] (Http::RequestCR request)
        {
        auto actualActivityId = request.GetHeaders().GetValue(HEADER_MasRequestId);
        EXPECT_NE(nullptr, actualActivityId);
        requestIds.insert(actualActivityId);
        return StubHttpResponse(HttpStatus::OK);
        });

    client->SendUpdateFileRequest(StubObjectId(), StubFile())->GetResult();

    auto uniqueRequestIds = requestIds.size();
    EXPECT_EQ(1, uniqueRequestIds);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV27AndAzureRedirectAndAzureUploadSuccessful_SendsAllRequestsWithSameActivityId)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    set<Utf8CP> requestIds;

    EXPECT_REQUEST_COUNT(GetHandler(), 4);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi27());
    GetHandler().ForRequest(2, [=, &requestIds] (Http::RequestCR request)
        {
        auto actualActivityId = request.GetHeaders().GetValue(HEADER_MasRequestId);
        EXPECT_NE(nullptr, actualActivityId);
        requestIds.insert(actualActivityId);
        return StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
                {HEADER_Location, "https://foozure.com/boo"},
                {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"},
                {HEADER_MasUploadConfirmationId, "TestUploadId"}});
        });
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        // TODO: It's Http Request to AzureBlobStorage. ActivityId should be set to header "x-ms-client-request-id"
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ForRequest(4, [=, &requestIds] (Http::RequestCR request)
        {
        auto actualActivityId = request.GetHeaders().GetValue(HEADER_MasRequestId);
        EXPECT_NE(nullptr, actualActivityId);
        requestIds.insert(actualActivityId);
        return StubHttpResponse(HttpStatus::OK);
        });

    client->SendUpdateFileRequest({"TestSchema", "TestClass", "TestId"}, StubFilePath())->GetResult();
    
    auto uniqueRequestIds = requestIds.size();
    EXPECT_EQ(1, uniqueRequestIds);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV2AndFileETagSentBack_ReturnsNewFileETag)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::ResumeIncomplete));
    GetHandler().ForRequest(3, StubHttpResponse(HttpStatus::OK, "", {{HEADER_MasFileETag, "NewTag"}}));

    auto result = client->SendUpdateFileRequest(StubObjectId(), StubFile())->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ("NewTag", result.GetValue().GetFileETag());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV24_SendsPutRequestWithAllowRedirect)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.4/Repositories/foo/TestSchema/TestClass/TestId/$file", request.GetUrl().c_str());
        EXPECT_STREQ("true", request.GetHeaders().GetValue("Mas-Allow-Redirect"));
        return StubHttpResponse(HttpStatus::OK);
        });

    auto response = client->SendUpdateFileRequest({"TestSchema", "TestClass", "TestId"}, StubFilePath())->GetResult();
    EXPECT_TRUE(response.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV25_SendsPutRequestWithCorrectApiVersion)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi25());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.5/Repositories/foo/TestSchema/TestClass/TestId/$file", request.GetUrl().c_str());
        return StubHttpResponse(HttpStatus::OK);
        });

    client->SendUpdateFileRequest({"TestSchema", "TestClass", "TestId"}, StubFilePath())->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV24AndAzureRedirectAndAzureUploadSuccessful_SendsConfirmationToServer)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 4);
    GetHandler().ExpectRequest(StubWSInfoHttpResponseWebApi24());
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
                {HEADER_Location, "https://foozure.com/boo"},
                {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"},
                {HEADER_MasUploadConfirmationId, "TestUploadId"}});
        });
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://foozure.com/boo", request.GetUrl().c_str());
        EXPECT_STREQ("BlockBlob", request.GetHeaders().GetValue("x-ms-blob-type"));
        return StubHttpResponse(HttpStatus::Created);
        });
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.4/Repositories/foo/TestSchema/TestClass/TestId/$file", request.GetUrl().c_str());
        EXPECT_EQ(nullptr, request.GetHeaders().GetValue("Mas-Allow-Redirect"));
        EXPECT_STREQ("TestUploadId", request.GetHeaders().GetValue(HEADER_MasUploadConfirmationId));
        return StubHttpResponse(HttpStatus::OK);
        });

    auto response = client->SendUpdateFileRequest({"TestSchema", "TestClass", "TestId"}, StubFilePath())->GetResult();
    EXPECT_TRUE(response.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV24AndAzureRedirectAndAzureUploadFails_ParsesError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 3);
    GetHandler().ExpectRequest(StubWSInfoHttpResponseWebApi24());
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
                {HEADER_Location, "https://foozure.com/boo"},
                {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"},
                {HEADER_MasUploadConfirmationId, "TestUploadId"}});
        });
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://foozure.com/boo", request.GetUrl().c_str());
        EXPECT_STREQ("BlockBlob", request.GetHeaders().GetValue("x-ms-blob-type"));
        Utf8CP body = "<?xml version=\"1.0\" encoding=\"utf-8\"?><Error><Code>BlobNotFound</Code><Message>TestMessage</Message></Error>";
        return StubHttpResponse(HttpStatus::NotFound, body, {{ "Content-Type" , REQUESTHEADER_ContentType_ApplicationXml }});
        });

    auto response = client->SendUpdateFileRequest({"TestSchema", "TestClass", "TestId"}, StubFilePath())->GetResult();
    ASSERT_FALSE(response.IsSuccess());
    EXPECT_EQ(WSError::Id::FileNotFound, response.GetError().GetId());
    EXPECT_EQ("TestMessage", response.GetError().GetDescription());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV24AndAzureRedirectWithoutConfirmationIdAndAzureUploadSuccessful_DoesNotSendConfirmationToServer)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 3);
    GetHandler().ExpectRequest(StubWSInfoHttpResponseWebApi24());
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
                {HEADER_Location, "https://foozure.com/boo"},
                {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"}});
        });
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://foozure.com/boo", request.GetUrl().c_str());
        EXPECT_STREQ("BlockBlob", request.GetHeaders().GetValue("x-ms-blob-type"));
        return StubHttpResponse(HttpStatus::Created);
        });

    auto response = client->SendUpdateFileRequest({"TestSchema", "TestClass", "TestId"}, StubFilePath())->GetResult();
    EXPECT_TRUE(response.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV24AndAzureRedirectAndUploadReturnsETag_ReturnsNewFileETag)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 4);
    GetHandler().ExpectRequest(StubWSInfoHttpResponseWebApi24());
    GetHandler().ExpectRequest(StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
            {HEADER_Location, "https://foozure.com/boo"},
            {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"},
            {HEADER_MasUploadConfirmationId, "TestUploadId"}}));
    GetHandler().ExpectRequest(StubHttpResponse(HttpStatus::Created, "", {{"ETag", "NewTag"}}));
    GetHandler().ExpectRequest(StubHttpResponse(HttpStatus::OK, "", {{"ETag", "WSGTag"}, {HEADER_MasFileETag, "WSGTag"}}));

    auto response = client->SendUpdateFileRequest({"TestSchema", "TestClass", "TestId"}, StubFilePath())->GetResult();
    ASSERT_TRUE(response.IsSuccess());
    EXPECT_EQ("NewTag", response.GetValue().GetFileETag());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV24AndAzureRedirectWithoutConfirmationIdAndAzureUploadSuccessful_ReturnsNewFileETag)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 3);
    GetHandler().ExpectRequest(StubWSInfoHttpResponseWebApi24());
    GetHandler().ExpectRequest(StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
            {HEADER_Location, "https://foozure.com/boo"},
            {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"}}));
    GetHandler().ExpectRequest(StubHttpResponse(HttpStatus::Created, "", {{"ETag", "NewTag"}}));

    auto response = client->SendUpdateFileRequest({"TestSchema", "TestClass", "TestId"}, StubFilePath())->GetResult();
    ASSERT_TRUE(response.IsSuccess());
    EXPECT_EQ("NewTag", response.GetValue().GetFileETag());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV24AndAzureRedirectAndUnknownUrlType_ReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi24());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::Found, "", {
                {HEADER_Location, "https://foo.com/boo"},
                {HEADER_MasFileAccessUrlType, "SomethingNotSupportedHere"},
        });
        });

    BeFileName filePath = StubFilePath();
    auto response = client->SendUpdateFileRequest(StubObjectId(), filePath)->GetResult();
    EXPECT_FALSE(response.IsSuccess());
    EXPECT_EQ(WSError::Status::ServerNotSupported, response.GetError().GetStatus());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_EnableJobsWebApiV2JobSucceedsResponseSucceeds_Success_KnownIssue)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Expect4_jSrS(GetHandler(), HttpStatus::OK);

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendUpdateFileRequestWithOptions({"TestSchema.TestClass", "TestId"}, StubFile("TestContent"), nullptr, options)->GetResult();
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().GetBody().IsValid());
    EXPECT_STREQ("Good Content", result.GetValue().GetBody()->AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV20AndNoDefaultSchema_Fails)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForFirstRequest(StubWSInfoHttpResponseWebApi20());
    auto result = client->SendGetSchemasRequest()->GetResult();

    EXPECT_EQ(WSError::Id::Unknown, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV1ResponseNotModified_ReturnsNotModified)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::NotModified));

    auto result = client->SendGetSchemasRequest()->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().IsModified());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV2ResponseWithSchema_ETagIsFromResponse)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        Utf8String schemaXml =
            R"( <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="4.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            </ECSchema>)";

        WriteStringToHttpBody(schemaXml, request.GetResponseBody());
        return StubHttpResponse(HttpStatus::OK, request.GetResponseBody(), {{"ETag", "TestETag"}});
        });

    auto result = client->SendGetSchemasRequest()->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue().IsModified());
    EXPECT_EQ("TestETag", result.GetValue().GetETag());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV2AndInvalidObjectId_ReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->SendGetFileRequest(ObjectId("", "Foo", ""), StubFilePath())->GetResult();
    EXPECT_EQ(result.GetError().GetId(), WSError::Id::NotSupported);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV2_SendsGetSchemasRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STRCASEEQ("https://srv.com/ws/v2.0/Repositories/foo/MetaSchema/ECSchemaDef", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetSchemasRequest()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV2ResponseContainsObjects_ReturnsObjects)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK, instances.ToJsonWebApiV2()));

    auto result = client->SendGetSchemasRequest()->GetResult();

    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), (*(result.GetValue().GetInstances().begin())).GetObjectId());
    }

#ifdef USE_GTEST

struct WSRepositoryClientTests_InvalidUrls : TestWithParam<Utf8String> {};
INSTANTIATE_TEST_CASE_P(, WSRepositoryClientTests_InvalidUrls, ValuesIn(vector<Utf8String>{
        "httpbbb://foo.com/v2.5/repositories/A--B/",
        "http://foo.com/v2.5/repositories/AbbbB/",
        "http://foo.com/svb2.5/repositories/A--B/",
        "http://foo.com/b2.5/repositories/A--B/",
        "http://foo.com/v2.a5/repositories/A--B/",
        "http://foo.com/v2.5b/repositories/A--B/",
        "http://foo.com/v2.5/rs/A--B/",
        "http://foo.com/sv2.a5/repositories/A--B/"
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(WSRepositoryClientTests_InvalidUrls, ParseRepositoryUrl_InvalidRepositoryUrl_ReturnsInvalid)
    {
    auto param = GetParam();
    WSRepository repository = WSRepositoryClient::ParseRepositoryUrl(param);
    EXPECT_FALSE(repository.IsValid());
    EXPECT_EQ("", repository.GetServerUrl());
    EXPECT_EQ("", repository.GetId());
    EXPECT_EQ("", repository.GetPluginId());
    EXPECT_EQ("", repository.GetLocation());
    EXPECT_EQ(BeVersion(), repository.GetServiceVersion());
    }

struct WSRepositoryClientTests_VariousServerUrls : TestWithParam<vector<Utf8String>> {};
INSTANTIATE_TEST_CASE_P(Host, WSRepositoryClientTests_VariousServerUrls, ValuesIn(vector<vector<Utf8String>>{
        {"http://foo.boo.com/foo/v2.5/repositories/A--B/", "http://foo.boo.com/foo"},
        {"https://foo-boo.com/foo/v2.5/Repositories/A--B/", "https://foo-boo.com/foo"}
    }));
INSTANTIATE_TEST_CASE_P(ServiceVersion, WSRepositoryClientTests_VariousServerUrls, ValuesIn(vector<vector<Utf8String>>{
        {"http://foo.boo.com/foo/sv4.2/repositories/A--B/", "http://foo.boo.com/foo"},
        {"https://foo-boo.com/foo/sv4.2/Repositories/A--B/", "https://foo-boo.com/foo"}
    }));
INSTANTIATE_TEST_CASE_P(ServicePath, WSRepositoryClientTests_VariousServerUrls, ValuesIn(vector<vector<Utf8String>>{
        {"https://foo.com/ws250/v2.5/repositories/A--B/", "https://foo.com/ws250"},
        {"https://foo.com/ws2/v2.5/repositories/A--B/", "https://foo.com/ws2"}
    })); 

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(WSRepositoryClientTests_VariousServerUrls, ParseRepositoryUrl_RepositoryUrl_ServerUrlParsed)
    {
    auto param = GetParam();
    WSRepository repository = WSRepositoryClient::ParseRepositoryUrl(param[0]);
    EXPECT_EQ(param[1], repository.GetServerUrl());
    EXPECT_EQ("A--B", repository.GetId());
    EXPECT_EQ("A", repository.GetPluginId());
    EXPECT_EQ("B", repository.GetLocation());
    }

struct WSRepositoryClientTests_ServerUrlEndings : TestWithParam<Utf8String> {};
INSTANTIATE_TEST_CASE_P(WebApiVersions, WSRepositoryClientTests_ServerUrlEndings, Values(
    "https://foo.com/foo/v1.0/repositories/A--B/",
    "https://foo.com/foo/v2.5/repositories/A--B/",
    "https://foo.com/foo/v1234.5678/repositories/A--B/"
    ));
INSTANTIATE_TEST_CASE_P(ServiceVersions, WSRepositoryClientTests_ServerUrlEndings, Values(
    "https://foo.com/foo/sv1.0/repositories/A--B/",
    "https://foo.com/foo/sv2.5/repositories/A--B/",
    "https://foo.com/foo/sv1234.5678/repositories/A--B/"
));
INSTANTIATE_TEST_CASE_P(Repositories, WSRepositoryClientTests_ServerUrlEndings, Values(
    "https://foo.com/foo/v2.5/Repositories/A--B/",
    "https://foo.com/foo/v2.5/RePosiTorieS/A--B/"
));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(WSRepositoryClientTests_ServerUrlEndings, ParseRepositoryUrl_RepositoryUrl_ServerUrlParsed)
    {
    auto param = GetParam();
    WSRepository repository = WSRepositoryClient::ParseRepositoryUrl(param);
    EXPECT_EQ("https://foo.com/foo", repository.GetServerUrl());
    EXPECT_EQ("A--B", repository.GetId());
    EXPECT_EQ("A", repository.GetPluginId());
    EXPECT_EQ("B", repository.GetLocation());
    }

struct WSRepositoryClientTests_RepositoryIdAndRemainingPath : TestWithParam<vector<Utf8String>> {};
INSTANTIATE_TEST_CASE_P(, WSRepositoryClientTests_RepositoryIdAndRemainingPath, ValuesIn(vector<vector<Utf8String>>{
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo", ""},
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo/", ""},
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo/Schema/Class", "/Schema/Class"},
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo/Schema/Class/RemoteId", "/Schema/Class/RemoteId"},
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo/Schema/Class/RemoteId/", "/Schema/Class/RemoteId/"},
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo?query=foo", "?query=foo"},
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo/?query=foo", "/?query=foo"},
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo#hashtag", "#hashtag"},
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo/Schema/Class/RemoteId?query=foo", "/Schema/Class/RemoteId?query=foo"},
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo/Schema/Class/RemoteId/?query=foo", "/Schema/Class/RemoteId/?query=foo"},
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo/Schema/Class/RemoteId#hashtag", "/Schema/Class/RemoteId#hashtag"}
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(WSRepositoryClientTests_RepositoryIdAndRemainingPath, ParseRepositoryUrl_Url_RepositoryIdAndRemainingPathReturned)
    {
    auto param = GetParam();
    Utf8String remainingUrlPath;
    auto repository = WSRepositoryClient::ParseRepositoryUrl(param[0], &remainingUrlPath);
    EXPECT_EQ("Foo--Boo", repository.GetId());
    EXPECT_EQ("https://foo.com/boo", repository.GetServerUrl());
    EXPECT_EQ(param[1], remainingUrlPath);
    }

struct WSRepositoryClientTests_PluginId : TestWithParam<vector<Utf8String>> {};
INSTANTIATE_TEST_CASE_P(, WSRepositoryClientTests_PluginId, ValuesIn(vector<vector<Utf8String>>{
        {"https://foo.com/boo/v2.5/repositories/Foo--Boo/", "Foo"},
        {"https://foo.com/boo/v2.5/repositories/Foo.Boo--Boo/", "Foo.Boo"},
        {"https://foo.com/boo/v2.5/repositories/Foo-Boo--Boo/", "Foo-Boo"}
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(WSRepositoryClientTests_PluginId, ParseRepositoryUrl_Url_PluginIdParsed)
    {
    auto param = GetParam();
    WSRepository repository = WSRepositoryClient::ParseRepositoryUrl(param[0]);
    EXPECT_EQ(param[1], repository.GetPluginId());
    EXPECT_EQ("https://foo.com/boo", repository.GetServerUrl());
    EXPECT_EQ("Boo", repository.GetLocation());
    }

struct WSRepositoryClientTests_Location : TestWithParam<vector<Utf8String>> {};
INSTANTIATE_TEST_CASE_P(, WSRepositoryClientTests_Location, ValuesIn(vector<vector<Utf8String>>{
        {"https://foo.com/boo/v2.5/repositories/A--Boo-Foo.com~3AFoo/", "Boo-Foo.com:Foo"},
        {"https://foo.com/boo/v2.5/repositories/A--Boo.Foo.com~3AFoo/", "Boo.Foo.com:Foo"},
        {"https://foo.com/boo/v2.5/repositories/A--Boo--Foo.com~3AFoo/", "Boo--Foo.com:Foo"},
        {"https://foo.com/boo/v2.5/repositories/A--Boo.A~3A~3A~3AFoo/", "Boo.A:::Foo"},
        {"https://foo.com/boo/v2.5/repositories/A--Boo%Foo/", "Boo%Foo"}
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(WSRepositoryClientTests_Location, ParseRepositoryUrl_Url_LocationParsed)
    {
    auto param = GetParam();
    auto repository = WSRepositoryClient::ParseRepositoryUrl(param[0]);
    EXPECT_STREQ(param[1].c_str(), repository.GetLocation().c_str());
    EXPECT_STREQ("https://foo.com/boo", repository.GetServerUrl().c_str());
    EXPECT_STREQ("A", repository.GetPluginId().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, ParseRepositoryUrl_RepositoryUrlWithWebApiVersion_ServiceVersionEmpty)
    {
    WSRepository repository = WSRepositoryClient::ParseRepositoryUrl("http://foo.boo.com/foo/v4.2/repositories/A--B/");
    EXPECT_TRUE(repository.IsValid());
    EXPECT_EQ(BeVersion(), repository.GetServiceVersion());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, ParseRepositoryUrl_RepositoryUrlWithServiceVersion_ServiceVersionExtracted)
    {
    WSRepository repository = WSRepositoryClient::ParseRepositoryUrl("http://foo.boo.com/foo/sv4.2/repositories/A--B/");
    EXPECT_TRUE(repository.IsValid());
    EXPECT_EQ(BeVersion(4, 2), repository.GetServiceVersion());
    }

#endif
