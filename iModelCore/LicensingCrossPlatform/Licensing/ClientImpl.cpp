/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/ClientImpl.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientImpl.h"
#include "GenerateSID.h"
#include "..\PublicAPI\Licensing\Utils\LogFileHelper.h"
#include "Logging.h"
#include "PolicyToken.h"
#include "UsageDb.h"

#include <BeHttp/HttpError.h>
#include <WebServices/Configuration/UrlProvider.h>

#include <filesystem>

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
    }
   
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus ClientImpl::StartApplication()
    {
    if (SUCCESS != m_usageDb->OpenOrCreate(m_dbPath))
        return LicenseStatus::Error;

    // Get policy token
    m_policyToken = GetPolicyToken();

    // Get product status
    LicenseStatus licStatus = GetProductStatus();

    if (LicenseStatus::Ok == licStatus)
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
    m_delayedExecutor->Delayed(USAGE_HEARTBEAT_INTERVAL_MS).then([this, currentTime]
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
    m_delayedExecutor->Delayed(POLICY_HEARTBEAT_INTERVAL_MS).then([this, currentTime]
        {
        if (currentTime != m_lastRunningPolicyheartbeatStartTime)
            return;

        int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();

        GetPolicyToken();
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
    m_delayedExecutor->Delayed(LOG_POSTING_HEARTBEAT_INTERVAL_MS).then([this, currentTime]
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

    m_usageDb->Close();

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

    LOG.debugv("RecordUsage - UsageLogEntry data: {ultimateId:%ld,principalId:%s,userId:%s,machineName:%s,machineSID:%s,userName:%s,userSID:%s,policyId:%s,securableId:%s,productId:%s,featureString:%s,version:%s,projectId:%s,correlationId:%s,eventTimeZ:%s,schemaVer:%f,source:%s,country:%s,usageType:%s}",
               m_policyToken->GetUltimateSAPId(),
               m_policyToken->GetPrincipalId(),
               m_policyToken->GetUserId(),
               m_clientInfo->GetDeviceId(),
               gsid.GetMachineSID(m_clientInfo->GetDeviceId()),
               m_userInfo.username,
               gsid.GetUserSID(m_userInfo.username, m_clientInfo->GetDeviceId()),
               m_policyToken->GetPolicyId(),
               m_policyToken->GetSecurableId(),
               atoi(m_clientInfo->GetApplicationProductId().c_str()),
               m_featureString,
               atoll(versionString.c_str()),
               m_projectId,
               m_correlationId,
               eventTimeZ,
               LICENSE_CLIENT_SCHEMA_VERSION,
               GetLoggingPostSource(LogPostingSource::RealTime),
               m_policyToken->GetCountry(),
               m_policyToken->GetUsageType());

    if (SUCCESS != m_usageDb->RecordUsage(m_policyToken->GetUltimateSAPId(),
                                          m_policyToken->GetPrincipalId(),
                                          m_policyToken->GetUserId(),
                                          m_clientInfo->GetDeviceId(),
                                          gsid.GetMachineSID(m_clientInfo->GetDeviceId()),
                                          m_userInfo.username,
                                          gsid.GetUserSID(m_userInfo.username, m_clientInfo->GetDeviceId()),
                                          m_policyToken->GetPolicyId(),
                                          m_policyToken->GetSecurableId(),
                                          atoi(m_clientInfo->GetApplicationProductId().c_str()),
                                          m_featureString,
                                          atoll(versionString.c_str()),
                                          m_projectId,
                                          m_correlationId,
                                          eventTimeZ,
                                          LICENSE_CLIENT_SCHEMA_VERSION,
                                          GetLoggingPostSource(LogPostingSource::RealTime),
                                          m_policyToken->GetCountry(),
                                          m_policyToken->GetUsageType()))
        {
        LOG.error("RecordUsage - ERROR: Usage failed to be recorded.");
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<PolicyToken> ClientImpl::GetPolicyToken()
    {
    auto policyToken = GetPolicy().get();

    if (policyToken != nullptr)
        {
        StorePolicyTokenInUsageDb(policyToken);
        }
    
    return policyToken;
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

    fileName.Sprintf("LicUsageLog.%s.csv", BeGuid(true).ToString());
    
    usageLogPath.AppendToPath(BeFileName(fileName));

    if (SUCCESS != m_usageDb->WriteUsageToCSVFile(usageLogPath))
        {
        LOG.error("PostUsageLogs - ERROR: Unable to write usage records to usage log.");
        return ERROR;
        }

    Utf8String ultimateId;
    ultimateId.Sprintf("%ld", m_policyToken->GetUltimateSAPId());  

    LogFileHelper lfh;
    logFiles = lfh.GetLogFiles(Utf8String(usageLogPath.GetDirectoryName()));

    if (!logFiles.empty())
        {
        for (auto const& logFile : logFiles)
            {
            SendUsage(BeFileName(logFile), ultimateId);
            }
        }
    
    m_usageDb->CleanUpUsages();

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

    LOG.debugv("SendUsage - UsageLoggingServiceLocation: %s", url);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::CleanUpPolicies()
	{
	for (auto policy : GetPolicies())
		{
		// if policy is not valid, remove it from the database; no point in keeping it
		if (!PolicyHelper::IsValid(policy))
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
	for (auto policy : GetPolicies())
		{
		if (policy->GetPolicyId().Equals(policyId))
			{
			return policy;
			}
		}
	// if nothing found, return nullptr
	return nullptr;
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
	// get all policies
	auto allPolicies = GetPolicies();
	// filter out policies that don't match userId
	for (auto policy : allPolicies)
		{
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
		// check if policy is valid
		if (!PolicyHelper::IsValid(policy))
			continue;
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

void ClientImpl::StorePolicyTokenInUsageDb(std::shared_ptr<PolicyToken> policyToken)
	{
	m_usageDb->AddOrUpdatePolicyFile(policyToken->GetPolicyId(),
		policyToken->GetExpirationDate(),
		policyToken->GetLastUpdateTime(),
		policyToken->GetPolicyFile());
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
	auto policyStatus = PolicyHelper::GetPolicyStatus(policy);
	// if not valid, return LicenseStatus::DisabledByPolicy
	if (policyStatus != PolicyHelper::PolicyStatus::Valid)
		{
		return LicenseStatus::DisabledByPolicy;
		}
	// get GetProductLicenseStatus
	auto productStatus = PolicyHelper::GetProductStatus(policy, productId, m_featureString);
	// if prodStatus is TrialExpired, return LicenseStatus::Expired
	if (productStatus == PolicyHelper::ProductStatus::TrialExpired)
		{
		return LicenseStatus::Expired;
		}
	// if prodStatus is Denied, return LicenseStatus::AccessDenied
	if (productStatus == PolicyHelper::ProductStatus::Denied)
		{
		return LicenseStatus::AccessDenied;
		}
	// if prodStatus is NoLicense, return LicenseStatus::Expired
	if (productStatus == PolicyHelper::ProductStatus::NoLicense)
		{
		return LicenseStatus::NotEntitled;
		}
	// if prodStatus is Allowed
	if (productStatus == PolicyHelper::ProductStatus::Allowed)
		{
		//     if (IsTrial) return LicenseStatus::Trial
		if (PolicyHelper::IsTrial(policy, productId, m_featureString))
			{
			return LicenseStatus::Trial;
			}
		//     hasOfflineGracePeriodStarted = HasOfflineGracePeriodStarted()
		//     daysLeftInOfflineGracePeriod = GetDaysLeftInOfflineGracePeriod()
		//     if (hasOfflineGracePeriodStarted && daysLeftInOfflineGracePeriod > 0
		//         return LicenseStatus::Offline
		//     else return LicenseStatus::Ok
		return LicenseStatus::Ok;
		}
	// return DisabledByPolicy
	return LicenseStatus::DisabledByPolicy;
	}
