/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Azure/EventServiceClient.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../Client/WebServicesClient.h"
#include <BeHttp/HttpError.h>
#include <BeHttp/HttpResponse.h>
#include <Bentley/Tasks/AsyncResult.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

typedef std::shared_ptr<struct EventServiceClient> EventServiceClientPtr;
typedef AsyncResult<Http::Response, HttpError> EventServiceResult;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jeehwan.cho   05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EventServiceClient
    {
    private:
        Utf8String m_fullAddress;
        Utf8String m_token;
        Utf8String m_baseAddress;
        SimpleCancellationTokenPtr m_ct;
        EventServiceClient(); //Need a default constructor for iModel Hub Client
        EventServiceClient(Utf8StringCR baseAddress, Utf8StringCR userId);

    public:
        WSCLIENT_EXPORT static EventServiceClientPtr Create(Utf8StringCR baseAddress, Utf8StringCR userId)
             {return EventServiceClientPtr(new EventServiceClient(baseAddress, userId)); }
        WSCLIENT_EXPORT AsyncTaskPtr<EventServiceResult> MakeReceiveDeleteRequest(bool longPolling = true);
        WSCLIENT_EXPORT void UpdateSASToken(Utf8StringCR sasToken);
        WSCLIENT_EXPORT void CancelRequest();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
