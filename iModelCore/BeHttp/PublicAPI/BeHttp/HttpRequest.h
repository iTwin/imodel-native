/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/HttpRequest.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/WString.h>
#include <functional>

#include <BeHttp/Http.h>
#include <BeHttp/BeUri.h>
#include <BeHttp/CompressionOptions.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/CancellationToken.h>
#include <BeHttp/Credentials.h>
#include <BeHttp/HttpResponse.h>
#include <BeHttp/IHttpHandler.h>
#include <folly/futures/Future.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Vincas.Razma      07/16
//=======================================================================================
struct Request
    {
    enum class RetryOption
        {
        DontRetry,
        ResetTransfer,
        ResumeTransfer 
        };

    typedef std::function<void (double bytesTransfered, double bytesTotal)> ProgressCallback;
    typedef const ProgressCallback& ProgressCallbackCR;

private:
    Utf8String m_url;
    Utf8String m_method;
    Utf8String m_proxyUrl;
    Credentials m_proxyCredentials;
    Credentials m_credentials;
    bool m_validateCertificate = true;
    HttpRequestHeaders m_requestHeaders;
    HttpBodyPtr m_requestBody;
    HttpBodyPtr m_responseBody;
    ICancellationTokenPtr m_cancellationToken;
    ProgressCallback m_uploadProgressCallback;
    ProgressCallback m_downloadProgressCallback;
    unsigned m_connectionTimeoutSeconds = 60;
    unsigned m_transferTimeoutSeconds = 60;
    RetryOption m_retryOption = RetryOption::DontRetry;
    unsigned m_retriesMax = 0;
    bool m_followRedirects = true;
    bool m_useNewConnection = false;
    bool m_retryOnCouldNotConnect = false;
    IHttpHandlerPtr m_httpHandler;
    CompressionOptions m_compressionOptions;

public:
    BEHTTP_EXPORT Request(Utf8String url, Utf8String method = "GET", IHttpHandlerPtr customHandler = nullptr);

    HttpRequestHeadersR  GetHeaders(){return m_requestHeaders;}
    HttpRequestHeadersCR GetHeaders() const {return m_requestHeaders;}

    Utf8StringCR GetUrl() const {return m_url;}
    Utf8StringCR GetMethod() const {return m_method;}

    void SetCredentials(Credentials credentials) {m_credentials = std::move(credentials);}
    CredentialsCR GetCredentials() const {return m_credentials;}

    //! Enable or disable server certificate and hostname validation
    void SetValidateCertificate(bool validate) {m_validateCertificate = validate;}
    bool GetValidateCertificate() const {return m_validateCertificate;}

    // Set proxy for request. Pass empty string to not use proxy
    void SetProxy(Utf8String proxyUrl) {m_proxyUrl = proxyUrl; BeUri::EscapeUnsafeCharactersInUrl(m_proxyUrl); }
    Utf8StringCR GetProxy() const {return m_proxyUrl;}

    // Set credentials for an authenticating proxy
    void SetProxyCredentials(Credentials credentials) {m_proxyCredentials = credentials;}
    CredentialsCR GetProxyCredentials() const {return m_proxyCredentials;}

    // Request body to send, default is no body
    void SetRequestBody(HttpBodyPtr body) {m_requestBody = body;}
    HttpBodyPtr GetRequestBody() const {return m_requestBody;}

    // Response body to write received data to
    void SetResponseBody(HttpBodyPtr body) {m_responseBody = body;}
    HttpBodyPtr GetResponseBody() const {return m_responseBody;}

    //! Sets connection and transfer timeouts with one value. Default - 60 seconds.
    void SetTimeoutSeconds(unsigned connectionAndTransferTimeout) {m_connectionTimeoutSeconds = connectionAndTransferTimeout; m_transferTimeoutSeconds = connectionAndTransferTimeout;}

    void SetConnectionTimeoutSeconds(unsigned connectionTimeout){m_connectionTimeoutSeconds = connectionTimeout;}
    unsigned GetConnectionTimeoutSeconds() const {return m_connectionTimeoutSeconds;}

    void SetTransferTimeoutSeconds(unsigned transferTimeout) {m_transferTimeoutSeconds = transferTimeout;}
    unsigned GetTransferTimeoutSeconds() const {return m_transferTimeoutSeconds;}

    //! Enable or disable following redirects. Default is true
    void SetFollowRedirects(bool follow){m_followRedirects = follow;}
    bool GetFollowRedirects() const {return m_followRedirects;}

    // Used to workaround a CURL bug:
    // https://sourceforge.net/p/curl/bugs/1275/
    void SetUseNewConnection(bool useNewConnection) {m_useNewConnection = useNewConnection;}
    bool GetUseNewConnection() const {return m_useNewConnection;}
    
    // Used to workaround a CURL bug:
    // https://github.com/curl/curl/issues/1294/
    void SetRetryOnCouldNotConnect(bool doRetry) { m_retryOnCouldNotConnect = doRetry; };
    bool GetRetryOnCouldNotConnect() const { return m_retryOnCouldNotConnect; };

    // Use this if you want to specify settings for poor conectivity. Useful for downloads.
    // Retries if connection lost happen for maximumRetries or indefinetely if maximumRetries = 0
    // Default - RetryOption::DontRetry
    void SetRetryOptions(RetryOption option, unsigned maximumRetries = 1) {m_retryOption = option;m_retriesMax = maximumRetries;}
    RetryOption const& GetRetryOption() const {return m_retryOption;}

    unsigned GetMaxRetries() const {return m_retriesMax;}

    //! Set compression options for this request. By default request is not compressed
    void SetCompressionOptions(CompressionOptions options) {m_compressionOptions = std::move(options);}
    CompressionOptionsCR GetCompressionOptions() const {return m_compressionOptions;}
    void SetCancellationToken(ICancellationTokenPtr token) {m_cancellationToken = token;}
    ICancellationTokenPtr GetCancellationToken() const {return m_cancellationToken;}

    void SetDownloadProgressCallback(ProgressCallbackCR onProgress) {m_downloadProgressCallback = onProgress;}
    ProgressCallbackCR GetDownloadProgressCallback() const {return m_downloadProgressCallback;}

    void SetUploadProgressCallback(ProgressCallbackCR onProgress) {m_uploadProgressCallback = onProgress;}
    ProgressCallbackCR GetUploadProgressCallback() const {return m_uploadProgressCallback;}

    // Execute request asynchronously
    BEHTTP_EXPORT folly::Future<Response> Perform();

    // DEPRECATED: Use Perform.
    Tasks::AsyncTaskPtr<Response> PerformAsync() {return m_httpHandler->_PerformRequest(*this);}
    };

END_BENTLEY_HTTP_NAMESPACE
