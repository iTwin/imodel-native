/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ClientImpl.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientImpl.h"
#include "GenerateSID.h"
#include "../PublicAPI/Licensing/Utils/LogFileHelper.h"
#include "Logging.h"
#include "UsageDb.h"

#include <BeHttp/HttpError.h>
#include <WebServices/Configuration/UrlProvider.h>

#if defined (BENTLEY_WIN32)
#include <filesystem>
#endif

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientImpl::ClientImpl
(
const ConnectSignInManager::UserInfo& userInfo,
ClientInfoPtr clientInfo,
std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
BeFileNameCR dbPath,
bool offlineMode,
Utf8String projectId,
Utf8String featureString,
IHttpHandlerPtr httpHandler
) :
m_userInfo(userInfo),
m_clientInfo(clientInfo),
m_authProvider(authenticationProvider),
m_dbPath(dbPath),
m_offlineMode(offlineMode),
m_featureString(featureString),
m_httpHandler(httpHandler)
    {
    m_usageDb = std::make_unique<UsageDb>();
    m_correlationId = BeGuid(true).ToString();
    m_timeRetriever = TimeRetriever::Get();
    m_delayedExecutor = DelayedExecutor::Get();

    if (projectId.empty())
        m_projectId = "00000000-0000-0000-0000-000000000000";

    if (m_offlineMode)
        {
        // Fake use m_offlineMode in order to avoid unused variable warning 
        // that is treated as error on clang compilers for iOS.
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus ClientImpl::StartApplication()
    {
    if (Utf8String::IsNullOrEmpty(m_userInfo.username.c_str()))
        return LicenseStatus::Error;

    if (m_clientInfo == nullptr)
        return LicenseStatus::Error;

    if (m_authProvider == nullptr)
        return LicenseStatus::Error;

    if (BeFileName::IsNullOrEmpty(m_dbPath))
        return LicenseStatus::Error;

    if (Utf8String::IsNullOrEmpty(m_correlationId.c_str()))
        return LicenseStatus::Error;

    if (m_timeRetriever == nullptr)
        return LicenseStatus::Error;

    if (m_delayedExecutor == nullptr)
        return LicenseStatus::Error;

    if (m_usageDb == nullptr)
        return LicenseStatus::Error;

    if (SUCCESS != m_usageDb->OpenOrCreate(m_dbPath))
        return LicenseStatus::Error;    

    m_policy = GetPolicyToken();

    if (m_policy == nullptr)
        return LicenseStatus::Error;

    // Get product status
    LicenseStatus licStatus = GetProductStatus();

    if ((LicenseStatus::Ok == licStatus) ||
        (LicenseStatus::Offline == licStatus) ||
        (LicenseStatus::Trial == licStatus))
        {
        if (ERROR == RecordUsage())
            return LicenseStatus::Error;

        // Begin licensing heartbeat
        int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
        UsageHeartbeat(currentTimeUnixMs);
        LogPostingHeartbeat(currentTimeUnixMs);
        PolicyHeartbeat(currentTimeUnixMs);
        }

    // This is only a logging example
    LOG.trace("StartApplication");

    return licStatus;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::UsageHeartbeat(int64_t currentTime)
    {
    LOG.trace("UsageHeartbeat");

    m_lastRunningUsageheartbeatStartTime = currentTime;
    int heartbeatInterval = m_policy->GetHeartbeatInterval(m_clientInfo->GetApplicationProductId(), m_featureString);

    m_delayedExecutor->Delayed(heartbeatInterval).then([this, currentTime]
        {
        if (currentTime != m_lastRunningUsageheartbeatStartTime)
            return;

        int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();

        RecordUsage();

        UsageHeartbeat(currentTimeUnixMs);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::PolicyHeartbeat(int64_t currentTime)
    {
    LOG.trace("PolicyHeartbeat");

    m_lastRunningPolicyheartbeatStartTime = currentTime;
    int policyInterval = m_policy->GetPolicyInterval(m_clientInfo->GetApplicationProductId(), m_featureString);

    m_delayedExecutor->Delayed(policyInterval).then([this, currentTime]
        {
        if (currentTime != m_lastRunningPolicyheartbeatStartTime)
            return;

        int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();

        auto policyToken = GetPolicyToken();
		// check if offline grace period should start
		if (policyToken == nullptr)
			{
			if (!HasOfflineGracePeriodStarted())
				m_usageDb->SetOfflineGracePeriodStart(DateHelper::GetCurrentTime());
			}
		// otherwise, check if offline grace period should be reset
		else
		{
			if (HasOfflineGracePeriodStarted())
				m_usageDb->ResetOfflineGracePeriod();
		}
        CleanUpPolicies();

        PolicyHeartbeat(currentTimeUnixMs);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::LogPostingHeartbeat(int64_t currentTime)
    {
    LOG.trace("LogPostingHeartbeat");
    
    m_lastRunningLogPostingheartbeatStartTime = currentTime;
    int logsPostingInterval = m_policy->GetTimeToKeepUnSentLogs(m_clientInfo->GetApplicationProductId(), m_featureString);

    m_delayedExecutor->Delayed(logsPostingInterval).then([this, currentTime]
        {
        if (currentTime != m_lastRunningLogPostingheartbeatStartTime)
            return;

        int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();

        PostUsageLogs();

        LogPostingHeartbeat(currentTimeUnixMs);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::StopApplication()
    {
    LOG.trace("StopApplication");

    m_lastRunningPolicyheartbeatStartTime = 0;      // This will stop Policy heartbeat
    m_lastRunningUsageheartbeatStartTime = 0;       // This will stop Usage heartbeat
    m_lastRunningLogPostingheartbeatStartTime = 0;  // This will stop log posting heartbeat

    if (m_usageDb->GetUsageRecordCount() > 0)
        PostUsageLogs();

    if (m_usageDb->GetFeatureRecordCount() > 0)
        PostFeatureLogs();

    m_usageDb->Close();

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::MarkFeature(Utf8String featureId, FeatureUserDataMap* featureUserData)
    {
    LOG.trace("MarkFeature");

    if (!m_usageDb->IsDbOpen())
        {
        LOG.error("MarkFeature: Usage DB not open.");
        return ERROR;
        }

    if (m_policy == nullptr)
        {
        LOG.error("MarkFeature: StartApplication function not called.");
        return ERROR;
        }

    LicenseStatus licStatus = GetProductStatus();

    if ((LicenseStatus::Ok != licStatus) &&
        (LicenseStatus::Offline != licStatus) &&
        (LicenseStatus::Trial != licStatus))
        {
        LOG.errorv("MarkFeature: Licenses status of product %s is %d. Feature %s cannot be tracked",
                   m_clientInfo->GetApplicationProductId().c_str(),
                   licStatus,
                   featureId.c_str());
        return ERROR;
        }

    Utf8String versionString;
    GenerateSID gsid;

    // Create feature record
    Utf8String eventTimeZ = DateTime::GetCurrentTimeUtc().ToString();

    Utf8StringVector featureUserDataKeys;
    Utf8String userDataString;

    if (featureUserData->GetKeys(featureUserDataKeys) > 0)
        {
        Utf8String featureUserDataValue;
        Utf8String userData;
        for (Utf8String featureUserDataKey : featureUserDataKeys)
            {
            featureUserData->GetValue(featureUserDataKey.c_str(), featureUserDataValue);
            featureUserDataValue.ReplaceAll(",", "");
            userData.Sprintf("%s^%s#", featureUserDataKey.c_str(), featureUserDataValue.c_str());
            userDataString.append(userData.c_str());
            }
        }

    versionString.Sprintf("%d%.4d%.4d%.4d", m_clientInfo->GetApplicationVersion().GetMajor(), m_clientInfo->GetApplicationVersion().GetMinor(),
                          m_clientInfo->GetApplicationVersion().GetSub1(), m_clientInfo->GetApplicationVersion().GetSub2());

    LOG.debugv("RecordFeature - UsageLogEntry data: {ultimateId:%ld,principalId:%s,userId:%s,machineName:%s,"
               "machineSID:%s,userName:%s,userSID:%s,policyId:%s,securableId:%s,productId:%s,featureString:%s,"
               "version:%s,projectId:%s,correlationId:%s,eventTimeZ:%s,schemaVer:%f,source:%s,country:%s,usageType:%s,"
               "featureId:%s, startTime:%s, endTime:%s, featureUserData:%s}",
               m_policy->GetUltimateSAPId(),
               m_policy->GetPrincipalId().c_str(),
               m_policy->GetAppliesToUserId().c_str(),
               m_clientInfo->GetDeviceId().c_str(),
               gsid.GetMachineSID(m_clientInfo->GetDeviceId()).c_str(),
               m_userInfo.username.c_str(),
               gsid.GetUserSID(m_userInfo.username, m_clientInfo->GetDeviceId()).c_str(),
               m_policy->GetPolicyId().c_str(),
               m_policy->GetSecurableId().c_str(),
               atoi(m_clientInfo->GetApplicationProductId().c_str()),
               m_featureString.c_str(),
               atoll(versionString.c_str()),
               m_projectId.c_str(),
               m_correlationId.c_str(),
               eventTimeZ.c_str(),
               LICENSE_CLIENT_SCHEMA_VERSION,
               GetLoggingPostSource(LogPostingSource::RealTime).c_str(),
               m_policy->GetCountry().c_str(),
               m_policy->GetUsageType().c_str(),
               featureId.c_str(),
               eventTimeZ.c_str(),
               eventTimeZ.c_str(),
               userDataString.c_str());

    if (SUCCESS != m_usageDb->RecordFeature(m_policy->GetUltimateSAPId(),
                                            m_policy->GetPrincipalId(),
                                            m_policy->GetAppliesToUserId(),
                                            m_clientInfo->GetDeviceId(),
                                            gsid.GetMachineSID(m_clientInfo->GetDeviceId()),
                                            m_userInfo.username,
                                            gsid.GetUserSID(m_userInfo.username, m_clientInfo->GetDeviceId()),
                                            m_policy->GetPolicyId(),
                                            m_policy->GetSecurableId(),
                                            atoi(m_clientInfo->GetApplicationProductId().c_str()),
                                            m_featureString,
                                            atoll(versionString.c_str()),
                                            m_projectId,
                                            m_correlationId,
                                            eventTimeZ,
                                            LICENSE_CLIENT_SCHEMA_VERSION,
                                            GetLoggingPostSource(LogPostingSource::RealTime),
                                            m_policy->GetCountry(),
                                            m_policy->GetUsageType(),
                                            featureId,
                                            eventTimeZ,
                                            eventTimeZ,
                                            userDataString))
        {
        LOG.error("RecordFeature - ERROR: Feature usage failed to be recorded.");
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::RecordUsage()
    {
    LOG.trace("RecordUsage");

    if (!m_usageDb->IsDbOpen())
        return ERROR;

    Utf8String versionString;
    GenerateSID gsid;

    // Create usage record
    Utf8String eventTimeZ = DateTime::GetCurrentTimeUtc().ToString();

    versionString.Sprintf("%d%.4d%.4d%.4d", m_clientInfo->GetApplicationVersion().GetMajor(), m_clientInfo->GetApplicationVersion().GetMinor(),
                          m_clientInfo->GetApplicationVersion().GetSub1(), m_clientInfo->GetApplicationVersion().GetSub2());

    LOG.debugv("RecordUsage - UsageLogEntry data: {ultimateId:%ld,principalId:%s,userId:%s,machineName:%s,machineSID:%s,userName:%s,userSID:%s,"
               "policyId:%s,securableId:%s,productId:%s,featureString:%s,version:%s,projectId:%s,correlationId:%s,eventTimeZ:%s,schemaVer:%f,"
               "source:%s,country:%s,usageType:%s}",
               m_policy->GetUltimateSAPId(),
               m_policy->GetPrincipalId().c_str(),
               m_policy->GetAppliesToUserId().c_str(),
               m_clientInfo->GetDeviceId().c_str(),
               gsid.GetMachineSID(m_clientInfo->GetDeviceId()).c_str(),
               m_userInfo.username.c_str(),
               gsid.GetUserSID(m_userInfo.username, m_clientInfo->GetDeviceId()).c_str(),
               m_policy->GetPolicyId().c_str(),
               m_policy->GetSecurableId().c_str(),
               atoi(m_clientInfo->GetApplicationProductId().c_str()),
               m_featureString.c_str(),
               atoll(versionString.c_str()),
               m_projectId.c_str(),
               m_correlationId.c_str(),
               eventTimeZ.c_str(),
               LICENSE_CLIENT_SCHEMA_VERSION,
               GetLoggingPostSource(LogPostingSource::RealTime).c_str(),
               m_policy->GetCountry().c_str(),
               m_policy->GetUsageType().c_str());

    if (SUCCESS != m_usageDb->RecordUsage(m_policy->GetUltimateSAPId(),
                                          m_policy->GetPrincipalId(),
                                          m_policy->GetAppliesToUserId(),
                                          m_clientInfo->GetDeviceId(),
                                          gsid.GetMachineSID(m_clientInfo->GetDeviceId()),
                                          m_userInfo.username,
                                          gsid.GetUserSID(m_userInfo.username, m_clientInfo->GetDeviceId()),
                                          m_policy->GetPolicyId(),
                                          m_policy->GetSecurableId(),
                                          atoi(m_clientInfo->GetApplicationProductId().c_str()),
                                          m_featureString,
                                          atoll(versionString.c_str()),
                                          m_projectId,
                                          m_correlationId,
                                          eventTimeZ,
                                          LICENSE_CLIENT_SCHEMA_VERSION,
                                          GetLoggingPostSource(LogPostingSource::RealTime),
                                          m_policy->GetCountry(),
                                          m_policy->GetUsageType()))
        {
        LOG.error("RecordUsage - ERROR: Usage failed to be recorded.");
        return ERROR;
        }
    
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy> ClientImpl::GetPolicyToken()
    {
    auto policy = GetPolicy().get();
    if (policy != nullptr)
        {
        StorePolicyInUsageDb(policy);
		DeleteAllOtherUserPolicies(policy);
        }
    
    return policy;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::PostUsageLogs()
    {    
    LOG.trace("PostUsageLogs");

    Utf8String fileName;
    bvector<WString> logFiles;
    BeFileName usageLogPath(m_dbPath.GetDirectoryName());

    fileName.Sprintf("LicUsageLog.%s.csv", BeGuid(true).ToString().c_str());
    
    usageLogPath.AppendToPath(BeFileName(fileName));

    if (SUCCESS != m_usageDb->WriteUsageToCSVFile(usageLogPath))
        {
        LOG.error("PostUsageLogs - ERROR: Unable to write usage records to usage log.");
        return ERROR;
        }

    m_usageDb->CleanUpUsages();

    Utf8String ultimateId;
    ultimateId.Sprintf("%ld", m_policy->GetUltimateSAPId());  

    LogFileHelper lfh;
    logFiles = lfh.GetLogFiles(Utf8String(usageLogPath.GetDirectoryName()));

    if (!logFiles.empty())
        {
        for (auto const& logFile : logFiles)
            {
            SendUsage(BeFileName(logFile), ultimateId);
            }
        }
    
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::PostFeatureLogs()
    {
    LOG.trace("PostFeatureLogs");

    Utf8String fileName;
    bvector<WString> logFiles;
    BeFileName featureLogPath(m_dbPath.GetDirectoryName());

    fileName.Sprintf("FeatureLog.%s.csv", BeGuid(true).ToString().c_str());

    featureLogPath.AppendToPath(BeFileName(fileName));

    if (SUCCESS != m_usageDb->WriteFeatureToCSVFile(featureLogPath))
        {
        LOG.error("PostFeatureLogs - ERROR: Unable to write feature usage records to features log.");
        return ERROR;
        }

    m_usageDb->CleanUpFeatures();

    Utf8String ultimateId;
    ultimateId.Sprintf("%ld", m_policy->GetUltimateSAPId());

    LogFileHelper lfh;
    logFiles = lfh.GetLogFiles(Utf8String(featureLogPath.GetDirectoryName()));

    if (!logFiles.empty())
        {
        for (auto const& logFile : logFiles)
            {
            SendFeatures(BeFileName(logFile), ultimateId);
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> ClientImpl::SendUsage(BeFileNameCR usageCSV, Utf8StringCR ultId)
    {
    LOG.trace("SendUsage");

    auto url = UrlProvider::Urls::UsageLoggingServicesLocation.Get();
    url += Utf8PrintfString("/usageLog?ultId=%s&prdId=%s&lng=%s", ultId.c_str(), m_clientInfo->GetApplicationProductId().c_str(),
                            m_clientInfo->GetLanguage().c_str());

    LOG.debugv("SendUsage - UsageLoggingServiceLocation: %s", url.c_str());

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
        uploadRequest.SetRequestBody(HttpFileBody::Create(usageCSV));
        return uploadRequest.Perform().then([=](Response response)
            {
            if (!response.IsSuccess())
                {
                LOG.errorv("Unable to post %s - %s", usageCSV.c_str(), HttpError(response).GetMessage().c_str());
                throw HttpError(response);
                }

            BeFileName(usageCSV).BeDeleteFile();

            return folly::makeFuture();
            });
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> ClientImpl::SendFeatures(BeFileNameCR featureCSV, Utf8StringCR ultId)
    {
    LOG.trace("SendFeatures");

    auto url = UrlProvider::Urls::UsageLoggingServicesLocation.Get();
    url += Utf8PrintfString("/featureLog?ultId=%s&prdId=%s&lng=%s", ultId.c_str(), m_clientInfo->GetApplicationProductId().c_str(),
                            m_clientInfo->GetLanguage().c_str());

    LOG.debugv("SendFeatures - UsageLoggingServiceLocation: %s", url.c_str());

    HttpClient client(nullptr, m_httpHandler);

    return client.CreateGetRequest(url).Perform().then(
        [=] (Response response)
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
        uploadRequest.SetRequestBody(HttpFileBody::Create(featureCSV));
        return uploadRequest.Perform().then([=] (Response response)
            {
            if (!response.IsSuccess())
                {
                LOG.errorv("Unable to post %s - %s", featureCSV.c_str(), HttpError(response).GetMessage().c_str());
                throw HttpError(response);
                }

            BeFileName(featureCSV).BeDeleteFile();

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
folly::Future<std::shared_ptr<Policy>> ClientImpl::GetPolicy()
    {
    return folly::collectAll(GetCertificate(), PerformGetPolicyRequest()).then(
        [] (const std::tuple<folly::Try<Utf8String>, folly::Try<Utf8String>>& tup)
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
void ClientImpl::CleanUpPolicies()
	{
	for (auto policy : GetPolicies())
		{
		// if policy is not valid, remove it from the database; no point in keeping it
		if (!policy->IsValid())
			{
			m_usageDb->DeletePolicyFile(policy->GetPolicyId());
			}
		}
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy> ClientImpl::GetPolicyWithId(Utf8StringCR policyId)
	{
	auto jsonpolicy = m_usageDb->GetPolicyFile(policyId);
	if (jsonpolicy == Json::Value::GetNull())
		return nullptr;
	return Policy::Create(jsonpolicy);
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy>> ClientImpl::GetPolicies()
	{
	std::list<std::shared_ptr<Policy>> policyList;
	auto jsonpolicies = m_usageDb->GetPolicyFiles();
	for (auto json : jsonpolicies)
		{
		auto policy = Policy::Create(json);
		policyList.push_back(policy);
		}
	return policyList;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy>> ClientImpl::GetUserPolicies()
	{
	std::list<std::shared_ptr<Policy>> policyList;
	auto jsonpolicies = m_usageDb->GetPolicyFiles(m_userInfo.userId);
	for (auto json : jsonpolicies)
		{
		auto policy = Policy::Create(json);
		if (!policy->IsValid())
			continue;
		if (policy->GetAppliesToUserId().Equals(m_userInfo.userId))
			{
			policyList.push_back(policy);
			}
		}
	return policyList;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy> ClientImpl::SearchForPolicy(Utf8String requestedProductId)
	{
	std::shared_ptr<Policy> matchingPolicy = nullptr;
	// get all of user's policies
	auto policies = GetUserPolicies();
	// find policy that contains appropriate productId and featureString
	Utf8String productId;
	if (requestedProductId.Equals(""))
		productId = m_clientInfo->GetApplicationProductId();
	else
		productId = requestedProductId;

	for (auto policy : policies)
		{
		// policy assumed to be valid due to IsValid check in GetUserPolicies()
		// look for a securable that matches productId and featureString
		for (auto securable : policy->GetSecurableData())
			{
			if (Utf8String(std::to_string(securable->GetProductId()).c_str()).Equals(productId) &&
				securable->GetFeatureString().Equals(m_featureString))
				{
				// if matches, return this policy
				return policy;
				}
			}
		}

	return matchingPolicy;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::DeleteAllOtherUserPolicies(std::shared_ptr<Policy> policy)
	{
	m_usageDb->DeleteAllOtherUserPolicyFiles(policy->GetPolicyId(),
		policy->GetUserData()->GetUserId());
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::StorePolicyInUsageDb(std::shared_ptr<Policy> policy)
	{
	auto expiration = policy->GetPolicyExpiresOn();
	auto lastUpdate = policy->GetRequestData()->GetClientDateTime();
	m_usageDb->AddOrUpdatePolicyFile(policy->GetPolicyId(),
		policy->GetAppliesToUserId(),
		expiration,
		lastUpdate,
		policy->GetJson());
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClientImpl::HasOfflineGracePeriodStarted()
	{
	auto graceStartString = m_usageDb->GetOfflineGracePeriodStart();
	return graceStartString != "";
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ClientImpl::GetDaysLeftInOfflineGracePeriod(std::shared_ptr<Policy> policy, Utf8String productId, Utf8String featureString)
	{
	Utf8String graceStartString = m_usageDb->GetOfflineGracePeriodStart();
	if (graceStartString == "")
	{
		return 0;
	}
	// check if online usage is allowed; 
	auto offlineDurationDays = policy->GetOfflineDuration(productId, featureString);
	auto gracePeriodEndTime = DateHelper::AddDaysToTime(graceStartString, offlineDurationDays);
	auto daysLeft = DateHelper::GetDaysLeftUntilTime(gracePeriodEndTime);
	return daysLeft;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus ClientImpl::GetProductStatus(int requestedProductId)
	{
	// productId to look for; allow custom product to be searched for (for testing purposes)
	Utf8String productId;
	if (requestedProductId < 0)
		productId = m_clientInfo->GetApplicationProductId();
	else
		productId = Utf8String(std::to_string(requestedProductId).c_str());
	// get valid policy for user
	auto policy = SearchForPolicy(productId);
	// if null, NotEntitled
	if (policy == nullptr)
		{
		return LicenseStatus::NotEntitled;
		}
	// get PolicyStatus
	auto policyStatus = policy->GetPolicyStatus();
	// if not valid, return LicenseStatus::DisabledByPolicy
	if (policyStatus != Policy::PolicyStatus::Valid)
		{
		return LicenseStatus::DisabledByPolicy;
		}
	// get GetProductLicenseStatus
	auto productStatus = policy->GetProductStatus(productId, m_featureString);
	// if prodStatus is TrialExpired, return LicenseStatus::Expired
	if (productStatus == Policy::ProductStatus::TrialExpired)
		{
		return LicenseStatus::Expired;
		}
	// if prodStatus is Denied, return LicenseStatus::AccessDenied
	if (productStatus == Policy::ProductStatus::Denied)
		{
		return LicenseStatus::AccessDenied;
		}
	// if prodStatus is NoLicense, return LicenseStatus::NotEntitled
	if (productStatus == Policy::ProductStatus::NoLicense)
		{
		return LicenseStatus::NotEntitled;
		}
	// if prodStatus is Allowed
	if (productStatus == Policy::ProductStatus::Allowed)
		{
		// if (IsTrial) return LicenseStatus::Trial
		if (policy->IsTrial(productId, m_featureString))
			{
			return LicenseStatus::Trial;
			}
		// if (hasOfflineGracePeriodStarted && daysLeftInOfflineGracePeriod > 0
		if (HasOfflineGracePeriodStarted())
			{
			// if not allowed to use offline, return LicenseStatus::DisabledByPolicy
			if (!policy->IsAllowedOfflineUsage(productId, m_featureString))
				{
				return LicenseStatus::DisabledByPolicy;
				}
			// if still has time left for offline usage, return LicenseStatus::Offline
			if (GetDaysLeftInOfflineGracePeriod(policy, productId, m_featureString) > 0)
				{
				return LicenseStatus::Offline;
				}
			// else offline grace period has expired, return LicenseStatus::Expired
			else
				{
				return LicenseStatus::Expired;
				}
			}
		// else return LicenseStatus::Ok
		return LicenseStatus::Ok;
		}
	// return DisabledByPolicy
	return LicenseStatus::DisabledByPolicy;
	}
