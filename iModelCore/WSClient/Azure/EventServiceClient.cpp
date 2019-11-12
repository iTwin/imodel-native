/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Azure/EventServiceClient.h>
#include <iomanip>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
EventServiceClient::EventServiceClient(Utf8StringCR baseAddress, Utf8StringCR userId)
    {
    m_baseAddress = baseAddress;
    m_fullAddress = m_baseAddress + "/Subscriptions/" + userId + "/messages/head?timeout=";
    m_ct = SimpleCancellationToken::Create ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Arvind.Venkateswaran   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
//Need a default constructor for iModel Hub Client
EventServiceClient::EventServiceClient()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
                                                       Arvind.Venkateswaran   07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<EventServiceResult> EventServiceClient::MakeReceiveDeleteRequest(bool longPolling)
    {
    return MakeReceiveRequest("DELETE", longPolling);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
                                                       Arvind.Venkateswaran   07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<EventServiceResult> EventServiceClient::MakeReceiveRequest(Utf8StringCR requestMethod, bool longPolling)
    {
    char numBuffer[10];
    if (longPolling)
        {
        //itoa is non-portable - itoa(230, numBuffer, 10);
        //Sure, you could snprintf, but this is so easy...
        numBuffer[0] = '2';
        numBuffer[1] = '3';
        numBuffer[2] = '0';
        numBuffer[3] = 0;
        }
    else
        {
        //itoa is non-portable - itoa(0, numBuffer, 10);
        //Sure, you could snprintf, but this is so easy...
        numBuffer[0] = '0';
        numBuffer[1] = 0;
        }

    Utf8String url = m_fullAddress + Utf8String(numBuffer);
    Http::Request request(url.c_str(), requestMethod, nullptr);
    request.GetHeaders().Clear();
    request.GetHeaders().SetValue("Content-Length", "0");
    request.GetHeaders().SetValue("Content-Type", "application/atom+xml;type=entry;charset=utf-8");
    request.GetHeaders().SetAuthorization(m_token);
    request.SetTransferTimeoutSeconds(230);
    m_ct = SimpleCancellationToken::Create();
    request.SetCancellationToken(m_ct);
    return request.PerformAsync()
        ->Then<EventServiceResult>([=] (Http::Response& httpResponse)
        {
        if (httpResponse.IsSuccess())
            return EventServiceResult::Success(httpResponse);
        return EventServiceResult::Error(httpResponse);
        });
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            08/2018
//---------------------------------------------------------------------------------------
AsyncTaskPtr<EventServiceResult> EventServiceClient::MakeReceivePeekRequest(bool longPolling)
    {
    return MakeReceiveRequest("POST", longPolling);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            08/2018
//---------------------------------------------------------------------------------------
AsyncTaskPtr<EventServiceResult> EventServiceClient::MakeDeleteEventRequest(Utf8StringCR lockUrl)
    {
    Http::Request request(lockUrl, "DELETE", nullptr);
    request.GetHeaders().Clear();
    request.GetHeaders().SetAuthorization(m_token);
    m_ct = SimpleCancellationToken::Create();
    request.SetCancellationToken(m_ct);
    return request.PerformAsync()
        ->Then<EventServiceResult>([=] (Http::Response& httpResponse)
        {
        if (httpResponse.IsSuccess())
            return EventServiceResult::Success(httpResponse);
        return EventServiceResult::Error(httpResponse);
        });
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
