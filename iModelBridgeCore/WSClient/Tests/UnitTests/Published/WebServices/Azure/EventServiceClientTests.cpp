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
	//Utf8String SA_KEY_NAME = "RootManageSharedAccessKey";
	//Utf8String SA_KEY_VALUE = "Pz2HmbIvXLj7ntcgiwgxKzYgcXZxyJrobQLkO+u1jVc=";
	Utf8String SERVICE_NAMESPACE = "testhubjeehwan-ns";
	Utf8String REPO_ID = "TEST";
	Utf8String USER_ID1 = "USER_ID1";
	Utf8String token = "SharedAccessSignature sig=Z7WLDGx%2f3Vas2o8pLN8YCL5CBxEWA4uLcNKobWFTGRA%3d&se=1463160725&skn=RootManageSharedAccessKey&sr=https%3a%2f%2ftesthubjeehwan-ns.servicebus.windows.net%2ftest";

	EventServiceClient* client = new EventServiceClient(SERVICE_NAMESPACE, REPO_ID, USER_ID1);
	printf("receiving msgs..\n");
	Utf8String msg = "";
	while (client->Receive(msg, token))
	{
		;
	}
	// EXPECT_TRUE(false);
}