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
Request::Request(Utf8StringCR url, Utf8StringCR method, IHttpHandlerPtr customHandler) :
    m_url(EscapeUnsafeSymbolsInUrl(url)), m_method(method), m_responseBody(HttpStringBody::Create()),
    m_httpHandler(customHandler == nullptr ? DefaultHttpHandler::GetInstance() : customHandler)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Request::EscapeUnsafeSymbolsInUrl(Utf8StringCR url)
    {
    Utf8String fixedUrl = url;

    // http://tools.ietf.org/html/rfc1738#section-2.2
    // http://tools.ietf.org/html/rfc2396

    fixedUrl.ReplaceAll(R"(<)", "%3C");
    fixedUrl.ReplaceAll(R"(>)", "%3E");
    fixedUrl.ReplaceAll(R"(")", "%22");
    fixedUrl.ReplaceAll(R"(#)", "%23");
    // "%" should be encoded by client
    fixedUrl.ReplaceAll(R"({)", "%7B");
    fixedUrl.ReplaceAll(R"(})", "%7D");
    fixedUrl.ReplaceAll(R"(|)", "%7C");
    fixedUrl.ReplaceAll(R"(\)", "%5C");
    fixedUrl.ReplaceAll(R"(^)", "%5E");
    // "~" is considered safe
    fixedUrl.ReplaceAll(R"([)", "%5B");
    fixedUrl.ReplaceAll(R"(])", "%5D");
    fixedUrl.ReplaceAll(R"(`)", "%60");

    return fixedUrl;
    }

