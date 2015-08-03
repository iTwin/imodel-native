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

HttpResponse StubWSErrorHttpResponse (HttpStatus status, Utf8StringCR errorId, Utf8StringCR message, Utf8StringCR description)
    {
    Json::Value errorJson;

    errorJson["errorId"] = errorId;
    errorJson["errorMessage"] = message;
    errorJson["errorDescription"] = description;

    return StubHttpResponse (status, errorJson.toStyledString (), {{"Content-Type", "application/json"}});
    }

WSInfo StubWSInfoWebApi (BeVersion webApiVersion, WSInfo::Type type)
    {
    BeVersion serverVersion;
    if (webApiVersion >= BeVersion (2, 0))
        {
        serverVersion = BeVersion (2, 0);
        }
    else if (webApiVersion >= BeVersion (1, 3))
        {
        serverVersion = BeVersion (1, 2);
        }
    else if (webApiVersion >= BeVersion (1, 2))
        {
        serverVersion = BeVersion (1, 1);
        }
    else if (webApiVersion >= BeVersion (1, 1))
        {
        serverVersion = BeVersion (1, 0);
        }
    return WSInfo (serverVersion, webApiVersion, type);
    }

HttpResponse StubWSInfoHttpResponseBentleyConnectV1 ()
    {
    auto bodyStub = R"(..stub.. Web Service Gateway for BentleyCONNECT ..stub.. <span id="versionLabel">1.1.0.0</span> ..stub..)";
    return StubHttpResponse (HttpStatus::OK, bodyStub, {{"Content-Type", "text/html"}});
    }

HttpResponse StubWSInfoHttpResponseWebApi11 ()
    {
    return StubWSInfoHttpResponseWebApi (BeVersion (1, 1));
    }

HttpResponse StubWSInfoHttpResponseWebApi12 ()
    {
    return StubWSInfoHttpResponseWebApi (BeVersion (1, 2));
    }

HttpResponse StubWSInfoHttpResponseWebApi13 ()
    {
    return StubWSInfoHttpResponseWebApi (BeVersion (1, 3));
    }

HttpResponse StubWSInfoHttpResponseWebApi20 ()
    {
    return StubWSInfoHttpResponseWebApi (BeVersion (2, 0));
    }

HttpResponse StubWSInfoHttpResponseWebApi21 ()
    {
    return StubWSInfoHttpResponseWebApi (BeVersion (2, 1));
    }

HttpResponse StubWSInfoHttpResponseWebApi22 ()
    {
    return StubWSInfoHttpResponseWebApi (BeVersion (2, 2));
    }

HttpResponse StubWSInfoHttpResponseWebApi (BeVersion webApiVersion)
    {
    auto info = StubWSInfoWebApi (webApiVersion);
    Utf8PrintfString serverHeader
        (
        "Bentley-WSG/%s, Bentley-WebAPI/%d.%d",
        info.GetVersion ().ToString ().c_str (),
        info.GetWebApiVersion ().GetMajor (),
        info.GetWebApiVersion ().GetMinor ()
        );
    return StubHttpResponse (HttpStatus::OK, "", {{"Server", serverHeader}});
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

ClientInfoPtr StubClientInfo ()
    {
    return std::shared_ptr<ClientInfo> (new ClientInfo ("Bentley-Test", BeVersion (1, 0), "TestAppGUID", "TestDeviceId", "TestSystem"));
    }
