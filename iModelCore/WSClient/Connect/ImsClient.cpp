/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ImsClient.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ImsClient.h>

#include <Bentley/Base64Utilities.h>
#include <BeHttp/HttpClient.h>
#include <WebServices/Connect/SamlToken.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

ImsClientPtr s_client;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ImsClient::InitializeShared(ClientInfoPtr info, IHttpHandlerPtr httpHandler)
    {
    s_client = ImsClient::Create(info, httpHandler);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ImsClient::UnitializeShared()
    {
    s_client = nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ImsClientPtr ImsClient::GetShared()
    {
    BeAssert(nullptr != s_client && "Call ImsClient::InitializeShared() first. Note - this is compatibility function that will be DEPRECATED");
    return s_client;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ImsClient::ImsClient(ClientInfoPtr clientInfo, IHttpHandlerPtr httpHandler) :
m_clientInfo(clientInfo),
m_httpHandler(httpHandler)
    {
    //this "username:apikey" is hardcoded into CONNECTION Client source code, so we have it here as well
    //it can be overriden by the client application by calling SetImsActiveSTSHelperServiceAuthKey
    SetImsActiveSTSHelperServiceAuthKey("BentleyConnectAppServiceUser@bentley.com", "A6u6I09FP70YQWHlbrfS0Ct2fTyIMt6JNnMtbjHSx6smCgSinlRFCXqM6wcuYuj");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ImsClientPtr ImsClient::Create(ClientInfoPtr clientInfo, IHttpHandlerPtr httpHandler)
    {
    return ImsClientPtr(new ImsClient(clientInfo, httpHandler));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SamlTokenResult> ImsClient::RequestToken(CredentialsCR creds, Utf8String rpUri, uint64_t lifetime)
    {
    if (rpUri.empty())
        rpUri = GetRelyingPartyUri(*m_clientInfo);

    Utf8PrintfString credsPair("%s:%s", creds.GetUsername().c_str(), creds.GetPassword().c_str());
    Utf8PrintfString authorization("Basic %s", Base64Utilities::Encode(credsPair).c_str());

    Json::Value params;
    params["ActAs"] = "";

    Utf8String stsUrl = UrlProvider::Urls::ImsStsAuth.Get();

    return RequestToken(authorization, params, rpUri, stsUrl, lifetime, creds.GetUsername());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SamlTokenResult> ImsClient::RequestToken(SamlTokenCR parentToken, Utf8String rpUri, uint64_t lifetime)
    {
    if (rpUri.empty())
        rpUri = GetRelyingPartyUri(*m_clientInfo);

    Utf8String cert;
    if (SUCCESS != parentToken.GetX509Certificate(cert))
        return CreateCompletedAsyncTask(SamlTokenResult::Error({}));

    Utf8PrintfString authorization("X509 access_token=%s", cert.c_str());

    Json::Value params;
    params["ActAs"] = parentToken.AsString();
    params["AppliesToBootstrapToken"] = rpUri;

    Utf8String stsUrl = UrlProvider::Urls::ImsActiveStsDelegationService.Get() + "/json/IssueEx";
    Utf8String username = ConnectSignInManager::ReadUserInfo(parentToken).username;

    return RequestToken(authorization, params, rpUri, stsUrl, lifetime, username);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SamlTokenResult> ImsClient::RequestToken
(
Utf8StringCR authorization,
JsonValueR params,
Utf8StringCR rpUri,
Utf8StringCR stsUrl,
uint64_t lifetime,
Utf8StringCR pastUsername
)
    {
    params["TokenType"] = "";
    params["KeyType"] = "";
    params["AppliesTo"] = rpUri;
    params["Claims"] = "";
    params["OnBehalfOf"] = "";
    params["Properties"] = "";
    params["DeviceId"] = m_clientInfo->GetDeviceId();
    params["AppId"] = m_clientInfo->GetProductToken();

    if (0 != lifetime)
        params["Lifetime"] = Utf8PrintfString("%llu", lifetime);

    HttpClient client(m_clientInfo, UrlProvider::GetSecurityConfigurator(m_httpHandler));
    Http::Request request = client.CreatePostRequest(stsUrl);
    request.GetHeaders().SetAuthorization(authorization);
    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);
    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter::ToString(params)));
    request.SetValidateCertificate(true); // Ensure secure connection when passing authentication information

    return request.PerformAsync()->Then<SamlTokenResult>([rpUri, stsUrl, pastUsername] (Http::ResponseCR response)
        {
        if (response.GetConnectionStatus() != ConnectionStatus::OK)
            return SamlTokenResult::Error({response});

        if (response.GetHttpStatus() == HttpStatus::InternalServerError)
            LOG.errorv("ImsClient: Received server error. Make sure that relying party URI '%s' is correct and registered for IMS service '%s'",
                rpUri.c_str(), stsUrl.c_str());

        if (response.GetHttpStatus() != HttpStatus::OK)
            return SamlTokenResult::Error({response});

        Json::Value body = Json::Reader::DoParse(response.GetBody().AsString());
        if (!body["RequestedSecurityToken"].isString())
            {
            BeAssert(false && "Token not found in IMS response");
            return SamlTokenResult::Error({});
            }

        auto tokenStr = body["RequestedSecurityToken"].asString();
        auto token = std::make_shared<SamlToken>(tokenStr);

        Utf8String newUsername = ConnectSignInManager::ReadUserInfo(*token).username;
        if (pastUsername.empty() || !newUsername.EqualsI(pastUsername))
            return SamlTokenResult::Error({});

        if (!token->IsSupported())
            {
            BeAssert(false && "Token not supported");
            return SamlTokenResult::Error({});
            }

        return SamlTokenResult::Success(token);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImsClient::IsLoginRedirect(Http::ResponseCR response)
    {
    if (HttpStatus::OK != response.GetHttpStatus())
        return false;

    if (Utf8String::npos == response.GetEffectiveUrl().find("/IMS/Account/Login?"))
        return false;

    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ron.Stewart     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImsClient::GetFederatedSignInUrl(ClientInfoCR info, Utf8StringCR windowsDomainName)
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

    Utf8String rpUri = GetClientRelyingPartyUriForWtrealm(info);
    if (rpUri.empty())
        return nullptr;

    Utf8String signInUrl = UrlProvider::Urls::ImsFederatedAuth.Get();
    signInUrl += "?wa=wsignin1.0&wtrealm=" + rpUri;

    if (!windowsDomainName.empty())
        signInUrl += "&ofh=" + Base64Utilities::Encode(windowsDomainName);

    return signInUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Ron.Stewart     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImsClient::GetRelyingPartyUri(ClientInfoCR info)
    {
    // Note: It seems weird that we get the wtrealm parameter and URL decode here (instead
    // of creating the raw RP URI here and URL-encoding for the wtrealm parameter. The
    // latter won't work because it would encode the '_' in the wtrealm parameter, and
    // that leads to obscure JS errors in the IMS web pages during sign-in.
    return BeStringUtilities::UriDecode(GetClientRelyingPartyUriForWtrealm(info).c_str());
    }

/*--------------------------------------------------------------------------------------+
* Return the client appliction's relying party URI, encoded in a form suitable for use
* in a federated sign-in URL's wtrealm query parameter.
* @bsimethod                                                    Ron.Stewart     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImsClient::GetClientRelyingPartyUriForWtrealm(ClientInfoCR info)
    {
    if (info.GetApplicationProductId().empty())
        {
        BeAssert(false && "Application ProductId is missing in ClientInfo");
        return nullptr;
        }

#if defined(BENTLEY_WIN32)
    Utf8String platformType("desktop");
#else
    Utf8String platformType("mobile");
#endif

    // URL-encode the RP URI, except for the underscore as that breaks IMS
    Utf8String rpUri = BeStringUtilities::UriEncode("sso://wsfed");
    rpUri += "_" + platformType + BeStringUtilities::UriEncode("/") + info.GetApplicationProductId();
    return rpUri;
        }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImsClient::GetLegacyRelyingPartyUri()
    {
    return UrlProvider::Urls::ConnectWsgGlobal.Get();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Tomas.Tamasauskas    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ImsClient::GetA2PUrl(Utf8String url, SamlTokenPtr token, Utf8String tokenGuid)
    {
    #if defined(BENTLEY_WIN32)
        Utf8String platformType("desktop");
    #else
        Utf8String platformType("mobile");
    #endif

    Utf8String rpUri = "sso://wsfed_" + platformType + "/" + m_clientInfo->GetApplicationProductId();

    Json::Value params;
    params["Realm"] = rpUri; //client relying party uri, token registretion service does not accept base64 encoded strings here
    params["TokenKey"] = tokenGuid;//BeGuid(true).ToString();
    params["TokenXml"] = token->AsString();
    params["TargetRealm"] = url;

    Utf8PrintfString authorization("Basic %s", Base64Utilities::Encode(m_ImsActiveSTSHelperServiceAuthKey).c_str());
    
    HttpClient client(m_clientInfo, UrlProvider::GetSecurityConfigurator(m_httpHandler));
    Http::Request request = client.CreatePostRequest(UrlProvider::Urls::ImsActiveSTSHelperService.Get() + "/json/RegisterToken");
    request.GetHeaders().SetAuthorization(authorization);
    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);
    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter::ToString(params)));
    request.SetValidateCertificate(true); // Ensure secure connection when passing authentication information

    auto response = request.Perform().get(folly::Duration(5000));
    if (response.IsSuccess())
        {
        Json::Value body = Json::Reader::DoParse(response.GetBody().AsString());
        return UrlProvider::Urls::ImsPassiveAuthUrl.Get() + "?key1=" + tokenGuid + "&key2=" + body.asString() + "&rpUrl=" + BeStringUtilities::UriEncode(url.c_str());
        }
    else
        {
        BeAssert(false && "ImsClient:GetA2PUrl: error occured while registering a token.");
        return url;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               Tomas.Tamasauskas    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/	
Utf8StringCR ImsClient::SetImsActiveSTSHelperServiceAuthKey(Utf8StringCR user, Utf8StringCR apiKey)
    {
    Utf8PrintfString credsPair("%s:%s", user.c_str(), apiKey.c_str());
    return m_ImsActiveSTSHelperServiceAuthKey = credsPair;
    }
