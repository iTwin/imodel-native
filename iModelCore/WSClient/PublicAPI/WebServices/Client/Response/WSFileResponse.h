/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/Response/WSFileResponse.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <BeHttp/HttpClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSFileResponse
    {
    private:
        BeFileName  m_filePath;
        bool        m_isModified;
        Utf8String  m_eTag;

    public:
        WSCLIENT_EXPORT WSFileResponse();
        WSCLIENT_EXPORT WSFileResponse(BeFileName filePath, HttpStatus status, Utf8String eTag);

        WSCLIENT_EXPORT bool IsModified() const;
        WSCLIENT_EXPORT BeFileNameCR GetFilePath() const;
        WSCLIENT_EXPORT Utf8StringCR GetETag() const;
    };

typedef const WSFileResponse& WSFileResponseCR;
typedef WSFileResponse& WSFileResponseR;

END_BENTLEY_WEBSERVICES_NAMESPACE
