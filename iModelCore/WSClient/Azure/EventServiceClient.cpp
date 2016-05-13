/*--------------------------------------------------------------------------------------+
|
|     $Source: Azure/EventServiceClient.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Azure/EventServiceClient.h>
#include <iomanip>
#include <Bentley/Base64Utilities.h>

EventServiceClient::EventServiceClient(Utf8StringCR nameSpace, Utf8StringCR repoId, Utf8StringCR userId, int longPollingTimeout)
{
	m_timeout = longPollingTimeout;
	char numBuffer[10];
	itoa(longPollingTimeout, numBuffer, 10);
	Utf8String timeout = Utf8String(numBuffer);
	m_nameSpace = nameSpace;
	m_repoId = repoId;
	m_userId = userId;
	Utf8String baseAddress = "https://" + nameSpace + "." + "servicebus.windows.net/";
	m_fullAddress = baseAddress + repoId + "/Subscriptions/" + userId + "/messages/head?timeout=" + timeout;
}

bool EventServiceClient::Receive (Utf8StringR msgOut, Utf8StringCR token)
{
	HttpRequest request(m_fullAddress.c_str(), "DELETE", nullptr);
	request.GetHeaders ().Clear ();
	request.GetHeaders ().SetValue ("Content-Length", "0");
	request.GetHeaders ().SetValue ("Content-Type", "application/atom+xml;type=entry;charset=utf-8");
	request.GetHeaders ().SetAuthorization (token);
	// request.SetTimeoutSeconds(m_timeout);
	HttpResponse response = request.Perform();

	if (!response.IsSuccess())
	{
		printf("!!!!!response.IsSuccess()\n");
		return false;
	}
	else
		printf("response.IsSuccess(): %s\n", response.GetBody().AsString().c_str());
	return true;
}