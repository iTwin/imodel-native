/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Connect/EulaClient.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "ClientInternal.h"
#include <WebServices/Connect/EulaClient.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <BeHttp/HttpRequest.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
EulaClient::EulaClient(ClientInfoPtr clientInfo, IConnectAuthenticationProvider& authProvider, IHttpHandlerPtr customHandler) :
m_cancelToken(SimpleCancellationToken::Create()),
m_authProvider(authProvider),
m_customHandler(customHandler)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
EulaClient::~EulaClient()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest EulaClient::CreateRequest(Utf8StringCR serverUrl, Utf8StringCR requestUrl, Utf8StringCR method)
    {
    HttpRequest request(requestUrl, method, m_authProvider.GetAuthenticationHandler(serverUrl, m_customHandler));
    request.SetCancellationToken(m_cancelToken);    
    return request;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<EulaResult> EulaClient::ResetEula(Utf8StringCR username)
    {
    auto finalResult = std::make_shared<EulaResult>();
    return UrlProvider::Urls::ConnectEula.GetAsync()->Then([=] (Utf8String eulaUrl)
        {
        // The agreement service stores usernames (email addresses) in lower case and performs case-sensitive checks on them,
        // so we must map accordingly.
        Utf8String usernameLowerCase = username;
        usernameLowerCase.ToLower();

        Utf8String url = eulaUrl + "/Agreements/RevokeAgreementService/" + usernameLowerCase;

        HttpRequest request = CreateRequest(eulaUrl, url, "POST");
        request.GetHeaders().SetContentType("application/json");

        request.PerformAsync()->Then([=] (HttpResponse httpResponse)
            {
            if (!httpResponse.IsSuccess())
                {
                finalResult->SetError(httpResponse);
                return;
                }

            finalResult->SetSuccess();
            });
        })
    ->Then<EulaResult>([=]
        {
        return *finalResult;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<EulaStatusResult> EulaClient::CheckEula()
    {
    auto finalResult = std::make_shared<EulaStatusResult>();
    return UrlProvider::Urls::ConnectEula.GetAsync()->Then([=] (Utf8String eulaUrl)
        {
        Utf8String url = eulaUrl + "/Agreements/1/Types/EULA/state";

        HttpRequest request = CreateRequest(eulaUrl, url, "GET");
        request.GetHeaders().SetAccept("application/json");

        request.PerformAsync()->Then([=] (HttpResponse httpResponse)
            {
            if (!httpResponse.IsSuccess())
                {
                finalResult->SetError(httpResponse);
                return;
                }

            Json::Value body = httpResponse.GetBody().AsJson();
            const Json::Value &accepted = body["accepted"];

            if (!accepted.isBool())
                {
                finalResult->SetError(HttpError());
                return;
                }

            finalResult->SetSuccess(accepted.asBool());
            });
        })
    ->Then<EulaStatusResult>([=]
        {
        return *finalResult;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<EulaDownloadResult> EulaClient::DownloadEula()
    {
    auto finalResult = std::make_shared<EulaDownloadResult>();
    return UrlProvider::Urls::ConnectEula.GetAsync()->Then([=] (Utf8String eulaUrl)
        {
        Utf8String url = eulaUrl + "/Agreements/1/Types/EULA";

        HttpRequest request = CreateRequest(eulaUrl, url, "GET");
        request.GetHeaders().SetAccept("application/json");

        request.PerformAsync()->Then([=] (HttpResponse httpResponse)
            {
            if (!httpResponse.IsSuccess())
                {
                finalResult->SetError(httpResponse);
                return;
                }

            Json::Value body = httpResponse.GetBody().AsJson();
            const Json::Value &eulaText = body["text"];

            if (!eulaText.isString())
                {
                finalResult->SetError(HttpError());
                return;
                }

            finalResult->SetSuccess(eulaText.asString());
            });
        })
        ->Then<EulaDownloadResult>([=]
            {
            return *finalResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<EulaResult> EulaClient::AcceptEula()
    {
    auto finalResult = std::make_shared<EulaResult>();
    return UrlProvider::Urls::ConnectEula.GetAsync()->Then([=] (Utf8String eulaUrl)
        {
        Utf8String url = eulaUrl + "/Agreements/1/Types/EULA/state";

        HttpRequest request = CreateRequest(eulaUrl, url, "POST");

        Json::Value params;
        params["accepted"] = true;

        request.SetRequestBody(HttpStringBody::Create(params.toStyledString()));

        request.PerformAsync()->Then([=] (HttpResponse httpResponse)
            {
            if (!httpResponse.IsSuccess())
                {
                finalResult->SetError(httpResponse);
                return;
                }

            finalResult->SetSuccess();
            });
        })
    ->Then<EulaResult>([=]
        {
        return *finalResult;
        });
    }
