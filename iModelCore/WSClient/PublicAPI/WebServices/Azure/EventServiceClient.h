/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Azure/EventServiceClient.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../Client/WebServicesClient.h"
#include <BeHttp/HttpError.h>
#include <BeHttp/HttpResponse.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

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

    public:
        EventServiceClient(); //Need a default constructor for DgnDbClientRepository
        WSCLIENT_EXPORT Http::Response MakeReceiveDeleteRequest(bool longPolling = true);
        WSCLIENT_EXPORT EventServiceClient(Utf8StringCR baseAddress, Utf8StringCR repoId, Utf8StringCR userId);
        WSCLIENT_EXPORT bool Receive(Utf8StringR msgOut, bool longPolling = true);
        WSCLIENT_EXPORT void UpdateSASToken(Utf8StringCR sasToken);
        WSCLIENT_EXPORT void CancelRequest();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE