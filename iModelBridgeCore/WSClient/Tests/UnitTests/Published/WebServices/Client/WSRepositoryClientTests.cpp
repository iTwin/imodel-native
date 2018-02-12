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

using namespace ::testing;
using namespace ::std;
USING_NAMESPACE_BENTLEY_WEBSERVICES

#define HEADER_MasFileETag          "Mas-File-ETag"
#define HEADER_MasConnectionInfo    "Mas-Connection-Info"
#define HEADER_MasFileAccessUrlType "Mas-File-Access-Url-Type"
#define HEADER_Location             "Location"

void WSRepositoryClientTests::SetUp()
    {
    ServerInfoProvider::InvalidateAllInfo();
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

void Expect4_jSrS(MockHttpHandler& handler)
    {
    handler.ExpectRequests(4);
    handler.ForRequest(1, StubWSInfoHttpResponseWebApi27());
    std::map<Utf8String, Utf8String> headers {{"Operation-Location", "FooBooBar"}};
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
                    "ResponseStatusCode":200,
                    "ResponseContent":"Good Content",
                    "ResponseHeaders":"",
                    "ScheduleTime": "%s",
                    "Status":"Succeeded"
                    }
                }]
            })", scheduleTime);

    handler.ForRequest(3, StubHttpResponse(HttpStatus::OK, body, headers));
    handler.ForRequest(4, StubHttpResponse(HttpStatus::OK));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, VerifyAccess_CredentialsPassed_SendsSameCredentials)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2)
        .ForRequest(1, StubWSInfoHttpResponseWebApi13())
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
        .ForRequest(1, StubWSInfoHttpResponseWebApi13())
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
        .ForRequest(1, StubWSInfoHttpResponseWebApi13())
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
        .ForRequest(1, StubWSInfoHttpResponseWebApi13())
        .ForRequest(2, StubWSErrorHttpResponse(HttpStatus::NotFound, "SchemaNotFound"));

    auto result = client->VerifyAccess()->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV11_SendsUrlWithoutWebApiVersion)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/DataSources/foo/Navigation", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetChildrenRequest(ObjectId())->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV1AndPropertiesToSelect_SendsGetRequestWithPropertyList)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v1.3/DataSources/foo/Navigation?properties=Boo,Foo", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    bset<Utf8String> propertiesToSelect;
    propertiesToSelect.insert("Foo");
    propertiesToSelect.insert("Boo");

    client->SendGetChildrenRequest(ObjectId(), propertiesToSelect)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV12AndPropertiesToSelect_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi12());

    bset<Utf8String> propertiesToSelect;
    propertiesToSelect.insert("TestProperty");

    auto response = client->SendGetChildrenRequest(ObjectId(), propertiesToSelect)->GetResult();

    EXPECT_EQ(WSError::Id::NotSupported, response.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV11WithoutSchemaProviderAndQueryNavigationRoot_ReturnsNotSupported)
    {
    IWSSchemaProviderPtr schemaProvider = nullptr;
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), schemaProvider, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK, R"({"TestClass" : [ { "$id" : "TestId" } ]})"));

    BeTest::SetFailOnAssert(false);
    auto response = client->SendGetChildrenRequest(ObjectId())->GetResult();
    BeTest::SetFailOnAssert(true);

    EXPECT_EQ(WSError::Id::NotSupported, response.GetError().GetId());
    }

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV11WithSchemaProviderButNoSchemaReturnedAndQueryNavigationRoot_ReturnsNotSupported)
    {
    auto schemaProvider = std::make_shared<MockWSSchemaProvider>();
    EXPECT_CALL(*schemaProvider, GetSchema(_)).WillOnce(Return(BeFileName()));

    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), schemaProvider, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK, R"({"TestClass" : [ { "$id" : "TestId" } ]})"));

    BeTest::SetFailOnAssert(false);
    auto response = client->SendGetChildrenRequest(ObjectId())->GetResult();
    BeTest::SetFailOnAssert(true);

    EXPECT_EQ(WSError::Id::NotSupported, response.GetError().GetId());
    }
