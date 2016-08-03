/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Ims/SolrClientTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#include "SolrClientTests.h"

#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include <WebServices/Connect/ImsClient.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Ims/SolrClient.h>
#include <WebServices/Ims/SolrQuery.h>

void SolrClientTests::SetUp()
    {
    WSClientBaseTest::SetUp();

    m_localState = StubLocalState();

    m_proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    m_serverUrl = "https://qa-waz-search.bentley.com/";
    m_credentials = Credentials("8cc45bd041514b58947ea6c09c@gmail.com", "qwe12312");

    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &m_localState);
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE_BENTLEY_HTTP, NativeLogging::LOG_INFO);
    }


TEST_F(SolrClientTests, SendQueryRequest_RequestWithTokenPreceededAuthenticationStringForIMSUserService_HttpResultIsInvalid)
    {
    Utf8String collection("IMS/User");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);
    auto result = client->SendGetRequest()->GetResult();
    ASSERT_FALSE(result.IsSuccess());

    auto error = result.GetError();
    EXPECT_EQ(HttpStatus::Unauthorized, error.GetHttpStatus());
    }

TEST_F(SolrClientTests, SendQueryRequest_RequestUsingInvalidBaseServerUrlForImsUserService_Returns404NotFound)
    {
    Utf8String serverUrl = "https://search.bentley.com/";
    Utf8String collection("IMS/User");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(serverUrl, manager->GetTokenProvider(serverUrl), m_proxy, true);

    auto client = SolrClient::Create(serverUrl, collection, StubValidClientInfo(), authHandler);

    auto result = client->SendGetRequest()->GetResult();
    ASSERT_FALSE(result.IsSuccess());

    auto error = result.GetError();
    EXPECT_EQ(ConnectionStatus::CouldNotConnect, error.GetConnectionStatus());
    EXPECT_EQ(HttpStatus::None, error.GetHttpStatus());
    }

TEST_F(SolrClientTests, SendQueryRequest_RequestUsingInvalidCollectionString_Returns404NotFound)
    {
    Utf8String collection("InvalidCollection");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    auto result = client->SendGetRequest()->GetResult();
    ASSERT_FALSE(result.IsSuccess());

    auto error = result.GetError();
    EXPECT_EQ(HttpStatus::NotFound, error.GetHttpStatus());
    }

TEST_F(SolrClientTests, SendQueryRequest_RequestUsingInvalidSelectParameterForImsUserService_Returns0Results)
    {
    Utf8String collection("IMS/User");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetSelect("InvalidCategory");
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Json::Value resultUserData = result.GetValue()["response"]["docs"][0];
    EXPECT_EQ(0, resultUserData.size());
    }

TEST_F(SolrClientTests, SendQueryRequest_RequestUsingInvalidFilterParameterForImsUserService_Returns400BadRequest)
    {
    Utf8String collection("IMS/User");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetFilter("InvalidCategory:InvalidValue");
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_FALSE(result.IsSuccess());

    auto error = result.GetError();
    EXPECT_EQ(HttpStatus::BadRequest, error.GetHttpStatus());
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsInvalidRowParameterAndRowsDefaultsTo1ForImsUserService_HttpRequestSucceedsWithRowsSetTo1)
    {
    Utf8String collection("IMS/User");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetRows(-1);
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    int numResultsFound = result.GetValue()["response"]["numFound"].asInt();
    Json::Value resultUserData = result.GetValue()["response"]["docs"][0];

    EXPECT_EQ(1, numResultsFound);
    EXPECT_EQ(32, resultUserData.size());
    }


TEST_F(SolrClientTests, SendQueryRequest_SetsInvalidStartParameterAndStartDefaultsTo0ForImsUserService_HttpRequestSucceedsWithStartSetTo0)
    {
    Utf8String collection("IMS/User");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetStart(-1);
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    int numResultsFound = result.GetValue()["response"]["numFound"].asInt();
    Json::Value resultUserData = result.GetValue()["response"]["docs"][0];

    EXPECT_EQ(1, numResultsFound);
    EXPECT_EQ(32, resultUserData.size());
    }

