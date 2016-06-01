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
    request.GetHeaders().SetValue("Content-Type", "application/atom+xml;type=entry;charset=utf-8");
    request.GetHeaders().SetAuthorization(m_token);
    request.SetTransferTimeoutSeconds(230);
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
* @bsimethod                                             Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponse EventServiceClient::MakeSendRequest(Utf8String msg) //temporary, for testing
    {
    char numBuffer[10];
    itoa(230, numBuffer, 10); //max timeout for service bus rest api is set to 230 seconds
    //Utf8String url = m_fullAddress + Utf8String(numBuffer);
    Utf8String url = m_fullAddress;
    url.ReplaceAll("head?timeout=", "");
    const char *test = url.c_str();
    printf("%s\n", test);
    HttpRequest request(url.c_str(), "POST");
    request.GetHeaders().Clear();
    //request.GetHeaders().SetValue("Content-Length", "0");
    request.GetHeaders().SetValue("Content-Type", "application/atom+xml;type=entry;charset=utf-8");
    request.GetHeaders().SetValue("BrokerProperties", "{ \"TimeToLive\":30, \"Label\":\"M1\"}");
    request.GetHeaders().SetAuthorization(m_token);
    request.SetTransferTimeoutSeconds(230);
    HttpStringBodyPtr body = HttpStringBody::Create(msg);
    request.SetRequestBody(body);
    return request.Perform();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Arvind.Venkateswaran   06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool EventServiceClient::Send(Utf8String msg) //temporary, for testing
    {
    HttpResponse response = MakeSendRequest(msg);
    switch (response.GetHttpStatus())
        {
            case HttpStatus::OK: 
                return true;
            case HttpStatus::Created: 
                return true;
            case HttpStatus::BadRequest: 
                return false;
            case HttpStatus::NoContent: 
                return false;
            case HttpStatus::NotFound: 
                return false;
            case HttpStatus::Unauthorized: 
                return false;
            default:
                return false;
        }
    }

