/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/BeUri.cpp $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <BeHttp/BeUri.h>
#include <curl/curl.h>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeUri::BeUri(Utf8String uri) : m_uri(uri)
    {
    EscapeUnsafeCharactersInUrl(m_uri);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String& BeUri::EscapeUnsafeCharactersInUrl(Utf8String& url)
    {
    // More information:
    // http://tools.ietf.org/html/rfc1738#section-2.2
    // http://tools.ietf.org/html/rfc2396

    url.ReplaceAll(R"(<)", "%3C");
    url.ReplaceAll(R"(>)", "%3E");
    url.ReplaceAll(R"(")", "%22");
    url.ReplaceAll(R"(#)", "%23");
    // "%" should be encoded by client
    url.ReplaceAll(R"({)", "%7B");
    url.ReplaceAll(R"(})", "%7D");
    url.ReplaceAll(R"(|)", "%7C");
    url.ReplaceAll(R"(\)", "%5C");
    url.ReplaceAll(R"(^)", "%5E");
    // "~" is considered safe
    url.ReplaceAll(R"([)", "%5B");
    url.ReplaceAll(R"(])", "%5D");
    url.ReplaceAll(R"(`)", "%60");

    return url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::EscapeString(Utf8StringCR str)
    {
    Utf8P escapedStr = curl_escape(str.c_str(), (int) str.length());
    Utf8String outStr = escapedStr;
    curl_free(escapedStr);
    return outStr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::UnescapeString(Utf8StringCR str)
    {
    Utf8P unescapedStr = curl_unescape(str.c_str(), (int) str.length());
    Utf8String outStr = unescapedStr;
    curl_free(unescapedStr);
    return outStr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::GetScheme(bool lowerCase) const
    {
    size_t schemeLength = m_uri.find("://");
    if (schemeLength >= m_uri.size())
        return "";
    Utf8String scheme = m_uri.substr(0, schemeLength);
    if (lowerCase)
        scheme.ToLower();
    return scheme;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::GetHost(bool lowerCase) const
    {
    Utf8String scheme = GetScheme();
    if (scheme.empty())
        return "";
    size_t hostStart = scheme.size() + 3;
    size_t hostEnd = m_uri.find('/', hostStart);
    if (hostEnd >= m_uri.size())
        hostEnd = m_uri.size();
    Utf8String host = m_uri.substr(hostStart, hostEnd - hostStart);
    if (lowerCase)
        host.ToLower();
    return host;
    }