TEST_F(SolrClientTests, SendQueryRequest_SendsVanillaRequestToGetAllTokenInformationForImsUserService_HttpResponseSuccessful)
    {
    Utf8String collection("IMS/User");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    auto result = client->SendGetRequest()->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Utf8String expectedCreatedDate("2015-04-03T07:48:46.517Z");
    Utf8String expectedJobTitle("Fiddler");
    Utf8String expectedFirstName("John");
    Utf8String expectedUserId("53E87DE9-ED14-4EF1-BD78-35E8218BB5CC");
    Utf8String expectedLastName("Stevenson");
    Utf8String expectedAccountName("Pennsylvania DOT");

    int numResultsFound = result.GetValue()["response"]["numFound"].asInt();
    Json::Value resultUserData = result.GetValue()["response"]["docs"][0];
    
    EXPECT_EQ(1, numResultsFound);
    EXPECT_STREQ(expectedCreatedDate.c_str(), resultUserData["CreatedDate"].asCString());
    EXPECT_STREQ(expectedJobTitle.c_str(), resultUserData["JobTitle"].asCString());
    EXPECT_STREQ(expectedFirstName.c_str(), resultUserData["FirstName"].asCString());
    EXPECT_STREQ(expectedUserId.c_str(), resultUserData["UserId"].asCString());
    EXPECT_STREQ(expectedLastName.c_str(), resultUserData["LastName"].asCString());
    EXPECT_STREQ(expectedAccountName.c_str(), resultUserData["AccountName"].asCString());
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsSelectForUserIdForImsUserService_HttpResponseOnlyReturnUserIdCategory)
    {
    Utf8String collection("IMS/User");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetSelect("UserId");
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Utf8String expectedUserId("53E87DE9-ED14-4EF1-BD78-35E8218BB5CC");

    Json::Value resultUserData = result.GetValue()["response"]["docs"][0];

    EXPECT_EQ(1, resultUserData.size());
    EXPECT_STREQ(expectedUserId.c_str(), resultUserData["UserId"].asCString());
    }


TEST_F(SolrClientTests, SendQueryRequest_SetsMultiSelectForFirstNameAndLastNameForImsUserService_HttpResponseOnlyReturnsFirstNameAndLastName)
    {
    Utf8String collection("IMS/User");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetSelect("FirstName")
        .AddSelect("LastName");
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Utf8String expectedFirstName("John");
    Utf8String expectedLastName("Stevenson");

    Json::Value resultUserData = result.GetValue()["response"]["docs"][0];

    EXPECT_EQ(2, resultUserData.size());
    EXPECT_STREQ(expectedFirstName.c_str(), resultUserData["FirstName"].asCString());
    EXPECT_STREQ(expectedLastName.c_str(), resultUserData["LastName"].asCString());
    }

TEST_F(SolrClientTests, SendQueryRequest_RequestWithTokenPreceededAuthenticationStringForNotificationService_HttpResultIsInvalid)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);
    auto result = client->SendGetRequest()->GetResult();
    ASSERT_FALSE(result.IsSuccess());

    auto error = result.GetError();
    EXPECT_EQ(HttpStatus::Unauthorized, error.GetHttpStatus());
    }

TEST_F(SolrClientTests, SendQueryRequest_RequestUsingInvalidBaseServerUrlForNotificationService_Returns404NotFound)
    {
    Utf8String collection("InvalidNotificationCollection");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    auto result = client->SendGetRequest()->GetResult();
    ASSERT_FALSE(result.IsSuccess());

    auto error = result.GetError();
    EXPECT_EQ(HttpStatus::NotFound, error.GetHttpStatus());
    }

TEST_F(SolrClientTests, SendQueryRequest_RequestUsingInvalidSelectParameterForNotificationService_Returns0Results)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetSelect("InvalidCategory");
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Json::Value resultNotificationData = result.GetValue()["response"]["docs"][0];
    EXPECT_EQ(0, resultNotificationData.size());
    }

TEST_F(SolrClientTests, SendQueryRequest_RequestUsingInvalidFilterParameterForNotificationService_Returns400BadRequest)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetFilter("InvalidCategory:InvalidValue");
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_FALSE(result.IsSuccess());

    auto error = result.GetError();
    EXPECT_EQ(HttpStatus::BadRequest, error.GetHttpStatus());
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsInvalidRowParameterAndRowsDefaultsTo1ForNotificationService_HttpRequestSucceedsWithRowsSetTo1)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetRows(-1);
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    int numResultsFound = result.GetValue()["response"]["numFound"].asInt();
    Json::Value resultNotificationData = result.GetValue()["response"]["docs"];

    EXPECT_EQ(25, numResultsFound);
    EXPECT_EQ(1, resultNotificationData.size());
    }


TEST_F(SolrClientTests, SendQueryRequest_SetsInvalidStartParameterAndStartDefaultsTo0ForNotificationService_HttpRequestSucceedsWithStartSetTo0)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetStart(-1);
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    int numResultsFound = result.GetValue()["response"]["numFound"].asInt();
    Json::Value resultNotificationData = result.GetValue()["response"]["docs"];

    EXPECT_EQ(25, numResultsFound);
    EXPECT_EQ(10, resultNotificationData.size());
    }

