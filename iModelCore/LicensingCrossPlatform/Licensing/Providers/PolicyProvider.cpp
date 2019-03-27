/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Providers/PolicyProvider.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PolicyProvider.h"
#include "../Logging.h"
#include "../GenerateSID.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_WEBSERVICES

PolicyProvider::PolicyProvider
    (
    IBuddiProviderPtr buddiProvider,
    WebServices::ClientInfoPtr clientInfo,
    IHttpHandlerPtr httpHandler,
    AuthType authType,
    std::shared_ptr<IAuthHandlerProvider> authHandlerProvider
    ) :
    m_buddiProvider(buddiProvider),
    m_clientInfo(clientInfo),
    m_httpHandler(httpHandler),
    m_authHandlerProvider(authHandlerProvider)
    {
	m_headerPrefix = GetHeaderPrefix(authType);
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<std::shared_ptr<Policy>> PolicyProvider::GetPolicy()
    {
    LOG.debug("ClientImpl::GetPolicy");

    return folly::collectAll(GetCertificate(), PerformGetPolicyRequest()).then(
        [](const std::tuple<folly::Try<Utf8String>, folly::Try<Utf8String>>& tup)
        {
        Utf8String cert = std::get<0>(tup).value();
        cert.ReplaceAll("\"", ""); // TODO move to the function

        Utf8String policyToken = std::get<1>(tup).value();
        policyToken.ReplaceAll("\"", ""); // TODO move to the function

        return Policy::Create(JWToken::Create(policyToken, cert));
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<std::shared_ptr<Policy>> PolicyProvider::GetPolicyWithKey(Utf8StringCR accessKey)
    {
    LOG.debug("ClientImpl::GetPolicyWithKey");

    return folly::collectAll(GetCertificate(), PerformGetPolicyWithKeyRequest(accessKey)).then(
        [](const std::tuple<folly::Try<Utf8String>, folly::Try<Utf8String>>& tup)
        {
        Utf8String cert = std::get<0>(tup).value();
        cert.ReplaceAll("\"", "");

        Utf8String policyToken = std::get<1>(tup).value();
        policyToken.ReplaceAll("\"", "");

        return Policy::Create(JWToken::Create(policyToken, cert));
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<Utf8String> PolicyProvider::GetCertificate()
    {
    LOG.debug("ClientImpl::GetCertificate");

    auto url = m_buddiProvider->EntitlementPolicyBaseUrl() + "/PolicyTokenSigningCertificate";

    LOG.debugv("GetCertificate - EntitlementPolicyService: %s", url.c_str());

    // EntitlementPolicyService /api/PolicyTokenSigningCertificate does not need header auth
    HttpClient client(nullptr, m_httpHandler);
    return client.CreateGetRequest(url).Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            {
            LOG.errorv("ClientImpl::GetCertificate ERROR: Unable to get certificate %s", HttpError(response).GetMessage().c_str());
            throw HttpError(response);
            }

        return response.GetBody().AsString();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<Utf8String> PolicyProvider::PerformGetPolicyRequest()
    {
    LOG.debug("ClientImpl::PerformGetPolicyRequest");

    if (m_authHandlerProvider == nullptr)
        {
        LOG.error("authHandlerProvider is null, EntilementPolicyService GetPolicy requires authentication");
        throw AsyncError("EntitlementPolicyService GetPolicy requires authentication");
        }

    auto url = m_buddiProvider->EntitlementPolicyBaseUrl() + "/GetPolicy";

    LOG.debugv("ClientImpl::PerformGetPolicyRequest - EntitlementPolicyService: %s", url.c_str());

    auto authHandler = m_authHandlerProvider->GetAuthHandler(url, m_headerPrefix);

    HttpClient client(nullptr, authHandler);

    auto request = client.CreatePostRequest(url);
    Json::Value requestJson(Json::objectValue);
    requestJson["MachineName"] = m_clientInfo->GetDeviceId();
    requestJson["ClientDateTime"] = DateTime::GetCurrentTimeUtc().ToString();
    requestJson["Locale"] = m_clientInfo->GetLanguage();
    requestJson["AppliesTo"] = GETPOLICY_RequestData_AppliesTo_Url;

    Json::Value requestedSecurable(Json::objectValue);
    requestedSecurable["ProductId"] = m_clientInfo->GetApplicationProductId();
    requestedSecurable["FeatureString"] = "";
    requestedSecurable["Version"] = m_clientInfo->GetApplicationVersion().ToString();

    Json::Value requestedSecurables(Json::arrayValue);
    requestedSecurables[0] = requestedSecurable;

    requestJson["RequestedSecurables"] = requestedSecurables;

    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(requestJson)));

    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);

    return request.Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            {
            LOG.errorv("ClientImpl::PerformGetPolicyRequest ERROR: Unable to perform policy request %s", HttpError(response).GetMessage().c_str());
            throw HttpError(response);
            }

        return response.GetBody().AsString();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<Utf8String> PolicyProvider::PerformGetPolicyWithKeyRequest(Utf8StringCR accessKey)
    {
    LOG.debug("ClientImpl::PerformGetPolicyWithKeyRequest");

    auto url = m_buddiProvider->EntitlementPolicyBaseUrl() + "/GetPolicyWithAccessKey";

    LOG.debugv("ClientImpl::PerformGetPolicyWithKeyRequest - EntitlementPolicyService: %s", url.c_str());

    GenerateSID gsid;

    // EntitlementPolicyService /api/GetPolicyWithAccessKey does not need auth
    HttpClient client(nullptr, m_httpHandler);

    auto request = client.CreatePostRequest(url);
    Json::Value requestJson(Json::objectValue);
    requestJson["MachineName"] = m_clientInfo->GetDeviceId();
    requestJson["ClientDateTime"] = DateTime::GetCurrentTimeUtc().ToString();
    requestJson["Locale"] = m_clientInfo->GetLanguage();
    requestJson["AppliesTo"] = GETPOLICY_RequestData_AppliesTo_Url;
    requestJson["MachineSID"] = gsid.GetMachineSID(m_clientInfo->GetDeviceId()).c_str();
    requestJson["AccessKey"] = accessKey;

    Json::Value requestedSecurable(Json::objectValue);
    requestedSecurable["ProductId"] = m_clientInfo->GetApplicationProductId();
    requestedSecurable["FeatureString"] = "";
    requestedSecurable["Version"] = m_clientInfo->GetApplicationVersion().ToString();

    Json::Value requestedSecurables(Json::arrayValue);
    requestedSecurables[0] = requestedSecurable;

    requestJson["RequestedSecurables"] = requestedSecurables;

    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(requestJson)));

    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);

    return request.Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            {
            LOG.errorv("ClientImpl::PerformGetPolicyWithKeyRequest ERROR: Unable to perform policy request %s", HttpError(response).GetMessage().c_str());
            throw HttpError(response);
            }

        return response.GetBody().AsString();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectAuthenticationProvider::HeaderPrefix PolicyProvider::GetHeaderPrefix(AuthType authType)
    {
    switch (authType)
        {
        case AuthType::OIDC:
            return IConnectAuthenticationProvider::HeaderPrefix::Bearer;
        case AuthType::SAML:
            return IConnectAuthenticationProvider::HeaderPrefix::Saml;
        default:
            return IConnectAuthenticationProvider::HeaderPrefix::Saml;
        }
    }