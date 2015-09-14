/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/HttpHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/DgnCore/HttpHandler.h>

#ifndef BENTLEY_WINRT
#include <curl/curl.h>
#endif

USING_NAMESPACE_BENTLEY_DGNPLATFORM

HttpHandler* HttpHandler::s_instance = nullptr;

#ifndef BENTLEY_WINRT
//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct CurlHolder
{
private:
    CURL* m_curl;
public:
    CurlHolder() : m_curl(curl_easy_init()) {}
    ~CurlHolder() {curl_easy_cleanup(m_curl);}
    CURL* Get() const {return m_curl;}
};
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HttpHandler::HttpHandler()
    {
#ifndef BENTLEY_WINRT
    curl_global_init(CURL_GLOBAL_ALL);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HttpHandler& HttpHandler::Instance()
    {
    if (nullptr == s_instance)
        s_instance = new HttpHandler();
    return *s_instance;
    }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpHandler::HttpHeaderParser(char* buffer, size_t size, size_t nItems, void* userData)
    {
    auto& header = * (bmap<Utf8String, Utf8String>*) userData;
    auto totalSize = size * nItems;

    Utf8String line(buffer, totalSize);
    line.Trim();
    if (Utf8String::IsNullOrEmpty(line.c_str()))
        return totalSize;
    
    if (header.empty())
        {
        // expecting the "HTTP/1.x 200 OK" header
        header["HTTP"] = line;
        }
    else
        {
        auto delimiterPos = line.find(':');
        if (Utf8String::npos == delimiterPos)
            {
            BeAssert(false);
            return totalSize;
            }
        Utf8String key(line.begin(), line.begin() + delimiterPos);
        Utf8String value(line.begin() + delimiterPos + 2, line.end());
        header[key] = value;
        }
    
    return totalSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int HttpHandler::HttpProgressCallback(void* clientp, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow)
    {
    BeAssert(nullptr != clientp);
    IHttpRequestCancellationToken* cancellationToken = (IHttpRequestCancellationToken*)clientp;
    return cancellationToken->_ShouldCancelHttpRequest();
    }

#ifndef BENTLEY_WINRT
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static HttpResponseStatus HttpStatusFromResponseCode(long responseCode) {return static_cast<HttpResponseStatus>(responseCode);}
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequestStatus HttpHandler::Request(HttpResponsePtr& response, HttpRequest const& request, IHttpRequestCancellationToken const* cancellationToken)
    {
#ifndef BENTLEY_WINRT
    CurlHolder curl;

    bmap<Utf8String, Utf8String> responseHeader;
    bvector<Byte> responseBody;
    curl_easy_setopt(curl.Get(), CURLOPT_URL, request.GetUrl().c_str());
    curl_easy_setopt(curl.Get(), CURLOPT_WRITEFUNCTION, &HttpHandler::HttpBodyParser);
    curl_easy_setopt(curl.Get(), CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl.Get(), CURLOPT_HEADERFUNCTION, &HttpHandler::HttpHeaderParser);
    curl_easy_setopt(curl.Get(), CURLOPT_HEADERDATA, &responseHeader);

    if (nullptr != cancellationToken)
        {
        curl_easy_setopt(curl.Get(), CURLOPT_XFERINFOFUNCTION, &HttpHandler::HttpProgressCallback);
        curl_easy_setopt(curl.Get(), CURLOPT_XFERINFODATA, cancellationToken);
        curl_easy_setopt(curl.Get(), CURLOPT_NOPROGRESS, 0);
        }

    curl_slist* curlRequestHeader = NULL;
    for (auto& line : request.GetHeader())
        curlRequestHeader = curl_slist_append(curlRequestHeader, Utf8PrintfString("%s: %s", line.first.c_str(), line.second.c_str()).c_str());
    curl_easy_setopt(curl.Get(), CURLOPT_HTTPHEADER, curlRequestHeader);

    CURLcode res = curl_easy_perform(curl.Get());
    curl_slist_free_all(curlRequestHeader);
    if (CURLE_OK != res)
        {
        switch(res)
            {
            case CURLE_COULDNT_CONNECT:         return HttpRequestStatus::NoConnection;
            case CURLE_COULDNT_RESOLVE_HOST:    return HttpRequestStatus::CouldNotResolveHost;
            case CURLE_COULDNT_RESOLVE_PROXY:   return HttpRequestStatus::CouldNotResolveProxy;
            case CURLE_ABORTED_BY_CALLBACK:     return HttpRequestStatus::Aborted;
            default:                            return HttpRequestStatus::UnknownError;
            }
        }

    long responseCode;
    if (CURLE_OK != curl_easy_getinfo(curl.Get(), CURLINFO_RESPONSE_CODE, &responseCode) || (0 == responseCode))
        return HttpRequestStatus::UnknownError;

    response = new HttpResponse(HttpStatusFromResponseCode(responseCode), std::move(responseHeader), std::move(responseBody));
    return HttpRequestStatus::Success;
#else
    BeAssert(false);
    return HttpRequestStatus::UnknownError;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponseStatus HttpResponse::GetStatus() const {return m_status;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<Utf8String, Utf8String> const& HttpResponse::GetHeader() const {return m_header;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Byte> const& HttpResponse::GetBody() const {return m_body;}