/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Client/WSRepositoryClientTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSRepositoryClientTests.h"

#include <fstream>
#include <iostream>

#include <Bentley/Base64Utilities.h>
#include <WebServices/Client/WSRepositoryClient.h>
#include <WebServices/Client/WSChangeset.h>
#include "../../../../Cache/Util/JsonUtil.h"

#include "MockWSSchemaProvider.h"

using namespace ::testing;
using namespace ::std;
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

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

TEST_F(WSRepositoryClientTests, VerifyAccess_CredentialsPassed_SendsSameCredentials)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2)
        .ForRequest(1, StubWSInfoHttpResponseWebApi13())
        .ForRequest(2, [] (HttpRequestCR request)
        {
        EXPECT_EQ(Credentials("TestUser", "TestPassword"), request.GetCredentials());
        return StubHttpResponse();
        });

    client->SetCredentials(Credentials("TestUser", "TestPassword"));
    client->VerifyAccess()->Wait();
    }

TEST_F(WSRepositoryClientTests, VerifyAccess_ResponseWithClassNotFound_ReturnsSuccess)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2)
        .ForRequest(1, StubWSInfoHttpResponseWebApi13())
        .ForRequest(2, StubWSErrorHttpResponse(HttpStatus::NotFound, "ClassNotFound"));

    auto result = client->VerifyAccess()->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(WSRepositoryClientTests, VerifyAccess_ResponseWithSchemaNotFound_ReturnsSuccess)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2)
        .ForRequest(1, StubWSInfoHttpResponseWebApi13())
        .ForRequest(2, StubWSErrorHttpResponse(HttpStatus::NotFound, "SchemaNotFound"));

    auto result = client->VerifyAccess()->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV11_SendsUrlWithoutWebApiVersion)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/DataSources/foo/Navigation", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetChildrenRequest(ObjectId())->Wait();
    }

TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV1AndPropertiesToSelect_SendsGetRequestWithPropertyList)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v1.3/DataSources/foo/Navigation?properties=Boo,Foo", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    bset<Utf8String> propertiesToSelect;
    propertiesToSelect.insert("Foo");
    propertiesToSelect.insert("Boo");

    client->SendGetChildrenRequest(ObjectId(), propertiesToSelect)->Wait();
    }

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

TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV1AndNavigationRoot_RetrievesSchemaAndReturnsCorrectResults)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v1.1/DataSources/foo/Navigation", request.GetUrl().c_str());
        return StubJsonHttpResponse(HttpStatus::OK, R"({"TestClass" : [ { "$id" : "TestId" } ]})");
        });
    GetHandler().ForRequest(3, [=] (HttpRequestCR request)
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

TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV1AndNavigationRootRequestedTwice_SendsGetSchemaRequestOnce)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    HttpResponse childrenResponse = StubJsonHttpResponse(HttpStatus::OK, R"({"TestClass" : [ { "$id" : "TestId" } ]})");

    GetHandler().ExpectRequests(4);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, childrenResponse);
    GetHandler().ForRequest(3, [=] (HttpRequestCR request)
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

TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV2AndNavigationRoot_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/Navigation/NavNode", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetChildrenRequest(ObjectId())->Wait();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());
    }

TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV2AndPropertiesSpecifiedToSelect_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
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

TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV2AndSpecificNavNode_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/Navigation/NavNode/Foo/NavNode", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetChildrenRequest(ObjectId("Navigation.NavNode", "Foo"))->Wait();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());
    }

TEST_F(WSRepositoryClientTests, SendGetChildrenRequest_WebApiV2AndResponseContainsInstance_SucceedsAndParsesInstance)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    StubInstances instances;
    instances.Add({"TestSchema.TestClass", "A"});
    HttpResponse response = StubHttpResponse(HttpStatus::OK, instances.ToJsonWebApiV2());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, response);

    auto result = client->SendGetChildrenRequest(ObjectId())->GetResult();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(ObjectId("TestSchema.TestClass", "A"), (*result.GetValue().GetInstances().begin()).GetObjectId());
    }

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV1AndQueryWithEmptyNavigationParentIdCustomParameter_MappedToNavigationRootQuery)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [&] (HttpRequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v1.1/DataSources/foo/Navigation", request.GetUrl());
        return StubHttpResponse();
        });

    WSQuery query("TestSchema", "TestClass");
    query.SetCustomParameter(WSQuery_CustomParameter_NavigationParentId, "");
    client->SendQueryRequest(query)->Wait();
    }

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV1AndQueryWithNavigationParentIdCustomParameter_MappedToNavigationQuery)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [&] (HttpRequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v1.1/DataSources/foo/Navigation/TestClass/TestId", request.GetUrl());
        return StubHttpResponse();
        });

    WSQuery query("TestSchema", "TestClass");
    query.SetCustomParameter(WSQuery_CustomParameter_NavigationParentId, "TestId");
    client->SendQueryRequest(query)->Wait();
    }

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV1AndQueryWithNavigationParentIdAndSelectProperties_MappedToNavigationQueryWithProperties)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [&] (HttpRequestCR request)
        {
        EXPECT_EQ("https://srv.com/ws/v1.3/DataSources/foo/Navigation?properties=Foo,Boo", request.GetUrl());
        return StubHttpResponse();
        });

    WSQuery query("TestSchema", "TestClass");
    query.SetCustomParameter(WSQuery_CustomParameter_NavigationParentId, "");
    query.SetSelect("Foo,Boo");
    client->SendQueryRequest(query)->Wait();
    }

TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1_SendsGetRequestWithFollowRedirects)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    BeFileName fileName = BeFileName("testFile");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v1.1/DataSources/foo/Files/TestClass/TestId", request.GetUrl().c_str());
        HttpFileBodyPtr httpFileBody = dynamic_cast<HttpFileBody*> (request.GetResponseBody().get());
        EXPECT_STREQ(fileName, httpFileBody->GetFilePath());
        return StubHttpResponse();
        });

    client->SendGetFileRequest({"TestSchema", "TestClass", "TestId"}, fileName)->Wait();
    }

TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1Connect_SendsGetRequestForRedirect)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    BeFileName fileName = BeFileName("testFile");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseBentleyConnectV1());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_FALSE(request.GetFollowRedirects());
        EXPECT_STREQ("https://srv.com/ws/DataSources/foo/Files/TestClass/TestId", request.GetUrl().c_str());
        EXPECT_EQ("", Utf8String(request.GetHeaders().GetIfNoneMatch()));
        EXPECT_EQ(nullptr, dynamic_cast<HttpFileBody*> (request.GetResponseBody().get()));
        return StubHttpResponse();
        });

    client->SendGetFileRequest({"TestSchema", "TestClass", "TestId"}, fileName, "TestETag")->Wait();
    }

TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1ConnectAndResponseFound_SendsGetRequestToLocation)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    BeFileName fileName = BeFileName("testFile");

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseBentleyConnectV1());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Found, "", {{"Location", "http://file.location/"}}));
    GetHandler().ForRequest(3, [=] (HttpRequestCR request)
        {
        EXPECT_EQ("http://file.location/", request.GetUrl());
        EXPECT_EQ("TestETag", Utf8String(request.GetHeaders().GetIfNoneMatch()));
        HttpFileBodyPtr httpFileBody = dynamic_cast<HttpFileBody*> (request.GetResponseBody().get());
        EXPECT_STREQ(fileName, httpFileBody->GetFilePath());
        return StubHttpResponse();
        });

    client->SendGetFileRequest({"TestSchema", "TestClass", "TestId"}, fileName, "TestETag")->Wait();
    }

TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1ConnectAndResponseOK_ReturnsServerNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    BeFileName fileName = BeFileName("testFile");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseBentleyConnectV1());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK));

    auto result = client->SendGetFileRequest({"TestSchema", "TestClass", "TestId"}, fileName)->GetResult();
    EXPECT_EQ(WSError::Status::ServerNotSupported, result.GetError().GetStatus());
    }

TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV2_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    BeFileName fileName = BeFileName("testFile");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass/TestId/$file", request.GetUrl().c_str());

        HttpFileBodyPtr httpFileBody = dynamic_cast<HttpFileBody*> (request.GetResponseBody().get());
        EXPECT_STREQ(fileName, httpFileBody->GetFilePath());

        return StubHttpResponse(HttpStatus::OK);
        });

    auto response = client->SendGetFileRequest({"TestSchema", "TestClass", "TestId"}, fileName)->GetResult();
    EXPECT_TRUE(response.IsSuccess());
    }

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