#endif

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV11WithSchemaProviderAndQueryNavigationRoot_UsesProviderSchemaAndReturnsCorrectResults)
    {
    Utf8String schemaXml =
        R"( <ECSchema schemaName="DefaultSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            </ECSchema>)";

    auto schemaProvider = std::make_shared<MockWSSchemaProvider>();
    EXPECT_CALL(*schemaProvider, GetSchema(_)).WillOnce(Return(StubFile(schemaXml)));

    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), schemaProvider, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());
    GetHandler().ForRequest(2, StubJsonHttpResponse(HttpStatus::OK, R"({"TestClass" : [ { "$id" : "TestId" } ]})"));

    auto response = client->SendGetChildrenRequest(ObjectId())->GetResult();

    ASSERT_TRUE(response.IsSuccess());
    EXPECT_EQ(ObjectId("DefaultSchema.TestClass", "TestId"), (*response.GetValue().GetInstances().begin()).GetObjectId());
    }
#endif

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
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
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV1AndNavigationRoot_RetrievesSchemaAndReturnsCorrectResults)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v1.1/DataSources/foo/Navigation", request.GetUrl().c_str());
        return StubJsonHttpResponse(HttpStatus::OK, R"({"TestClass" : [ { "$id" : "TestId" } ]})");
        });
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v1.2/DataSources/foo/Schema", request.GetUrl().c_str());
        Utf8String schemaXml =
            R"( <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            </ECSchema>)";

        WriteStringToHttpBody(schemaXml, request.GetResponseBody());
        return StubHttpResponse(HttpStatus::OK, request.GetResponseBody());
        });

    auto response = client->SendGetChildrenRequest(ObjectId())->GetResult();

    ASSERT_TRUE(response.IsSuccess());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "TestId"), (*response.GetValue().GetInstances().begin()).GetObjectId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV1AndNavigationRootRequestedTwice_SendsGetSchemaRequestOnce)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Http::Response childrenResponse = StubJsonHttpResponse(HttpStatus::OK, R"({"TestClass" : [ { "$id" : "TestId" } ]})");

    GetHandler().ExpectRequests(4);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, childrenResponse);
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        Utf8String schemaXml =
            R"( <ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            </ECSchema>)";

        WriteStringToHttpBody(schemaXml, request.GetResponseBody());
        return StubHttpResponse(HttpStatus::OK, request.GetResponseBody());
        });
    GetHandler().ForRequest(4, childrenResponse);

    client->SendGetChildrenRequest(ObjectId())->Wait();
    auto response = client->SendGetChildrenRequest(ObjectId())->GetResult();

    ASSERT_TRUE(response.IsSuccess());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "TestId"), (*response.GetValue().GetInstances().begin()).GetObjectId());
    }

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
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV1AndQueryWithEmptyNavigationParentIdCustomParameter_MappedToNavigationRootQuery)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [&] (Http::RequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v1.1/DataSources/foo/Navigation", request.GetUrl());
        return StubHttpResponse();
        });

    WSQuery query("TestSchema", "TestClass");
    query.SetCustomParameter(WSQuery_CustomParameter_NavigationParentId, "");
    client->SendQueryRequest(query)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV1AndQueryWithNavigationParentIdCustomParameter_MappedToNavigationQuery)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [&] (Http::RequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v1.1/DataSources/foo/Navigation/TestClass/TestId", request.GetUrl());
        return StubHttpResponse();
        });

    WSQuery query("TestSchema", "TestClass");
    query.SetCustomParameter(WSQuery_CustomParameter_NavigationParentId, "TestId");
    client->SendQueryRequest(query)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV1AndQueryWithNavigationParentIdAndSelectProperties_MappedToNavigationQueryWithProperties)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [&] (Http::RequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v1.3/DataSources/foo/Navigation?properties=Foo,Boo", request.GetUrl());
        return StubHttpResponse();
        });

    WSQuery query("TestSchema", "TestClass");
    query.SetCustomParameter(WSQuery_CustomParameter_NavigationParentId, "");
    query.SetSelect("Foo,Boo");
    client->SendQueryRequest(query)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1AndEmptyObjectId_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    BeFileName fileName = StubFilePath();
    auto result = client->SendGetFileRequest(ObjectId(), fileName)->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
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
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1AndEmptyFilePath_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto result = client->SendGetFileRequest(StubObjectId(), BeFileName())->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV2AndEmptyFilePath_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->SendGetFileRequest(StubObjectId(), BeFileName())->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1_SendsGetRequestWithFollowRedirects)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v1.1/DataSources/foo/Files/TestClass/TestId", request.GetUrl().c_str());
        WriteStringToHttpBody("TestResponseBody", request.GetResponseBody());
        return StubHttpResponse();
        });

    BeFileName filePath = StubFilePath();
    client->SendGetFileRequest({"TestSchema", "TestClass", "TestId"}, filePath)->Wait();
    EXPECT_EQ("TestResponseBody", SimpleReadFile(filePath));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1Connect_SendsGetRequestForRedirect)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseBentleyConnectV1());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_FALSE(request.GetFollowRedirects());
        EXPECT_STREQ("https://srv.com/ws/DataSources/foo/Files/TestClass/TestId", request.GetUrl().c_str());
        EXPECT_EQ("", Utf8String(request.GetHeaders().GetIfNoneMatch()));
        WriteStringToHttpBody("ThisShouldNotGoToFile", request.GetResponseBody());
        return StubHttpResponse();
        });

    BeFileName filePath = StubFilePath();
    client->SendGetFileRequest({"TestSchema", "TestClass", "TestId"}, filePath, "TestETag")->Wait();
    EXPECT_FALSE(filePath.DoesPathExist());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1ConnectAndResponseFound_SendsGetRequestToLocation)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseBentleyConnectV1());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Found, "", {{HEADER_Location, "http://file.location/"}}));
    GetHandler().ForRequest(3, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("GET", request.GetMethod().c_str());
        EXPECT_EQ("http://file.location/", request.GetUrl());
        WriteStringToHttpBody("TestResponseBody", request.GetResponseBody());
        return StubHttpResponse();
        });

    BeFileName filePath = StubFilePath();
    client->SendGetFileRequest(StubObjectId(), filePath)->Wait();
    EXPECT_EQ("TestResponseBody", SimpleReadFile(filePath));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1ConnectAndResponseOK_ReturnsServerNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseBentleyConnectV1());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK));

    auto result = client->SendGetFileRequest(StubObjectId(), StubFilePath())->GetResult();
    EXPECT_EQ(WSError::Status::ServerNotSupported, result.GetError().GetStatus());
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
TEST_F(WSRepositoryClientTests, SendGetObjectRequest_WebApiV1AndInvalidObjectId_ReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());
    
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto result = client->SendGetObjectRequest(ObjectId("", "Foo", ""))->GetResult();
    EXPECT_EQ(result.GetError().GetId(), WSError::Id::NotSupported);
    }

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
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV1_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v1.3/DataSources/foo/Objects/class1,class2?$select=testSelect", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    WSQuery query("testSchema", set<Utf8String> {"class1", "class2"});
    query.SetSelect("testSelect");

    client->SendQueryRequest(query)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV12_SendsCorrectUrlWithMaxWebApi)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi12());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v1.2/DataSources/foo/Objects/class1,class2?$select=testSelect", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    WSQuery query("testSchema", set<Utf8String> {"class1", "class2"});
    query.SetSelect("testSelect");

    client->SendQueryRequest(query)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV11_SendsCorrectUrlWithoutWebApi)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/DataSources/foo/Objects/class1,class2?$select=testSelect", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    WSQuery query("testSchema", set<Utf8String> {"class1", "class2"});
    query.SetSelect("testSelect");

    client->SendQueryRequest(query)->Wait();
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
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV1SkipTokenSuppliedAndSentBack_IgnoresSkipTokens)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_EQ(nullptr, request.GetHeaders().GetValue("SkipToken"));
        return StubHttpResponse(HttpStatus::OK, StubInstances().ToJsonWebApiV1(),
        {{"SkipToken", "ServerSkipToken"}, {"Content-Type", REQUESTHEADER_ContentType_ApplicationJson}});
        });

    auto result = client->SendQueryRequest(StubWSQuery(), nullptr, "SomeSkipToken")->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue().IsFinal());
    EXPECT_EQ("", result.GetValue().GetSkipToken());
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
        return StubHttpResponse(HttpStatus::OK, StubInstances().ToJsonWebApiV1(),
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
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV26_CapsWebApiToV25)
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
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV28_CapsWebApiToV27)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi({2, 8}));
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
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV11_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "className" : "TestClass",
                "properties" : { "TestProperty" : "TestValue" }
                }
            })");

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());

    auto result = client->SendCreateObjectRequest(objectCreationJson)->GetResult();
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV1WithCorrectJson_TakesClassInfoFromJsonAndSendsPropertiesWithRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "className" : "TestClass",
                "properties" : { "TestProperty" : "TestValue" }
                }
            })");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v1.2/DataSources/foo/Objects/TestClass", request.GetUrl().c_str());
        EXPECT_EQ(objectCreationJson["instance"]["properties"], Json::Reader::DoParse(request.GetRequestBody()->AsString()));
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(objectCreationJson)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV1WithOneRelationship_RelatedInstanceTreatedAsParentInRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "className" : "TestClass",
                "properties" : {},
                "relationshipInstances" :
                    [{
                    "relatedInstance" :
                        {
                        "instanceId" : "ParentId",
                        "className" : "ParentClass"
                        }
                    }]
                }
            })");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v1.2/DataSources/foo/Objects/TestClass?parentClass=ParentClass&parentObjectId=ParentId", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(objectCreationJson)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV1WithNestedRelationship_DoesNotSendCreateRequestAndReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "className" : "TestClass",
                "properties" : {},
                "relationshipInstances" :
                    [{
                    "relatedInstance" :
                        {
                        "instanceId" : "ParentId",
                        "className" : "ParentClass",
                        "relationshipInstances" :
                            [{
                            "relatedInstance" :
                                {
                                "instanceId" : "ParentId2",
                                "className" : "ParentClass2"
                                }
                            }]
                        }
                    }]
                }
            })");

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto result = client->SendCreateObjectRequest(objectCreationJson)->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Status::ReceivedError, result.GetError().GetStatus());
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV1WithMoreThanOneRelationship_DoesNotSendCreateRequestAndReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "className" : "TestClass",
                "properties" : {},
                "relationshipInstances" : [{}, {}]
                }
            })");

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto result = client->SendCreateObjectRequest(objectCreationJson)->GetResult();
    ASSERT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Status::ReceivedError, result.GetError().GetStatus());
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV1WithRelatedObjectId_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "className" : "TestClass",
                "properties" : {}
                }
            })");
    ObjectId relatedObject("A", "B", "C");

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    BeTest::SetFailOnAssert(false);
    auto result = client->SendCreateObjectRequest(relatedObject, objectCreationJson)->GetResult();
    BeTest::SetFailOnAssert(true);
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV1WithInvalidRelatedObjectId_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value objectCreationJson = ToJson(
        R"( {
            "instance" :
                {
                "className" : "TestClass",
                "properties" : {}
                }
            })");
    ObjectId relatedObject;

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    BeTest::SetFailOnAssert(false);
    auto result = client->SendCreateObjectRequest(relatedObject, objectCreationJson)->GetResult();
    BeTest::SetFailOnAssert(true);
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
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
        return StubHttpResponse(ConnectionStatus::OK);
        });
    client->SendCreateObjectRequest(relatedObject, objectCreationJson)->Wait();
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
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV1_ConstructsWSG2FormatResponseFromResponseId)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Created, R"({ "id" : "NewId" })"));

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
    Json::Value expectedObject = ToJson(
        R"( {
            "changedInstance" :
                {
                "instanceAfterChange" :
                    {
                    "schemaName": "TestSchema",
                    "className": "TestClass",
                    "instanceId": "NewId"
                    }
                }
            })");

    Json::Value jsonBody;
    response.GetValue().GetJson(jsonBody);
    EXPECT_EQ(expectedObject, jsonBody);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV1WithRelationshipWithWSChangeset_ConstructsWSG2FormatResponseForWSChangeset)
    {
    // Arrange
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Created, R"({ "id" : "NewRemoteId" })"));

    WSChangeset changeset(WSChangeset::Format::SingeInstance);
    changeset
        .AddInstance({"TestSchema.TestClass", "LocalId"}, WSChangeset::Created, nullptr)
        .AddRelatedInstance(ObjectId("TestRelSchema.TestRelClass", "RelId"), WSChangeset::Created, ECRelatedInstanceDirection::Forward, {"TestSchema.ParentClass", "ParentId"}, WSChangeset::Existing, nullptr);

    // Act
    auto response = client->SendCreateObjectRequest(ToJson(changeset.ToRequestString()))->GetResult();
    ASSERT_TRUE(response.IsSuccess());

    // Assert
    rapidjson::Document responseJson;
    response.GetValue().GetJson(responseJson);

    bmap<ObjectId, ObjectId> ids;
    EXPECT_EQ(SUCCESS, changeset.ExtractNewIdsFromResponse(responseJson, [&] (ObjectId oldId, ObjectId newId)
        {
        ids[oldId] = newId;
        return SUCCESS;
        }));

    EXPECT_EQ(2, ids.size());

    auto it = ids.find(ObjectId("TestSchema.TestClass", "LocalId"));
    ASSERT_FALSE(it == ids.end());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "NewRemoteId"), it->second);

    it = ids.find(ObjectId("TestRelSchema.TestRelClass", "RelId"));
    ASSERT_FALSE(it == ids.end());
    EXPECT_EQ(ObjectId("TestRelSchema", "TestRelClass", ""), it->second);
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
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV1WithFilePath_AddsFileNameToContentDisposition)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    auto filePath = StubFile();
    auto fileName = Utf8String(filePath.GetFileNameAndExtension());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_THAT(request.GetHeaders().GetContentDisposition(), HasSubstr(fileName.c_str()));
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(StubWSObjectCreationJson(), filePath)->Wait();
    }

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

    Expect4_jSrS(GetHandler());

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendCreateObjectRequestWithOptions(relatedObject, objectCreationJson, BeFileName(), nullptr, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                        Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_WebApiV13_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto result = client->SendChangesetRequest(HttpStringBody::Create("TestChangeset"), nullptr, nullptr)->GetResult();
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
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
        EXPECT_EQ(IWSRepositoryClient::Timeout::Transfer::Default, request.GetTransferTimeoutSeconds());
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
* @bsitest                                    julius.cepukenas                    02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendChangesetRequest_EnableJobsWebApiV2JobSucceedsResponseSucceeds_SuccessAndReturnsBody)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Expect4_jSrS(GetHandler());

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendChangesetRequestWithOptions(HttpStringBody::Create("TestChangeset"), nullptr, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ("Good Content", result.GetValue()->AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV1AndInvalidObjectId_ReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());
    
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto result = client->SendUpdateObjectRequest(ObjectId("", "Foo", ""), ToJson(R"({"TestProperty" : "TestValue" })"))->GetResult();
    EXPECT_EQ(result.GetError().GetId(), WSError::Id::NotSupported);
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
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV1_SendsPostRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value propertiesJson = ToJson(R"({"TestProperty" : "TestValue" })");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v1.1/DataSources/foo/Objects/TestClass/TestId", request.GetUrl().c_str());
        EXPECT_EQ(propertiesJson, Json::Reader::DoParse(request.GetRequestBody()->AsString()));
        return StubHttpResponse();
        });

    client->SendUpdateObjectRequest({"TestSchema.TestClass", "TestId"}, propertiesJson)->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV1AndETagPassed_SendsRequestWithIfMatch)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("TestETag", request.GetHeaders().GetIfMatch());
        return StubHttpResponse();
        });

    client->SendUpdateObjectRequest({"A.B", "C"}, Json::objectValue, "TestETag")->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV1ResponseIsOK_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK));

    auto result = client->SendUpdateObjectRequest({"TestSchema.TestClass", "TestId"}, Json::objectValue)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
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
TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV1WithFile_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());

    BeTest::SetFailOnAssert(false);
    auto result = client->SendUpdateObjectRequest(StubObjectId(), Json::objectValue, nullptr, StubFilePath())->GetResult();
    BeTest::SetFailOnAssert(true);

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
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

    Expect4_jSrS(GetHandler());

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendUpdateObjectRequestWithOptions(StubObjectId(), Json::objectValue, nullptr, BeFileName(), nullptr, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_WebApiV1AndInvalidObjectId_ReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());
    
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto result = client->SendDeleteObjectRequest(ObjectId("", "Foo", ""))->GetResult();
    EXPECT_EQ(result.GetError().GetId(), WSError::Id::NotSupported);
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
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_WebApiV1_SendsDeleteRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("DELETE", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v1.1/DataSources/foo/Objects/TestClass/TestId", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendDeleteObjectRequest({"TestSchema.TestClass", "TestId"})->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_WebApiV1ResponseIsOK_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK));

    auto result = client->SendDeleteObjectRequest({"TestSchema.TestClass", "TestId"})->GetResult();
    EXPECT_TRUE(result.IsSuccess());
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
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV1AndInvalidObjectId_ReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());
    
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto result = client->SendUpdateFileRequest(ObjectId("", "Foo", ""), StubFile())->GetResult();
    EXPECT_EQ(result.GetError().GetId(), WSError::Id::NotSupported);
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
TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsWebApiV1_SendsSimpleDeleteRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("DELETE", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v1.1/DataSources/foo/Objects/TestClass/TestId", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    client->SendDeleteObjectRequestWithOptions({"TestSchema.TestClass", "TestId"}, options)->Wait();
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
            })", scheduleTime);

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
            })", scheduleTime);

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
            })", scheduleTime);

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
            })", scheduleTime);

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

TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_EnableJobsWebApiV2JobSucceedsResponseSucceeds_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Expect4_jSrS(GetHandler());

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
            })", scheduleTime);

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
            })", scheduleTime);

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

TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV1_SendsPutRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v1.1/DataSources/foo/Files/TestClass/TestId", request.GetUrl().c_str());
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

    EXPECT_REQUEST_COUNT(GetHandler(), 5);
    GetHandler().ExpectRequest(StubWSInfoHttpResponseWebApi24());
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
                {HEADER_Location, "https://foozure.com/boo"},
                {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"},
                {"Mas-Upload-Confirmation-Id", "TestUploadId"}});
        });
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://foozure.com/boo&comp=block&blockid=MDAwMDA=", request.GetUrl().c_str());
        EXPECT_STREQ("BlockBlob", request.GetHeaders().GetValue("x-ms-blob-type"));
        return StubHttpResponse(HttpStatus::OK);
        });
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://foozure.com/boo&comp=blocklist", request.GetUrl().c_str());
        return StubHttpResponse(HttpStatus::OK);
        });
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.4/Repositories/foo/TestSchema/TestClass/TestId/$file", request.GetUrl().c_str());
        EXPECT_EQ(nullptr, request.GetHeaders().GetValue("Mas-Allow-Redirect"));
        EXPECT_STREQ("TestUploadId", request.GetHeaders().GetValue("Mas-Upload-Confirmation-Id"));
        return StubHttpResponse(HttpStatus::OK);
        });

    auto response = client->SendUpdateFileRequest({"TestSchema", "TestClass", "TestId"}, StubFilePath())->GetResult();
    EXPECT_TRUE(response.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV24AndAzureRedirectWithoutConfirmationIdAndAzureUploadSuccessful_DoesNotSendConfirmationToServer)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    EXPECT_REQUEST_COUNT(GetHandler(), 4);
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
        EXPECT_STREQ("https://foozure.com/boo&comp=block&blockid=MDAwMDA=", request.GetUrl().c_str());
        EXPECT_STREQ("BlockBlob", request.GetHeaders().GetValue("x-ms-blob-type"));
        return StubHttpResponse(HttpStatus::OK);
        });
    GetHandler().ExpectRequest([=] (Http::RequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://foozure.com/boo&comp=blocklist", request.GetUrl().c_str());
        return StubHttpResponse(HttpStatus::OK);
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

    EXPECT_REQUEST_COUNT(GetHandler(), 5);
    GetHandler().ExpectRequest(StubWSInfoHttpResponseWebApi24());
    GetHandler().ExpectRequest(StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
            {HEADER_Location, "https://foozure.com/boo"},
            {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"},
            {"Mas-Upload-Confirmation-Id", "TestUploadId"}}));
    GetHandler().ExpectRequest(StubHttpResponse(HttpStatus::OK, "", {{"ETag", "OtherTag"}}));
    GetHandler().ExpectRequest(StubHttpResponse(HttpStatus::OK, "", {{"ETag", "NewTag"}}));
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

    EXPECT_REQUEST_COUNT(GetHandler(), 4);
    GetHandler().ExpectRequest(StubWSInfoHttpResponseWebApi24());
    GetHandler().ExpectRequest(StubHttpResponse(HttpStatus::TemporaryRedirect, "", {
            {HEADER_Location, "https://foozure.com/boo"},
            {HEADER_MasFileAccessUrlType, "AzureBlobSasUrl"}}));
    GetHandler().ExpectRequest(StubHttpResponse(HttpStatus::OK, "", {{"ETag", "OtherTag"}}));
    GetHandler().ExpectRequest(StubHttpResponse(HttpStatus::OK, "", {{"ETag", "NewTag"}}));

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
TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_EnableJobsWebApiV2JobSucceedsResponseSucceeds_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Expect4_jSrS(GetHandler());

    IWSRepositoryClient::RequestOptionsPtr options = std::make_shared<IWSRepositoryClient::RequestOptions>();
    options->GetJobOptions()->EnableJobsIfPossible();
    auto result = client->SendUpdateFileRequestWithOptions({"TestSchema.TestClass", "TestId"}, StubFile("TestContent"), nullptr, options)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_STREQ("Good Content", result.GetValue().GetBody()->AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV11AndNoDefaultSchema_Fails)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForFirstRequest(StubWSInfoHttpResponseWebApi11());
    auto result = client->SendGetSchemasRequest()->GetResult();

    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV1WithSchemaProvider_ReturnsObjectForProviderSchema)
    {
    Utf8String schemaXml =
        R"( <ECSchema schemaName="DefaultSchema" nameSpacePrefix="TS" version="4.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            </ECSchema>)";

    auto schemaProvider = std::make_shared<MockWSSchemaProvider>();
    EXPECT_CALL(*schemaProvider, GetSchema(_)).WillOnce(Return(StubFile(schemaXml)));

    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), schemaProvider, GetHandlerPtr());

    GetHandler().ForFirstRequest(StubWSInfoHttpResponseWebApi13());

    auto result = client->SendGetSchemasRequest()->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.GetValue().IsModified());
    auto schemaInstance = *(result.GetValue().GetInstances().begin());
    EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "DUMMY_SCHEMA_OBJECT-DefaultSchema.04.02"), schemaInstance.GetObjectId());
    EXPECT_STREQ("DefaultSchema", schemaInstance.GetProperties()["Name"].GetString());
    EXPECT_EQ(4, schemaInstance.GetProperties()["VersionMajor"].GetInt());
    EXPECT_EQ(2, schemaInstance.GetProperties()["VersionMinor"].GetInt());
    }
