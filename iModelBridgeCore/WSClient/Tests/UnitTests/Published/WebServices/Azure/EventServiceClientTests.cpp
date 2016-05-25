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

TEST_F(EventServiceClientTests, Test1)
    {
    Utf8String NAMESPACE1 = "bentleyeventservice-792202";
    Utf8String REPO_ID = "TEST1";
    Utf8String USER_ID1 = "CONNECTION_ID1";
    Utf8String TOKEN = "SharedAccessSignature sr=https%3A%2F%2FBentleyEventService-792202.servicebus.windows.net%2FTEST1&sig=FuJ4YftYW%2FMm6mUKtVLY3tLxzbWPHHmeK8%2F5ejfDsBA%3D&se=1464198677&skn=EventReceivePolicy";
    EventServiceClient* client = new EventServiceClient(NAMESPACE1, REPO_ID, USER_ID1);
    client->UpdateSASToken(TOKEN);
    printf("receiving msgs..\n");
    Utf8String msg = "";
    while (client->Receive(msg))
        {
        printf("msg: %s\n", msg.c_str());
        }
    }