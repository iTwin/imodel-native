/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/HttpResponse.cpp $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Response::Response(ConnectionStatus status) : m_connectionStatus(status)
    {
    if (ConnectionStatus::OK != status)
        return;

    BeAssert(false && "Bad creation of HttpResponse: ConnectionStatus is ok, no other parameters given");
    m_effectiveUrl = "";
    m_httpStatus = HttpStatus::InternalServerError;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Response::Response(HttpStatus httpStatus, Utf8String effectiveUrl, HttpResponseContentPtr responseData)
    :   m_content(responseData),
        m_effectiveUrl(effectiveUrl),
        m_connectionStatus(ConnectionStatus::OK),
        m_httpStatus(httpStatus)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response::Response(HttpStatus httpStatus, Utf8String effectiveUrl, Utf8String headers, Utf8String responseContent)
    :
    m_effectiveUrl(effectiveUrl),
    m_connectionStatus(ConnectionStatus::OK),
    m_httpStatus(httpStatus)
    {
    auto content = HttpResponseContent::Create(HttpStringBody::Create(responseContent));

    if (!headers.empty())
        {
        bvector<Utf8String> headerPairs;
        BeStringUtilities::Split(headers.c_str(), "\n", headerPairs);
        for (auto headerPair : headerPairs)
            {
            bvector<Utf8String> header;
            if (':' == headerPair[0])
                continue;

            BeStringUtilities::Split(headerPair.c_str(), ":", header);
            if (0 == header.size() || "" == header[0])
                continue;

            if (1 == header.size())
                {
                content->GetHeaders().SetValue(header[0], " ");
                continue;
                }

            if (2 == header.size())
                {
                content->GetHeaders().SetValue(header[0], header[1]);
                continue;
                }

            Utf8String value;
            for (size_t i = 1; i < header.size(); ++i)
                {
                value += header[i];
                if (header.size() - 1 != i)
                    value += ":";
                }

            content->GetHeaders().SetValue(header[0], value);
            }
        }

    m_content = content;
    }

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
