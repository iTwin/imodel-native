/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../Client/WebServicesClient.h"
#include <BeHttp/HttpError.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct AzureError : public Http::HttpError
    {
    private:
        Utf8String m_code;
        void ParseBody(Http::ResponseCR response);
    public:
        WSCLIENT_EXPORT AzureError();
        WSCLIENT_EXPORT AzureError(Http::ResponseCR response);
        Utf8StringCR GetCode() const { return m_code; }
    };

DEFINE_POINTER_SUFFIX_TYPEDEFS(AzureError)
END_BENTLEY_WEBSERVICES_NAMESPACE