TEST_F(WSRepositoryClientTests, SendGetObjectRequest_WebApiV2_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/testSchema/testClass/testId", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetObjectRequest({"testSchema", "testClass", "testId"})->Wait();
    }

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV1_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v1.3/DataSources/foo/Objects/class1,class2?$select=testSelect", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    WSQuery query("testSchema", set<Utf8String> {"class1", "class2"});
    query.SetSelect("testSelect");

    client->SendQueryRequest(query)->Wait();
    }

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV12_SendsCorrectUrlWithMaxWebApi)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi12());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v1.2/DataSources/foo/Objects/class1,class2?$select=testSelect", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    WSQuery query("testSchema", set<Utf8String> {"class1", "class2"});
    query.SetSelect("testSelect");

    client->SendQueryRequest(query)->Wait();
    }

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV11_SendsCorrectUrlWithoutWebApi)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/DataSources/foo/Objects/class1,class2?$select=testSelect", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    WSQuery query("testSchema", set<Utf8String> {"class1", "class2"});
    query.SetSelect("testSelect");

    client->SendQueryRequest(query)->Wait();
    }

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV2_SendsCorrectUrl)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/testSchema/class1,class2?$select=testSelect", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    WSQuery query("testSchema", set<Utf8String> {"class1", "class2"});
    query.SetSelect("testSelect");

    client->SendQueryRequest(query)->Wait();
    }

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV21_ParsesInstanceETagAndAddsQuotes)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
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

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV22_ParsesInstanceETagDirectly)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi22());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
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

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV1SkipTokenSuppliedAndSentBack_IgnoresSkipTokens)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
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

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApi2SkipTokenEmpty_DoesNotSendSkipToken)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ(nullptr, request.GetHeaders().GetValue("SkipToken"));
        return StubHttpResponse();
        });

    client->SendQueryRequest(StubWSQuery(), nullptr, nullptr)->GetResult();
    }

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV2SkipTokenSupplied_SendsSkipToken)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("SomeSkipToken", request.GetHeaders().GetValue("SkipToken"));
        return StubHttpResponse();
        });

    client->SendQueryRequest(StubWSQuery(), nullptr, "SomeSkipToken")->GetResult();
    }

TEST_F(WSRepositoryClientTests, SendQueryRequest_WebApiV2AndHttpResponseHasSkipToken_AddsSkipTokenToResponse)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        return StubHttpResponse(HttpStatus::OK, StubInstances().ToJsonWebApiV2(), {{"SkipToken", "ServerSkipToken"}});
        });

    auto result = client->SendQueryRequest(StubWSQuery(), nullptr, nullptr)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().IsFinal());
    EXPECT_EQ("ServerSkipToken", result.GetValue().GetSkipToken());
    }

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
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v1.2/DataSources/foo/Objects/TestClass", request.GetUrl().c_str());
        EXPECT_EQ(objectCreationJson["instance"]["properties"], request.GetRequestBody()->AsJson());
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(objectCreationJson)->Wait();
    }

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
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v1.2/DataSources/foo/Objects/TestClass?parentClass=ParentClass&parentObjectId=ParentId", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(objectCreationJson)->Wait();
    }

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

TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2WithCorrectJson_TakesClassInfoFromJsonAndSendsObjectCreationJsonWithRequest)
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
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass", request.GetUrl().c_str());
        EXPECT_EQ(objectCreationJson, request.GetRequestBody()->AsJson());
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(objectCreationJson)->Wait();
    }

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
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass/TestId", request.GetUrl().c_str());
        EXPECT_EQ(objectCreationJson, request.GetRequestBody()->AsJson());
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(objectCreationJson)->Wait();
    }

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

    EXPECT_EQ(expectedObject, response.GetValue().GetObject());
    }

TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV1WithRelationshipWithWSChangeset_ConstructsWSG2FormatResponseForWSChangeset)
    {
    // Arrange
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Created, R"({ "id" : "NewRemoteId" })"));

    WSChangeset changeset (WSChangeset::Format::SingeInstance);
    changeset
        .AddInstance({"TestSchema.TestClass", "LocalId"}, WSChangeset::Created, nullptr)
        .AddRelatedInstance(ObjectId("TestRelSchema.TestRelClass", "RelId"), WSChangeset::Created, ECRelatedInstanceDirection::Forward, {"TestSchema.ParentClass", "ParentId"}, WSChangeset::Existing, nullptr);

    // Act
    auto response = client->SendCreateObjectRequest(ToJson(changeset.ToRequestString()))->GetResult();
    ASSERT_TRUE(response.IsSuccess());

    // Assert
    auto a = response.GetValue().GetObject().toStyledString();
    rapidjson::Document responseJson;
    JsonUtil::ToRapidJson(response.GetValue().GetObject(), responseJson);

    bmap<ObjectId, ObjectId> ids;
    EXPECT_EQ (SUCCESS, changeset.ExtractNewIdsFromResponse(responseJson, [&] (ObjectId oldId, ObjectId newId)
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

TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2_PassesResponseJsonAsObject)
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
    EXPECT_EQ(responseObject, response.GetValue().GetObject());
    }

