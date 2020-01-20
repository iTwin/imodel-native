/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "CurlHttpRequest.h"

#include <Bentley/BeThread.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/Base64Utilities.h>
#include <BeHttp/HttpClient.h>
#include <BeHttp/HttpProxy.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "CurlTaskRunner.h"
#include "../WebLogging.h"
#include "../TrustManager.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

#define PROGRESS_REPORT_INTERVAL_MILLIS 200

#define HTTP_STATUS_PARTIALCONTENT 206

BeMutex                                 CurlHttpRequest::s_transfersCS;
bset<CurlHttpRequest::TransferInfo*>    CurlHttpRequest::s_transfers;
std::function<void()>                   CurlHttpRequest::s_transfersChangeListener = [] {};

BeMutex                                 CurlHttpRequest::s_numberCS;
uint64_t                                CurlHttpRequest::s_number = 0;

ICancellationTokenPtr                   CurlHttpRequest::s_globalCt;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProgressInfo
    {
    struct TransferProgress
        {
        private:
            Request::ProgressCallbackCR m_onProgressChange;
            double m_lastBytesCompleted;
            double m_lastBytesTotal;

        public:
            TransferProgress(Request::ProgressCallbackCR onProgressChange) :
                m_onProgressChange(onProgressChange),
                m_lastBytesCompleted(-1),
                m_lastBytesTotal(-1)
                {}
            void SendTransferProgress(double bytesStarted, double bytesTransfered, double bytesTotal);
        };

    ProgressInfo
        (
        Request::ProgressCallbackCR uploadProgressCallback,
        Request::ProgressCallbackCR downloadProgressCallback,
        ICancellationTokenPtr cancellationToken
        ) :
        upload(uploadProgressCallback),
        download(downloadProgressCallback),
        cancellationToken(cancellationToken),
        timeMillisLastProgressReported(0),
        wasCanceled(false)
        {}

    TransferProgress upload;
    TransferProgress download;

    ICancellationTokenPtr cancellationToken;
    uint64_t timeMillisLastProgressReported;
    bool   wasCanceled;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool CurlHttpRequest::ShouldCompressRequestBody(Request request)
    {
    if (request.GetRequestBody() == nullptr)
        return false;

    if (!request.GetCompressionOptions().IsRequestCompressionEnabled())
        return false;

    if (!request.GetCompressionOptions().IsContentTypeSupported(request.GetHeaders().GetContentType()))
        return false;

    return request.GetCompressionOptions().GetMinimumSizeToCompress() <= request.GetRequestBody()->GetLength();
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct CurlHttpRequest::TransferInfo
    {
    bool                isActive = false;

    ConnectionStatus    status = ConnectionStatus::None;

    ProgressInfo        progressInfo;
    HttpResponseContentPtr responseContent;

    uint64_t            bytesStarted = 0;
    uint64_t            bytesDownloaded = 0;

    uint64_t            bytesUploaded = 0;
    HttpBodyPtr         requestBody;

    bool                bodyPositionSet = false;
    unsigned            retriesLeft = 0;
    CURL*               curl = nullptr;
    BeAtomic<bool>      requestNeedsToReset;
    bool                requestCouldNotConnectRetried = false;
    bvector<HttpProxy>  proxies;

    TransferInfo(HttpResponseContentPtr responseContent, const ProgressInfo& progressInfo) :
        progressInfo(progressInfo),
        responseContent(responseContent),
        requestNeedsToReset(false)
        {}

    ~TransferInfo()
        {
        BeAssert(!isActive);
        if (isActive)
            UnregisterTransferInfo(this);
        }

    void SetActive()
        {
        isActive = true;
        RegisterTransferInfo(this);
        }

    void SetInactive()
        {
        isActive = false;
        UnregisterTransferInfo(this);
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::RegisterTransferInfo(TransferInfo* transfer)
    {
    BeMutexHolder lock(s_transfersCS);
    s_transfers.insert(transfer);
    s_transfersChangeListener();
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::UnregisterTransferInfo(TransferInfo* transfer)
    {
    BeMutexHolder lock(s_transfersCS);
    s_transfers.erase(transfer);
    s_transfersChangeListener();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::SetOnActiveRequestCountChanged(std::function<void()> listener)
    {
    BeMutexHolder lock(s_transfersCS);
    s_transfersChangeListener = listener;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::SetGlobalCancellationToken(ICancellationTokenPtr ct)
    {
    s_globalCt = ct;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::ResetAllActiveRequests()
    {
    BeMutexHolder lock(s_transfersCS);
    for (CurlHttpRequest::TransferInfo* transfer : s_transfers)
        {
        transfer->requestNeedsToReset.store(true);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::GetActiveRequestCount()
    {
    BeMutexHolder lock(s_transfersCS);
    return s_transfers.size();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t CurlHttpRequest::GetNextNumber()
    {
    BeMutexHolder holder(s_numberCS);
    return ++s_number;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurlHttpRequest::CurlHttpRequest(RequestCR httpRequest, CurlPool& curlPool) :
m_httpRequest(httpRequest),
m_curlPool(curlPool),
m_transferInfo(nullptr),
m_curl(nullptr),
m_headers(nullptr),
m_number(GetNextNumber())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurlHttpRequest::~CurlHttpRequest()
    {
    if (nullptr != m_curl)
        {
        m_curlPool.ReturnHandle(m_curl);
        }
    if (m_headers)
        {
        curl_slist_free_all(m_headers);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CURL* CurlHttpRequest::GetCurlHandle()
    {
    return m_curl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RequestCR CurlHttpRequest::GetHttpRequest() const
    {
    return m_httpRequest;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t CurlHttpRequest::GetNumber() const
    {
    return m_number;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CURLcode CurlHttpRequest::CurlSslCtxCallback(CURL *curl, void *sslctx, CurlHttpRequest* request)
    {
    LOG.tracev("* #%lld Adding system certificates to SSL CTX", request->GetNumber());

    std::shared_ptr<bvector<X509*>> certs = TrustManager::GetSystemTrustedCertificates();
    if (nullptr == certs)
        return CURLE_ABORTED_BY_CALLBACK;

    X509_STORE* store = nullptr;
    store = SSL_CTX_get_cert_store((SSL_CTX *) sslctx);
    if (nullptr == store)
        return CURLE_ABORTED_BY_CALLBACK;

    for (X509* cert : *certs)
        {
        if (X509_STORE_add_cert(store, cert))
            continue;

        unsigned long error = ERR_peek_last_error();
        ERR_clear_error();

        if (ERR_GET_LIB(error) == ERR_LIB_X509 && ERR_GET_REASON(error) == X509_R_CERT_ALREADY_IN_HASH_TABLE)
            continue;

        char buffer[256];
        ERR_error_string_n(error, buffer, sizeof(buffer));
        LOG.errorv("* #%lld Error adding certificate to SSL CTX : %s", request->GetNumber(), buffer);
        }

    return CURLE_OK;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::CurlWriteHeaderCallback(void* buffer, size_t size, size_t count, CurlHttpRequest* request)
    {
    return request->OnWriteHeaders(buffer, size, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::CurlWriteDataCallback(void* buffer, size_t size, size_t count, CurlHttpRequest* request)
    {
    return request->OnWriteData(buffer, size, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::CurlReadDataCallback(void* buffer, size_t size, size_t count, CurlHttpRequest* request)
    {
    return request->OnReadData(buffer, size, count);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int CurlHttpRequest::CurlProgressCallback(CurlHttpRequest* request, double dltotal, double dlnow, double ultotal, double ulnow)
    {
    return request->OnProgress(dltotal, dlnow, ultotal, ulnow);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int CurlHttpRequest::CurlDebugCallback(CURL* handle, curl_infotype type, char* data, size_t size, CurlHttpRequest* request)
    {
    if (type != CURLINFO_TEXT && !HttpClient::IsFullLoggingEnabled())
        return 0;

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
            return 0;

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
        if (!LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
            return 0;

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
void CurlHttpRequest::SetupCurlCallbacks()
    {
    curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, FALSE);
    curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, CurlProgressCallback);
    curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, this);

    curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, CurlWriteHeaderCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEHEADER, this);

    if (!m_transferInfo->requestBody.IsNull())
        {
        curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, CurlReadDataCallback);
        curl_easy_setopt(m_curl, CURLOPT_READDATA, this);
        }

    if (!m_httpRequest.GetResponseBody().IsNull())
        {
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, CurlWriteDataCallback);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, this);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::SetPrematureError(ConnectionStatus status)
    {
    if (nullptr != m_curl)
        m_curlPool.ReturnHandle(m_curl);

    m_curl = nullptr;
    m_transferInfo->curl = nullptr;

    m_transferInfo->status = status;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CurlHttpRequest::SetupCurl ()
    {
    if (m_httpRequest.GetUseNewConnection())
        {
        if (nullptr != m_curl)
            {
            m_curlPool.ReturnHandle(m_curl);
            }
        m_curl = curl_easy_init();
        }
    else
        {
        if (nullptr == m_curl)
            {
            m_curl = m_curlPool.RetrieveHandle();
            }
        else
            {
            curl_easy_reset(m_curl);
            }
        }

    m_transferInfo->curl = m_curl;

    // Request related settings
    curl_easy_setopt(m_curl, CURLOPT_URL, m_httpRequest.GetUrl().c_str());

    if (!m_httpRequest.GetCredentials().IsEmpty())
        {
        curl_easy_setopt(m_curl, CURLOPT_USERNAME, m_httpRequest.GetCredentials().GetUsername().c_str());
        curl_easy_setopt(m_curl, CURLOPT_PASSWORD, m_httpRequest.GetCredentials().GetPassword().c_str());
        }
    else
        {
        curl_easy_setopt(m_curl, CURLOPT_USERNAME, NULL);
        curl_easy_setopt(m_curl, CURLOPT_PASSWORD, NULL);
        }

    // Request method
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, m_httpRequest.GetMethod().c_str());

    // Request body size
    curl_off_t requestBodySize = m_transferInfo->requestBody.IsNull() ? 0 : m_transferInfo->requestBody->GetLength();
    curl_easy_setopt(m_curl, CURLOPT_INFILESIZE_LARGE, requestBodySize);
    if (!m_httpRequest.GetMethod().Equals("GET"))
        {
        curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 1L); // Force adding content length header
        }

    // Connection settings
    if (m_httpRequest.GetFollowRedirects())
        {
        curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(m_curl, CURLOPT_UNRESTRICTED_AUTH, 1L);
        curl_easy_setopt(m_curl, CURLOPT_MAXREDIRS, 10L);
        }
    else
        {
        curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 0L);
        }

    if (m_transferInfo->proxies.empty())
        {
        if (!m_httpRequest.GetProxy().empty())
            {
            m_transferInfo->proxies.push_back(HttpProxy(m_httpRequest.GetProxy(), m_httpRequest.GetProxyCredentials()));
            }
        else
            {
            HttpProxy defaultProxy = HttpProxy::GetDefaultProxy ();
            if (SUCCESS != defaultProxy.GetProxiesForUrl(m_httpRequest.GetUrl(), m_transferInfo->proxies))
                {
                SetPrematureError(ConnectionStatus::CouldNotResolveProxy);
                return ERROR;
                }
            }
        }

    if (!m_transferInfo->proxies.empty())
        {
        HttpProxyCR proxy = m_transferInfo->proxies.front();
        LOG.tracev("* #%lld Will use proxy: %s", GetNumber(), proxy.ToString().c_str());

        CredentialsCR credentials = proxy.GetCredentials();
        // Note: empty URL means that PAC script specified "DIRECT" as one of the proxies.
        // We want to treat that as a valid entry in the proxyUrls list, and setting
        // CURLOPT_PROXY to an empty string disables the proxy, which is what we want.
        curl_easy_setopt(m_curl, CURLOPT_PROXY, proxy.GetProxyUrl().c_str());

        if (credentials.IsValid())
            {
            Utf8String usernamePasswordOption(BeStringUtilities::UriEncode(credentials.GetUsername().c_str()));
            usernamePasswordOption.append(":");
            usernamePasswordOption.append(BeStringUtilities::UriEncode(credentials.GetPassword().c_str()));
            curl_easy_setopt(m_curl, CURLOPT_PROXYUSERPWD, usernamePasswordOption.c_str());

            // Not sure if this is required, or it has any drawbacks. Using mainly so CURL may attempt to use CURLAUTH_NTLM on Windows if
            // CURLAUTH_BASIC authentication fails.
            curl_easy_setopt(m_curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
            }
        m_transferInfo->proxies.erase(m_transferInfo->proxies.begin());
        }

    // Without this Unix app will crash after timeout in multithread environment http://curl.haxx.se/libcurl/c/curl_easy_setopt.html#CURLOPTNOSIGNAL
    curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);

    // Will timeout where transfer speed is less than CURLOPT_LOW_SPEED_LIMIT for CURLOPT_LOW_SPEED_TIME seconds.
    // Detect lost connection when transfer hangs
    curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_LIMIT, 1L); // B/s
    curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_TIME, (long) m_httpRequest.GetTransferTimeoutSeconds());
    // Timeout for connecting to server
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, (long) m_httpRequest.GetConnectionTimeoutSeconds());

    // TLS versions before v1.2 are deprecated.
    curl_easy_setopt(m_curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    if (m_httpRequest.GetValidateCertificate())
        {
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 2L);

        // Check if built with OpenSSL backend
        auto info = curl_version_info(CURLVERSION_NOW);
        if (nullptr != info && nullptr != info->ssl_version && nullptr != strstr(info->ssl_version, "OpenSSL"))
            {
            if (TrustManager::CanUseSystemTrustedCertificates())
                {
                if (nullptr == TrustManager::GetSystemTrustedCertificates())
                    {
                    LOG.errorv("* #%lld Failed to get system trusted certificates", GetNumber());
                    SetPrematureError(ConnectionStatus::CertificateError);
                    return ERROR;
                    }
                curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_FUNCTION, CurlSslCtxCallback);
                curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_DATA, this);
                }
            else
                {
                BeFileName caBundlePath = HttpClient::GetAssetsDirectoryPath();
                caBundlePath.AppendToPath(L"http").AppendToPath(L"cabundle.pem");

                BeAssert(caBundlePath.DoesPathExist() && "Make sure 'http/cabundle.pem' file is delivered to your application root and HttpClient::Initialize() was called.");
                curl_easy_setopt(m_curl, CURLOPT_CAINFO, caBundlePath.GetNameUtf8().c_str());
                }
            }
        }
    else
        {
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(m_curl, CURLOPT_CAINFO, NULL);
        }

    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        static bool versionLogged = false;
        if (!versionLogged)
            {
            Utf8String trustDescription;
            if (TrustManager::CanUseSystemTrustedCertificates())
                trustDescription = ". " + TrustManager::GetImplementationDescription();

            LOG.infov("HTTP API implementation: %s%s", curl_version(), trustDescription.c_str());
            versionLogged = true;
            }
        }

    m_errorBuffer[0] = '\0';
    curl_easy_setopt(m_curl, CURLOPT_ERRORBUFFER, m_errorBuffer);

    if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        {
        curl_easy_setopt(m_curl, CURLOPT_DEBUGFUNCTION, CurlDebugCallback);
        curl_easy_setopt(m_curl, CURLOPT_DEBUGDATA, this);
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
        }

    // Setup transfer callbacks
    SetupCurlCallbacks();

    // Setup headers
    SetupHeaders();
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, m_headers);

    if (m_httpRequest.GetMethod().Equals("HEAD"))
        curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1L);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::SetupHeaders()
    {
    if (nullptr != m_headers)
        {
        curl_slist_free_all(m_headers);
        m_headers = nullptr;
        }

    Utf8String header;

    for (auto headerPair : m_httpRequest.GetHeaders().GetMap())
        {
        if (headerPair.first.empty() || headerPair.second.empty())
            {
            continue;
            }
        header.Sprintf("%s: %s", headerPair.first.c_str(), headerPair.second.c_str());
        m_headers = curl_slist_append(m_headers, header.c_str());
        }

    if (ShouldCompressRequestBody(m_httpRequest))
        {
        header.Sprintf("%s: %s", "Content-Encoding", "gzip");
        m_headers = curl_slist_append(m_headers, header.c_str());
        }

    if (Request::RetryOption::ResumeTransfer == m_httpRequest.GetRetryOption() && nullptr != m_transferInfo)
        {
        Utf8CP previousResponseEtag = m_transferInfo->responseContent->GetHeaders().GetValue("ETag");
        if (nullptr != previousResponseEtag)
            {
            header.Sprintf("If-Range: %s", previousResponseEtag);
            m_headers = curl_slist_append(m_headers, header.c_str());

            header.Sprintf("Range: bytes=%llu-", m_transferInfo->bytesDownloaded);
            m_headers = curl_slist_append(m_headers, header.c_str());

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
void CurlHttpRequest::PrepareRequest()
    {
    // Not null if request is retried
    if (nullptr == m_transferInfo)
        {
    	HttpBodyPtr requestBody;
        if (m_httpRequest.GetRequestBody().IsNull())
            {
            requestBody = nullptr;
            }
        else
            {
            requestBody = m_httpRequest.GetRequestBody();
            if (ShouldCompressRequestBody(m_httpRequest))
                requestBody = HttpCompressedBody::Create(requestBody);

            requestBody->Open();
            }

        if (!m_httpRequest.GetResponseBody().IsNull())
            {
            // Starding new download, reset response body because it might be reused from older requests
            auto status = m_httpRequest.GetResponseBody()->Reset();
            BeAssert(SUCCESS == status);

            m_httpRequest.GetResponseBody()->Open();
            }

        HttpBodyPtr responseBody;
        if (m_httpRequest.GetResponseBody().IsNull())
            {
            // Write callbacks are disabled so body will not be written, adding placeholder
            responseBody = HttpStringBody::Create();
            }
        else
            {
            responseBody = m_httpRequest.GetResponseBody();
            }

        auto responseContent = HttpResponseContent::Create(responseBody);
        auto ct = MergeCancellationToken::Create(s_globalCt, m_httpRequest.GetCancellationToken());
        m_transferInfo = std::make_shared <TransferInfo>
            (
            responseContent,
            ProgressInfo(m_httpRequest.GetUploadProgressCallback(), m_httpRequest.GetDownloadProgressCallback(), ct)
            );
        m_transferInfo->retriesLeft = m_httpRequest.GetMaxRetries();
        m_transferInfo->requestBody = requestBody;
        }

    if (m_transferInfo->requestBody.IsValid())
        m_transferInfo->requestBody->SetPosition(0);

    m_transferInfo->status = ConnectionStatus::None;
    m_transferInfo->responseContent->GetHeaders().Clear();
    m_transferInfo->bodyPositionSet = false;

    m_transferInfo->requestNeedsToReset.store(false);
    m_transferInfo->progressInfo.wasCanceled = false;

    LOG.infov("> HTTP #%lld %s %s", GetNumber(), m_httpRequest.GetMethod().c_str(), m_httpRequest.GetUrl().c_str());

    if (m_transferInfo->progressInfo.cancellationToken->IsCanceled())
        {
        SetPrematureError(ConnectionStatus::Canceled);
        return;
        }

    if (SUCCESS != SetupCurl())
        return;

    LOG.tracev("* #%lld CURL handle 0x%" PRIXPTR, GetNumber(), m_curl);

    m_transferInfo->SetActive();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::FinalizeRequest(CURLcode curlStatus)
    {
    m_transferInfo->SetInactive();
    m_transferInfo->status = ResolveConnectionStatus(curlStatus);

    if (m_errorBuffer[0] != '\0')
        LOG.errorv("* HTTP #%lld Connection error: %s '%s' (%s)", GetNumber(), curl_easy_strerror(curlStatus), m_errorBuffer, GetEffectiveUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CurlHttpRequest::ShouldRetry()
    {
    bool retry = GetShouldRetry();

    if (retry && LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        LOG.infov("RETRY HTTP #%lld %s", GetNumber(), Response::ToStatusString(m_transferInfo->status, HttpStatus::None).c_str());

    return retry;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool CurlHttpRequest::GetShouldRetry()
    {
    if (nullptr == m_transferInfo)
        return false;
    if (nullptr == m_curl)
        return false;

    ConnectionStatus status = m_transferInfo->status;

    if (!m_transferInfo->proxies.empty() &&
        (
        ConnectionStatus::CouldNotConnect == status ||
        ConnectionStatus::CouldNotResolveProxy == status
        ))
        return true; // More proxies to try. Do this BEFORE checking GetRetryOption.

    if (Request::RetryOption::DontRetry == m_httpRequest.GetRetryOption())
        return false;

    if (ConnectionStatus::CouldNotConnect == status &&
        m_httpRequest.GetRetryOnCouldNotConnect() &&
        !m_transferInfo->requestCouldNotConnectRetried)
        {
        m_transferInfo->requestCouldNotConnectRetried = true;
        return true;
        }

    if (ConnectionStatus::Timeout != status &&
        ConnectionStatus::ConnectionLost != status)
        return false;

    if (0 == m_httpRequest.GetMaxRetries())
        return true;

    if (m_transferInfo->retriesLeft <= 0)
        return false;

    m_transferInfo->retriesLeft--;
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Response CurlHttpRequest::ResolveResponse()
    {
    if (!m_transferInfo->requestBody.IsNull())
        m_transferInfo->requestBody->Close();

    long httpStatusCode = 0;
    curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &httpStatusCode);

    HttpStatus httpStatus = HttpStatus::None;

    if (m_transferInfo->status == ConnectionStatus::OK)
        httpStatus = ResolveHttpStatus(httpStatusCode);

    Response response(m_transferInfo->responseContent, GetEffectiveUrl().c_str(), m_transferInfo->status, httpStatus);

    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        double totalTimeSeconds = 0;
        curl_easy_getinfo(m_curl, CURLINFO_TOTAL_TIME, &totalTimeSeconds);

        LOG.infov("< HTTP #%lld %s [%.2fs] %s",
                  GetNumber(),
                  Response::ToStatusString(response.GetConnectionStatus(), response.GetHttpStatus()).c_str(),
                  totalTimeSeconds,
                  response.GetEffectiveUrl().c_str());
        }

    m_transferInfo->responseContent->GetBody()->Close();
    m_transferInfo = nullptr;

    m_curlPool.ReturnHandle(m_curl);
    m_curl = nullptr;

    return response;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CurlHttpRequest::GetEffectiveUrl()
    {
    const char* effectiveUrl = nullptr;

    if (nullptr != m_curl)
        curl_easy_getinfo(m_curl, CURLINFO_EFFECTIVE_URL, &effectiveUrl);

    if (effectiveUrl == nullptr)
        effectiveUrl = m_httpRequest.GetUrl().c_str();

    return effectiveUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStatus CurlHttpRequest::ResolveHttpStatus(int httpStatusInt)
    {
    HttpStatus httpStatus = static_cast<HttpStatus>(httpStatusInt);

    // Check if this is last response for partial content download and return 200
    if (HttpStatus::PartialContent == httpStatus)
        {
        HttpResponseContentPtr content = m_transferInfo->responseContent;
        ContentRangeHeaderValue contentRange;
        if (SUCCESS == ContentRangeHeaderValue::Parse(content->GetHeaders().GetContentRange(), contentRange))
            {
            if (contentRange.HasLength() && contentRange.GetLength() == content->GetBody()->GetLength())
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
ConnectionStatus CurlHttpRequest::ResolveConnectionStatus(CURLcode curlStatus)
    {
    if (CURLE_OK != curlStatus && LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        LOG.debugv("* HTTP #%lld Connection status: '%s' (%d)", GetNumber(), curl_easy_strerror(curlStatus), curlStatus);

    if (m_transferInfo->requestNeedsToReset)
        return ConnectionStatus::ConnectionLost;

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
        case CURLE_URL_MALFORMAT:
        case CURLE_COULDNT_CONNECT:
        case CURLE_COULDNT_RESOLVE_HOST:
            return ConnectionStatus::CouldNotConnect;
        case CURLE_COULDNT_RESOLVE_PROXY:                   // TODO: In some cases CURLE_RECV_ERROR is returned when proxy password is wrong and HTTPS URL was accessed
            return ConnectionStatus::CouldNotResolveProxy;
        case CURLE_ABORTED_BY_CALLBACK:                     // Aborted from curl callback
            return ConnectionStatus::Canceled;
        case CURLE_OPERATION_TIMEDOUT:                      // Connection or transfer timeout
            return ConnectionStatus::Timeout;
        case CURLE_GOT_NOTHING:                             // Connection did not return any data for some reason
        case CURLE_SEND_ERROR:                              // Happens with uploads when iOS app is sent to background
        case CURLE_RECV_ERROR:                              // Server killed or other error
            return ConnectionStatus::ConnectionLost;
        case CURLE_PEER_FAILED_VERIFICATION:
        case CURLE_SSL_CONNECT_ERROR:                       // Something wrong with server SSL configuration. WinSSL error when certificate is not valid.
            return ConnectionStatus::CertificateError;      // Server uses invalid certificate or one that we cannot validate
        default:
            LOG.errorv("* HTTP #%lld CURL status '%d' not handled: '%s'", GetNumber(), curlStatus, curl_easy_strerror(curlStatus));
            BeAssert(false && "CULR status not handled");
            return ConnectionStatus::UnknownError;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::OnWriteHeaders(void* buffer, size_t size, size_t count)
    {
    size_t bufferSize = size * count;
    Utf8String header((char*) buffer, bufferSize);

    if (header.compare(0, 5, "HTTP/") == 0)
        {
        // If there is a redirect, clear out all headers from previous
        // location prior to adding headers from new location.
        m_transferInfo->responseContent->GetHeaders().Clear();

        // Ensure that body was written with zero size
        OnWriteData(nullptr, 0, 0);

        return bufferSize;
        }

    size_t seperatorPos = header.find_first_of(':');
    if (Utf8String::npos != seperatorPos)
        {
        Utf8String key = header.substr(0, seperatorPos);
        Utf8String value = header.substr(seperatorPos + 1, header.length());

        key.Trim();
        value.Trim();

        m_transferInfo->responseContent->GetHeaders().AddValue(key, value);
        }

    return bufferSize;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::OnWriteData(void* buffer, size_t size, size_t count)
    {
    if (!m_transferInfo->bodyPositionSet)
        {
        long httpStatus = 0;
        curl_easy_getinfo(m_transferInfo->curl, CURLINFO_RESPONSE_CODE, &httpStatus);

        if (HTTP_STATUS_PARTIALCONTENT != httpStatus)
            {
            m_transferInfo->bytesStarted = 0;
            m_transferInfo->bytesDownloaded = 0;
            }

        if (SUCCESS != m_transferInfo->responseContent->GetBody()->SetPosition(m_transferInfo->bytesStarted))
            {
            return -1;
            }
        m_transferInfo->bodyPositionSet = true;
        }

    size_t bufferSize = size * count;
    size_t bytesWritten = m_transferInfo->responseContent->GetBody()->Write((char*) buffer, bufferSize);
    m_transferInfo->bytesDownloaded += bytesWritten;

    return bytesWritten;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CurlHttpRequest::OnReadData(void* buffer, size_t size, size_t count)
    {
    uint64_t requestBodySize = m_transferInfo->requestBody->GetLength();

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

    return m_transferInfo->requestBody->Read((char*) buffer, static_cast<size_t>(chunkSize));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurlHttpRequest::ProgressResult CurlHttpRequest::OnProgress(double dltotal, double dlnow, double ultotal, double ulnow)
    {
    SendProgressCallback(dltotal, dlnow, ultotal, ulnow);

    ProgressInfo& progressInfo = m_transferInfo->progressInfo;

    if (m_transferInfo->requestNeedsToReset) 
        {
        LOG.debugv("* HTTP #%lld Cancelled via forced reset", GetNumber());
        progressInfo.wasCanceled = true; // Stop request for possible reset
        }

    if (progressInfo.cancellationToken && progressInfo.cancellationToken->IsCanceled())
        {
        LOG.debugv("* HTTP #%lld Cancelled via cancellation token", GetNumber());
        progressInfo.wasCanceled = true;
        }

    // Always check for wasCanceled because curl makes one more call to progress callback after CURL_READFUNC_ABORT
    if (progressInfo.wasCanceled)
        return ProgressResult::Abort;

    return ProgressResult::Continue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpRequest::SendProgressCallback(double dltotal, double dlnow, double ultotal, double ulnow)
    {
    ProgressInfo& progressInfo = m_transferInfo->progressInfo;

    uint64_t currentTimeMillis = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    uint64_t timePassedMillis = currentTimeMillis - progressInfo.timeMillisLastProgressReported;
    bool isFinalProgress = (dltotal != 0 && dltotal == dlnow) || (ultotal != 0 && ultotal == ulnow);

    if (timePassedMillis >= PROGRESS_REPORT_INTERVAL_MILLIS || isFinalProgress)
        {
        progressInfo.timeMillisLastProgressReported = currentTimeMillis;

        progressInfo.upload.SendTransferProgress(0, ulnow, ultotal);
        progressInfo.download.SendTransferProgress((double) m_transferInfo->bytesStarted, dlnow, dltotal);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ProgressInfo::TransferProgress::SendTransferProgress(double bytesStarted, double bytesTransfered, double bytesTotal)
    {
    if (!m_onProgressChange)
        return;

    double previousLastBytesCompleted = m_lastBytesCompleted;
    double previousLastBytesTotal = m_lastBytesTotal;

    // Add for using resumable transfer
    m_lastBytesCompleted = bytesStarted + bytesTransfered;
    m_lastBytesTotal = bytesStarted + bytesTotal;

    if (previousLastBytesCompleted == m_lastBytesCompleted && previousLastBytesTotal == m_lastBytesTotal)
        return;

    if (m_lastBytesCompleted != m_lastBytesTotal && CurlTaskRunner::IsSuspended())
        return;

    m_onProgressChange(m_lastBytesCompleted, m_lastBytesTotal);
    }
