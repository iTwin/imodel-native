/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Azure/EventServiceClientTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "EventServiceClientTests.h"
#include <WebServices/Azure/EventServiceClient.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

void EventServiceClientTests::SetUp()
    {
    credentials.push_back(*(new Credentials("eventservicetest1@mailinator.com", "Bentley123")));
    credentials.push_back(*(new Credentials("eventservicetest2@mailinator.com", "Bentley123")));
    servers.push_back("https://127.255.0.0:444");
    }

TEST_F(EventServiceClientTests, Test1)
    {
    //Utf8String REPO_ID = "TEST0";
    Utf8String REPO_ID = "CSim";
    Utf8String USER_ID = "USER_ID1";
    //EventServiceClient* client = new EventServiceClient(REPO_ID, USER_ID1);
    EventServiceClient* client = new EventServiceClient(REPO_ID, USER_ID, servers[0], credentials[0]);
    printf("receiving msgs..\n");
    Utf8String msg = "";
    while (client->Receive(msg))
        {
        printf("msg: %s\n", msg.c_str());
        }
    }

TEST_F(EventServiceClientTests, BasicFromWSRepositoryClient)
    {
    Utf8String sasToken = nullptr, ns = nullptr;
    Utf8String REPO_ID = "CSim";
    Utf8String USER_ID = "USER_ID1";
    EventServiceClient* client = new EventServiceClient(REPO_ID, USER_ID, servers[0], credentials[0]);
    bool result = client->GetInfoThroughWSRepositoryClient(sasToken, ns);
    EXPECT_TRUE(result);
    }

TEST_F(EventServiceClientTests, BasicFromIMSClient)
    {
    Utf8String sasToken = nullptr, ns = nullptr;
    Utf8String REPO_ID = "CSim";
    Utf8String USER_ID = "USER_ID1";
    EventServiceClient* client = new EventServiceClient(REPO_ID, USER_ID, servers[0], credentials[0]);
    bool result = client->GetInfoThroughIMSClient(sasToken, ns);
    EXPECT_TRUE(result);
    }