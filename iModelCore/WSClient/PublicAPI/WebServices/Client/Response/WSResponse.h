/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <BeHttp/HttpClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                julius.cepukenas    05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSResponse
    {
    private:
        bool        m_isModified;
        Utf8String  m_eTag;

    public:
        WSResponse(bool isModified = false) : m_isModified(isModified), m_eTag() {};
        WSResponse(HttpStatus status, Utf8String eTag) : m_isModified(HttpStatus::OK == status), m_eTag(eTag) {};
        WSResponse(bool isModified, Utf8String eTag) : m_isModified(isModified), m_eTag(eTag) {};

        bool IsModified() const { return m_isModified; };
        Utf8StringCR GetETag() const { return m_eTag; };
    };

typedef const WSResponse& WSResponseCR;
typedef WSResponse& WSResponseR;

END_BENTLEY_WEBSERVICES_NAMESPACE
