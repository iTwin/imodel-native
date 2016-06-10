/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Azure/EventServiceClientTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "EventServiceClientTests.h"

#include <WebServices/Azure/EventServiceClient.h>
#include <Bentley/BeThread.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

unsigned __stdcall workThreadHandler(void* arg)
    {
    EventServiceClient** clPtr = (EventServiceClient**)arg;
    printf("Entering thread\n");
    printf("receiving msgs..\n");
    Utf8String msg = "";
    while ((*clPtr)->Receive(msg))
        {
        printf("msg: %s\n", msg.c_str());
        }
    printf("Exiting thread\n");
    return 0;
    }

TEST_F(EventServiceClientTests, Test1)
    {
    Utf8String NAMESPACE1 = "BentleyEventService-579998";
    Utf8String REPO_ID = "TEST1";
    Utf8String USER_ID1 = "CONNECTION_ID1";
    Utf8String TOKEN = "SharedAccessSignature sr=https%3A%2F%2FBentleyEventService-579998.servicebus.windows.net%2FTEST1&sig=PvnMXq39FEngP%2Fiu5SYpz7JEMms4e%2BLQsPQFd6xOx5s%3D&se=1465574880&skn=EventReceivePolicy";
    EventServiceClient* client = new EventServiceClient(NAMESPACE1, REPO_ID, USER_ID1);
    client->UpdateSASToken(TOKEN);
    BeThreadUtilities::StartNewThread (1024 * 1024, workThreadHandler, &client);
    BeThreadUtilities::BeSleep(5000);
    printf("client->CancelRequest()\n");
    client->CancelRequest();
    BeThreadUtilities::BeSleep(10000);
    }