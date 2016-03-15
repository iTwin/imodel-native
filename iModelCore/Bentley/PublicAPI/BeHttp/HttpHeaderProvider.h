/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/HttpHeaderProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/IHttpHeaderProvider.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpHeaderProvider : public IHttpHeaderProvider
    {
private:
    HttpRequestHeaders m_headers;

public:
    static IHttpHeaderProviderPtr Create (HttpRequestHeadersCR headers = HttpRequestHeaders ())
        {
        return std::make_shared<HttpHeaderProvider> (headers);
        };
    HttpHeaderProvider (HttpRequestHeadersCR headers) : m_headers (headers)
        {
        };
    virtual ~HttpHeaderProvider ()
        {
        };
    virtual void FillHttpRequestHeaders (HttpRequestHeaders& headersOut) const override
        {
        for (auto& pair : m_headers.GetMap ())
            {
            headersOut.SetValue (pair.first, pair.second);
            }
        };
    };

END_BENTLEY_HTTP_NAMESPACE
