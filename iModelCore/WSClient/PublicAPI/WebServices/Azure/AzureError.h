/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
        WSCLIENT_EXPORT AzureError(Http::ConnectionStatus connectionStatus, Http::HttpStatus httpStatus, Utf8StringCR code, Utf8StringCR message, Utf8StringCR description);
        Utf8StringCR GetCode() const { return m_code; }
    };

DEFINE_POINTER_SUFFIX_TYPEDEFS(AzureError)
END_BENTLEY_WEBSERVICES_NAMESPACE
