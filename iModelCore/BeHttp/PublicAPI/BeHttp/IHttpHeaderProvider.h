/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeHttp/IHttpHeaderProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/HttpHeaders.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IHttpHeaderProvider> IHttpHeaderProviderPtr;
struct IHttpHeaderProvider
    {
    virtual ~IHttpHeaderProvider ()
        {
        };
    //! Set default header values. Should be thread safe as can be called in any thread that creates requests.
    virtual void FillHttpRequestHeaders (HttpRequestHeaders& headersOut) const = 0;
    };

END_BENTLEY_HTTP_NAMESPACE
