/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Connect/Passport.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/Passport.h>

#include <Bentley/Base64Utilities.h>
#include <MobileDgn/Utils/Http/HttpClient.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

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
    return UrlProvider::GetPassportUrl();
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
    HttpRequest request = client.CreateGetRequest(url);
    request.GetHeaders().SetContentType("application/json");
    request.GetHeaders().SetAuthorization(authorization);

    HttpResponse httpResponse = request.Perform();
    if (httpResponse.GetConnectionStatus() != ConnectionStatus::OK)
        {
        return PASSPORT_ERROR;
        }

    Json::Value retVal = httpResponse.GetBody().AsJson();

    if (!retVal.asBool())
        {
        return NO_PASSPORT;
        }

    return HAS_PASSPORT;
    }
