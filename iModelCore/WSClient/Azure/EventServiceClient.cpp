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
EventServiceClient::EventServiceClient(Utf8StringCR repoId, Utf8StringCR userId)
    {
    m_repoId = repoId;
    m_userId = userId;
    UpdateToken();  
    Utf8String baseAddress = "https://" + m_nameSpace + "." + "servicebus.windows.net/";
    m_fullAddress = baseAddress + repoId + "/Subscriptions/" + userId + "/messages/head?timeout=";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool EventServiceClient::UpdateToken()
    {
    return MakeEventServiceRequest(m_token, m_nameSpace); //todo: need to do some sanity check
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Arvind   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool EventServiceClient::MakeEventServiceRequest(Utf8StringR outToken, Utf8StringR outNameSpace)
    {
    outNameSpace = "testhubjeehwan-ns";
    outToken = "SharedAccessSignature sig=TOk40ce29TwpOYCFG7EWqHL5%2bmi9fIDX%2fYA0Ckv7Urs%3d&se=1463758026&skn=EventReceivePolicy&sr=https%3a%2f%2ftesthubjeehwan-ns.servicebus.windows.net%2ftest";
    return true; //not yet implemented
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
bool EventServiceClient::Receive(Utf8StringR msgOut, int retry, bool longPolling)
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
                if (retry >= 1 || !UpdateToken())
                    return false;
                return Receive(msgOut, retry + 1, longPolling);
            default:
                return false;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool EventServiceClient::Receive(Utf8StringR msgOut, bool longPolling)
    {
    return Receive(msgOut, 0, longPolling);
    }