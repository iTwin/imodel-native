/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/HttpResponse.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/Http.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/RefCounted.h>
#include <Bentley/WString.h>
#include <Bentley/bmap.h>

#include "HttpHeaders.h"
#include "HttpBody.h"
#include "HttpStatus.h"

BEGIN_BENTLEY_HTTP_NAMESPACE

struct HttpResponse;
typedef HttpResponse& HttpResponseR;
typedef const HttpResponse& HttpResponseCR;

struct HttpResponseContent;
typedef RefCountedPtr<HttpResponseContent> HttpResponseContentPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                             Benediktas.Lipnickas    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
enum class ConnectionStatus
    {
    None,
    OK,
    Canceled,
    CouldNotConnect,
    ConnectionLost,
    Timeout,
    CertificateError,
    UnknownError
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpResponse : RefCountedBase
    {
private:
    Utf8String               m_effectiveUrl;
    
    HttpStatus               m_httpStatus;
    ConnectionStatus         m_connectionStatus;
    
    HttpResponseContentPtr   m_content;
    
public:
    //! Create invalid response with ConnectionStatus::None
    BEHTTP_EXPORT HttpResponse();
    //! Create response
    BEHTTP_EXPORT HttpResponse(HttpResponseContentPtr responseData, Utf8CP effectiveUrl, ConnectionStatus connectionStatus, HttpStatus httpStatus);

    // Get last used url where response came from
    BEHTTP_EXPORT Utf8String GetEffectiveUrl() const;

    BEHTTP_EXPORT HttpBodyCR GetBody() const;
    BEHTTP_EXPORT HttpResponseHeadersCR GetHeaders() const;
    BEHTTP_EXPORT HttpResponseContentPtr GetContent();
    
    BEHTTP_EXPORT ConnectionStatus GetConnectionStatus() const;
    BEHTTP_EXPORT HttpStatus       GetHttpStatus() const;
    
    BEHTTP_EXPORT bool IsSuccess() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpResponseContent : RefCountedBase
    {
private:
    HttpResponseHeaders m_headers;
    HttpBodyPtr m_body;

    HttpResponseContent (HttpBodyPtr responseBody);

public:
    BEHTTP_EXPORT static HttpResponseContentPtr Create(HttpBodyPtr responseBody);

    BEHTTP_EXPORT HttpBodyPtr GetBody();
    BEHTTP_EXPORT HttpResponseHeadersR    GetHeaders();
    BEHTTP_EXPORT HttpResponseHeadersCR   GetHeaders() const;
    };

END_BENTLEY_HTTP_NAMESPACE
