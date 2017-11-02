/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/HttpResponse.cpp $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <BeHttp/HttpResponse.h>

#include <BeHttp/HttpBody.h>
#include <BeHttp/HttpStatusHelper.h>
#include <map>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponseContent::HttpResponseContent(HttpBodyPtr responseBody) :
m_body(responseBody)
    {
    BeAssert(!m_body.IsNull());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Response::Response() :
m_content(HttpResponseContent::Create(HttpStringBody::Create())),
m_connectionStatus(ConnectionStatus::None),
m_httpStatus(HttpStatus::None)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Response::Response(HttpResponseContentPtr responseContent, Utf8CP effectiveUrl, ConnectionStatus connectionStatus, HttpStatus httpStatus) :
m_content(responseContent),
m_effectiveUrl(effectiveUrl),
m_connectionStatus(connectionStatus),
m_httpStatus(httpStatus)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Response::ToStatusString(ConnectionStatus connnectionStatus, HttpStatus httpStatus)
    {
    static std::map<ConnectionStatus, Utf8String> connectionStatusStrings =
        {
            {ConnectionStatus::Canceled, "Canceled"},
            {ConnectionStatus::ConnectionLost, "ConnectionLost"},
            {ConnectionStatus::CouldNotConnect, "CouldNotConnect"},
            {ConnectionStatus::CouldNotResolveProxy, "CouldNotResolveProxy"},
            {ConnectionStatus::CertificateError, "CertificateError"},
            {ConnectionStatus::None, "None"},
            {ConnectionStatus::OK, "OK"},
            {ConnectionStatus::Timeout, "Timeout"},
            {ConnectionStatus::UnknownError, "UnknownError"},
        };

    if (connnectionStatus == ConnectionStatus::OK)
        return Utf8PrintfString("%d", httpStatus);

    return connectionStatusStrings[connnectionStatus];
    }
