/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebTestsHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WebTestsHelper.h"
USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS


HttpRequest UnitTests::StubHttpRequest (Utf8StringCR url, Utf8StringCR method, Utf8StringCR body, const bmap<Utf8String, Utf8String>& headers)
    {
    HttpRequest request (url, method);
    for (auto& header : headers)
        {
        request.GetHeaders ().SetValue (header.first, header.second);
        }
    if (!body.empty ())
        {
        request.SetRequestBody (HttpStringBody::Create (body));
        }
    return request;
    }

HttpRequest UnitTests::StubHttpGetRequest (Utf8StringCR url, const bmap<Utf8String, Utf8String>& headers)
    {
    return StubHttpRequest (url, "GET", "", headers);
    }

HttpResponse UnitTests::StubHttpResponse (ConnectionStatus status)
    {
    HttpStatus httpStatus = HttpStatus::None;
    if (status == ConnectionStatus::OK)
        {
        httpStatus = HttpStatus::OK;
        }
    return HttpResponse (HttpResponseContent::Create (HttpStringBody::Create ()), "", status, httpStatus);
    }

HttpResponse UnitTests::StubHttpResponse (HttpStatus httpStatus, Utf8StringCR body, const bmap<Utf8String, Utf8String>& headers)
    {
    return StubHttpResponse (httpStatus, HttpStringBody::Create (body), headers);
    }

HttpResponse UnitTests::StubHttpResponse (HttpStatus httpStatus, HttpBodyPtr body, const bmap<Utf8String, Utf8String>& headers)
    {
    ConnectionStatus status = ConnectionStatus::OK;
    if (httpStatus == HttpStatus::None)
        {
        status = ConnectionStatus::CouldNotConnect;
        }
    auto content = HttpResponseContent::Create (body);
    for (const auto& header : headers)
        {
        content->GetHeaders ().SetValue (header.first, header.second);
        }
    return HttpResponse (content, "", status, httpStatus);
    }

HttpResponse UnitTests::StubJsonHttpResponse (HttpStatus httpStatus, Utf8StringCR body, const bmap<Utf8String, Utf8String>& headers)
    {
    auto newHeaders = headers;
    newHeaders["Content-Type"] = "application/json";
    return StubHttpResponse (httpStatus, body, newHeaders);
    }

HttpResponse UnitTests::StubHttpResponseWithUrl (HttpStatus httpStatus, Utf8StringCR url)
    {
    auto content = HttpResponseContent::Create (HttpStringBody::Create ());
    return HttpResponse (content, url.c_str (), ConnectionStatus::OK, httpStatus);
    }

void UnitTests::WriteStringToHttpBody (Utf8StringCR string, HttpBodyPtr body)
    {
    size_t bytesToWrite = string.length ();
    size_t bytesWritten = 0;
    while (bytesToWrite)
        {
        auto written = body->Write (bytesWritten + string.c_str (), bytesToWrite);
        if (!written)
            {
            BeAssert (false);
            break;
            }
        bytesWritten += written;
        bytesToWrite -= written;
        }
    }

Utf8String UnitTests::ReadHttpBody (HttpBodyPtr body)
    {
    Utf8String bodyStr;
    if (body.IsNull ())
        {
        return bodyStr;
        }

    char buffer[1000];

    body->Open ();

    size_t read;
    while (read = body->Read (buffer, 1000))
        {
        bodyStr += Utf8String (buffer, read);
        }

    body->Close ();

    return bodyStr;
    }