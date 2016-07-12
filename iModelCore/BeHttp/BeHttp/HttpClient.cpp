/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/HttpClient.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <BeHttp/HttpClient.h>
#include <Bentley/BeThread.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeTimeUtilities.h>
#include <fstream>
#include <ostream>
#include "NetworkIndicator.h"

#if defined (HTTP_LIB_CURL)
#include <curl/curl.h>
#include <openssl/ssl.h>
#endif

USING_NAMESPACE_BENTLEY_HTTP

BeAtomic<int> HttpClient::s_tasksInProgressCount(0);
BeFileName HttpClient::s_assetsDirectoryPath;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpClient::HttpClient(IHttpHeaderProviderPtr defaultHeadersProvider, IHttpHandlerPtr customHandler) :
m_defaultHeadersProvider(nullptr == defaultHeadersProvider ? HttpHeaderProvider::Create() : defaultHeadersProvider),
m_handler(customHandler)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpClient::BeginNetworkActivity()
    {
    if (++s_tasksInProgressCount == 1)
        NetworkIndicator::SetVisible(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpClient::EndNetworkActivity()
    {
    if (--s_tasksInProgressCount == 0)
        NetworkIndicator::SetVisible(false);
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
#if defined (HTTP_LIB_CURL)
    Utf8P escapedStr = curl_escape(inStr.c_str(), (int)inStr.length());
    Utf8String outStr = escapedStr;
    curl_free(escapedStr);
    return outStr;
#else
    return BeStringUtilities::UriEncode(inStr.c_str());
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Request HttpClient::CreateRequest(Utf8StringCR url, Utf8StringCR method) const
    {
    Request request(url, method, m_handler);

    m_defaultHeadersProvider->FillHttpRequestHeaders(request.GetHeaders());
    request.SetCredentials(m_credentials);

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
    request.GetHeaders().SetAccept("application/json");

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
