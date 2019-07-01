/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ImageppInternal.h>
#include <ImagePPInternal/HttpConnection.h>
#include <ImagePP/all/h/HFCCallbacks.h>
#include <ImagePP/all/h/HFCCallbackRegistry.h>

BEGIN_IMAGEPP_NAMESPACE

static std::atomic<uint32_t> s_curl_initTermCount(0);

//----------------------------------------------------------------------------------------
// @bsiclass
//----------------------------------------------------------------------------------------
struct HttpHandler : NonCopyableClass
{
    static size_t HttpBodyParser(void* ptr, size_t size, size_t nmemb, void* userp);
    static size_t HttpHeaderParser(char* buffer, size_t size, size_t nItems, void* userData);
    static int32_t    HttpProgressCallback(void* clientp, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpHandler::HttpBodyParser(void* ptr, size_t size, size_t nmemb, void* userp)
    {
    size_t totalSize = size * nmemb;
    auto buffer = (bvector<Byte>*) userp;
    buffer->insert(buffer->end(),(Byte*) ptr,(Byte*) ptr + totalSize);
    return totalSize;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
size_t HttpHandler::HttpHeaderParser(char* buffer, size_t size, size_t nItems, void* userData)
    {
    auto& header = * (HttpResponse::HeaderMap*) userData;
    size_t totalSize = size * nItems;

    Utf8String headerLine(buffer, totalSize);

    if (headerLine.compare (0, 5, "HTTP/") == 0)  // ex: "HTTP/1.x 200 OK"
        {
        header.clear();
        header.insert({"HTTP", headerLine.c_str()});
        }
    else
        {
        auto seperatorPos = headerLine.find_first_of (':');
        if (Utf8String::npos != seperatorPos)
            {
            Utf8String key = headerLine.substr (0, seperatorPos);
            Utf8String value = headerLine.substr (seperatorPos + 1, headerLine.length ());

            key.Trim ();
            value.Trim ();

            header.insert({key, value});
            }       
        }
   
    return totalSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t HttpHandler::HttpProgressCallback(void* clientp, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow)
    {
    BeAssert(nullptr != clientp);
    IHttpRequestCancellationToken* cancellationToken = (IHttpRequestCancellationToken*)clientp;
    return cancellationToken->_ShouldCancelHttpRequest();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
static HttpRequestStatus HttpRequestStatusFromCURLcode(CURLcode code)
    {
    if(CURLE_OK == code)
        return HttpRequestStatus::Success;

    switch(code)
        {
        case CURLE_COULDNT_RESOLVE_PROXY:       return HttpRequestStatus::CouldNotResolveProxy;
        case CURLE_COULDNT_RESOLVE_HOST:        return HttpRequestStatus::CouldNotResolveHost;
        case CURLE_COULDNT_CONNECT:             return HttpRequestStatus::CouldNotConnect;
        case CURLE_OPERATION_TIMEDOUT:          return HttpRequestStatus::OperationTimedOut;
        case CURLE_ABORTED_BY_CALLBACK:         return HttpRequestStatus::Aborted;
        default:                                return HttpRequestStatus::UnknownError;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static HttpResponseStatus HttpStatusFromResponseCode(long responseCode) {return static_cast<HttpResponseStatus>(responseCode);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponseStatus HttpResponse::GetStatus() const {return m_status;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponse::HeaderMap const& HttpResponse::GetHeader() const {return m_header;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Byte> const& HttpResponse::GetBody() const {return m_body;}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2015
//----------------------------------------------------------------------------------------
HttpSession::HttpSession()
    {
    if(s_curl_initTermCount.fetch_add(1) == 1)  // curl_global_xxx are not thread safe.
        curl_global_init(CURL_GLOBAL_ALL);      
        
    m_curl = curl_easy_init();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2015
//----------------------------------------------------------------------------------------
HttpSession::~HttpSession()
    {
    curl_easy_cleanup(m_curl);

// Once init keep it alive. Unfortunately we can't cleanup during static cleanup.  Where we should do it?
//     if(s_curl_initTermCount.fetch_sub(1) == 0)
//         curl_global_cleanup();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2015
//----------------------------------------------------------------------------------------
HttpRequestStatus HttpSession::Request(HttpResponsePtr& response, HttpRequest const& request, IHttpRequestCancellationToken const* cancellationToken)
    {
    if(request.GetConnectOnly())
        {
        // CURL handle is not reliable after a connectOnly request so we use a temporary session and never reuse it.
        // From CURL 'KNOWN_BUGS' file:
        // 63. When CURLOPT_CONNECT_ONLY is used, the handle cannot reliably be re-used
        // for any further requests or transfers. The work-around is then to close that
        // handle with curl_easy_cleanup() and create a new. Some more details:
        // http://curl.haxx.se/mail/lib-2009-04/0300.html

        HttpSession tempSession;
        return tempSession.InternalRequest(response, request, cancellationToken);
        }

    return InternalRequest(response, request, cancellationToken); 
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2015
//----------------------------------------------------------------------------------------
HttpRequestStatus HttpSession::InternalRequest(HttpResponsePtr& response, HttpRequest const& request, IHttpRequestCancellationToken const* cancellationToken)
    {
    curl_easy_reset (m_curl);   // reset to original state for the incoming connection.

    HttpResponse::HeaderMap responseHeader;
    bvector<Byte> responseBody;
    curl_easy_setopt(m_curl, CURLOPT_URL, request.GetUrl().c_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &HttpHandler::HttpBodyParser);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, &HttpHandler::HttpHeaderParser);
    curl_easy_setopt(m_curl, CURLOPT_HEADERDATA, &responseHeader);
// for debugging.
//     curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
//     curl_easy_setopt(m_curl, CURLOPT_STDERR, stdout); 

    if (nullptr != cancellationToken)
        {
        curl_easy_setopt(m_curl, CURLOPT_XFERINFOFUNCTION, &HttpHandler::HttpProgressCallback);
        curl_easy_setopt(m_curl, CURLOPT_XFERINFODATA, cancellationToken);
        curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0);
        }

    //&&MM todo we need to provide certificate authorities. For now ignore in debug build.
    //From http://curl.haxx.se/docs/caextract.html
    //curl_easy_setopt(m_curl, CURLOPT_CAINFO, "C:\\down\\!Ca_certificate\\cacert.perm");

    if (!request.GetCertificateAuthoritiesFileUrl().empty())
        {        
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(m_curl, CURLOPT_CAINFO, request.GetCertificateAuthoritiesFileUrl().c_str());
        }       
    else
        {        
#ifndef NDEBUG
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(m_curl, CURLOPT_CAINFO, nullptr);
#endif
        }
    
    if(request.GetConnectOnly())
        curl_easy_setopt(m_curl, CURLOPT_CONNECT_ONLY, 1L);
    
    // user/password
    if (!request.GetCredentials ().GetUsername().empty())
        curl_easy_setopt (m_curl, CURLOPT_USERNAME, request.GetCredentials ().GetUsername().c_str ());
    if (!request.GetCredentials ().GetPassword().empty())
        curl_easy_setopt (m_curl, CURLOPT_PASSWORD, request.GetCredentials ().GetPassword().c_str ());

    // Proxy user/password
    if (!request.GetProxyCredentials ().GetUsername().empty())
        curl_easy_setopt (m_curl, CURLOPT_PROXYUSERNAME, request.GetProxyCredentials ().GetUsername().c_str ());
    if (!request.GetProxyCredentials ().GetPassword().empty())
        curl_easy_setopt (m_curl, CURLOPT_PROXYPASSWORD, request.GetProxyCredentials ().GetPassword().c_str ());
    
    if (!request.GetProxyUrl().empty())
        { 
        curl_easy_setopt(m_curl, CURLOPT_PROXY, request.GetProxyUrl().c_str());
        curl_easy_setopt(m_curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
        }
              
    curl_easy_setopt (m_curl, CURLOPT_TIMEOUT_MS, (long)request.GetTimeoutMs ());

    curl_slist* curlRequestHeader = NULL;
    for (auto& line : request.GetHeader())
        curlRequestHeader = curl_slist_append(curlRequestHeader, Utf8PrintfString("%s: %s", line.first.c_str(), line.second.c_str()).c_str());
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, curlRequestHeader);

    CURLcode resquestCode = curl_easy_perform(m_curl);
    curl_slist_free_all(curlRequestHeader);
    if (CURLE_OK != resquestCode)
        return HttpRequestStatusFromCURLcode(resquestCode);        

    long responseCode;
    if (CURLE_OK != curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &responseCode))
        return HttpRequestStatus::UnknownError;

    response = new HttpResponse(HttpStatusFromResponseCode(responseCode), std::move(responseHeader), std::move(responseBody));

    // A request may return CURLE_OK but with an invalid responseCode(ex: 401). 
    // A 2xx code is a success : http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
    if(!request.GetConnectOnly() && !IN_RANGE(responseCode, 200, 299))
        return HttpRequestStatus::ResponseCodeError;

    return HttpRequestStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                   Mathieu.St-Pierre  10/2017
 +---------------+---------------+---------------+---------------+---------------+------*/
void SetProxyInfo(HttpRequest& request)
    {
    HFCAuthenticationCallback* pCallback = (HFCAuthenticationCallback*)HFCCallbackRegistry::GetInstance()->GetCallback(HFCAuthenticationCallback::CLASS_ID);
    HFCProxyAuthentication proxyAuthentication;

    if (pCallback != nullptr && pCallback->GetAuthentication(&proxyAuthentication))
        {
        Utf8String userStr(proxyAuthentication.GetUser().c_str());
        Utf8String passStr(proxyAuthentication.GetPassword().c_str());

        Credentials proxyCredential(userStr.c_str(), passStr.c_str());

        request.SetProxyCredentials(proxyCredential);

        Utf8String proxyUrlStr(proxyAuthentication.GetServer().c_str());
        request.SetProxyUrl(proxyUrlStr.c_str());
        }
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.St-Pierre  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SetCertificateAuthoritiesInfo(HttpRequest& request)
{
    HFCAuthenticationCallback* pCallback = (HFCAuthenticationCallback*)HFCCallbackRegistry::GetInstance()->GetCallback(HFCAuthenticationCallback::CLASS_ID);
    HFCCertificateAutoritiesAuthentication certificateAuthentication;

    if (pCallback != nullptr && pCallback->GetAuthentication(&certificateAuthentication))
    {        
        Utf8String fileUrlStr(certificateAuthentication.GetCertificateAuthFileUrl().c_str());
        request.SetCertificateAuthoritiesFileUrl(fileUrlStr.c_str());
    }
}

END_IMAGEPP_NAMESPACE