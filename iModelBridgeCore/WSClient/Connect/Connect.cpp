/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/Connect.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/Connect.h>

#include <Bentley/Base64Utilities.h>
#include <DgnClientFx/Utils/Http/HttpClient.h>
#include <WebServices/Connect/SamlToken.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

static ClientInfoPtr s_clientInfo;
static IHttpHandlerPtr s_customHttpHandler;
static bool s_connectInitialized = false;
static bool s_tokenBasedAuthentication = false;

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
void Connect::Initialize(ClientInfoPtr clientInfo, IHttpHandlerPtr customHttpHandler, bool tokenBasedAuthentication)
    {
    BeAssert(nullptr != clientInfo);

    if (!s_connectInitialized)
        {
        s_clientInfo = clientInfo;
        s_customHttpHandler = customHttpHandler;
        s_connectInitialized = true;
        s_tokenBasedAuthentication = tokenBasedAuthentication;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void Connect::Uninintialize()
    {
    s_clientInfo = nullptr;
    s_customHttpHandler = nullptr;
    s_connectInitialized = false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Connect::GetStsToken(CredentialsCR creds, SamlTokenR tokenOut, Utf8CP appliesToUrlString, Utf8CP stsUrl)
    {
    Json::Value issueExParams;
    issueExParams["ActAs"] = "";

    Utf8PrintfString credsPair("%s:%s", creds.GetUsername().c_str(), creds.GetPassword().c_str());
    Utf8PrintfString authorization("Basic %s", Base64Utilities::Encode(credsPair).c_str());

    return GetStsToken(authorization, issueExParams, tokenOut, appliesToUrlString, stsUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Connect::GetStsToken(SamlTokenCR parentToken, SamlTokenR tokenOut, Utf8CP appliesToUrlString, Utf8CP stsUrl)
    {
    Json::Value issueExParams;

    issueExParams["ActAs"] = parentToken.AsString();
    issueExParams["AppliesToBootstrapToken"] = appliesToUrlString;

    // Note: the X509 certificate extracted from the token below is already base64 encoded.
    Utf8String cert;
    if (SUCCESS != parentToken.GetX509Certificate(cert))
        {
        return BC_ERROR;
        }

    Utf8PrintfString authorization("X509 access_token=%s", cert.c_str());

    return GetStsToken(authorization, issueExParams, tokenOut, appliesToUrlString, stsUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Connect::GetStsToken(Utf8StringCR authorization, JsonValueCR issueExParams, SamlTokenR tokenOut, Utf8CP appliesToUrlString, Utf8CP stsUrl)
    {
    BeAssert(s_connectInitialized);

    Json::Value issueExParamsValue = issueExParams;

    // Make a REST request
    issueExParamsValue["TokenType"] = "";
    issueExParamsValue["KeyType"] = "";
    issueExParamsValue["AppliesTo"] = appliesToUrlString;
    issueExParamsValue["Claims"] = "";
    issueExParamsValue["Lifetime"] = Utf8PrintfString("%d", CONNECT_TOKEN_LIFETIME);
    issueExParamsValue["OnBehalfOf"] = "";
    issueExParamsValue["Properties"] = "";
    issueExParamsValue["DeviceId"] = s_clientInfo->GetDeviceId();
    issueExParamsValue["AppId"] = s_clientInfo->GetProductToken();

    HttpClient client(s_clientInfo, UrlProvider::GetSecurityConfigurator(s_customHttpHandler));
    HttpRequest request = client.CreatePostRequest(stsUrl);
    request.GetHeaders().SetContentType("application/json");
    request.GetHeaders().SetAuthorization(authorization);

    HttpStringBodyPtr requestBody = HttpStringBody::Create(Json::FastWriter().write(issueExParamsValue));
    request.SetRequestBody(requestBody);

    HttpResponse httpResponse = request.Perform();
    if (httpResponse.GetConnectionStatus() != ConnectionStatus::OK)
        {
        return BC_ERROR;
        }
    if (httpResponse.GetHttpStatus() == HttpStatus::Unauthorized)
        {
        return BC_LOGIN_ERROR;
        }
    Json::Value root = httpResponse.GetBody().AsJson();
    if (root["RequestedSecurityToken"].empty())
        {
        BeAssert(false && "Bentley::MobileUtils::Connect::GetStsToken(): Token not found.");
        return BC_ERROR;
        }

    tokenOut = SamlToken(Utf8String(root["RequestedSecurityToken"].asString()));
    if (!tokenOut.IsSupported())
        {
        return BC_ERROR;
        }

    return BC_SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Bentley Systems 04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Connect::Login(CredentialsCR creds, SamlTokenR tokenOut, Utf8CP appliesToUrl, Utf8CP stsUrl)
    {
    Utf8String appliesToUrlString;
    if (appliesToUrl != nullptr)
        {
        appliesToUrlString = appliesToUrl;
        }
    else
        {
        appliesToUrlString = UrlProvider::Urls::ConnectWsgGlobal.Get(); // TODO: applies to URL should be mandatory
        }

    Utf8String stsUrlString;
    if (stsUrl != nullptr)
        {
        stsUrlString = stsUrl;
        }
    else
        {
        stsUrlString = UrlProvider::Urls::ImsStsAuth.Get();
        }

    return GetStsToken(creds, tokenOut, appliesToUrlString.c_str(), stsUrlString.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool Connect::IsImsLoginRedirect(HttpResponseCR response)
    {
    if (HttpStatus::OK != response.GetHttpStatus())
        {
        return false;
        }
    if (Utf8String::npos == response.GetEffectiveUrl().find("/IMS/Account/Login?"))
        {
        return false;
        }
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Connect::IsTokenBasedAuthorization()
    {
    return s_tokenBasedAuthentication;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ron.Stewart     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Connect::GetFederatedSignInUrl(Utf8String windowsDomainName)
    {
    // The sign in URL looks like this:
    //     https://ims.bentley.com?wa=wsignin1.0&wtrealm=<URL-encoded-client-RP-URI>
    //
    // where the RP URI looks like:
    //     sso://wsfed_mobile/<productId>  (iOS or Android)
    //     sso://wsfed_desktop/<productId> (Windows)
    //
    // Windows clients can optionally specify the user's domain name to enable single-sign-in,
    // in which case "&ofh=<Base64-domain-name>" is appended to the above URL.

    // For federated sign in, the ClientInfo object used to initialize this class must contain a Bentley product ID.
    Utf8String clientProductId = s_clientInfo->GetApplicationProductId();
    BeAssert(!clientProductId.empty());
    if (clientProductId.empty())
        {
        return Utf8String();
        }

    Utf8String signInUrl = UrlProvider::Urls::ImsFederatedAuth.Get();
    signInUrl += "?wa=wsignin1.0&wtrealm=" + GetClientRelyingPartyUriForWtrealm();

    if (!windowsDomainName.empty())
        {
        signInUrl += "&ofh=" + Base64Utilities::Encode(windowsDomainName);
        }

    return signInUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ron.Stewart     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Connect::GetClientRelyingPartyUri()
    {
    // Note: It seems weird that we get the wtrealm parameter and URL decode here (instead
    // of creating the raw RP URI here and URL-encoding for the wtrealm parameter. The
    // latter won't work because it would encode the '_' in the wtrealm parameter, and
    // that leads to obscure JS errors in the IMS web pages during sign-in.
    return BeStringUtilities::UriDecode(GetClientRelyingPartyUriForWtrealm().c_str());
    }

/*--------------------------------------------------------------------------------------+
* Return the client appliction's relying party URI, encoded in a form suitable for use
* in a federated sign-in URL's wtrealm query parameter.
* @bsimethod                                                    Ron.Stewart     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Connect::GetClientRelyingPartyUriForWtrealm()
    {
#if defined(BENTLEY_WIN32)
    Utf8String platformType("desktop");
#else
    Utf8String platformType("mobile");
#endif

    // URL-encode the RP URI, except for the underscore in wsfed_<platformType>.
    Utf8String rpUri(BeStringUtilities::UriEncode("sso://wsfed"));
    rpUri += "_" + platformType + BeStringUtilities::UriEncode("/") + s_clientInfo->GetApplicationProductId();
    return rpUri;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Vytautas.Barkauskas     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Connect::RenewToken(SamlTokenCR parentToken, SamlTokenR tokenOut, Utf8CP appliesToUrl, Utf8CP stsUrl)
    {
    Utf8String appliesToUrlString;
    if (appliesToUrl != nullptr)
        {
        appliesToUrlString = appliesToUrl;
        }
    else
        {
        appliesToUrlString = GetClientRelyingPartyUri();
        }

    Utf8String stsUrlString;
    if (stsUrl != nullptr)
        {
        stsUrlString = stsUrl;
        }
    else
        {
        stsUrlString = UrlProvider::Urls::ImsActiveStsDelegationService.Get() + "/json/IssueEx";
        }

    return  GetStsToken(parentToken, tokenOut, appliesToUrlString.c_str(), stsUrlString.c_str());
    }
