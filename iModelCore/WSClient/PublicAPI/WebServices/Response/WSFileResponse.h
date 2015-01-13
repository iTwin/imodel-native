/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Response/WSFileResponse.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Common.h>
#include <MobileDgn/Utils/Http/HttpClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

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
        WS_EXPORT WSFileResponse ();
        WS_EXPORT WSFileResponse (BeFileName filePath, MobileDgn::Utils::HttpStatus status, Utf8String eTag);

        WS_EXPORT bool IsModified () const;
        WS_EXPORT BeFileNameCR GetFilePath () const;
        WS_EXPORT Utf8StringCR GetETag () const;
    };

typedef const WSFileResponse& WSFileResponseCR;
typedef WSFileResponse& WSFileResponseR;

END_BENTLEY_WEBSERVICES_NAMESPACE
