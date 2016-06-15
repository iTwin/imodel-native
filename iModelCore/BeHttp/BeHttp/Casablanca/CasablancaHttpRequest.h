/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Casablanca/CasablancaHttpRequest.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>
#include <Bentley/bmap.h>
#include <functional>

#include <BeHttp/Http.h>
#include <BeHttp/IHttpHandler.h>
#include <BeHttp/HttpResponse.h>
#include <BeHttp/HttpRequest.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/CancellationToken.h>

#if (_MSC_VER >= 1800)
    #undef _CONCAT
#endif
#include "cpprest/http_client.h"

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct CasablancaHttpRequest
    {
private:
    HttpRequest m_httpRequest;

    struct TransferInfo;

    std::shared_ptr<TransferInfo> m_transferInfo;

    Utf8String m_effectiveUrl;
    std::shared_ptr<pplx::cancellation_token_source> m_tokenSource;

    static const int m_bufferSize = 8192;

private:
    struct StatusWrapper;

    void SetupClient (web::http::client::http_client& client, web::http::http_request& request);
    void SetupHeaders (web::http::http_request& request);
    void CorrectAcceptLanguageHeader (web::http::http_headers& headers);

    concurrency::streams::basic_ostream<uint8_t> PrepareRequest (web::http::http_request& request);
    void ConvertResponseHeaders (web::http::http_headers pHeader);
    void SetupClientCallbacks ();

    HttpResponse ResolveResponse (const StatusWrapper& status);

    ConnectionStatus GetConnectionStatus (const StatusWrapper& status);

    pplx::task<StatusWrapper> SendCasablancaRequest ();
    pplx::task<StatusWrapper> SendCasablancaRequestWithRetries ();
    
    utility::string_t PrepareAuthorizationHeader ();
    void AddRequestBody (web::http::http_request & httpRequest);

    static web::http::method StringToHttpMethod (Utf8String methodStr);
    static Utf8String ToUtf8String (const utility::string_t& platformString);
    static utility::string_t ToUtilityString (Utf8StringCR utf8String);

    static bool IsChunked (web::http::http_headers httpHeaders);

    HttpStatus ResolveHttpStatus (int httpStatusInt);
    bool ShouldRetry (ConnectionStatus curlStatus);
    HttpResponse ResolveResponse (ConnectionStatus curlStatus);

    void SendProgressCallback (double dltotal, double dlnow, double ultotal, double ulnow);

protected:
    enum ProgressResult
        {
        Continue = 0,
        Pause = 1,
        Abort = 2
        };

    ProgressResult OnProgress (double dltotal, double dlnow, double ultotal, double ulnow);

public:
    CasablancaHttpRequest (HttpRequestCR httpRequest);

    static pplx::task<HttpResponse> PerformPplxAsync (std::shared_ptr<CasablancaHttpRequest> request);
    };

END_BENTLEY_HTTP_NAMESPACE
