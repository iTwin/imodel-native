/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Ims/SolrClientTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SolrClientTests.h"
#include <WebServices/Ims/SolrQuery.h>
#include <WebServices/Ims/SolrClient.h>


using namespace ::testing;
using namespace ::std;

USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(SolrClientTests, Ctor_ValidBaseUrl_SetsValidBaseUrl)
    {
    Utf8String collection("IMS/User");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    EXPECT_STREQ("https://srv.com/token", client->GetBaseUrl().c_str());
    }

TEST_F(SolrClientTests, Ctor_ValidCollection_SetsValidCollectionPath)
    {
    Utf8String collection("IMS/User");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    EXPECT_STREQ("IMS/User", client->GetCollectionPath().c_str());
    }

TEST_F(SolrClientTests, Ctor_StringWithLeadingForwardSlashes_ShouldRemoveSlashesAndSetValidCollectionPath)
    {
    Utf8String collection(R"(/IMS/User)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    EXPECT_STREQ("IMS/User", client->GetCollectionPath().c_str());
    }

TEST_F(SolrClientTests, Ctor_StringWithTrailingForwardSlashes_ShouldRemoveSlashesAndSetValidCollectionPath)
    {
    Utf8String collection(R"(IMS/User/)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    EXPECT_STREQ("IMS/User", client->GetCollectionPath().c_str());
    }

TEST_F(SolrClientTests, Ctor_StringWithLeadingBackwardSlashes_ShouldRemoveSlashesAndSetValidCollectionPath)
    {
    Utf8String collection(R"(\IMS/User)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    EXPECT_STREQ("IMS/User", client->GetCollectionPath().c_str());
    }

TEST_F(SolrClientTests, Ctor_StringWithTrailingBackwardSlashes_ShouldRemoveSlashesAndSetValidCollectionPath)
    {
    Utf8String collection(R"(IMS/User\)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    EXPECT_STREQ("IMS/User", client->GetCollectionPath().c_str());
    }

TEST_F(SolrClientTests, Ctor_StringWithLeadingAndTrailingBackwardSlashes_ShouldRemoveSlashesAndSetValidCollectionPath)
    {
    Utf8String collection(R"(\IMS/User\)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    EXPECT_STREQ("IMS/User", client->GetCollectionPath().c_str());
    }

TEST_F(SolrClientTests, Ctor_StringWithLeadingAndTrailingForwardSlashes_ShouldRemoveSlashesAndSetValidCollectionPath)
    {
    Utf8String collection(R"(/IMS/User/)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    EXPECT_STREQ("IMS/User", client->GetCollectionPath().c_str());
    }

TEST_F(SolrClientTests, Ctor_StringWithLeadingAndTrailingSlashes_ShouldRemoveSlashesAndSetValidCollectionPath)
    {
    Utf8String collection(R"(\/\IMS/User\/)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    EXPECT_STREQ("IMS/User", client->GetCollectionPath().c_str());
    }

TEST_F(SolrClientTests, SendGetRequest_ValidCollection_SendsCorrectUrl)
    {
    Utf8String collection("IMS/User");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRequest()->Wait();
    }

TEST_F(SolrClientTests, SendGetRequest_BaseUrlWithTrailingForwardSlashAndCollectionWithLeadingForwardSlash_SendsCorrectUrl)
    {
    Utf8String collection(R"(/IMS/User)");

    auto client = SolrClient::Create("https://srv.com/token/", collection, StubClientInfo(), GetHandlerPtr());
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRequest()->Wait();
    }

TEST_F(SolrClientTests, SendGetRequest_StringWithLeadingForwardSlashes_ShouldRemoveSlashesAndSendCorrectUrl)
    {
    Utf8String collection(R"(/IMS/User)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRequest()->Wait();
    }

TEST_F(SolrClientTests, SendGetRequest_StringWithTrailingForwardSlashes_ShouldRemoveSlashesAndSendCorrectUrl)
    {
    Utf8String collection(R"(IMS/User/)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRequest()->Wait();
    }

TEST_F(SolrClientTests, SendGetRequest_StringWithLeadingBackwardSlashes_ShouldRemoveSlashesAndSendCorrectUrl)
    {
    Utf8String collection(R"(\IMS/User)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRequest()->Wait();
    }

TEST_F(SolrClientTests, SendGetRequest_StringWithTrailingBackwardSlashes_ShouldRemoveSlashesAndSendCorrectUrl)
    {
    Utf8String collection(R"(IMS/User\)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRequest()->Wait();
    }

TEST_F(SolrClientTests, SendGetRequest_StringWithLeadingAndTrailingBackwardSlashes_ShouldRemoveSlashesAndSendCorrectUrl)
    {
    Utf8String collection(R"(\IMS/User\)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRequest()->Wait();
    }

TEST_F(SolrClientTests, SendGetRequest_StringWithLeadingAndTrailingForwardSlashes_ShouldRemoveSlashesAndSendCorrectUrl)
    {
    Utf8String collection(R"(/IMS/User/)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRequest()->Wait();
    }

TEST_F(SolrClientTests, SendGetRequest_StringWithLeadingAndTrailingSlashes_ShouldRemoveSlashesAndSendCorrectUrl)
    {
    Utf8String collection(R"(\/\IMS/User\/)");

    auto client = SolrClient::Create("https://srv.com/token", collection, StubClientInfo(), GetHandlerPtr());
    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRequest()->Wait();
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsQueryStringForEmail_ConstructsCorrectQueryUrl)
    {
    auto client = SolrClient::Create("https://srv.com/token", Utf8String("IMS/User"), StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select?q=Email:%22test.email@testemail.com%22", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    SolrQuery query;
    query.SetQuery("Email:\"test.email@testemail.com\"");
    client->SendGetRequest(query)->Wait();
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsQueryStringForEmailAndJsonResponse_ConstructsCorrectQueryUrl)
    {
    auto client = SolrClient::Create("https://srv.com/token", Utf8String("IMS/User"), StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select?q=Email:%22test.email@testemail.com%22&wt=json", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    SolrQuery query;
    query.SetQuery("Email:\"test.email@testemail.com\"")
        .SetResponseFormat("json");
    client->SendGetRequest(query)->Wait();
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsQueryStringForEmailAndJsonResponseAndIndentation_ConstructsCorrectQueryUrl)
    {
    auto client = SolrClient::Create("https://srv.com/token", Utf8String("IMS/User"), StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select?q=Email:%22test.email@testemail.com%22&wt=json&indent=true", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    SolrQuery query;
    query.SetQuery("Email:\"test.email@testemail.com\"")
        .SetResponseFormat("json")
        .SetIndent(true);
    client->SendGetRequest(query)->Wait();
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsQueryStringForEmailAndJsonResponseAndIndentationAndSelectionForUserId_ConstructsCorrectQueryUrl)
    {
    auto client = SolrClient::Create("https://srv.com/token", Utf8String("IMS/User"), StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=](HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select?q=Email:%22test.email@testemail.com%22&fl=UserId&wt=json&indent=true", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    SolrQuery query;
    query.SetQuery("Email:\"test.email@testemail.com\"")
        .SetResponseFormat("json")
        .SetIndent(true)
        .SetSelect("UserId");
    client->SendGetRequest(query)->Wait();
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsQueryStringForRoleIdAndJsonResponseAndIndentationAndFilterForOrgIdAndSelectForUserId_ConstructsCorrectQueryUrl)
    {
    auto client = SolrClient::Create("https://srv.com/token", Utf8String("IMS/User"), StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=](HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select?q=RoleId:%22123456-123456-123456%22&fq=OrganizationId:%22654321-654321-654321%22&fl=UserId&wt=json&indent=true", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    SolrQuery query;
    query.SetQuery("RoleId:\"123456-123456-123456\"")
        .SetResponseFormat("json")
        .SetIndent(true)
        .SetFilter("OrganizationId:\"654321-654321-654321\"")
        .SetSelect("UserId");
    client->SendGetRequest(query)->Wait();
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsAllQueryParametersExceptSort_ConstructsCorrectQueryUrl)
    {
    auto client = SolrClient::Create("https://srv.com/token", Utf8String("IMS/User"), StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=](HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select?q=*:*&fq=UserId:(%22654321-654321-654321%22 OR %22567890-567890-567890%22)&start=0&rows=100&wt=json&indent=true", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    SolrQuery query;
    query.SetQuery("*:*")
        .SetResponseFormat("json")
        .SetIndent(true)
        .SetFilter("UserId:(\"654321-654321-654321\" OR \"567890-567890-567890\")")
        .SetStart(0)
        .SetRows(100);
    client->SendGetRequest(query)->Wait();
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsQueryStringForUserIdAndJsonResponseAndIndentationAndSelectForEmail_ConstructsCorrectQueryUrl)
    {
    auto client = SolrClient::Create("https://srv.com/token", Utf8String("IMS/User"), StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=](HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select?q=UserId:%221234567890-1234567890%22&fl=Email&wt=json&indent=true", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    SolrQuery query;
    query.SetQuery("UserId:\"1234567890-1234567890\"")
        .SetResponseFormat("json")
        .SetIndent(true)
        .SetSelect("Email");
    client->SendGetRequest(query)->Wait();
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsJsonResponseAndIndentationAndSort_ConstructsCorrectQueryUrl)
    {
    auto client = SolrClient::Create("https://srv.com/token", Utf8String("IMS/User"), StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=](HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select?sort=FirstName desc&wt=json&indent=true", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    SolrQuery query;
        query.SetResponseFormat("json")
        .SetIndent(true)
        .SetSort("FirstName desc");
    client->SendGetRequest(query)->Wait();
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsSortMultipleTimes_ConstructsCorrectQueryUrlWithMultipleSortsCommaDelimeted)
    {
    auto client = SolrClient::Create("https://srv.com/token", Utf8String("IMS/User"), StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=] (HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select?sort=FirstName desc,LastName asc,UserId desc&wt=json&indent=true", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    SolrQuery query;
    query.SetResponseFormat("json")
        .SetIndent(true)
        .SetSort("FirstName desc")
        .AddSort("LastName asc")
        .AddSort("UserId desc");
    client->SendGetRequest(query)->Wait();  
    }

TEST_F(SolrClientTests, SendQueryRequest_NoQueryProvided_ConstructsCorrectQueryUrlAsVanillaUrl)
    {
    auto client = SolrClient::Create("https://srv.com/token", Utf8String("IMS/User"), StubClientInfo(), GetHandlerPtr());

    GetHandler().ExpectRequests(1);
    GetHandler().ForRequest(1, [=](HttpRequestCR request)
        {
        EXPECT_STREQ("https://srv.com/token/IMS/User/select", request.GetUrl().c_str());
        return StubHttpResponse();
        });

    client->SendGetRequest()->Wait();
    }