#endif

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV1WithSchemaProviderSchema_ETagIsSchemaId)
    {
    Utf8String schemaXml =
        R"( <ECSchema schemaName="DefaultSchema" nameSpacePrefix="TS" version="4.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            </ECSchema>)";

    auto schemaProvider = std::make_shared<MockWSSchemaProvider>();
    EXPECT_CALL(*schemaProvider, GetSchema(_)).WillOnce(Return(StubFile(schemaXml)));

    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), schemaProvider, GetHandlerPtr());

    GetHandler().ForFirstRequest(StubWSInfoHttpResponseWebApi13());

    auto result = client->SendGetSchemasRequest()->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue().IsModified());
    EXPECT_EQ("DUMMY_SCHEMA_OBJECT-DefaultSchema.04.02", result.GetValue().GetETag());
    }
#endif

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV1WithDefaultSchemaAndSendingETag_ResponseNotModified)
    {
    Utf8String schemaXml =
        R"( <ECSchema schemaName="DefaultSchema" nameSpacePrefix="TS" version="4.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            </ECSchema>)";

    auto schemaProvider = std::make_shared<MockWSSchemaProvider>();
    EXPECT_CALL(*schemaProvider, GetSchema(_)).WillOnce(Return(StubFile(schemaXml)));

    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), schemaProvider, GetHandlerPtr());

    GetHandler().ForFirstRequest(StubWSInfoHttpResponseWebApi13());

    auto result = client->SendGetSchemasRequest("DUMMY_SCHEMA_OBJECT-DefaultSchema.04.02")->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().IsModified());
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV1_SendsGetSchemaRequestWithSuppliedETag)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("TestETag", request.GetHeaders().GetIfNoneMatch());
        EXPECT_STREQ("https://srv.com/ws/v1.2/DataSources/foo/Schema", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetSchemasRequest("TestETag")->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV11Connect_SendsGetSchemaRequestWithSuppliedETag)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseBentleyConnectV1());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("TestETag", request.GetHeaders().GetIfNoneMatch());
        EXPECT_STREQ("https://srv.com/ws/DataSources/foo/Schema", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetSchemasRequest("TestETag")->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV1ResponseNotModified_ReturnsNotModified)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::NotModified));

    auto result = client->SendGetSchemasRequest()->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().IsModified());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV1ResponseWithSchema_ReturnsSchemaObject)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
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
    auto schemaInstance = *(result.GetValue().GetInstances().begin());

    EXPECT_EQ(ObjectId("MetaSchema.ECSchemaDef", "DUMMY_SCHEMA_OBJECT-TestSchema.04.02"), schemaInstance.GetObjectId());
    EXPECT_STREQ("TestSchema", schemaInstance.GetProperties()["Name"].GetString());
    EXPECT_EQ(4, schemaInstance.GetProperties()["VersionMajor"].GetInt());
    EXPECT_EQ(2, schemaInstance.GetProperties()["VersionMinor"].GetInt());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV1ResponseWithSchema_ETagIsFromResponse)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
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
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1AndInvalidObjectId_ReturnsError)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());
    
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto result = client->SendGetFileRequest(ObjectId("", "Foo", ""), StubFilePath())->GetResult();
    EXPECT_EQ(result.GetError().GetId(), WSError::Id::NotSupported);
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
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1AndDummySchemaObjectId_SendsGetSchemaRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("TestETag", request.GetHeaders().GetIfNoneMatch());
        EXPECT_STRCASEEQ("https://srv.com/ws/v1.2/DataSources/foo/Schema", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetFileRequest({"MetaSchema.ECSchemaDef", "DUMMY_SCHEMA_OBJECT..."}, StubFilePath(), "TestETag")->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1BentleyConnectAndDummySchemaObjectId_SendsGetSchemaRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseBentleyConnectV1());
    GetHandler().ForRequest(2, [=] (Http::RequestCR request)
        {
        EXPECT_STREQ("TestETag", request.GetHeaders().GetIfNoneMatch());
        EXPECT_STRCASEEQ("https://srv.com/ws/DataSources/foo/Schema", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetFileRequest({"MetaSchema.ECSchemaDef", "DUMMY_SCHEMA_OBJECT..."}, StubFilePath(), "TestETag")->Wait();
    }

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1AndDummySchemaObjectIdAndHasProvidedSchema_ReturnsProvidedSchema)
    {
    Utf8String schemaXml =
        R"( <ECSchema schemaName="DefaultSchema" nameSpacePrefix="TS" version="4.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            </ECSchema>)";

    auto schemaProvider = std::make_shared<MockWSSchemaProvider>();
    EXPECT_CALL(*schemaProvider, GetSchema(_)).WillOnce(Return(StubFile(schemaXml)));

    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), schemaProvider, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto result = client->SendGetFileRequest({"MetaSchema.ECSchemaDef", "DUMMY_SCHEMA_OBJECT..."}, StubFilePath(), "Foo")->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.GetValue().IsModified());
    EXPECT_EQ(schemaXml, SimpleReadFile(result.GetValue().GetFilePath()));
    }