#ifdef USE_GTEST
TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV1WithFilePath_AddsFileNameToContentDisposition)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    auto filePath = StubFile();
    auto fileName = Utf8String(filePath.GetFileNameAndExtension());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_THAT(request.GetHeaders().GetContentDisposition(), HasSubstr(fileName.c_str()));
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(StubWSObjectCreationJson(), filePath)->Wait();
    }

TEST_F(WSRepositoryClientTests, SendCreateObjectRequest_WebApiV2WithFilePath_AddsFileNameToContentDisposition)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    auto filePath = StubFile();
    auto fileName = Utf8String(filePath.GetFileNameAndExtension());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_THAT(request.GetHeaders().GetContentDisposition(), HasSubstr(fileName.c_str()));
        return StubHttpResponse();
        });

    client->SendCreateObjectRequest(StubWSObjectCreationJson(), filePath)->Wait();
    }
#endif

TEST_F(WSRepositoryClientTests, SendChangesetRequest_WebApiV13_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());

    auto result = client->SendChangesetRequest(HttpStringBody::Create("TestChangeset"), nullptr, nullptr)->GetResult();
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

TEST_F(WSRepositoryClientTests, SendChangesetRequest_WebApiV20_ErrorNotSupported)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());

    auto result = client->SendChangesetRequest(HttpStringBody::Create("TestChangeset"), nullptr, nullptr)->GetResult();
    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

TEST_F(WSRepositoryClientTests, SendChangesetRequest_WebApiV21_SendsRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.1/Repositories/foo/$changeset", request.GetUrl().c_str());
        EXPECT_STREQ("TestChangeset", request.GetRequestBody()->AsString().c_str());
        return StubHttpResponse();
        });

    client->SendChangesetRequest(HttpStringBody::Create("TestChangeset"), nullptr, nullptr)->Wait();
    }

TEST_F(WSRepositoryClientTests, SendChangesetRequest_WebApiV21AndReceives201_Error)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi21());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::Created, "{}"));

    auto result = client->SendChangesetRequest(HttpStringBody::Create("TestChangeset"), nullptr, nullptr)->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    }

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

TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV1_SendsPostRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    Json::Value propertiesJson = ToJson(R"({"TestProperty" : "TestValue" })");

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("POST", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v1.1/DataSources/foo/Objects/TestClass/TestId", request.GetUrl().c_str());
        EXPECT_EQ(propertiesJson, request.GetRequestBody()->AsJson());
        return StubHttpResponse();
        });

    client->SendUpdateObjectRequest({"TestSchema.TestClass", "TestId"}, propertiesJson)->Wait();
    }

TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV1AndETagPassed_SendsRequestWithIfMatch)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("TestETag", request.GetHeaders().GetIfMatch());
        return StubHttpResponse();
        });

    client->SendUpdateObjectRequest({"A.B", "C"}, Json::objectValue, "TestETag")->Wait();
    }

TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV1ResponseIsOK_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK));

    auto result = client->SendUpdateObjectRequest({"TestSchema.TestClass", "TestId"}, Json::objectValue)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV2_SendsPostRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
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
        EXPECT_EQ(expectedBodyJson, request.GetRequestBody()->AsJson());
        return StubHttpResponse();
        });

    client->SendUpdateObjectRequest({"TestSchema.TestClass", "TestId"}, ToJson(R"({"TestProperty" : "TestValue" })"))->Wait();
    }

TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV2AndETagPassed_SendsRequestWithoutIfMatch)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_EQ(nullptr, request.GetHeaders().GetIfMatch());
        return StubHttpResponse();
        });

    client->SendUpdateObjectRequest({"A.B", "C"}, Json::objectValue, "TestETag")->Wait();
    }

TEST_F(WSRepositoryClientTests, SendUpdateObjectRequest_WebApiV2ResponseIsOK_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK));

    auto result = client->SendUpdateObjectRequest({"TestSchema.TestClass", "TestId"}, Json::objectValue)->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_WebApiV1_SendsDeleteRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("DELETE", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v1.1/DataSources/foo/Objects/TestClass/TestId", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendDeleteObjectRequest({"TestSchema.TestClass", "TestId"})->Wait();
    }

TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_WebApiV1ResponseIsOK_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi11());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK));

    auto result = client->SendDeleteObjectRequest({"TestSchema.TestClass", "TestId"})->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_WebApiV2_SendsDeleteRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("DELETE", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass/TestId", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendDeleteObjectRequest({"TestSchema.TestClass", "TestId"})->Wait();
    }