TEST_F(SolrClientTests, SendQueryRequest_SendsVanillaRequestToGetAllTokenInformationForNotificationService_HttpResponseSuccessful)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetRows(100);
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Utf8String expectedSenderEmail("David.Jones@BENTLEY.COM");
    Utf8String expectedUserId("53E87DE9-ED14-4EF1-BD78-35E8218BB5CC");

    int numResultsFound = result.GetValue()["response"]["numFound"].asInt();
    Json::Value resultNotificationData = result.GetValue()["response"]["docs"];
    
    EXPECT_EQ(25, numResultsFound);
    EXPECT_EQ(25, resultNotificationData.size());
    for (Json::Value notification : resultNotificationData)
        {
        EXPECT_STREQ(expectedSenderEmail.c_str(), notification["Sender"].asCString());
        EXPECT_STREQ(expectedUserId.c_str(), notification["UserId"].asCString());
        }
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsSelectForUserIdForNotificationService_HttpResponseOnlyReturnUserIdCategory)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetSelect("UserId");
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Utf8String expectedUserId("53E87DE9-ED14-4EF1-BD78-35E8218BB5CC");

    Json::Value resultNotificationData = result.GetValue()["response"]["docs"];
    for (Json::Value notification : resultNotificationData)
        {
        EXPECT_EQ(1, notification.size());
        EXPECT_STREQ(expectedUserId.c_str(), notification["UserId"].asCString());
        }
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsMultiSelectForUserIdAndSenderForNotificationService_HttpResponseOnlyReturnsUserIdAndSender)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetSelect("UserId")
        .AddSelect("Sender");
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Utf8String expectedSenderEmail("David.Jones@BENTLEY.COM");
    Utf8String expectedUserId("53E87DE9-ED14-4EF1-BD78-35E8218BB5CC");

    Json::Value resultNotificationData = result.GetValue()["response"]["docs"];
    for (Json::Value notification : resultNotificationData)
        {
        EXPECT_EQ(2, notification.size());
        EXPECT_STREQ(expectedSenderEmail.c_str(), notification["Sender"].asCString());
        EXPECT_STREQ(expectedUserId.c_str(), notification["UserId"].asCString());
        }
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsRowsTo5ForNotificationService_HttpResponseOnlySendBack5Results)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetRows(5);
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Json::Value resultNotificationData = result.GetValue()["response"]["docs"];
    EXPECT_EQ(5, resultNotificationData.size());
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsRowsTo100ForNotificationService_HttpResponseSendsBackAllData)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetRows(100);
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Json::Value resultNotificationData = result.GetValue()["response"]["docs"];
    EXPECT_EQ(25, resultNotificationData.size());
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsRowsTo100AndStartAt10ForNotificationService_HttpResponseSendsBackResultsFrom11To25)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetRows(100)
        .SetStart(10);
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Json::Value resultNotificationData = result.GetValue()["response"]["docs"];
    EXPECT_EQ(15, resultNotificationData.size());
    }

TEST_F(SolrClientTests, SendQueryRequest_SetsStartPastNumberOfUserNotificationForNotificationService_HttpResponseSendsBack0Results)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetRows(100)
        .SetStart(25);
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Json::Value resultNotificationData = result.GetValue()["response"]["docs"];
    EXPECT_EQ(0, resultNotificationData.size());
    }

TEST_F(SolrClientTests, SendQueryRequest_SetIndentTrueForNotificationService_HttpResponseHeaderHasIndentationSetToTrue)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetIndent(true);
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Json::Value resultNotificationParams = result.GetValue()["responseHeader"]["params"];
    EXPECT_STREQ("true", resultNotificationParams["indent"].asCString());
    }

TEST_F(SolrClientTests, SendQueryRequest_SetIndentFalseForNotificationService_HttpResponseHeaderHasIndentationSetToFalse)
    {
    Utf8String collection("Notification");

    auto manager = ConnectSignInManager::Create(StubValidClientInfo(), m_proxy, &m_localState);
    ASSERT_TRUE(manager->SignInWithCredentials(m_credentials)->GetResult().IsSuccess());
    auto authHandler = std::make_shared<ConnectAuthenticationHandler>(m_serverUrl, manager->GetTokenProvider(m_serverUrl), m_proxy, true);

    auto client = SolrClient::Create(m_serverUrl + "token", collection, StubValidClientInfo(), authHandler);

    SolrQuery query;
    query.SetIndent(false);
    auto result = client->SendGetRequest(query)->GetResult();
    ASSERT_TRUE(result.IsSuccess());

    Json::Value resultNotificationParams = result.GetValue()["responseHeader"]["params"];
    EXPECT_STREQ("false", resultNotificationParams["indent"].asCString());
    }