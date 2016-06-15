/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/CurlHttpRequest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CurlHttpRequest.h"

#include <Bentley/BeThread.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/Base64Utilities.h>
#include <BeHttp/HttpClient.h>
#include "../WebLogging.h"


USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

#define PROGRESS_REPORT_INTERVAL_MILLIS 200

#define HTTP_STATUS_PARTIALCONTENT 206

BeMutex                      CurlHttpRequest::s_transfersCS;
bset<CurlHttpRequest::TransferInfo*>    CurlHttpRequest::s_transfers;

BeMutex                       CurlHttpRequest::s_numberCS;
uint64_t                                CurlHttpRequest::s_number = 0;

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
struct CurlHttpRequest::TransferInfo
    {
    ProgressInfo        progressInfo;
    HttpResponseContentPtr responseContent;

    uint64_t            bytesStarted;
    uint64_t            bytesDownloaded;

    uint64_t            bytesUploaded;
    HttpBodyPtr         requestBody;

    bool                bodyPositionSet;
    unsigned            retriesLeft;
    CURL*               curl;
    BeAtomic<bool>      requestNeedsToReset;

    TransferInfo (HttpResponseContentPtr responseContent, const ProgressInfo& progressInfo) :
        progressInfo (progressInfo),
        responseContent (responseContent),
        bytesStarted (0),
        bytesDownloaded (0),
        bytesUploaded (0),
        retriesLeft (0),
        bodyPositionSet (false),
        requestNeedsToReset (false)
        {
        RegisterTransferInfo (this);
        }

    ~TransferInfo ()
        {
        UnregisterTransferInfo (this);
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::RegisterTransferInfo (TransferInfo* transfer)
    {
    BeMutexHolder holder (s_transfersCS);
    s_transfers.insert (transfer);
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::UnregisterTransferInfo (TransferInfo* transfer)
    {
    BeMutexHolder holder (s_transfersCS);
    s_transfers.erase (transfer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::ResetAllRequests ()
    {
    BeMutexHolder holder (s_transfersCS);
    for (CurlHttpRequest::TransferInfo* transfer : s_transfers)
        {
        transfer->requestNeedsToReset.store(true);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t CurlHttpRequest::GetNextNumber ()
    {
    BeMutexHolder holder (s_numberCS);
    return ++s_number;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurlHttpRequest::CurlHttpRequest (HttpRequestCR httpRequest, CurlPool& curlPool) :
m_httpRequest (httpRequest),
m_curlPool (curlPool),
m_transferInfo (nullptr),
m_curl (nullptr),
m_headers (nullptr),
m_number (GetNextNumber ())
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurlHttpRequest::~CurlHttpRequest ()
    {
    if (nullptr != m_curl)
        {
        m_curlPool.ReturnHandle (m_curl);
        }
    if (m_headers)
        {
        curl_slist_free_all (m_headers);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CURL* CurlHttpRequest::GetCurlHandle ()
    {
    return m_curl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequestCR CurlHttpRequest::GetHttpRequest () const
    {
    return m_httpRequest;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t CurlHttpRequest::GetNumber () const
    {
    return m_number;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::CurlWriteHeaderCallback (void* buffer, size_t size, size_t count, CurlHttpRequest* request)
    {
    return request->OnWriteHeaders (buffer, size, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::CurlWriteDataCallback (void* buffer, size_t size, size_t count, CurlHttpRequest* request)
    {
    return request->OnWriteData (buffer, size, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::CurlReadDataCallback (void* buffer, size_t size, size_t count, CurlHttpRequest* request)
    {
    return request->OnReadData (buffer, size, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int CurlHttpRequest::CurlProgressCallback (CurlHttpRequest* request, double dltotal, double dlnow, double ultotal, double ulnow)
    {
    return request->OnProgress (dltotal, dlnow, ultotal, ulnow);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int CurlHttpRequest::CurlDebugCallback(CURL* handle, curl_infotype type, char* data, size_t size, CurlHttpRequest* request)
    {
    Utf8CP text;

    switch (type)
        {
        case CURLINFO_TEXT:
            text = "* ";
            break;
        case CURLINFO_HEADER_IN:
            text = "< ";
            break;
        case CURLINFO_HEADER_OUT:
            text = "> ";
            break;
        case CURLINFO_DATA_IN:
            text = "<< ";
            break;
        case CURLINFO_DATA_OUT:
            text = ">> ";
            break;
        case CURLINFO_SSL_DATA_IN:
        case CURLINFO_SSL_DATA_OUT:
            // Binary data
            return 0;
        default:
            BeAssert(false && "Unknown CURLINFO");
            return 0;
        }

    if (type == CURLINFO_DATA_IN || type == CURLINFO_DATA_OUT)
        {
        if (!LOG.isSeverityEnabled(NativeLogging::LOG_TRACE))
            {
            return 0;
            }

        // Show only ASCII symbols
        char* buffer = new char[size];
        memcpy(buffer, data, size);

        for (size_t i = 0; i < size; i++)
            {
            Utf8Char symbol = buffer[i];

            if (symbol == 0)
                {
                Utf8PrintfString message("#BinaryData# (%d bytes)", size);
                memcpy(buffer, message.c_str(), message.length());
                size = message.length();
                break;
                }

            if ((symbol < ' ' || symbol > '~') && !Utf8String::IsAsciiWhiteSpace(symbol))
                {
                buffer[i] = '?';
                }
            }

        LOG.tracev("%s#%lld %.*s", text, request->GetNumber(), size, buffer);

        delete[] buffer;
        }
    else
        {
        // Log headers and connection info as seperate lines
        Utf8String dataStr(data, size);
        dataStr.ReplaceAll("\r\n", "\n");
        dataStr.ReplaceAll("\n\n", "\n");
        dataStr.TrimEnd();

        bvector<Utf8String> lines;
        BeStringUtilities::Split(dataStr.c_str(), "\n", lines);

        for (Utf8StringCR line : lines)
            {
            if (!line.empty())
                {
                LOG.debugv("%s#%lld %s", text, request->GetNumber(), line.c_str());
                }
            }
        }

    return 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::SetupCurlCallbacks ()
    {
    curl_easy_setopt (m_curl, CURLOPT_NOPROGRESS, FALSE);
    curl_easy_setopt (m_curl, CURLOPT_PROGRESSFUNCTION, CurlProgressCallback);
    curl_easy_setopt (m_curl, CURLOPT_PROGRESSDATA, this);

    curl_easy_setopt (m_curl, CURLOPT_HEADERFUNCTION, CurlWriteHeaderCallback);
    curl_easy_setopt (m_curl, CURLOPT_WRITEHEADER, this);

    if (!m_httpRequest.GetRequestBody ().IsNull ())
        {
        curl_easy_setopt (m_curl, CURLOPT_READFUNCTION, CurlReadDataCallback);
        curl_easy_setopt (m_curl, CURLOPT_READDATA, this);
        }

    if (!m_httpRequest.GetResponseBody ().IsNull ())
        {
        curl_easy_setopt (m_curl, CURLOPT_WRITEFUNCTION, CurlWriteDataCallback);
        curl_easy_setopt (m_curl, CURLOPT_WRITEDATA, this);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::SetupCurl ()
    {
    if (m_httpRequest.GetUseNewConnection ())
        {
        if (nullptr != m_curl)
            {
            m_curlPool.ReturnHandle (m_curl);
            }
        m_curl = curl_easy_init ();
        }
    else
        {
        if (nullptr == m_curl)
            {
            m_curl = m_curlPool.RetrieveHandle ();
            }
        else
            {
            curl_easy_reset (m_curl);
            }
        }

    m_transferInfo->curl = m_curl;

    // Request related settings
    curl_easy_setopt (m_curl, CURLOPT_URL, m_httpRequest.GetUrl ().c_str ());

    if (!m_httpRequest.GetCredentials ().IsEmpty ())
        {
        curl_easy_setopt (m_curl, CURLOPT_USERNAME, m_httpRequest.GetCredentials ().GetUsername ().c_str ());
        curl_easy_setopt (m_curl, CURLOPT_PASSWORD, m_httpRequest.GetCredentials ().GetPassword ().c_str ());
        }
    else
        {
        curl_easy_setopt (m_curl, CURLOPT_USERNAME, nullptr);
        curl_easy_setopt (m_curl, CURLOPT_PASSWORD, nullptr);
        }

    // Request method
    curl_easy_setopt (m_curl, CURLOPT_CUSTOMREQUEST, m_httpRequest.GetMethod ().c_str ());

    // Request body size
    curl_off_t requestBodySize = m_httpRequest.GetRequestBody ().IsNull () ? 0 : m_httpRequest.GetRequestBody ()->GetLength ();
    curl_easy_setopt (m_curl, CURLOPT_INFILESIZE_LARGE, requestBodySize);
    if (!m_httpRequest.GetMethod ().Equals ("GET"))
        {
        curl_easy_setopt (m_curl, CURLOPT_UPLOAD, 1L); // Force adding content length header
        }

    // Connection settings
    if (m_httpRequest.GetFollowRedirects ())
        {
        curl_easy_setopt (m_curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt (m_curl, CURLOPT_UNRESTRICTED_AUTH, 1L);
        curl_easy_setopt (m_curl, CURLOPT_MAXREDIRS, 10L);
        }
    else
        {
        curl_easy_setopt (m_curl, CURLOPT_FOLLOWLOCATION, 0L);
        }

    if (!m_httpRequest.GetProxy ().empty ())
        {
        curl_easy_setopt (m_curl, CURLOPT_PROXY, m_httpRequest.GetProxy ().c_str ());

        CredentialsCR proxyCredentials = m_httpRequest.GetProxyCredentials ();
        if (proxyCredentials.IsValid())
            {
            Utf8String usernamePasswordOption (BeStringUtilities::UriEncode (proxyCredentials.GetUsername ().c_str()));
            usernamePasswordOption.append (":");
            usernamePasswordOption.append (BeStringUtilities::UriEncode (proxyCredentials.GetPassword ().c_str()));
            curl_easy_setopt (m_curl, CURLOPT_PROXYUSERPWD, usernamePasswordOption.c_str ());

            // Not sure if this is required, or it has any drawbacks. Using mainly so CURL may attempt to use CURLAUTH_NTLM on Windows if
            // CURLAUTH_BASIC authentication fails.
            curl_easy_setopt (m_curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
            }
        }

    // Without this Unix app will crash after timeout in multithread environment http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPTNOSIGNAL
    curl_easy_setopt (m_curl, CURLOPT_NOSIGNAL, 1L);

    // Will timeout where transfer speed is less than CURLOPT_LOW_SPEED_LIMIT for CURLOPT_LOW_SPEED_TIME seconds.
    // Detect lost connection when transfer hangs
    curl_easy_setopt (m_curl, CURLOPT_LOW_SPEED_LIMIT, 1L); // B/s
    curl_easy_setopt (m_curl, CURLOPT_LOW_SPEED_TIME, (long)m_httpRequest.GetTransferTimeoutSeconds ());
    // Timeout for connecting to server
    curl_easy_setopt (m_curl, CURLOPT_CONNECTTIMEOUT, (long)m_httpRequest.GetConnectionTimeoutSeconds ());

    if (m_httpRequest.GetValidateCertificate())
        {
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 2L);

        BeFileName caBundlePath = HttpClient::GetAssetsDirectoryPath();
        caBundlePath.AppendToPath(L"http").AppendToPath(L"cabundle.pem");
            
        BeAssert(caBundlePath.DoesPathExist());
        curl_easy_setopt(m_curl, CURLOPT_CAINFO, caBundlePath.GetNameUtf8().c_str());
        }
    else
        {
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(m_curl, CURLOPT_CAINFO, nullptr);
        }

    if (LOG.isSeverityEnabled (NativeLogging::LOG_DEBUG))
        {
        curl_easy_setopt(m_curl, CURLOPT_DEBUGFUNCTION, CurlDebugCallback);
        curl_easy_setopt(m_curl, CURLOPT_DEBUGDATA, this);
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
        }

    // Setup transfer callbacks
    SetupCurlCallbacks ();

    // Setup headers
    SetupHeaders ();
    curl_easy_setopt (m_curl, CURLOPT_HTTPHEADER, m_headers);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::SetupHeaders ()
    {
    if (nullptr != m_headers)
        {
        curl_slist_free_all (m_headers);
        m_headers = nullptr;
        }

    Utf8String header;

    for (auto headerPair : m_httpRequest.GetHeaders ().GetMap ())
        {
        if (headerPair.first.empty () || headerPair.second.empty ())
            {
            continue;
            }
        header.Sprintf ("%s: %s", headerPair.first.c_str (), headerPair.second.c_str ());
        m_headers = curl_slist_append (m_headers, header.c_str ());
        }

    if (HttpRequest::RetryOption::ResumeTransfer == m_httpRequest.GetRetryOption () && nullptr != m_transferInfo)
        {
        Utf8CP previousResponseEtag = m_transferInfo->responseContent->GetHeaders ().GetValue ("ETag");
        if (nullptr != previousResponseEtag)
            {
            header.Sprintf ("If-Range: %s", previousResponseEtag);
            m_headers = curl_slist_append (m_headers, header.c_str ());

            header.Sprintf ("Range: bytes=%llu-", m_transferInfo->bytesDownloaded);
            m_headers = curl_slist_append (m_headers, header.c_str ());

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
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::PrepareRequest ()
    {
    // Not null if request is retried
    if (nullptr == m_transferInfo)
        {
        if (!m_httpRequest.GetRequestBody ().IsNull ())
            {
            m_httpRequest.GetRequestBody ()->Open ();
            }

        if (!m_httpRequest.GetResponseBody ().IsNull ())
            {
            // Starding new download, reset response body because it might be reused from older requests
            auto status = m_httpRequest.GetResponseBody()->Reset();
            BeAssert(SUCCESS == status);

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

    SetupCurl ();

    m_transferInfo->responseContent->GetHeaders ().Clear ();
    m_transferInfo->bodyPositionSet = false;

    m_transferInfo->requestNeedsToReset.store(false);
    m_transferInfo->progressInfo.wasCanceled = false;

    if (LOG.isSeverityEnabled (NativeLogging::LOG_INFO))
        {
        LOG.infov ("> HTTP #%lld %s %s", GetNumber (), m_httpRequest.GetMethod ().c_str (), m_httpRequest.GetUrl ().c_str ());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool CurlHttpRequest::ShouldRetry (ConnectionStatus status)
    {
    bool retry = false;

    if (ConnectionStatus::Timeout != status &&
        ConnectionStatus::ConnectionLost != status)
        {
        retry = false;
        }

    else if (HttpRequest::RetryOption::DontRetry == m_httpRequest.GetRetryOption ())
        {
        retry = false;
        }

    else if (0 == m_httpRequest.GetMaxRetries ())
        {
        retry = true;
        }

    else if (m_transferInfo->retriesLeft > 0)
        {
        m_transferInfo->retriesLeft--;
        retry = true;
        }

    if (retry && LOG.isSeverityEnabled (NativeLogging::LOG_INFO))
        {
        LOG.infov ("RETRY HTTP #%lld %s", GetNumber (), GetDebugStatusString (status, HttpStatus::None).c_str ());
        }

    return retry;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponse CurlHttpRequest::ResolveResponse (ConnectionStatus connectionStatus)
    {
    if (!m_httpRequest.GetRequestBody ().IsNull ())
        {
        m_httpRequest.GetRequestBody ()->Close ();
        }

    long httpStatusCode = 0;
    curl_easy_getinfo (m_curl, CURLINFO_RESPONSE_CODE, &httpStatusCode);

    HttpStatus httpStatus = HttpStatus::None;

    if (connectionStatus == ConnectionStatus::OK)
        {
        httpStatus = ResolveHttpStatus (httpStatusCode);
        }

    char* effectiveUrl = nullptr;
    curl_easy_getinfo (m_curl, CURLINFO_EFFECTIVE_URL, &effectiveUrl);

    HttpResponse response (m_transferInfo->responseContent, effectiveUrl, connectionStatus, httpStatus);

    m_transferInfo->responseContent->GetBody ()->Close ();
    m_transferInfo = nullptr;

    m_curlPool.ReturnHandle (m_curl);
    m_curl = nullptr;

    if (LOG.isSeverityEnabled (NativeLogging::LOG_INFO))
        {
        LOG.infov ("< HTTP #%lld %s %s",
            GetNumber (),
            GetDebugStatusString (response.GetConnectionStatus (), response.GetHttpStatus ()).c_str (),
            response.GetEffectiveUrl ().c_str ());
        }

    return response;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStatus CurlHttpRequest::ResolveHttpStatus (int httpStatusInt)
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
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectionStatus CurlHttpRequest::GetConnectionStatus (CURLcode curlStatus)
    {
    if (m_transferInfo->requestNeedsToReset)
        {
        return ConnectionStatus::ConnectionLost;
        }

    if (m_transferInfo->progressInfo.wasCanceled)
        {
        // This is workaround for several unrecorded curl bugs (libcurl version - 7.29.0).
        // Canceling downloads (returning CURL_READFUNC_ABORT from progress function) results in the following issues:
        // 1. Sometimes, specific to network situation, curl could return CURLE_OK & 200 instead of CURLE_ABORTED_BY_CALLBACK & 0
        // 2. Canceling download when connection was not established could lead to curl returning CURLE_GOT_NOTHING
        // Both of these cases are ambiguous so additional flag is needed
        return ConnectionStatus::Canceled;
        }

    switch (curlStatus)
        {
        case CURLE_OK:
            return ConnectionStatus::OK;
        case CURLE_COULDNT_CONNECT:
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_SSL_CONNECT_ERROR:                       // Something wrong with server SSL configuration
            return ConnectionStatus::CouldNotConnect;
        case CURLE_ABORTED_BY_CALLBACK:                     // Aborted from curl callback
            return ConnectionStatus::Canceled;
        case CURLE_OPERATION_TIMEDOUT:                      // Connection or transfer timeout
            return ConnectionStatus::Timeout;
        case CURLE_GOT_NOTHING:                             // Connection did not return any data for some reason
        case CURLE_SEND_ERROR:                              // Happens with uploads when iOS app is sent to background
        case CURLE_RECV_ERROR:                              // Server killed or other error
            return ConnectionStatus::ConnectionLost;
        case CURLE_SSL_CACERT:
            return ConnectionStatus::CertificateError;      // Server uses invalid certificate or one that we cannot validate
        default:
            LOG.errorv ("Curl status '%d' not handled", curlStatus);
            BeAssert (false);
            return ConnectionStatus::UnknownError;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CurlHttpRequest::GetDebugStatusString (ConnectionStatus status, HttpStatus httpStatus)
    {
    static bmap<ConnectionStatus, Utf8String> connectionStatusStrings;
    connectionStatusStrings[ConnectionStatus::Canceled] = "Canceled";
    connectionStatusStrings[ConnectionStatus::ConnectionLost] = "ConnectionLost";
    connectionStatusStrings[ConnectionStatus::CouldNotConnect] = "CouldNotConnect";
    connectionStatusStrings[ConnectionStatus::CertificateError] = "CertificateError";
    connectionStatusStrings[ConnectionStatus::None] = "None";
    connectionStatusStrings[ConnectionStatus::OK] = "OK";
    connectionStatusStrings[ConnectionStatus::Timeout] = "Timeout";
    connectionStatusStrings[ConnectionStatus::UnknownError] = "UnknownError";

    if (status == ConnectionStatus::OK)
        {
        return Utf8PrintfString ("%d", httpStatus);
        }

    return connectionStatusStrings[status];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::OnWriteHeaders (void* buffer, size_t size, size_t count)
    {
    size_t bufferSize = size * count;
    Utf8String header ((char*)buffer, bufferSize);

    if (header.compare (0, 5, "HTTP/") == 0)
        {
        // If there is a redirect, clear out all headers from previous
        // location prior to adding headers from new location.
        m_transferInfo->responseContent->GetHeaders().Clear();

        // Ensure that body was written with zero size
        OnWriteData(nullptr, 0, 0);

        return bufferSize;
        }

    size_t seperatorPos = header.find_first_of (':');
    if (Utf8String::npos != seperatorPos)
        {
        Utf8String key = header.substr (0, seperatorPos);
        Utf8String value = header.substr (seperatorPos + 1, header.length ());

        key.Trim ();
        value.Trim ();

        m_transferInfo->responseContent->GetHeaders ().AddValue (key, value);
        }

    return bufferSize;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::OnWriteData (void* buffer, size_t size, size_t count)
    {
    if (!m_transferInfo->bodyPositionSet)
        {
        long httpStatus = 0;
        curl_easy_getinfo (m_transferInfo->curl, CURLINFO_RESPONSE_CODE, &httpStatus);

        if (HTTP_STATUS_PARTIALCONTENT != httpStatus)
            {
            m_transferInfo->bytesStarted = 0;
            m_transferInfo->bytesDownloaded = 0;
            }

        if (SUCCESS != m_transferInfo->responseContent->GetBody ()->SetPosition (m_transferInfo->bytesStarted))
            {
            return -1;
            }
        m_transferInfo->bodyPositionSet = true;
        }

    size_t bufferSize = size * count;
    size_t bytesWritten = m_transferInfo->responseContent->GetBody ()->Write ((char*)buffer, bufferSize);
    m_transferInfo->bytesDownloaded += bytesWritten;

    return bytesWritten;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::OnReadData (void* buffer, size_t size, size_t count)
    {
    uint64_t requestBodySize = m_transferInfo->requestBody->GetLength ();

    uint64_t start = m_transferInfo->bytesUploaded;
    if (start >= requestBodySize)
        {
        return 0;
        }

    uint64_t chunkSize = requestBodySize - start;
    size_t bufferSize = size * count;

    if (chunkSize > bufferSize)
        {
        chunkSize = bufferSize;
        }

    return m_transferInfo->requestBody->Read ((char*)buffer, static_cast<size_t>(chunkSize));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurlHttpRequest::ProgressResult CurlHttpRequest::OnProgress (double dltotal, double dlnow, double ultotal, double ulnow)
    {
    SendProgressCallback (dltotal, dlnow, ultotal, ulnow);

    ProgressInfo& progressInfo = m_transferInfo->progressInfo;

    if (progressInfo.wasCanceled ||             // Check for wasCanceled because curl makes one more call to progress callback after CURL_READFUNC_ABORT
        m_transferInfo->requestNeedsToReset ||    // Stop request for possible reset
        (progressInfo.cancellationToken && progressInfo.cancellationToken->IsCanceled ()))
        {
        progressInfo.wasCanceled = true;
        return ProgressResult::Abort;
        }

    return ProgressResult::Continue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::SendProgressCallback (double dltotal, double dlnow, double ultotal, double ulnow)
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
