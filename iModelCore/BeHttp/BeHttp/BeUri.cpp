/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/BeUri.cpp $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <BeHttp/BeUri.h>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Travis.Cobbs           10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeUri::GetScheme(bool lowerCase) const
    {
    size_t schemeLength = m_content.find("://");
    if (schemeLength >= m_content.size())
        return "";
    Utf8String scheme = m_content.substr(0, schemeLength);
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
    size_t hostEnd = m_content.find('/', hostStart);
    if (hostEnd >= m_content.size())
        hostEnd = m_content.size();
    Utf8String host = m_content.substr(hostStart, hostEnd - hostStart);
    if (lowerCase)
        host.ToLower();
    return host;
    }
