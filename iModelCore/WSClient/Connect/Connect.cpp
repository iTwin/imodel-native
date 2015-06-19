/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Connect/Connect.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/Connect.h>

#include <Bentley/Base64Utilities.h>
#include <MobileDgn/Utils/Http/HttpClient.h>
#include <WebServices/Connect/SamlToken.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

static ClientInfoPtr s_clientInfo;
static IHttpHandlerPtr s_customHttpHandler;
static bool s_connectInitialized = false;

static Utf8String s_wsgUrl;
static Utf8String s_eulaUrl;
static Utf8String s_stsUrl;

/////////////////////////////////////////////////////////////
// Authentication related stuff

#ifdef DEBUG
#define CONNECT_TOKEN_LIFETIME 30
#else // DEBUG
#define CONNECT_TOKEN_LIFETIME 1440
#endif // !DEBUG

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void Connect::Initialize (ClientInfoPtr clientInfo, IHttpHandlerPtr customHttpHandler)
    {
    BeAssert (nullptr != clientInfo);

    if (!s_connectInitialized)
        {
        s_clientInfo = clientInfo;
        s_customHttpHandler = customHttpHandler;
        s_connectInitialized = true;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void Connect::Uninintialize ()
    {
    s_clientInfo = nullptr;
    s_customHttpHandler = nullptr;
    s_connectInitialized = false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR Connect::GetWsgUrl ()
    {
    if (s_wsgUrl.empty ())
        {
        s_wsgUrl = UrlProvider::GetConnectWsgUrl ();
        }
    return s_wsgUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void Connect::SetWsgUrl (Utf8StringCR url)
    {
    s_wsgUrl = url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR Connect::GetEulaUrl ()
    {
    if (s_eulaUrl.empty ())
        {
        s_eulaUrl = UrlProvider::GetConnectEulaUrl ();
        }
    return s_eulaUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Rolandas.Rimkus    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR Connect::GetStsUrl ()
    {
    if (s_stsUrl.empty ())
        {
        s_stsUrl = UrlProvider::GetConnectLearnStsAuthUri ();
        }
    return s_stsUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void Connect::SetEulaUrl (Utf8StringCR url)
    {
    s_eulaUrl = url;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Connect::GetStsToken (CredentialsCR creds, SamlTokenR tokenOut, Utf8CP appliesToUrlString, Utf8CP stsUrl)
    {
    Json::Value issueExParams;
    issueExParams["ActAs"] = "";

    Utf8PrintfString credsPair ("%s:%s", creds.GetUsername ().c_str (), creds.GetPassword ().c_str ());
    Utf8PrintfString authorization ("Basic %s", Base64Utilities::Encode (credsPair).c_str ());

    return GetStsToken (authorization, issueExParams, tokenOut, appliesToUrlString, stsUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Connect::GetStsToken (SamlTokenCR parentToken, SamlTokenR tokenOut, Utf8CP appliesToUrlString, Utf8CP stsUrl)
    {
    Json::Value issueExParams;

    issueExParams["ActAs"] = parentToken.AsString ();
    issueExParams["AppliesToBootstrapToken"] = appliesToUrlString;

    // Note: the X509 certificate extracted from the token below is already base64 encoded.
    Utf8String cert;
    if (SUCCESS != parentToken.GetX509Certificate (cert))
        {
        return BC_ERROR;
        }

    Utf8PrintfString authorization ("X509 access_token=%s", cert.c_str ());

    return GetStsToken (authorization, issueExParams, tokenOut, appliesToUrlString, stsUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Connect::GetStsToken (Utf8StringCR authorization, JsonValueCR issueExParams, SamlTokenR tokenOut, Utf8CP appliesToUrlString, Utf8CP stsUrl)
    {
    BeAssert (s_connectInitialized);

    Json::Value issueExParamsValue = issueExParams;

    // Make a REST request
    issueExParamsValue["TokenType"] = "";
    issueExParamsValue["KeyType"] = "";
    issueExParamsValue["AppliesTo"] = appliesToUrlString;
    issueExParamsValue["Claims"] = "";
    issueExParamsValue["Lifetime"] = Utf8PrintfString ("%d", CONNECT_TOKEN_LIFETIME);
    issueExParamsValue["OnBehalfOf"] = "";
    issueExParamsValue["Properties"] = "";
    issueExParamsValue["DeviceId"] = s_clientInfo->GetDeviceId ();
    issueExParamsValue["AppId"] = s_clientInfo->GetProductToken ();

    HttpClient client (s_clientInfo, s_customHttpHandler);
    HttpRequest request = client.CreatePostRequest (stsUrl);
    request.GetHeaders ().SetContentType ("application/json");
    request.GetHeaders ().SetAuthorization (authorization);

    HttpStringBodyPtr requestBody = HttpStringBody::Create (Json::FastWriter ().write (issueExParamsValue));
    request.SetRequestBody (requestBody);

    HttpResponse httpResponse = request.Perform ();
    if (httpResponse.GetConnectionStatus () != ConnectionStatus::OK)
        {
        return BC_ERROR;
        }
    if (httpResponse.GetHttpStatus () == HttpStatus::Unauthorized)
        {
        return BC_LOGIN_ERROR;
        }
    Json::Value root = httpResponse.GetBody ().AsJson ();
    if (root["RequestedSecurityToken"].empty ())
        {
        BeAssert (false && "Bentley::MobileUtils::Connect::GetStsToken(): Token not found.");
        return BC_ERROR;
        }

    tokenOut = SamlToken (Utf8String (root["RequestedSecurityToken"].asString ()));
    if (!tokenOut.IsSupported ())
        {
        return BC_ERROR;
        }

    return BC_SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Bentley Systems 04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Connect::Login (CredentialsCR creds, SamlTokenR tokenOut, Utf8CP appliesToUrl /*= nullptr*/, Utf8CP stsUrl /*= nullptr*/)
    {
    Utf8String appliesToUrlString;
    if (appliesToUrl != nullptr)
        {
        appliesToUrlString = appliesToUrl;
        }
    else
        {
        appliesToUrlString = GetWsgUrl ();
        }

    Utf8String stsUrlString;
    if (stsUrl != nullptr)
        {
        stsUrlString = stsUrl;
        }
    else
        {
        stsUrlString = UrlProvider::GetConnectLearnStsAuthUri().c_str();
        }

    return GetStsToken (creds, tokenOut, appliesToUrlString.c_str (), stsUrlString.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool Connect::IsImsLoginRedirect (HttpResponseCR response)
    {
    if (HttpStatus::OK != response.GetHttpStatus ())
        {
        return false;
        }
    if (Utf8String::npos == response.GetEffectiveUrl ().find ("/IMS/Account/Login?"))
        {
        return false;
        }
    return true;
    }
