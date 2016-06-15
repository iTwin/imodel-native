/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/HttpResponse.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <BeHttp/HttpResponse.h>

#include <BeHttp/HttpBody.h>
#include <BeHttp/HttpStatusHelper.h>

USING_NAMESPACE_BENTLEY_HTTP

#pragma mark - HttpResponseContent

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponseContent::HttpResponseContent (HttpBodyPtr responseBody)
    : m_body (responseBody)
    {
    BeAssert (!m_body.IsNull());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponseContentPtr HttpResponseContent::Create (HttpBodyPtr responseBody)
    {
    return new HttpResponseContent (responseBody);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpBodyPtr HttpResponseContent::GetBody()
    {
    return m_body;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponseHeadersR HttpResponseContent::GetHeaders()
    {
    return m_headers;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponseHeadersCR HttpResponseContent::GetHeaders() const
    {
    return m_headers;
    }

#pragma mark - HttpResponse

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponse::HttpResponse ()
    :   m_content (HttpResponseContent::Create (HttpStringBody::Create())),
        m_connectionStatus (ConnectionStatus::None),
        m_httpStatus (HttpStatus::None)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponse::HttpResponse (HttpResponseContentPtr responseContent, Utf8CP effectiveUrl, ConnectionStatus connectionStatus, HttpStatus httpStatus)
    :   m_content (responseContent),
        m_effectiveUrl (effectiveUrl),
        m_connectionStatus (connectionStatus),
        m_httpStatus (httpStatus)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpResponse::GetEffectiveUrl() const
    {
    return m_effectiveUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpBodyCR HttpResponse::GetBody () const
    {
    return *m_content->GetBody();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponseHeadersCR HttpResponse::GetHeaders() const
    {
    return m_content->GetHeaders();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponseContentPtr HttpResponse::GetContent ()
    {
    return m_content;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStatus HttpResponse::GetHttpStatus() const
    {
    return m_httpStatus;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectionStatus HttpResponse::GetConnectionStatus() const
    {
    return m_connectionStatus;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HttpResponse::IsSuccess() const
    {
    if (m_connectionStatus != ConnectionStatus::OK)
        {
        return false;
        }

    return HttpStatusHelper::GetType (m_httpStatus) == HttpStatusType::Success;
    }

