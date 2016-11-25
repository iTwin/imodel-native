/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebTestsHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WebTestsHelper.h"
USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

Http::Request UnitTests::StubHttpRequest(Utf8StringCR url, Utf8StringCR method, Utf8StringCR body, const bmap<Utf8String, Utf8String>& headers)
    {
    Http::Request request(url, method);
    for (auto& header : headers)
        {
        request.GetHeaders().SetValue(header.first, header.second);
        }
    if (!body.empty())
        {
        request.SetRequestBody(HttpStringBody::Create(body));
        }
    return request;
    }

Http::Request UnitTests::StubHttpGetRequest(Utf8StringCR url, const bmap<Utf8String, Utf8String>& headers)
    {
    return StubHttpRequest(url, "GET", "", headers);
    }

Http::Response UnitTests::StubHttpResponse(ConnectionStatus status)
    {
    HttpStatus httpStatus = HttpStatus::None;
    if (status == ConnectionStatus::OK)
        {
        httpStatus = HttpStatus::OK;
        }
    return Http::Response(HttpResponseContent::Create(HttpStringBody::Create()), "", status, httpStatus);
    }

Http::Response UnitTests::StubHttpResponse(HttpStatus httpStatus, Utf8StringCR body, const bmap<Utf8String, Utf8String>& headers)
    {
    return StubHttpResponse(httpStatus, HttpStringBody::Create(body), headers);
    }

Http::Response UnitTests::StubHttpResponse(HttpStatus httpStatus, HttpBodyPtr body, const bmap<Utf8String, Utf8String>& headers)
    {
    ConnectionStatus status = ConnectionStatus::OK;
    if (httpStatus == HttpStatus::None)
        {
        status = ConnectionStatus::CouldNotConnect;
        }
    auto content = HttpResponseContent::Create(body);
    for (const auto& header : headers)
        {
        content->GetHeaders().SetValue(header.first, header.second);
        }
    return Http::Response(content, "", status, httpStatus);
    }

Http::Response UnitTests::StubJsonHttpResponse(HttpStatus httpStatus, Utf8StringCR body, const bmap<Utf8String, Utf8String>& headers)
    {
    auto newHeaders = headers;
    newHeaders["Content-Type"] = "application/json";
    return StubHttpResponse(httpStatus, body, newHeaders);
    }

Http::Response UnitTests::StubHttpResponseWithUrl(HttpStatus httpStatus, Utf8StringCR url)
    {
    auto content = HttpResponseContent::Create(HttpStringBody::Create());
    return Http::Response(content, url.c_str(), ConnectionStatus::OK, httpStatus);
    }

void UnitTests::WriteStringToHttpBody(Utf8StringCR string, HttpBodyPtr body)
    {
    size_t bytesToWrite = string.length();
    size_t bytesWritten = 0;
    while (bytesToWrite)
        {
        auto written = body->Write(bytesWritten + string.c_str(), bytesToWrite);
        if (!written)
            {
            BeAssert(false);
            break;
            }
        bytesWritten += written;
        bytesToWrite -= written;
        }
    }

Utf8String UnitTests::ReadHttpBody(HttpBodyPtr body)
    {
    Utf8String bodyStr;
    if (body.IsNull())
        {
        return bodyStr;
        }

    char buffer[1000];

    body->Open();

    size_t read;
    while (read = body->Read(buffer, 1000))
        {
        bodyStr += Utf8String(buffer, read);
        }

    body->Close();

    return bodyStr;
    }