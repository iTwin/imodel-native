/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ClientImpl.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientImpl.h"
#include "GenerateSID.h"
#include <Licensing/Utils/LogFileHelper.h>
#include <Licensing/Utils/UsageJsonHelper.h>
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
IBuddiProviderPtr buddiProvider,
IPolicyProviderPtr policyProvider,
IUlasProviderPtr ulasProvider,
Utf8StringCR projectId,
Utf8StringCR featureString,
IHttpHandlerPtr httpHandler
) :
m_userInfo(userInfo),
m_clientInfo(clientInfo),
m_authProvider(authenticationProvider),
m_dbPath(dbPath),
m_featureString(featureString),
m_buddiProvider(buddiProvider),
m_policyProvider(policyProvider),
m_ulasProvider(ulasProvider),
m_httpHandler(httpHandler)
    {
    m_usageDb = std::make_unique<UsageDb>();
    m_correlationId = BeGuid(true).ToString();
    m_timeRetriever = TimeRetriever::Get();
    m_delayedExecutor = DelayedExecutor::Get();

    if (Utf8String::IsNullOrEmpty(projectId.c_str()))
        m_projectId = "00000000-0000-0000-0000-000000000000";

    m_offlineMode = offlineMode;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus ClientImpl::StartApplication()
    {
    LOG.debug("ClientImpl::StartApplication");

    if (Utf8String::IsNullOrEmpty(m_userInfo.username.c_str()))
        {
        LOG.error("ClientImpl::StartApplication ERROR - Username string is null or empty.");
        return LicenseStatus::Error;
        }

    if (m_clientInfo == nullptr)
        {
        LOG.error("ClientImpl::StartApplication ERROR - Client Information object is null.");
        return LicenseStatus::Error;
        }

    if (m_authProvider == nullptr)
        {
        LOG.error("ClientImpl::StartApplication ERROR - Authorization Provicer object is null.");
        return LicenseStatus::Error;
        }

    if (BeFileName::IsNullOrEmpty(m_dbPath))
        {
        LOG.error("ClientImpl::StartApplication ERROR - Database path string is null or empty.");
        return LicenseStatus::Error;
        }

    if (Utf8String::IsNullOrEmpty(m_correlationId.c_str()))
        {
        LOG.error("ClientImpl::StartApplication ERROR - Correlation ID (Usage ID) string is null or empty.");
        return LicenseStatus::Error;
        }

    if (m_timeRetriever == nullptr)
        {
        LOG.error("ClientImpl::StartApplication ERROR - Time retriever object is null.");
        return LicenseStatus::Error;
        }

    if (m_delayedExecutor == nullptr)
        {
        LOG.error("StartApplication ERROR - Delayed executor object is null.");
        return LicenseStatus::Error;
        }

    if (m_usageDb == nullptr)
        {
        LOG.error("ClientImpl::StartApplication ERROR - Database object is null.");
        return LicenseStatus::Error;
        }

    if (SUCCESS != m_usageDb->OpenOrCreate(m_dbPath))
        {
        LOG.error("ClientImpl::StartApplication ERROR - Database creation failed.");
        return LicenseStatus::Error;
        }

    m_policy = GetPolicyToken();

    if (m_policy == nullptr)
        {
        LOG.error("ClientImpl::StartApplication ERROR - Policy token object is null.");
        return LicenseStatus::Error;
        }

    // Get product status
    LicenseStatus licStatus = GetProductStatus();

    if ((LicenseStatus::Ok == licStatus) ||
        (LicenseStatus::Offline == licStatus) ||
        (LicenseStatus::Trial == licStatus))
        {
        int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();

        UsageHeartbeat(currentTimeUnixMs);
        LogPostingHeartbeat(currentTimeUnixMs);
        PolicyHeartbeat(currentTimeUnixMs);
        }
    else
        {
        LOG.errorv("ClientImpl::StartApplication - LicenseStatus Returned: %d", licStatus);
        }

    return licStatus;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::StopApplication()
    {
    LOG.debug("ClientImpl::StopApplication");

    StopUsageHeartbeat();
    StopLogPostingHeartbeat();
    StopPolicyHeartbeat();

    m_usageDb->Close();

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::UsageHeartbeat(int64_t currentTime)
    {
    LOG.debug("ClientImpl::UsageHeartbeat");

    if (m_startUsageHeartbeat)
        {
        RecordUsage();
        m_startUsageHeartbeat = false;
        m_lastRunningUsageheartbeatStartTime = currentTime;
        }

    m_delayedExecutor->Delayed(HEARTBEAT_THREAD_DELAY_MS).then([this, currentTime]
        {
        int64_t heartbeatInterval = m_policy->GetHeartbeatInterval(m_clientInfo->GetApplicationProductId(), m_featureString);

        int64_t time_elapsed = currentTime - m_lastRunningUsageheartbeatStartTime;

        if (time_elapsed >= heartbeatInterval)
            {
            RecordUsage();
            m_lastRunningUsageheartbeatStartTime = currentTime;
            }

        if (!m_stopUsageHeartbeat)
            {
            int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
            UsageHeartbeat(currentTimeUnixMs);
            }
        else
            {
            m_usageHeartbeatStopped = true;
            }
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::StopUsageHeartbeat()
    {
    LOG.debug("ClientImpl::StopUsageHeartbeat");

    if (m_lastRunningUsageheartbeatStartTime == 0)
        return;

    m_stopUsageHeartbeat = true;

    while (!m_usageHeartbeatStopped)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

    // Reset
    m_lastRunningUsageheartbeatStartTime = 0;
    m_startUsageHeartbeat = true;
    m_stopUsageHeartbeat = false;
    m_usageHeartbeatStopped = false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::PolicyHeartbeat(int64_t currentTime)
    {
    LOG.debug("ClientImpl::PolicyHeartbeat");

    if (m_startPolicyHeartbeat)
        {
        m_lastRunningPolicyheartbeatStartTime = currentTime;
        m_startPolicyHeartbeat = false;
        }

    m_delayedExecutor->Delayed(HEARTBEAT_THREAD_DELAY_MS).then([this, currentTime]
        {
        int64_t policyInterval = m_policy->GetPolicyInterval(m_clientInfo->GetApplicationProductId(), m_featureString);

        int64_t time_elapsed = currentTime - m_lastRunningPolicyheartbeatStartTime;

        if (time_elapsed >= policyInterval)
            {
            auto policyToken = GetPolicyToken();
            // check if offline grace period should start
            if (policyToken == nullptr)
                {
                m_policy = policyToken;
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
            m_lastRunningPolicyheartbeatStartTime = currentTime;
            }

        if (!m_stopPolicyHeartbeat)
            {
            int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
            PolicyHeartbeat(currentTimeUnixMs);
            }
        else
            {
            m_policyHeartbeatStopped = true;
            }
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::StopPolicyHeartbeat()
    {
    LOG.debug("ClientImpl::StopPolicyHeartbeat");

    if (m_lastRunningPolicyheartbeatStartTime == 0)
        return;

    m_stopPolicyHeartbeat = true;

    while (!m_policyHeartbeatStopped)
        {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

    // Reset
    m_lastRunningPolicyheartbeatStartTime = 0;
    m_startPolicyHeartbeat = true;
    m_stopPolicyHeartbeat = false;
    m_policyHeartbeatStopped = false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::LogPostingHeartbeat(int64_t currentTime)
    {
    LOG.debug("ClientImpl::LogPostingHeartbeat");

    if (m_startLogPostingHeartbeat)
        {
        m_lastRunningLogPostingheartbeatStartTime = currentTime;
        m_startLogPostingHeartbeat = false;
        }

    m_delayedExecutor->Delayed(HEARTBEAT_THREAD_DELAY_MS).then([this, currentTime]
        {
        int64_t logsPostingInterval = m_policy->GetTimeToKeepUnSentLogs(m_clientInfo->GetApplicationProductId(), m_featureString);

        int64_t time_elapsed = currentTime - m_lastRunningLogPostingheartbeatStartTime;

        if (time_elapsed >= logsPostingInterval)
            {
            m_ulasProvider->PostUsageLogs(*m_usageDb, m_policy);
            m_ulasProvider->PostFeatureLogs(*m_usageDb, m_policy);
            m_lastRunningLogPostingheartbeatStartTime = currentTime;
            }

        if (!m_stopLogPostingHeartbeat)
            {
            int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
            LogPostingHeartbeat(currentTimeUnixMs);
            }
        else
            {
            if (m_usageDb->GetUsageRecordCount() > 0)
                m_ulasProvider->PostUsageLogs(*m_usageDb, m_policy);

            if (m_usageDb->GetFeatureRecordCount() > 0)
                m_ulasProvider->PostFeatureLogs(*m_usageDb, m_policy);

            m_logPostingHeartbeatStopped = true;
            }
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::StopLogPostingHeartbeat()
    {
    LOG.debug("ClientImpl::StopLogPostingHeartbeat");

    if (m_lastRunningLogPostingheartbeatStartTime == 0)
        return;

    m_stopLogPostingHeartbeat = true;

    while (!m_logPostingHeartbeatStopped)
        {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

    // Reset
    m_lastRunningLogPostingheartbeatStartTime = 0;
    m_startLogPostingHeartbeat = true;
    m_stopLogPostingHeartbeat = false;
    m_logPostingHeartbeatStopped = false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::MarkFeature(Utf8StringCR featureId, FeatureUserDataMap* featureUserData)
    {
    LOG.debug("ClientImpl::MarkFeature");

    if (Utf8String::IsNullOrEmpty(featureId.c_str()))
        {
        LOG.error("ClientImpl::MarkFeature: ERROR - featureId is null or empty.");
        return ERROR;
        }

    if (!m_usageDb->IsDbOpen())
        {
        LOG.error("ClientImpl::MarkFeature ERROR - Usage DB not open.");
        return ERROR;
        }

    if (m_policy == nullptr)
        {
        LOG.error("ClientImpl::MarkFeature ERROR - StartApplication function not called.");
        return ERROR;
        }

    LicenseStatus licStatus = GetProductStatus();

    if ((LicenseStatus::Ok != licStatus) &&
        (LicenseStatus::Offline != licStatus) &&
        (LicenseStatus::Trial != licStatus))
        {
        LOG.errorv("ClientImpl::MarkFeature ERROR - Licenses status of product %s is %d. Feature %s cannot be tracked",
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

    if (featureUserData != nullptr)
        {
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
        }

    versionString.Sprintf("%d%.4d%.4d%.4d", m_clientInfo->GetApplicationVersion().GetMajor(), m_clientInfo->GetApplicationVersion().GetMinor(),
                          m_clientInfo->GetApplicationVersion().GetSub1(), m_clientInfo->GetApplicationVersion().GetSub2());

    LOG.debugv("ClientImpl::MarkFeature - UsageLogEntry: ultimateId:%ld, principalId:%s, userId:%s, machineName:%s, machineSID:%s, userName:%s, userSID:%s, policyId:%s, securableId:%s, "
               "productId:%ld, featureString:%s, version:%ld, projectId:%s, correlationId:%s, eventTimeZ:%s, schemaVer:%f, source:%s, country:%s, usageType:%s, featureId:%s, startTime:%s, "
               "endTime:%s, featureUserData:%s",
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
               GetLoggingPostSource().c_str(),
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
                                            GetLoggingPostSource(),
                                            m_policy->GetCountry(),
                                            m_policy->GetUsageType(),
                                            featureId,
                                            eventTimeZ,
                                            eventTimeZ,
                                            userDataString))
        {
        LOG.error("ClientImpl::MarkFeature ERROR - Feature usage failed to be recorded.");
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::RecordUsage()
    {
    LOG.debug("ClientImpl::RecordUsage");

    if (!m_usageDb->IsDbOpen())
        {
        LOG.error("ClientImpl::RecordUsage ERROR - Usage DB is not open.");
        return ERROR;
        }

    Utf8String versionString;
    GenerateSID gsid;

    // Create usage record
    Utf8String eventTimeZ = DateTime::GetCurrentTimeUtc().ToString();

    versionString.Sprintf("%d%.4d%.4d%.4d", m_clientInfo->GetApplicationVersion().GetMajor(), m_clientInfo->GetApplicationVersion().GetMinor(),
                          m_clientInfo->GetApplicationVersion().GetSub1(), m_clientInfo->GetApplicationVersion().GetSub2());

    LOG.debugv("ClientImpl::RecordUsage - UsageLogEntry: ultimateId:%ld, principalId:%s, userId:%s, machineName:%s, machineSID:%s, userName:%s, userSID:%s, policyId:%s, securableId:%s, "
               "productId:%ld, featureString:%s, version:%ld, projectId:%s, correlationId:%s, eventTimeZ:%s, schemaVer:%f, source:%s, country:%s, usageType:%s",
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
               GetLoggingPostSource().c_str(),
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
                                          GetLoggingPostSource(),
                                          m_policy->GetCountry(),
                                          m_policy->GetUsageType()))
        {
        LOG.error("ClientImpl::RecordUsage - ERROR: Usage failed to be recorded.");
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy> ClientImpl::GetPolicyToken()
    {
    LOG.debug("ClientImpl::GetPolicyToken");

    auto policy = m_policyProvider->GetPolicy().get();
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
UsageDb& ClientImpl::GetUsageDb()
    {
    return *m_usageDb;
    }

///*--------------------------------------------------------------------------------------+
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<std::shared_ptr<Policy>> ClientImpl::GetPolicy()
    {
    return m_policyProvider->GetPolicy();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientImpl::GetLoggingPostSource() const
    {
    LOG.debug("ClientImpl::GetLoggingPostSource");

    // Check for checked out license
    auto qualifier = m_policy->GetQualifier("IsCheckedOut", m_clientInfo->GetApplicationProductId().c_str(), m_featureString);

    if (qualifier->GetValue().Equals("true"))
        return "Checkout";

    if (m_offlineMode)
        return "Offline";
    else
        return "RealTime";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::CleanUpPolicies()
	{
    LOG.debug("ClientImpl::CleanUpPolicies");

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
    LOG.debug("ClientImpl::GetPolicyWithId");

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
    LOG.debug("ClientImpl::GetPolicies");

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
    LOG.debug("ClientImpl::GetUserPolicies");

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
    LOG.debug("ClientImpl::SearchForPolicy");

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
    LOG.debug("ClientImpl::DeleteAllOtherUserPolicies");

	m_usageDb->DeleteAllOtherUserPolicyFiles(policy->GetPolicyId(),
		policy->GetUserData()->GetUserId());
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::StorePolicyInUsageDb(std::shared_ptr<Policy> policy)
	{
    LOG.debug("ClientImpl::StorePolicyInUsageDb");

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
    LOG.debug("ClientImpl::HasOfflineGracePeriodStarted");

	auto graceStartString = m_usageDb->GetOfflineGracePeriodStart();
	return graceStartString != "";
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ClientImpl::GetDaysLeftInOfflineGracePeriod(std::shared_ptr<Policy> policy, Utf8String productId, Utf8String featureString)
	{
    LOG.debug("ClientImpl::GetDaysLeftInOfflineGracePeriod");

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
    LOG.debug("ClientImpl::GetProductStatus");

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