TEST_F(WSRepositoryClientTests, SendDeleteObjectRequest_WebApiV2ResponseIsOK_Success)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, StubHttpResponse(HttpStatus::OK));

    auto result = client->SendDeleteObjectRequest({"TestSchema.TestClass", "TestId"})->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV1_SendsPutRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v1.1/DataSources/foo/Files/TestClass/TestId", request.GetUrl().c_str());
        return StubHttpResponse(HttpStatus::ResumeIncomplete);
        });
    GetHandler().ForRequest(3, [=] (HttpRequestCR request)
        {
        EXPECT_EQ("TestContent", ReadHttpBody(request.GetRequestBody()));
        return StubHttpResponse();
        });

    client->SendUpdateFileRequest({"TestSchema.TestClass", "TestId"}, StubFile("TestContent"))->Wait();
    }

TEST_F(WSRepositoryClientTests, SendUpdateFileRequest_WebApiV2_SendsPutRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(3);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("PUT", request.GetMethod().c_str());
        EXPECT_STREQ("https://srv.com/ws/v2.0/Repositories/foo/TestSchema/TestClass/TestId/$file", request.GetUrl().c_str());
        return StubHttpResponse(HttpStatus::ResumeIncomplete);
        });
    GetHandler().ForRequest(3, [=] (HttpRequestCR request)
        {
        EXPECT_EQ("TestContent", ReadHttpBody(request.GetRequestBody()));
        return StubHttpResponse();
        });

    client->SendUpdateFileRequest({"TestSchema.TestClass", "TestId"}, StubFile("TestContent"))->Wait();
    }

TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV11AndNoDefaultSchema_Fails)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ForFirstRequest(StubWSInfoHttpResponseWebApi11());
    auto result = client->SendGetSchemasRequest()->GetResult();

    EXPECT_EQ(WSError::Id::NotSupported, result.GetError().GetId());
    }

#ifdef USE_GTEST
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

TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV1_SendsGetSchemaRequestWithSuppliedETag)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("TestETag", request.GetHeaders().GetIfNoneMatch());
        EXPECT_STREQ("https://srv.com/ws/v1.2/DataSources/foo/Schema", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetSchemasRequest("TestETag")->Wait();
    }

TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV11Connect_SendsGetSchemaRequestWithSuppliedETag)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseBentleyConnectV1());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("TestETag", request.GetHeaders().GetIfNoneMatch());
        EXPECT_STREQ("https://srv.com/ws/DataSources/foo/Schema", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetSchemasRequest("TestETag")->Wait();
    }

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

TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV1ResponseWithSchema_ReturnsSchemaObject)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
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

TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV1ResponseWithSchema_ETagIsFromResponse)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
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

TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1AndDummySchemaObjectId_SendsGetSchemaRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi13());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("TestETag", request.GetHeaders().GetIfNoneMatch());
        EXPECT_STRCASEEQ("https://srv.com/ws/v1.2/DataSources/foo/Schema", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetFileRequest({"MetaSchema.ECSchemaDef", "DUMMY_SCHEMA_OBJECT..."}, StubFilePath(), "TestETag")->Wait();
    }

TEST_F(WSRepositoryClientTests, SendGetFileRequest_WebApiV1BentleyConnectAndDummySchemaObjectId_SendsGetSchemaRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseBentleyConnectV1());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("TestETag", request.GetHeaders().GetIfNoneMatch());
        EXPECT_STRCASEEQ("https://srv.com/ws/DataSources/foo/Schema", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetFileRequest({"MetaSchema.ECSchemaDef", "DUMMY_SCHEMA_OBJECT..."}, StubFilePath(), "TestETag")->Wait();
    }

#ifdef USE_GTEST
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

TEST_F(WSRepositoryClientTests, SendGetSchemasRequest_WebApiV2_SendsGetSchemasRequest)
    {
    auto client = WSRepositoryClient::Create("https://srv.com/ws", "foo", StubClientInfo(), nullptr, GetHandlerPtr());

    GetHandler().ExpectRequests(2);
    GetHandler().ForRequest(1, StubWSInfoHttpResponseWebApi20());
    GetHandler().ForRequest(2, [=] (HttpRequestCR request)
        {
        EXPECT_STRCASEEQ("https://srv.com/ws/v2.0/Repositories/foo/MetaSchema/ECSchemaDef", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetSchemasRequest()->Wait();
    }

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
