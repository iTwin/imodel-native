/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/Passport.h>

#include <Bentley/Base64Utilities.h>
#include <BeHttp/HttpClient.h>
#include <WebServices/Configuration/UrlProvider.h>

static IHttpHandlerPtr s_httpHandler;
static bool s_passportInitialized = false;

#define HEADER_USERNAME "BentleyConnectAppServiceUser@bentley.com"
#define HEADER_API_KEY "A6u6I09FP70YQWHlbrfS0Ct2fTyIMt6JNnMtbjHSx6smCgSinlRFCXqM6wcuYuj"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Passport::Initialize(IHttpHandlerPtr customHttpHandler)
    {
    if (!s_passportInitialized)
        {
        s_httpHandler = UrlProvider::GetSecurityConfigurator(customHttpHandler);
        s_passportInitialized = true;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void Passport::Uninintialize()
    {
    s_httpHandler = nullptr;
    s_passportInitialized = false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Passport::GetServiceUrl()
    {
    return UrlProvider::Urls::Passport.Get();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Passport::HasUserPassport(Utf8StringCR userGuid)
    {
    Utf8PrintfString url("%s/%s", GetServiceUrl().c_str(), userGuid.c_str());
    Utf8PrintfString credsPair("%s:%s", HEADER_USERNAME, HEADER_API_KEY);
    Utf8PrintfString authorization("Basic %s", Base64Utilities::Encode(credsPair).c_str());

    HttpClient client(nullptr, s_httpHandler);
    Http::Request request = client.CreateGetRequest(url);
    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);
    request.GetHeaders().SetAuthorization(authorization);

    Http::Response httpResponse = request.Perform().get();
    if (httpResponse.GetConnectionStatus() != ConnectionStatus::OK)
        {
        return PASSPORT_ERROR;
        }

    Json::Value retVal;
    if (!Json::Reader::Parse(httpResponse.GetBody().AsString(), retVal))
        return NO_PASSPORT;

    if (!retVal.asBool())
        return NO_PASSPORT;

    return HAS_PASSPORT;
    }
