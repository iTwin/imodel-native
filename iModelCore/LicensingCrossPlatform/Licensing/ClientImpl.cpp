/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/ClientImpl.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientImpl.h"

#include "UsageDb.h"
#include "PolicyToken.h"
#include "GenerateSID.h"

#include <WebServices/Configuration/UrlProvider.h>

#include "Logging.h"

#include <BeHttp/HttpError.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientImpl::ClientImpl
(
BeFileNameCR dbPath, 
std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
ClientInfoPtr clientInfo,
const ConnectSignInManager::UserInfo& userInfo,
IHttpHandlerPtr httpHandler,
uint64_t heartbeatInterval, 
ITimeRetrieverPtr timeRetriever,
IDelayedExecutorPtr delayedExecutor
) :
m_dbPath (dbPath),
m_authProvider (authenticationProvider),
m_clientInfo (clientInfo),
m_userInfo (userInfo),
m_httpHandler (httpHandler),
m_heartbeatInterval(heartbeatInterval),
m_timeRetriever(timeRetriever),
m_delayedExecutor(delayedExecutor)
    {
    m_usageDb = std::make_unique<UsageDb>();
    m_correlationId = BeGuid(true).ToString();
    }
   
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::StartApplication()
    {
    if (SUCCESS != m_usageDb->OpenOrCreate(m_dbPath))
        return ERROR;

    Utf8String versionString;
    GenerateSID gsid;

    // Check for policy
    auto policyToken = GetPolicy().get();

    m_usageDb->AddOrUpdatePolicyFile(policyToken->GetPolicyId(),
                                     policyToken->GetExpirationDate(),
                                     policyToken->GetLastUpdateTime(),
                                     policyToken->GetPolicyFile());

    // Check for entitlement

    // Create usage record
    m_featureString = "";
    m_projectId = "";
    
    versionString.Sprintf("%d%.4d%.4d%.4d", m_clientInfo->GetApplicationVersion().GetMajor(), m_clientInfo->GetApplicationVersion().GetMinor(),
                          m_clientInfo->GetApplicationVersion().GetSub1(), m_clientInfo->GetApplicationVersion().GetSub2());

    m_usageDb->RecordUsage(policyToken->GetUltimateSAPId(),
                           policyToken->GetPrincipalId(),
                           policyToken->GetUserId(),
                           m_clientInfo->GetDeviceId(),
                           gsid.GetMachineSID(m_clientInfo->GetDeviceId()),
                           m_userInfo.username,
                           gsid.GetUserSID(m_userInfo.username, m_clientInfo->GetDeviceId()),
                           policyToken->GetPolicyId(),
                           policyToken->GetSecurableId(),
                           atoi(m_clientInfo->GetApplicationProductId().c_str()),
                           m_featureString,
                           atoll(versionString.c_str()),
                           m_projectId,
                           m_correlationId,
                           DateTime::GetCurrentTimeUtc().ToString(),
                           LICENSE_CLIENT_SCHEMA_VERSION,
                           GetLoggingPostSource(LogPostingSource::RealTime),
                           policyToken->GetCountry(),
                           policyToken->GetUsageType());

    // Begin licensing heartbeat

    auto lastRecEndTime = m_usageDb->GetLastRecordEndTime();
    int64_t curentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();

    if (lastRecEndTime == 0 || lastRecEndTime < curentTimeUnixMs)
        {
        if (SUCCESS != m_usageDb->InsertNewRecord(curentTimeUnixMs, curentTimeUnixMs + m_heartbeatInterval))
            return ERROR;
        }
    else
        {
        if (SUCCESS != m_usageDb->UpdateLastRecordEndTime(curentTimeUnixMs + m_heartbeatInterval))
            return ERROR;
        }

    // This is only a logging example
    LOG.info("StartApplication");
    
    HeartbeatUsage(curentTimeUnixMs);

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::HeartbeatUsage(int64_t currentTime)
    {
    m_lastRunningheartbeatStartTime = currentTime;
    m_delayedExecutor->Delayed(m_heartbeatInterval).then([this, currentTime]
        {
        if (currentTime != m_lastRunningheartbeatStartTime)
            return;

        if (!m_usageDb->IsDbOpen())
            return;

        int64_t curentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
        if (SUCCESS != m_usageDb->UpdateLastRecordEndTime(curentTimeUnixMs + m_heartbeatInterval))
            return;

        HeartbeatUsage(curentTimeUnixMs);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::StopApplication()
    {
    m_lastRunningheartbeatStartTime = 0; // This will stop Usage heartbeat

    if (!m_usageDb->IsDbOpen())
        return SUCCESS;

    int64_t curentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();

    if (SUCCESS != m_usageDb->UpdateLastRecordEndTime(curentTimeUnixMs))
        return ERROR;

    m_usageDb->Close();
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> ClientImpl::SendUsage(BeFileNameCR usageSCV, Utf8StringCR ultId)
    {
    auto url = UrlProvider::Urls::UsageLoggingServicesLocation.Get();
    url += Utf8PrintfString("/usageLog?ultId=%s&prdId=%s&lng=%s", ultId.c_str(), m_clientInfo->GetApplicationProductId().c_str(),
                            m_clientInfo->GetLanguage().c_str());

    HttpClient client(nullptr, m_httpHandler);
    return client.CreateGetRequest(url).Perform().then(
        [=](Response response)
        {
        if (!response.IsSuccess())
            throw HttpError(response);

        Json::Value jsonBody = Json::Value::From(response.GetBody().AsString());
        auto status = jsonBody["status"].asString();
        auto epUri = jsonBody["epUri"].asString();
        auto sharedAccessSignature = jsonBody["epInfo"]["SharedAccessSignature"].asString();
 
        HttpClient client(nullptr, m_httpHandler);
        auto uploadRequest = client.CreateRequest(epUri + sharedAccessSignature, "PUT");
        uploadRequest.GetHeaders().SetValue("x-ms-blob-type", "BlockBlob");
        uploadRequest.SetRequestBody(HttpFileBody::Create(usageSCV));
        return uploadRequest.Perform().then([=](Response response)
            {
            if (!response.IsSuccess())
                throw HttpError(response);

            return folly::makeFuture();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<Utf8String> ClientImpl::PerformGetPolicyRequest()
    {
    auto url = UrlProvider::Urls::EntitlementPolicyService.Get() + WEBAPI_GetPolicy;

    auto authHandler = m_authProvider->GetAuthenticationHandler(url, m_httpHandler, IConnectAuthenticationProvider::HeaderPrefix::Saml);

    HttpClient client(nullptr, authHandler);

    auto request = client.CreatePostRequest(url);
    Json::Value requestJson(Json::objectValue);
    requestJson[GETPOLICY_RequestData_MachineName] = m_clientInfo->GetDeviceId();
    requestJson[GETPOLICY_RequestData_ClientDateTime] = DateTime::GetCurrentTimeUtc().ToString();
    requestJson[GETPOLICY_RequestData_Locale] = m_clientInfo->GetLanguage();
    requestJson[GETPOLICY_RequestData_AppliesTo] = GETPOLICY_RequestData_AppliesTo_Url;

    Json::Value requestedSecurable(Json::objectValue);
    requestedSecurable[GETPOLICY_RequestData_ProductId] = m_clientInfo->GetApplicationProductId();
    requestedSecurable[GETPOLICY_RequestData_FeatureString] = "";
    requestedSecurable[GETPOLICY_RequestData_Version] = m_clientInfo->GetApplicationVersion().ToString();

    Json::Value requestedSecurables(Json::arrayValue);
    requestedSecurables[0] = requestedSecurable;

    requestJson[GETPOLICY_RequestData_RequestedSecurable] = requestedSecurables;

    request.SetRequestBody(HttpStringBody::Create(Json::FastWriter().write(requestJson)));

    request.GetHeaders().SetContentType(REQUESTHEADER_ContentType_ApplicationJson);

    return request.Perform().then(
        [=] (Response response)
        {
        if (!response.IsSuccess())
            throw HttpError(response);

        return response.GetBody().AsString();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<Utf8String> ClientImpl::GetCertificate()
    {
    auto url = UrlProvider::Urls::EntitlementPolicyService.Get() + "/PolicyTokenSigningCertificate";

    auto authHandler = m_authProvider->GetAuthenticationHandler(url, m_httpHandler, IConnectAuthenticationProvider::HeaderPrefix::Saml);

    HttpClient client(nullptr, authHandler);
    return client.CreateGetRequest(url).Perform().then(
        [=] (Response response)
        {
        if (!response.IsSuccess())
            throw HttpError(response);

        return response.GetBody().AsString();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UsageDb& ClientImpl::GetUsageDb()
    {
    return *m_usageDb;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<std::shared_ptr<PolicyToken>> ClientImpl::GetPolicy()
    {
    return folly::collectAll(GetCertificate(), PerformGetPolicyRequest()).then(
        [] (const std::tuple<folly::Try<Utf8String>, folly::Try<Utf8String>>& tup)
        {
        Utf8String cert = std::get<0>(tup).value();
        cert.ReplaceAll("\"", "");

        Utf8String policyToken = std::get<1>(tup).value();
        policyToken.ReplaceAll("\"", "");

        return PolicyToken::Create(JWToken::Create(policyToken, cert));
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientImpl::GetLoggingPostSource(LogPostingSource lps) const
    {
    switch (lps)
        {
            case RealTime:
                return "RealTime";

            case Offline:
                return "Offline";

            case Checkout:
                return "Checkout";

            default:
                return "Unknown";
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus ClientImpl::GetProductStatus()
	{
	return LicenseStatus::Ok;
	// Cases for Ok:
	//   
	// Cases for Offline:
	// Cases for Expired:
	// Cases for AccessDenied:
	// Cases for DisabledByLogSend:
	// Cases for DisabledByPolicy:
	// Cases for Trial:
	// Cases for NotEntitled:
	}
