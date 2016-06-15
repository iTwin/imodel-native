/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/HttpRequest.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/WString.h>
#include <functional>

#include <BeHttp/Http.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/CancellationToken.h>
#include <BeHttp/Credentials.h>
#include <BeHttp/HttpResponse.h>
#include <BeHttp/IHttpHandler.h>

USING_NAMESPACE_BENTLEY_TASKS
BEGIN_BENTLEY_HTTP_NAMESPACE

struct HttpRequest;
typedef HttpRequest& HttpRequestR;
typedef const HttpRequest& HttpRequestCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpRequest
    {
public:
    enum RetryOption
        {
        DontRetry,
        ResetTransfer,
        ResumeTransfer
        };

    typedef std::function<void (double bytesTransfered, double bytesTotal)> ProgressCallback;
    typedef const ProgressCallback& ProgressCallbackCR;

private:
    // Request setup
    Utf8String m_url;
    Utf8String m_method;
    Utf8String m_proxyUrl;
    Credentials m_proxyCredentials;

    Credentials m_credentials;
    bool m_validateCertificate;

    HttpRequestHeaders m_requestHeaders;
    HttpBodyPtr m_requestBody;

    HttpBodyPtr m_responseBody;

    ICancellationTokenPtr m_cancellationToken;

    ProgressCallback m_uploadProgressCallback;
    ProgressCallback m_downloadProgressCallback;

    unsigned m_connectionTimeoutSeconds;
    unsigned m_transferTimeoutSeconds;

    RetryOption m_retryOption;
    unsigned    m_retriesMax;

    bool m_followRedirects;
    bool m_useNewConnection;

    IHttpHandlerPtr m_httpHandler;

private:
    Utf8String EscapeUnsafeSymbolsInUrl (Utf8StringCR url);

public:
    BEHTTP_EXPORT HttpRequest (Utf8StringCR url, Utf8StringCR method = "GET", IHttpHandlerPtr customHandler = nullptr);

    BEHTTP_EXPORT HttpRequestHeadersR  GetHeaders ();
    BEHTTP_EXPORT HttpRequestHeadersCR GetHeaders () const;

    BEHTTP_EXPORT Utf8StringCR GetUrl () const;
    BEHTTP_EXPORT Utf8StringCR GetMethod () const;

    BEHTTP_EXPORT void           SetCredentials (Credentials credentials);
    BEHTTP_EXPORT CredentialsCR  GetCredentials () const;

    //! Enable or disable server certificate and hostname validation
    BEHTTP_EXPORT void SetValidateCertificate(bool validate);
    BEHTTP_EXPORT bool GetValidateCertificate() const;

    // Set proxy for request. Pass empty string to not use proxy
    BEHTTP_EXPORT void         SetProxy (Utf8StringCR proxyUrl);
    BEHTTP_EXPORT Utf8StringCR GetProxy () const;

    // Set credentials for an authenticating proxy
    BEHTTP_EXPORT void           SetProxyCredentials (Credentials credentials);
    BEHTTP_EXPORT CredentialsCR  GetProxyCredentials() const;

    // Request body to send, default is no body
    BEHTTP_EXPORT void         SetRequestBody (HttpBodyPtr body);
    BEHTTP_EXPORT HttpBodyPtr  GetRequestBody () const;

    // Response body to write received data to
    BEHTTP_EXPORT void         SetResponseBody (HttpBodyPtr body);
    BEHTTP_EXPORT HttpBodyPtr  GetResponseBody () const;

    //! Sets connection and transfer timeouts with one value. Default - 60 seconds.
    BEHTTP_EXPORT void         SetTimeoutSeconds (unsigned connectionAndTransferTimeout);

    BEHTTP_EXPORT void         SetConnectionTimeoutSeconds (unsigned connectionTimeout);
    BEHTTP_EXPORT unsigned     GetConnectionTimeoutSeconds () const;

    BEHTTP_EXPORT void         SetTransferTimeoutSeconds (unsigned transferTimeout);
    BEHTTP_EXPORT unsigned     GetTransferTimeoutSeconds () const;

    //! Enable or disable following redirects. Default is true
    BEHTTP_EXPORT void         SetFollowRedirects (bool follow);
    BEHTTP_EXPORT bool         GetFollowRedirects () const;

    // Used to workaround a CURL bug:
    // https://sourceforge.net/p/curl/bugs/1275/
    BEHTTP_EXPORT void         SetUseNewConnection (bool useNewConnection);
    BEHTTP_EXPORT bool         GetUseNewConnection () const;

    // Use this if you want to specify settings for poor conectivity. Useful for downloads.
    // Retries if connection lost happen for maximumRetries or indefinetely if maximumRetries = 0
    // Default - RetryOption::DontRetry
    BEHTTP_EXPORT void                  SetRetryOptions (RetryOption option, unsigned maximumRetries = 1);
    BEHTTP_EXPORT RetryOption const&    GetRetryOption () const;
    BEHTTP_EXPORT unsigned              GetMaxRetries () const;

    BEHTTP_EXPORT void                  SetCancellationToken (ICancellationTokenPtr token);
    BEHTTP_EXPORT ICancellationTokenPtr GetCancellationToken () const;

    BEHTTP_EXPORT void                  SetDownloadProgressCallback (ProgressCallbackCR onProgress);
    BEHTTP_EXPORT ProgressCallbackCR    GetDownloadProgressCallback () const;

    BEHTTP_EXPORT void                  SetUploadProgressCallback (ProgressCallbackCR onProgress);
    BEHTTP_EXPORT ProgressCallbackCR    GetUploadProgressCallback () const;

    // DEPRECATED: Use PerformAsync to avoid blocking caller thread.
    // Execute request and block until finished.
    BEHTTP_EXPORT HttpResponse Perform ();

    // Execute request asynchronously
    BEHTTP_EXPORT AsyncTaskPtr<HttpResponse> PerformAsync ();
    };

END_BENTLEY_HTTP_NAMESPACE
