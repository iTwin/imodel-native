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
#include <BeHttp/HttpStatusHelper.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

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
struct HttpResponseContent : RefCountedBase
{
private:
    HttpResponseHeaders m_headers;
    HttpBodyPtr m_body;
    BEHTTP_EXPORT HttpResponseContent(HttpBodyPtr responseBody);

public:
    static HttpResponseContentPtr Create(HttpBodyPtr responseBody) {return new HttpResponseContent(responseBody);}

    HttpBodyPtr GetBody() {return m_body;}
    HttpResponseHeadersR GetHeaders() {return m_headers;}
    HttpResponseHeadersCR GetHeaders() const {return m_headers;}
};

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct Response
{
private:
    Utf8String m_effectiveUrl;
    HttpStatus m_httpStatus;
    ConnectionStatus m_connectionStatus;
    HttpResponseContentPtr m_content;
    
public:
    //! Create invalid response with ConnectionStatus::None
    BEHTTP_EXPORT Response();

    //! Create response
    BEHTTP_EXPORT Response(HttpResponseContentPtr responseData, Utf8CP effectiveUrl, ConnectionStatus connectionStatus, HttpStatus httpStatus);

    // Get last used url where response came from
    Utf8String GetEffectiveUrl() const {return m_effectiveUrl;}
    HttpBodyCR GetBody() const {return *m_content->GetBody();}

    HttpResponseHeadersCR GetHeaders() const {return m_content->GetHeaders();}
    HttpResponseContentPtr GetContent() {return m_content;}
   
    ConnectionStatus GetConnectionStatus() const {return m_connectionStatus;}
    HttpStatus GetHttpStatus() const {return m_httpStatus;}
    
    bool IsSuccess() const {if (m_connectionStatus != ConnectionStatus::OK) return false; return HttpStatusHelper::GetType(m_httpStatus) == HttpStatusType::Success;}
};

END_BENTLEY_HTTP_NAMESPACE