#endif

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1AndDummySchemaObjectIdAndHasProvidedSchemaWithETag_ReturnsNotModified)
    {
    Utf8String schemaXml =
        R"( <ECSchema schemaName="DefaultSchema" nameSpacePrefix="TS" version="4.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            </ECSchema>)";

    auto schemaProvider = std::make_shared<MockWSSchemaProvider>();
    EXPECT_CALL(*schemaProvider, GetSchema(_)).WillOnce(Return(StubFile(schemaXml)));

    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), schemaProvider, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto eTag = "DUMMY_SCHEMA_OBJECT-DefaultSchema.04.02";
    auto result = client->SendGetFileRequest({"MetaSchema.ECSchemaDef", "DUMMY_SCHEMA_OBJECT..."}, StubFilePath(), eTag)->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().IsModified());
    }
#endif

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
struct WSRepositoryClientTests_VariousServerUrls : TestWithParam<vector<Utf8String>> {};
INSTANTIATE_TEST_CASE_P(, WSRepositoryClientTests_VariousServerUrls, ValuesIn(vector<vector<Utf8String>>{
        // Host
        {"http://foo.boo.com/foo/v2.5/repositories/A--B/", "http://foo.boo.com/foo"},
        {"https://foo-boo.com/foo/v2.5/Repositories/A--B/", "https://foo-boo.com/foo"},
        // Service path
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
INSTANTIATE_TEST_CASE_P(, WSRepositoryClientTests_ServerUrlEndings, Values(
    // Web API version
    "https://foo.com/foo/v1.0/repositories/A--B/",
    "https://foo.com/foo/v2.5/repositories/A--B/",
    "https://foo.com/foo/v1234.5678/repositories/A--B/",
    // Repositories
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
#endif
