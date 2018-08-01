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
Utf8String userName,
ClientInfoPtr clientInfo,
std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
BeFileNameCR dbPath,
IHttpHandlerPtr httpHandler
) :
m_userName(userName),
m_clientInfo(clientInfo),
m_authProvider(authenticationProvider),
m_dbPath(dbPath),
m_httpHandler(httpHandler)
    {
    m_usageDb = std::make_unique<UsageDb>();
    m_correlationId = BeGuid(true).ToString();
    m_projectId = LICCLIENT_NO_PROJECT_ID_STR;
    m_featureString = "";
    m_timeRetriever = TimeRetriever::Get();
    m_delayedExecutor = DelayedExecutor::Get();
    }
   
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus ClientImpl::StartApplication()
    {
    if (SUCCESS != m_usageDb->OpenOrCreate(m_dbPath))
        return LicenseStatus::LICENSE_STATUS_Error;

    // Get policy token
    m_policyToken = GetPolicyToken();

    //TODO: Get product status

    if (RecordUsage() == ERROR)
        return LicenseStatus::LICENSE_STATUS_Error;
     
    // Begin licensing heartbeat
    int64_t curentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
    UsageHeartbeat(curentTimeUnixMs);
    LogPostingHeartbeat(curentTimeUnixMs);
    //TODO: PolicyHeartbeat(curentTimeUnixMs);

    // This is only a logging example
    LOG.info("StartApplication");

    return LicenseStatus::LICENSE_STATUS_Ok;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::UsageHeartbeat(int64_t currentTime)
    {
    m_lastRunningUsageheartbeatStartTime = currentTime;
    m_delayedExecutor->Delayed(USAGE_HEARTBEAT_INTERVAL_MS).then([this, currentTime]
        {
        if (currentTime != m_lastRunningUsageheartbeatStartTime)
            return;

        if (!m_usageDb->IsDbOpen())
            return;

        int64_t curentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
        if (SUCCESS != RecordUsage())
            return;

        UsageHeartbeat(curentTimeUnixMs);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::PolicyHeartbeat(int64_t currentTime)
    {
    m_lastRunningPolicyheartbeatStartTime = currentTime;
    m_delayedExecutor->Delayed(POLICY_HEARTBEAT_INTERVAL_MS).then([this, currentTime]
        {
        if (currentTime != m_lastRunningPolicyheartbeatStartTime)
            return;

        if (!m_usageDb->IsDbOpen())
            return;

        int64_t curentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
        if (nullptr != GetPolicyToken())
            return;

        PolicyHeartbeat(curentTimeUnixMs);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::LogPostingHeartbeat(int64_t currentTime)
    {
    m_lastRunningLogPostingheartbeatStartTime = currentTime;
    m_delayedExecutor->Delayed(LOG_POSTING_HEARTBEAT_INTERVAL_MS).then([this, currentTime]
        {
        if (currentTime != m_lastRunningLogPostingheartbeatStartTime)
            return;

        if (!m_usageDb->IsDbOpen())
            return;

        int64_t curentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
        if (SUCCESS != PostUsageLogs())
            return;

        LogPostingHeartbeat(curentTimeUnixMs);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::StopApplication()
    {
    m_lastRunningPolicyheartbeatStartTime = 0; // This will stop Policy heartbeat
    m_lastRunningUsageheartbeatStartTime = 0; // This will stop Usage heartbeat
    m_lastRunningLogPostingheartbeatStartTime = 0; // This will stop log posting heartbeat

    m_usageDb->Close();
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::SetProjectId(Utf8String projectId)
    {
    m_projectId = projectId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::SetFeatureString(Utf8String featureString)
    {
    m_featureString = featureString;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::RecordUsage()
    {
    if (!m_usageDb->IsDbOpen())
        return ERROR;

    Utf8String versionString;
    GenerateSID gsid;

    //TODO: Check product status

    // Create usage record

    versionString.Sprintf("%d%.4d%.4d%.4d", m_clientInfo->GetApplicationVersion().GetMajor(), m_clientInfo->GetApplicationVersion().GetMinor(),
                          m_clientInfo->GetApplicationVersion().GetSub1(), m_clientInfo->GetApplicationVersion().GetSub2());

    m_usageDb->RecordUsage(m_policyToken->GetUltimateSAPId(),
                           m_policyToken->GetPrincipalId(),
                           m_policyToken->GetUserId(),
                           m_clientInfo->GetDeviceId(),
                           gsid.GetMachineSID(m_clientInfo->GetDeviceId()),
                           m_userName,
                           gsid.GetUserSID(m_userName, m_clientInfo->GetDeviceId()),
                           m_policyToken->GetPolicyId(),
                           m_policyToken->GetSecurableId(),
                           atoi(m_clientInfo->GetApplicationProductId().c_str()),
                           m_featureString,
                           atoll(versionString.c_str()),
                           m_projectId,
                           m_correlationId,
                           DateTime::GetCurrentTimeUtc().ToString(),
                           LICENSE_CLIENT_SCHEMA_VERSION,
                           GetLoggingPostSource(LogPostingSource::RealTime),
                           m_policyToken->GetCountry(),
                           m_policyToken->GetUsageType());

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<PolicyToken> ClientImpl::GetPolicyToken()
    {
    auto policyToken = GetPolicy().get();

    m_usageDb->AddOrUpdatePolicyFile(policyToken->GetPolicyId(),
                                     policyToken->GetExpirationDate(),
                                     policyToken->GetLastUpdateTime(),
                                     policyToken->GetPolicyFile());
    
    return policyToken;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::PostUsageLogs()
    {
    Utf8String fileName;
    
    BeFileName usageLogPath(m_dbPath.GetDirectoryName());

    fileName.Sprintf("LicUsageLog.%s.csv", BeGuid(true).ToString());
    
    usageLogPath.AppendToPath(BeFileName(fileName));

    if (SUCCESS != m_usageDb->WriteUsageToCSVFile(usageLogPath))
        return ERROR;

    Utf8String ultimateId;

    ultimateId.Sprintf("%ld", m_policyToken->GetUltimateSAPId());

    //SendUsage(usageLogPath, ultimateId);
    
    //TODO: if send usage is successful, clean up the database.
    m_usageDb->CleanUpUsages();

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
    auto url = UrlProvider::Urls::EntitlementPolicyService.Get() + "/GetPolicy";

    auto authHandler = m_authProvider->GetAuthenticationHandler(url, m_httpHandler, IConnectAuthenticationProvider::HeaderPrefix::Saml);

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
