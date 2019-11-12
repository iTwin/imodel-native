/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeHttp/HttpResponse.h>

#include <BeHttp/HttpBody.h>
#include <BeHttp/HttpStatusHelper.h>
#include <map>

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpResponseContent::HttpResponseContent(HttpBodyPtr responseBody) :
m_body(responseBody.IsValid() ? responseBody : static_cast<HttpBodyPtr>(HttpStringBody::Create()))
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Response::Response() : Response(nullptr, nullptr, ConnectionStatus::None, HttpStatus::None)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Response::Response(HttpResponseContentPtr responseContent, Utf8CP effectiveUrl, ConnectionStatus connectionStatus, HttpStatus httpStatus) :
m_content(responseContent.IsValid() ? responseContent : HttpResponseContent::Create()),
m_effectiveUrl(effectiveUrl),
m_connectionStatus(connectionStatus),
m_httpStatus(httpStatus)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Response::Response(ConnectionStatus status) : Response(nullptr, nullptr, status, HttpStatus::None)
    {
    if (ConnectionStatus::OK != status)
        return;

    BeAssert(false && "Bad creation of HttpResponse: ConnectionStatus is ok, no other parameters given");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Response::Response(HttpStatus httpStatus, Utf8StringCR effectiveUrl, HttpResponseContentPtr responseContent) :
Response(responseContent, effectiveUrl.c_str(), ConnectionStatus::OK, httpStatus)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Response::Response(HttpStatus httpStatus, Utf8StringCR effectiveUrl, Utf8StringCR headers, Utf8StringCR body) :
Response(HttpResponseContent::Create(HttpStringBody::Create(body)), effectiveUrl.c_str(), ConnectionStatus::OK, httpStatus)
    {
    if (headers.empty())
        return;

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
            m_content->GetHeaders().SetValue(header[0], " ");
            continue;
            }

        if (2 == header.size())
            {
            m_content->GetHeaders().SetValue(header[0], header[1]);
            continue;
            }

        Utf8String value;
        for (size_t i = 1; i < header.size(); ++i)
            {
            value += header[i];
            if (header.size() - 1 != i)
                value += ":";
            }

        m_content->GetHeaders().SetValue(header[0], value);
        }
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
