/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/HttpRequest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeHttp/HttpRequest.h>
#include <BeHttp/DefaultHttpHandler.h>

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest::HttpRequest (Utf8StringCR url, Utf8StringCR method, IHttpHandlerPtr customHandler) :
m_url (EscapeUnsafeSymbolsInUrl (url)),
m_method (method),

m_validateCertificate (false),

m_responseBody (HttpStringBody::Create ()),

m_connectionTimeoutSeconds (60),
m_transferTimeoutSeconds (60),

m_retryOption (RetryOption::DontRetry),
m_retriesMax (0),

m_followRedirects (true),
m_useNewConnection (false),

m_httpHandler (customHandler == nullptr ? DefaultHttpHandler::GetInstance () : customHandler)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpRequest::EscapeUnsafeSymbolsInUrl (Utf8StringCR url)
    {
    Utf8String fixedUrl = url;

    // http://tools.ietf.org/html/rfc1738#section-2.2
    // http://tools.ietf.org/html/rfc2396

    fixedUrl.ReplaceAll (R"(<)", "%3C");
    fixedUrl.ReplaceAll (R"(>)", "%3E");
    fixedUrl.ReplaceAll (R"(")", "%22");
    fixedUrl.ReplaceAll (R"(#)", "%23");
    // "%" should be encoded by client
    fixedUrl.ReplaceAll (R"({)", "%7B");
    fixedUrl.ReplaceAll (R"(})", "%7D");
    fixedUrl.ReplaceAll (R"(|)", "%7C");
    fixedUrl.ReplaceAll (R"(\)", "%5C");
    fixedUrl.ReplaceAll (R"(^)", "%5E");
    // "~" is considered safe
    fixedUrl.ReplaceAll (R"([)", "%5B");
    fixedUrl.ReplaceAll (R"(])", "%5D");
    fixedUrl.ReplaceAll (R"(`)", "%60");

    return fixedUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR HttpRequest::GetUrl () const
    {
    return m_url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR HttpRequest::GetMethod () const
    {
    return m_method;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetCredentials (Credentials credentials)
    {
    m_credentials = std::move (credentials);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CredentialsCR HttpRequest::GetCredentials () const
    {
    return m_credentials;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetValidateCertificate(bool validate)
    {
    m_validateCertificate = validate;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HttpRequest::GetValidateCertificate() const
    {
    return m_validateCertificate;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetProxy (Utf8StringCR proxyUrl)
    {
    m_proxyUrl = EscapeUnsafeSymbolsInUrl (proxyUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR HttpRequest::GetProxy () const
    {
    return m_proxyUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ron.Stewart     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetProxyCredentials (Credentials credentials)
    {
    m_proxyCredentials = credentials;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ron.Stewart     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CredentialsCR  HttpRequest::GetProxyCredentials() const
    {
    return m_proxyCredentials;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetRequestBody (HttpBodyPtr body)
    {
    m_requestBody = body;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HttpBodyPtr HttpRequest::GetRequestBody () const
    {
    return m_requestBody;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetResponseBody (HttpBodyPtr body)
    {
    m_responseBody = body;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HttpBodyPtr HttpRequest::GetResponseBody () const
    {
    return m_responseBody;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequestHeadersR HttpRequest::GetHeaders ()
    {
    return m_requestHeaders;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequestHeadersCR HttpRequest::GetHeaders () const
    {
    return m_requestHeaders;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetTimeoutSeconds (unsigned connectionAndTransferTimeout)
    {
    m_connectionTimeoutSeconds = connectionAndTransferTimeout;
    m_transferTimeoutSeconds = connectionAndTransferTimeout;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetConnectionTimeoutSeconds (unsigned connectionTimeout)
    {
    m_connectionTimeoutSeconds = connectionTimeout;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned HttpRequest::GetConnectionTimeoutSeconds () const
    {
    return m_connectionTimeoutSeconds;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetTransferTimeoutSeconds (unsigned transferTimeout)
    {
    m_transferTimeoutSeconds = transferTimeout;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned HttpRequest::GetTransferTimeoutSeconds () const
    {
    return m_transferTimeoutSeconds;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetRetryOptions (HttpRequest::RetryOption option, unsigned maximumRetries)
    {
    m_retryOption = option;
    m_retriesMax = maximumRetries;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest::RetryOption const& HttpRequest::GetRetryOption () const
    {
    return m_retryOption;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned HttpRequest::GetMaxRetries () const
    {
    return m_retriesMax;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetCancellationToken (ICancellationTokenPtr token)
    {
    m_cancellationToken = token;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ICancellationTokenPtr HttpRequest::GetCancellationToken () const
    {
    return m_cancellationToken;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetDownloadProgressCallback (ProgressCallbackCR onProgress)
    {
    m_downloadProgressCallback = onProgress;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest::ProgressCallbackCR HttpRequest::GetDownloadProgressCallback () const
    {
    return m_downloadProgressCallback;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetFollowRedirects (bool follow)
    {
    m_followRedirects = follow;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool HttpRequest::GetFollowRedirects () const
    {
    return m_followRedirects;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetUseNewConnection (bool useNewConnection)
    {
    m_useNewConnection = useNewConnection;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool HttpRequest::GetUseNewConnection () const
    {
    return m_useNewConnection;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRequest::SetUploadProgressCallback (ProgressCallbackCR onProgress)
    {
    m_uploadProgressCallback = onProgress;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequest::ProgressCallbackCR HttpRequest::GetUploadProgressCallback () const
    {
    return m_uploadProgressCallback;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponse HttpRequest::Perform ()
    {
    return PerformAsync ()->GetResult ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<HttpResponse> HttpRequest::PerformAsync ()
    {
    return m_httpHandler->PerformRequest (*this);
    }
