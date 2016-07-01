/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/HttpHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/HttpHandler.h>
#include <mutex>

#ifndef BENTLEY_WINRT
#include <curl/curl.h>
#endif

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_UNNAMED_NAMESPACE

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t httpBodyParser(void* ptr, size_t size, size_t nmemb, void* userp)
    {
    size_t totalSize = size * nmemb;
    ByteStream* buffer = (ByteStream*) userp;
    buffer->Append((uint8_t const*)ptr, (uint32_t)totalSize);
    return totalSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t httpHeaderParser(char* buffer, size_t size, size_t nItems, void* userData)
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
static int httpProgressCallback(void* clientp, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow)
    {
    BeAssert(nullptr != clientp);
    Http::CancellationToken* cancellationToken = (Http::CancellationToken*)clientp;
    return cancellationToken->_ShouldCancelHttpRequest();
    }

#endif

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Http::Request::Status Http::PerformRequest(Response& response, Request const& request, CancellationToken const* cancellationToken)
    {
#ifndef BENTLEY_WINRT
    static std::once_flag s_flag;
    std::call_once(s_flag, curl_global_init, CURL_GLOBAL_ALL);

    CurlHolder curl;
    curl_easy_setopt(curl.Get(), CURLOPT_URL, request.GetUrl().c_str());
    curl_easy_setopt(curl.Get(), CURLOPT_WRITEFUNCTION, &httpBodyParser);
    curl_easy_setopt(curl.Get(), CURLOPT_WRITEDATA, &response.m_body);
    curl_easy_setopt(curl.Get(), CURLOPT_HEADERFUNCTION, &httpHeaderParser);
    curl_easy_setopt(curl.Get(), CURLOPT_HEADERDATA, &response.m_header);

    if (nullptr != cancellationToken)
        {
        curl_easy_setopt(curl.Get(), CURLOPT_XFERINFOFUNCTION, &httpProgressCallback);
        curl_easy_setopt(curl.Get(), CURLOPT_XFERINFODATA, cancellationToken);
        curl_easy_setopt(curl.Get(), CURLOPT_NOPROGRESS, 0);
        }

    curl_slist* curlRequestHeader = NULL;
    for (auto& line : request.GetHeader())
        curlRequestHeader = curl_slist_append(curlRequestHeader, (line.first + ": " + line.second).c_str());
    curl_easy_setopt(curl.Get(), CURLOPT_HTTPHEADER, curlRequestHeader);

    CURLcode res = curl_easy_perform(curl.Get());
    curl_slist_free_all(curlRequestHeader);
    if (CURLE_OK != res)
        {
        switch(res)
            {
            case CURLE_COULDNT_CONNECT:         return Request::Status::NoConnection;
            case CURLE_COULDNT_RESOLVE_HOST:    return Request::Status::CouldNotResolveHost;
            case CURLE_COULDNT_RESOLVE_PROXY:   return Request::Status::CouldNotResolveProxy;
            case CURLE_ABORTED_BY_CALLBACK:     return Request::Status::Aborted;
            default:                            return Request::Status::UnknownError;
            }
        }

    long responseCode;
    if (CURLE_OK != curl_easy_getinfo(curl.Get(), CURLINFO_RESPONSE_CODE, &responseCode) || (0 == responseCode))
        return Request::Status::UnknownError;

    response.m_status = static_cast<Response::Status>(responseCode); 
    return Request::Status::Success;

#else
    BeAssert(false);
    return Request::Status::UnknownError;
#endif
    }
