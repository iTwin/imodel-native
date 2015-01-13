/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/WebServicesTestsHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WebServicesTestsHelper.h"

HttpResponse StubHttpResponse (ConnectionStatus status)
    {
    HttpStatus httpStatus = HttpStatus::None;
    if (status == ConnectionStatus::OK)
        {
        httpStatus = HttpStatus::OK;
        }
    return HttpResponse (HttpResponseContent::Create (HttpStringBody::Create ()), "", status, httpStatus);
    }

HttpResponse StubHttpResponse (HttpStatus httpStatus, Utf8StringCR body, const std::map<Utf8String, Utf8String>& headers)
    {
    return StubHttpResponse (httpStatus, HttpStringBody::Create (body), headers);
    }

HttpResponse StubHttpResponse (HttpStatus httpStatus, HttpBodyPtr body, const std::map<Utf8String, Utf8String>& headers)
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

HttpResponse StubJsonHttpResponse (HttpStatus httpStatus, Utf8StringCR body, const std::map<Utf8String, Utf8String>& headers)
    {
    auto newHeaders = headers;
    newHeaders["Content-Type"] = "application/json";
    return StubHttpResponse (httpStatus, body, newHeaders);
    }

HttpResponse StubHttpResponseWithUrl (HttpStatus httpStatus, Utf8StringCR url)
    {
    auto content = HttpResponseContent::Create (HttpStringBody::Create ());
    return HttpResponse (content, url.c_str (), ConnectionStatus::OK, httpStatus);
    }

void WriteStringToHttpBody (Utf8StringCR string, HttpBodyPtr body)
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

Utf8String ReadHttpBody (HttpBodyPtr body)
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

HttpResponse StubWSErrorHttpResponse (HttpStatus status, Utf8StringCR errorId, Utf8StringCR message, Utf8StringCR description)
    {
    Json::Value errorJson;

    errorJson["errorId"] = errorId;
    errorJson["errorMessage"] = message;
    errorJson["errorDescription"] = description;

    return StubHttpResponse (status, errorJson.toStyledString (), {{"Content-Type", "application/json"}});
    }

WSInfo StubWSInfo (BeVersion version, WSInfo::Type type)
    {
    return WSInfo (version, type);
    }

HttpResponse StubWSInfoHttpResponseV1 ()
    {
    return StubWSInfoHttpResponse (BeVersion (1, 3));
    }

HttpResponse StubWSInfoHttpResponseV1BentleyConnect ()
    {
    auto bodyStub = R"(..stub.. Web Service Gateway for BentleyCONNECT ..stub.. <span id="versionLabel">1.1.0.0</span> ..stub..)";
    return StubHttpResponse (HttpStatus::OK, bodyStub, {{"Content-Type", "text/html"}});
    }

HttpResponse StubWSInfoHttpResponseV2 ()
    {
    return StubWSInfoHttpResponse (BeVersion (2, 0));
    }

HttpResponse StubWSInfoHttpResponse (BeVersion serverVersion)
    {
    Utf8PrintfString body (R"({ "serverVersion" : "%s"  })", serverVersion.ToString ().c_str ());
    return StubHttpResponse (HttpStatus::OK, body, {{"Content-Type", "application/json"}});
    }

WSError StubWSConnectionError ()
    {
    return WSError (StubHttpResponse ());
    }

WSError StubWSCanceledError ()
    {
    return WSError (StubHttpResponse (ConnectionStatus::Canceled));
    }

WSError StubWSConflictError ()
    {
    return WSError (StubHttpResponse (ConnectionStatus::Canceled));
    }