/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/HttpClient.cpp $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <BeHttp/HttpClient.h>

#include <Bentley/BeThread.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeTimeUtilities.h>
#include <fstream>
#include <ostream>
#include <curl/curl.h>
#include <openssl/ssl.h>

#include "ApplicationEvents.h"

USING_NAMESPACE_BENTLEY_HTTP

static BeAtomic<int> s_tasksInProgressCount;
static HttpClient::Options s_options;

#if defined(DEBUG)
bool s_isFullLoggingEnabled = true;
#else
bool s_isFullLoggingEnabled = false;
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpClient::Initialize(HttpClient::Options const& options)
    {
    s_options = options;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR HttpClient::GetAssetsDirectoryPath() 
    {
    BeAssert(!s_options.GetAssetsDirectoryPath().empty() && "HttpClient::Initialize() not called!");
    return s_options.GetAssetsDirectoryPath();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HttpClient::Options const& HttpClient::GetOptions() { return s_options; }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpClient::HttpClient(IHttpHeaderProviderPtr defaultHeadersProvider, IHttpHandlerPtr customHandler) :
m_defaultHeadersProvider(nullptr == defaultHeadersProvider ? HttpHeaderProvider::Create() : defaultHeadersProvider),
m_handler(customHandler)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpClient::BeginNetworkActivity()
    {
    if (++s_tasksInProgressCount == 1)
        ApplicationEventsManager::SetNetworkActivityIndicatorVisible(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpClient::EndNetworkActivity()
    {
    if (--s_tasksInProgressCount == 0)
        ApplicationEventsManager::SetNetworkActivityIndicatorVisible(false);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool HttpClient::IsNetworkActive()
    {
    return s_tasksInProgressCount > 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Pavan.Emani     03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpClient::EscapeString(Utf8StringCR inStr)
    {
    Utf8P escapedStr = curl_escape(inStr.c_str(), (int) inStr.length());
    Utf8String outStr = escapedStr;
    curl_free(escapedStr);
    return outStr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jonathan Que     02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpClient::UnescapeString(Utf8StringCR inStr)
    {
    Utf8P unescapedStr = curl_unescape(inStr.c_str(), (int) inStr.length());
    Utf8String outStr = unescapedStr;
    curl_free(unescapedStr);
    return outStr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
BentleyStatus HttpClient::HttpDateToUnixMillis(uint64_t& unixMilliseconds, Utf8CP dateStr)
    {
    time_t time = curl_getdate(dateStr, nullptr);
    if (-1 == time)
        return ERROR;

    unixMilliseconds = time * 1000;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                  Giedrius.Kairys    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpClient::EnableFullLogging(bool enable)
    {
    s_isFullLoggingEnabled = enable;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                  Giedrius.Kairys    03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool HttpClient::IsFullLoggingEnabled()
    {
    return s_isFullLoggingEnabled;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Request HttpClient::CreateRequest(Utf8StringCR url, Utf8StringCR method) const
    {
    Request request(url, method, m_handler);

    m_defaultHeadersProvider->FillHttpRequestHeaders(request.GetHeaders());
    request.SetCredentials(m_credentials);
    request.SetCompressionOptions(m_compressionOptions);

    return request;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Request HttpClient::CreateGetRequest(Utf8StringCR url) const
    {
    Request request = CreateRequest(url, "GET");
    return request;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Request HttpClient::CreateGetRequest(Utf8StringCR url, Utf8StringCR etag) const
    {
    Request request = CreateRequest(url, "GET");

    request.GetHeaders().SetIfNoneMatch(etag);

    return request;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Request HttpClient::CreateGetJsonRequest(Utf8StringCR url, Utf8StringCR etag) const
    {
    Request request = CreateRequest(url, "GET");

    request.GetHeaders().SetIfNoneMatch(etag);

    request.GetHeaders().SetAccept(REQUESTHEADER_ContentType_ApplicationJson);

    return request;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Request HttpClient::CreatePostRequest(Utf8StringCR url) const
    {
    Request request = CreateRequest(url, "POST");
    return request;
    }
