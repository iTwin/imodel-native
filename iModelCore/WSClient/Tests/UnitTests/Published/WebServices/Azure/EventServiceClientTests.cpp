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

TEST_F(EventServiceClientTests, Test1)
    {
    Utf8String SERVICE_NAMESPACE = "testhubjeehwan-ns";
    Utf8String REPO_ID = "TEST0";
    Utf8String USER_ID1 = "USER_ID1";
    EventServiceClient* client = new EventServiceClient(SERVICE_NAMESPACE, REPO_ID, USER_ID1);
    printf("receiving msgs..\n");
    Utf8String msg = "";
    while (client->Receive(msg))
        {
        printf("msg: %s\n", msg.c_str());
        }
    }