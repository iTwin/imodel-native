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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
EventServiceClient::EventServiceClient(Utf8StringCR nameSpace, Utf8StringCR repoId, Utf8StringCR userId)
    {
    m_nameSpace = nameSpace;
    m_repoId = repoId;
    m_userId = userId;
    Utf8String baseAddress = "https://" + m_nameSpace + "." + "servicebus.windows.net/";
    m_fullAddress = baseAddress + repoId + "/Subscriptions/" + userId + "/messages/head?timeout=";
    m_ct = SimpleCancellationToken::Create ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
//Need a default constructor for DgnDbClientRepository
EventServiceClient::EventServiceClient()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponse EventServiceClient::MakeReceiveDeleteRequest(bool longPolling)
    {
    char numBuffer[10];
    if (longPolling)
        itoa(230, numBuffer, 10); //max timeout for service bus rest api is set to 230 seconds
    else
        itoa(0, numBuffer, 10);
    Utf8String url = m_fullAddress + Utf8String(numBuffer);
    HttpRequest request(url.c_str(), "DELETE", nullptr);
    request.GetHeaders().Clear();
    request.GetHeaders().SetValue("Content-Length", "0");
    //request.GetHeaders().SetContentType("application/json");
    request.GetHeaders().SetValue("Content-Type", "application/atom+xml;type=entry;charset=utf-8");
    request.GetHeaders().SetAuthorization(m_token);
    request.SetTransferTimeoutSeconds(230);
    m_ct = SimpleCancellationToken::Create ();
    request.SetCancellationToken(m_ct);
    return request.Perform();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
//todo: need to return with more error status
bool EventServiceClient::Receive(Utf8StringR msgOut, bool longPolling)
    {
    HttpResponse response = MakeReceiveDeleteRequest(longPolling);
    switch (response.GetHttpStatus())
        {
            case HttpStatus::OK: //received a message
                msgOut = response.GetBody().AsString();
                return true;
            case HttpStatus::NoContent: //no incoming message
                msgOut = NULL;
                return true;
            case HttpStatus::NotFound: //subscription not setup yet
                return false;
            case HttpStatus::Unauthorized: //token may be expired
                return false;
            default:
                return false;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void EventServiceClient::UpdateSASToken(Utf8StringCR sasToken)
    {
    m_token = sasToken;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void EventServiceClient::CancelRequest()
    {
    m_ct->SetCanceled();
    }