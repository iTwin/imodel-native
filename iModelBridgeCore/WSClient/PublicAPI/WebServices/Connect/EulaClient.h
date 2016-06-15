/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Connect/EulaClient.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <BeHttp/IHttpHandler.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE 

typedef AsyncResult<bool, HttpError> EulaStatusResult;
typedef AsyncResult<Utf8String, HttpError> EulaDownloadResult;
typedef AsyncResult<void, HttpError> EulaResult;

//=======================================================================================
// @bsiclass                                              Vytautas.Barkauskas   01/2016
//=======================================================================================
typedef std::shared_ptr<struct EulaClient> EulaClientPtr;
struct EulaClient
    {
    private:
        IConnectAuthenticationProvider& m_authProvider;
        SimpleCancellationTokenPtr m_cancelToken;
        IHttpHandlerPtr m_customHandler;

    private:
        HttpRequest CreateRequest(Utf8StringCR serverUrl, Utf8StringCR requestUrl, Utf8StringCR action);

    public:
        WSCLIENT_EXPORT EulaClient(ClientInfoPtr clientInfo, IConnectAuthenticationProvider& authenticationProvider, IHttpHandlerPtr customHandler = nullptr);
        WSCLIENT_EXPORT ~EulaClient();

        WSCLIENT_EXPORT AsyncTaskPtr<EulaStatusResult> CheckEula();
        WSCLIENT_EXPORT AsyncTaskPtr<EulaDownloadResult> DownloadEula();
        WSCLIENT_EXPORT AsyncTaskPtr<EulaResult> AcceptEula();
        WSCLIENT_EXPORT AsyncTaskPtr<EulaResult> ResetEula(Utf8StringCR username);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
