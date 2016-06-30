/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Casablanca/CasablancaHttpRequest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CasablancaHttpRequest.h"

#include <Bentley/Base64Utilities.h>
#include <BeHttp/HttpClient.h>
#include <Bentley/Tasks/WorkerThread.h>

#include "CasablancaTaskRunner.h"

#include <Bentley/BeThread.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/Bentley.h>

#include <cpprest/filestream.h>
#include "HttpBodyAsyncStreamBuffer.h"
#include "../WebLogging.h"

#if !defined (__cplusplus_winrt)
#include <windows.h>
#include <Winhttp.h>
#endif

using namespace utility;
using namespace web;

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

#define PROGRESS_REPORT_INTERVAL_MILLIS 200

#define HTTP_STATUS_OK 200
#define HTTP_STATUS_PARTIALCONTENT 206

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProgressInfo
    {
    struct TransferProgress
        {
        private:
            HttpRequest::ProgressCallbackCR m_onProgressChange;
            double m_lastBytesCompleted;
            double m_lastBytesTotal;
        public:
            TransferProgress (HttpRequest::ProgressCallbackCR onProgressChange) :
                m_onProgressChange (onProgressChange),
                m_lastBytesCompleted (-1),
                m_lastBytesTotal (-1)
                {
                }
            void SendTransferProgress (double bytesStarted, double bytesTransfered, double bytesTotal);
        };

    ProgressInfo
        (
        HttpRequest::ProgressCallbackCR uploadProgressCallback,
        HttpRequest::ProgressCallbackCR downloadProgressCallback,
        ICancellationTokenPtr cancellationToken
        ) : 
        upload (uploadProgressCallback),
        download (downloadProgressCallback),
        cancellationToken (cancellationToken),
        timeMillisLastProgressReported (0),
        wasCanceled (false)
        {
        }

    TransferProgress upload;
    TransferProgress download;

    ICancellationTokenPtr cancellationToken;
    uint64_t timeMillisLastProgressReported;
    bool   wasCanceled;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct CasablancaHttpRequest::TransferInfo
    {
    ProgressInfo            progressInfo;
    HttpResponseContentPtr  responseContent;

    uint64_t                bytesStarted;
    uint64_t                bytesDownloaded;

    uint64_t                bytesUploaded; // WIP resumable upload?
    HttpBodyPtr             requestBody;

    bool                    bodyPositionSet;
    unsigned                retriesLeft;

    TransferInfo (HttpResponseContentPtr responseContent, const ProgressInfo& progressInfo) :
        progressInfo (progressInfo),
        responseContent (responseContent),
        bytesStarted (0),
        bytesDownloaded (0),
        bytesUploaded (0),
        retriesLeft (0),
        bodyPositionSet (false)
        {
        }

    ~TransferInfo ()
        {
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Sam.Rockwell    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CasablancaHttpRequest::StatusWrapper
    {
    http::http_exception   httpException;
    http::status_code      httpStatusCode;
    bool                   isUriException;

    StatusWrapper ()
        : httpException (0), httpStatusCode (0), isUriException (false)
        {
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CasablancaHttpRequest::CasablancaHttpRequest (HttpRequestCR httpRequest) :
m_httpRequest (httpRequest),
m_transferInfo (nullptr)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CasablancaHttpRequest::SetupHeaders (http::http_request &request)
    {
    // Remove content type that was added when setting up streams
    request.headers ().remove (http::header_names::content_type);

    // Add authorization header
    if (!m_httpRequest.GetCredentials ().IsEmpty ())
        {
        request.headers ().add (http::header_names::authorization, PrepareAuthorizationHeader ());
        }

    // Add provided headers
    for (const auto& headerPair : m_httpRequest.GetHeaders ().GetMap ())
        {
        if (headerPair.first.empty () || headerPair.second.empty ())
            {
            continue;
            }
        request.headers ().add (ToUtilityString (headerPair.first), ToUtilityString (headerPair.second));
        }

    // Correct non standard language codes
    CorrectAcceptLanguageHeader (request.headers ());

    // ?
    if (request.headers ().has (http::header_names::content_type))
        {
        request.headers ().add (http::header_names::expect, L"100-continue");
        }

    // Resume interrupted transfer
    if (HttpRequest::RetryOption::ResumeTransfer == m_httpRequest.GetRetryOption () && nullptr != m_transferInfo)
        {
        Utf8CP previousResponseEtag = m_transferInfo->responseContent->GetHeaders ().GetValue ("ETag");
        if (nullptr != previousResponseEtag)
            {
            request.headers ().add (http::header_names::if_range, ToUtilityString (previousResponseEtag));

            Utf8PrintfString rangeHeaderValue ("bytes=%llu-", m_transferInfo->bytesDownloaded + m_transferInfo->bytesStarted);

            request.headers ().add (http::header_names::range, ToUtilityString (rangeHeaderValue));
            m_transferInfo->bytesStarted = m_transferInfo->bytesDownloaded;
            }
        else
            {
            m_transferInfo->bytesStarted = 0;
            m_transferInfo->bytesDownloaded = 0;
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CasablancaHttpRequest::CorrectAcceptLanguageHeader (http::http_headers& headers)
    {
    if (headers.has (http::header_names::accept_language))
        {
        Utf8String languageCodes = ToUtf8String (headers[http::header_names::accept_language]);

        // Fix non standard language codes:
        // Chinese with Hans part
        languageCodes.ReplaceAll ("-Hans-", "-");

        headers[http::header_names::accept_language] = ToUtilityString (languageCodes);
        }
    else
        {
        // Force empty value so that system language would not be added
        headers[http::header_names::accept_language] = L"";
        }
    }

#pragma mark - Manage request

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
concurrency::streams::basic_ostream<uint8_t> CasablancaHttpRequest::PrepareRequest (http::http_request &httpRequest)
    {
    if (nullptr == m_transferInfo)
        {
        if (!m_httpRequest.GetRequestBody ().IsNull ())
            {
            m_httpRequest.GetRequestBody ()->Open ();
            }

        if (!m_httpRequest.GetResponseBody ().IsNull ())
            {
            m_httpRequest.GetResponseBody ()->Open ();
            }

        HttpBodyPtr responseBody;
        if (m_httpRequest.GetResponseBody ().IsNull ())
            {
            // Write callbacks are disabled so body will not be written, adding placeholder
            responseBody = HttpStringBody::Create ();
            }
        else
            {
            responseBody = m_httpRequest.GetResponseBody ();
            }

        auto responseContent = HttpResponseContent::Create (responseBody);
        m_transferInfo = std::make_shared <TransferInfo>
            (
            responseContent,
            ProgressInfo (m_httpRequest.GetUploadProgressCallback (),
                          m_httpRequest.GetDownloadProgressCallback (),
                          m_httpRequest.GetCancellationToken ())
            );
        m_transferInfo->retriesLeft = m_httpRequest.GetMaxRetries ();
        m_transferInfo->requestBody = m_httpRequest.GetRequestBody ();
        }

    if (!m_httpRequest.GetRequestBody ().IsNull ())
        {
        m_httpRequest.GetRequestBody ()->SetPosition (0);
        }


    httpRequest.set_progress_handler ([this] (http::message_direction::direction direction, utility::size64_t now)
        {
        if (m_transferInfo->progressInfo.wasCanceled)
            return;
        if (direction == http::message_direction::download)
            {
            m_transferInfo->bytesDownloaded = now;
            OnProgress (0, (double) now, 0, 0);
            }
        else if (direction == http::message_direction::upload)
            {
            m_transferInfo->bytesUploaded = now;
            OnProgress (0, 0, 0, (double) now);
            }
        });

    //Setup request stream
    if (!m_httpRequest.GetRequestBody ().IsNull ())
        {
        concurrency::streams::streambuf<uint8_t> requestStreamBuffer (std::make_shared<HttpBodyAsyncStreamBuffer<uint8_t>> (m_httpRequest.GetRequestBody ()));
        concurrency::streams::basic_istream<uint8_t> requestBodyStream (requestStreamBuffer);

        utility::size64_t content_length = m_httpRequest.GetRequestBody ()->GetLength ();
        httpRequest.set_body (requestBodyStream, content_length, L"");
        }


    m_transferInfo->bodyPositionSet = false;
    m_transferInfo->progressInfo.wasCanceled = false;

    //Setup response stream
    concurrency::streams::streambuf<uint8_t> responseStreamBuffer (std::make_shared<HttpBodyAsyncStreamBuffer<uint8_t>> (m_httpRequest.GetResponseBody ()));
    concurrency::streams::basic_ostream<uint8_t> responseBodyStream (responseStreamBuffer);

    httpRequest.set_response_stream (responseBodyStream);

    SetupHeaders (httpRequest);

    m_transferInfo->responseContent->GetHeaders ().Clear ();
    return responseBodyStream;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool CasablancaHttpRequest::ShouldRetry (ConnectionStatus status)
    {
    if (!m_transferInfo)
        {
        return false;
        }

    if (status == ConnectionStatus::Canceled)
        {
        return false;
        }

    // If there is content length or content range header use this to verify the response body is the correct size.
    // If neither are available  the response body could end up being the incorrect size, due to lost connection.
    Utf8CP contentLength = m_transferInfo->responseContent->GetHeaders ().GetValue ("Content-Length");
    Utf8CP contentRange = m_transferInfo->responseContent->GetHeaders ().GetValue ("Content-Range");
    
    if (contentRange != nullptr)
        {
        ContentRangeHeaderValue contentRangeValue;
        if (SUCCESS == ContentRangeHeaderValue::Parse (contentRange, contentRangeValue))
            {
            if (m_httpRequest.GetResponseBody ()->GetLength () != contentRangeValue.length)
                {
                return true;
                }
            }
        }
    else if (contentLength != nullptr)
        {
        uint64_t responseContentLength;
        BeStringUtilities::ParseUInt64(responseContentLength, contentLength);
        if (responseContentLength != m_httpRequest.GetResponseBody ()->GetLength ())
            {
            return true;
            }
        }

    if (ConnectionStatus::Timeout != status &&
        ConnectionStatus::ConnectionLost != status)
        {
        return false;
        }

    if (HttpRequest::RetryOption::DontRetry == m_httpRequest.GetRetryOption ())
        {
        return false;
        }

    if (0 == m_httpRequest.GetMaxRetries ())
        {
        return true;
        }

    if (m_transferInfo->retriesLeft > 0)
        {
        m_transferInfo->retriesLeft--;
        return true;
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ryan.McNulty    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponse CasablancaHttpRequest::ResolveResponse (const StatusWrapper& statusWrapper)
    {
    if (!m_httpRequest.GetRequestBody ().IsNull ())
        {
        m_httpRequest.GetRequestBody ()->Close ();
        }

    int httpStatusCode = static_cast<int>(statusWrapper.httpStatusCode);
    ConnectionStatus connectionStatus = GetConnectionStatus (statusWrapper);

    HttpStatus httpStatus = HttpStatus::None;
    if (connectionStatus == ConnectionStatus::OK)
        {
        httpStatus = ResolveHttpStatus (httpStatusCode);
        }

    HttpResponseContentPtr content;
    if (m_transferInfo)
        {
        content = m_transferInfo->responseContent;
        m_transferInfo->responseContent->GetBody ()->Close ();
        m_transferInfo = nullptr;
        }
    else
        {
        content = HttpResponseContent::Create (HttpStringBody::Create ());
        }

    return HttpResponse (content, m_effectiveUrl.c_str (), connectionStatus, httpStatus);;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStatus CasablancaHttpRequest::ResolveHttpStatus (int httpStatusInt)
    {
    HttpStatus httpStatus = static_cast<HttpStatus>(httpStatusInt);

    // Check if this is last response for partial content download and return 200
    if (HttpStatus::PartialContent == httpStatus)
        {
        HttpResponseContentPtr content = m_transferInfo->responseContent;
        ContentRangeHeaderValue contentRange;
        if (SUCCESS == ContentRangeHeaderValue::Parse (content->GetHeaders ().GetContentRange (), contentRange))
            {
            if (contentRange.HasLength () && contentRange.length == content->GetBody ()->GetLength ())
                {
                httpStatus = HttpStatus::OK;
                }
            }
        }

    return httpStatus;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ryan.McNulty    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
http::method CasablancaHttpRequest::StringToHttpMethod (Utf8String methodStr)
    {
    return ToUtilityString (methodStr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ryan.McNulty    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CasablancaHttpRequest::ConvertResponseHeaders (http::http_headers casaHeaders)
    {
    Utf8String logMessage = "Response Headers:\n";
    for (const std::pair<string_t, string_t>& casaHeader : casaHeaders)
        {
        Utf8String key = ToUtf8String (casaHeader.first);
        Utf8String value = ToUtf8String (casaHeader.second);

        key.Trim ();
        value.Trim ();

        logMessage = Utf8PrintfString ("%s \n%s : %s", logMessage, key, value);

        m_transferInfo->responseContent->GetHeaders ().AddValue (key, value);
        }
    LOG.info (logMessage.c_str ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ryan.McNulty    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CasablancaHttpRequest::ToUtf8String (const string_t& platformString)
    {
    WCharCP data = platformString.c_str ();
    return Utf8String (data);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ryan.McNulty    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
string_t CasablancaHttpRequest::ToUtilityString (Utf8StringCR utf8String)
    {
    WString wString (utf8String.c_str (), true);
    return string_t (wString.GetWCharCP ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
string_t CasablancaHttpRequest::PrepareAuthorizationHeader ()
    {
    Utf8PrintfString credentials ("%s:%s", m_httpRequest.GetCredentials ().GetUsername (), m_httpRequest.GetCredentials ().GetPassword ());
    Utf8PrintfString headerValue ("Basic %s", Base64Utilities::Encode (credentials).c_str ());
    return ToUtilityString (headerValue);
    }

#pragma mark - Statuses

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectionStatus CasablancaHttpRequest::GetConnectionStatus (const StatusWrapper& statusWrapper)
    {
    if (statusWrapper.isUriException == true)
        {
        BeAssert (false && "<Error> Incorrect URL format");
        return ConnectionStatus::CouldNotConnect;
        }

    if (m_transferInfo->progressInfo.wasCanceled)
        {
        LOG.info ("Was Cancelled");
        return ConnectionStatus::Canceled;
        }

    std::error_code errorCode = statusWrapper.httpException.error_code ();
    int errorCodeValue = errorCode.value ();
    LOG.debugv ("[GetConnectionStatus] '%x': %s", errorCodeValue, errorCode.message ().c_str ());

    // ! ADD COMMENTS FOR FUTURE REFERENCE WHEN ADDING HANDLING FOR NEW ERROR CODES !
    switch (errorCodeValue)
        {
        case 0:                                             // No connection errors
            return ConnectionStatus::OK;
        case 0x800C0008:                                    // WinRT: Network suddenly terminates during connection
            return ConnectionStatus::ConnectionLost;
        case 0x800C000B:                                    // WinRT: Server did not respond fast enough
        case 12002:                                         // Win7: ERROR_INTERNET_TIMEOUT    
            return ConnectionStatus::Timeout;
        case 0x800C0005:                                    // WinRT: Request occurred while network was off
        case 12007:                                         // Win7: The server name or address could not be resolved
            return ConnectionStatus::CouldNotConnect;
        default:
            LOG.errorv ("Casablanca error not handled '0x%x' (%d) : %s", errorCodeValue, errorCodeValue, errorCode.message ().c_str ());
            BeAssert (false);
            return ConnectionStatus::UnknownError;
        }
    }

#pragma mark - Request runtime

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CasablancaHttpRequest::ProgressResult CasablancaHttpRequest::OnProgress (double dltotal, double dlnow, double ultotal, double ulnow)
    {
    SendProgressCallback (dltotal, dlnow, ultotal, ulnow);

    ProgressInfo& progressInfo = m_transferInfo->progressInfo;

    if (progressInfo.wasCanceled ||
        (progressInfo.cancellationToken != NULL && progressInfo.cancellationToken->IsCanceled ()))
        {
        progressInfo.wasCanceled = true;
        m_tokenSource->cancel ();
        return ProgressResult::Abort;
        }

    return ProgressResult::Continue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CasablancaHttpRequest::SendProgressCallback (double dltotal, double dlnow, double ultotal, double ulnow)
    {
    ProgressInfo& progressInfo = m_transferInfo->progressInfo;

    uint64_t currentTimeMillis = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();
    uint64_t timePassedMillis = currentTimeMillis - progressInfo.timeMillisLastProgressReported;

    if (timePassedMillis >= PROGRESS_REPORT_INTERVAL_MILLIS)
        {
        progressInfo.timeMillisLastProgressReported = currentTimeMillis;

        progressInfo.upload.SendTransferProgress (0, ulnow, ultotal);
        progressInfo.download.SendTransferProgress ((double) m_transferInfo->bytesStarted, dlnow, dltotal);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ProgressInfo::TransferProgress::SendTransferProgress (double bytesStarted, double bytesTransfered, double bytesTotal)
    {
    if (!m_onProgressChange)
        {
        return;
        }

    double previousLastBytesCompleted = m_lastBytesCompleted;
    double previousLastBytesTotal = m_lastBytesTotal;

    // Add for using resumable transfer
    m_lastBytesCompleted = bytesStarted + bytesTransfered;
    m_lastBytesTotal = bytesStarted + bytesTotal;

    if (previousLastBytesCompleted != m_lastBytesCompleted ||
        previousLastBytesTotal != m_lastBytesTotal)
        {
        m_onProgressChange (m_lastBytesCompleted, m_lastBytesTotal);
        }
    }

#pragma mark - Perform requests

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
pplx::task<HttpResponse> CasablancaHttpRequest::PerformPplxAsync (std::shared_ptr<CasablancaHttpRequest> request)
    {
    HttpClient::BeginNetworkActivity ();

    return request->SendCasablancaRequestWithRetries ()
        .then ([=] (StatusWrapper status)
        {
        HttpClient::EndNetworkActivity ();
        return request->ResolveResponse (status);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
pplx::task<CasablancaHttpRequest::StatusWrapper> CasablancaHttpRequest::SendCasablancaRequestWithRetries ()
    {
    // create_iterative_task would be nice, but not available
    return SendCasablancaRequest ().then ([=] (pplx::task<StatusWrapper> requestTask)
        {
        StatusWrapper statusWrapper;
        try
            {
            statusWrapper = requestTask.get ();
            }
        catch (http::http_exception& e)
            {
            statusWrapper.httpException = e;
            }

        if (ShouldRetry (GetConnectionStatus (statusWrapper)))
            {
            return SendCasablancaRequestWithRetries ();
            }

        return pplx::task<StatusWrapper> ([=]
            {
            return statusWrapper;
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ryan.McNulty    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
pplx::task<CasablancaHttpRequest::StatusWrapper> CasablancaHttpRequest::SendCasablancaRequest ()
    {
    //Prepare Client
    http::client::http_client_config config;

    config.set_timeout (utility::seconds (m_httpRequest.GetTransferTimeoutSeconds ()));
    config.set_chunksize (m_bufferSize);

#if defined (__cplusplus_winrt)
    //Set IXMLRequest2 properties
    config.set_nativehandle_options (
        [&] (web::http::client::native_handle handle)
        {
        // native_handle - IXMLHTTPRequest2

        /* TODO:  download all the certificates and bundle them with the App
        and then check if a certificate is actually valid and then have a user
        provided option to allow for self signed certificates in the App. */

        // Ignore all certificate errors
        handle->SetProperty (XHR_PROP_IGNORE_CERT_ERRORS, XHR_CERT_IGNORE_FLAG::XHR_CERT_IGNORE_ALL_SERVER_ERRORS);

        // Disable redirects
        if (!m_httpRequest.GetFollowRedirects ())
            {
            BeAssert (false && "Redirects cannot be disabled");
            }
        });
#else
    // Win desktop
    config.set_validate_certificates (false);
    config.set_nativehandle_options (
        [&] (web::http::client::native_handle handle)
        {
        // native_handle - WinHttp

        // Disable redirects
        if (!m_httpRequest.GetFollowRedirects ())
            {
            DWORD data = WINHTTP_DISABLE_REDIRECTS;
            ::WinHttpSetOption (handle, WINHTTP_OPTION_DISABLE_FEATURE, &data, sizeof(data));
            }
        });
#endif

    //handle malformed URLs
    uri requestUrl;
    try
        {
        requestUrl = uri (ToUtilityString (m_httpRequest.GetUrl ()));
        if (!m_httpRequest.GetProxy ().empty ())
            {
            config.set_proxy (http::client::web_proxy (uri (ToUtilityString (m_httpRequest.GetProxy ()))));

            BeAssert (false && "<Not implemented> Proxy authentication is not implemented for Casablanca");

            // The code for implementing it might look something like this, but it has not been compiled or tested.
            // Note that this implementation replaces the config.set_proxy() call above.
#if defined (UNCOMPILED_PROXY_AUTHENTICATION)
            http::client::web_proxy webProxy (uri (ToUtilityString (m_httpRequest.GetProxy ())));

            CredentialsCR proxyCredentials = m_httpRequest.GetProxyCredentials ();
            if (proxyCredentials.IsValid())
                {
                web::credentials::credentials webProxyCredentials (ToUtilityString (proxyCredentials.GetUsername ()), ToUtilityString (proxyCredentials.GetPassword ()));
                webProxy.set_credentials (webProxyCredentials);
                }

            config.set_proxy (webProxy);
#endif
            }
        }
    catch (web::uri_exception& ex)
        {
        StatusWrapper statusWrapper;
        LOG.error (ex.what ());
        statusWrapper.isUriException = true;

        return pplx::task<StatusWrapper> ([=]
            {
            return statusWrapper;
            });
        }
    LOG.error (requestUrl.query ().c_str ());
    http::client::http_client httpClient (requestUrl, config);

    //Prepare Request
    http::http_request httpRequest (StringToHttpMethod (m_httpRequest.GetMethod ()));

    concurrency::streams::basic_ostream<uint8_t> responseBodyStream = PrepareRequest (httpRequest);

    LOG.infov ("%s \n%s\n", ToUtf8String (httpClient.base_uri ().to_string ()).c_str (), ToUtf8String (httpRequest.to_string ()).c_str ());

    m_tokenSource = std::make_shared<pplx::cancellation_token_source> ();
    pplx::cancellation_token token = m_tokenSource->get_token ();

    // Execute request
    auto requestTask = httpClient.request (httpRequest, token);
    auto responseTask = requestTask.then ([=] (pplx::task<http::http_response> requestTask)
        {
        http::http_response httpResponse = requestTask.get ();

        ConvertResponseHeaders (httpResponse.headers ());
        m_effectiveUrl = ToUtf8String (httpClient.base_uri ().to_string ());

        StatusWrapper status;
        status.httpStatusCode = httpResponse.status_code ();

        bvector<pplx::task<void>> completionTasks;

        completionTasks.push_back (responseBodyStream.close ());
        completionTasks.push_back (httpResponse.content_ready ().then ([] (http::http_response)
            {}));

        return pplx::when_all (completionTasks.begin (), completionTasks.end ()).then ([status] (pplx::task<void> task) mutable
            {
            return status;
            });
        });

    auto finishTask = responseTask.then ([] (pplx::task<StatusWrapper> statusTask) mutable
        {
        try
            {
            // Cach exceptions that might have occurred while executing requests
            return statusTask.get ();
            }
        catch (http::http_exception& ex)
            {
            StatusWrapper status;
            status.httpException = ex;
            return status;
            }
        });

    return finishTask;
    }
