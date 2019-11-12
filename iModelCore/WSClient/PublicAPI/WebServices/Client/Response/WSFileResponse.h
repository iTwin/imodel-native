/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Client/Response/WSResponse.h>
#include <BeHttp/HttpClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSFileResponse : public WSResponse
    {
    private:
        BeFileName  m_filePath;

    public:
        WSCLIENT_EXPORT WSFileResponse();
        WSCLIENT_EXPORT WSFileResponse(BeFileName filePath, HttpStatus status, Utf8String eTag);

        WSCLIENT_EXPORT BeFileNameCR GetFilePath() const;
    };

typedef const WSFileResponse& WSFileResponseCR;
typedef WSFileResponse& WSFileResponseR;

END_BENTLEY_WEBSERVICES_NAMESPACE
