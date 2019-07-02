/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ClientImpl.h"
#include "GenerateSID.h"
#include <Licensing/Utils/LogFileHelper.h>
#include <Licensing/Utils/UsageJsonHelper.h>
#include "Logging.h"
#include "LicensingDb.h"
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
    ApplicationInfoPtr applicationInfo,
    BeFileNameCR dbPath,
    bool offlineMode,
    IPolicyProviderPtr policyProvider,
    IUlasProviderPtr ulasProvider,
    Utf8StringCR projectId,
    Utf8StringCR featureString,
    ILicensingDbPtr licensingDb
    ) :
    m_userInfo(userInfo),
    m_applicationInfo(applicationInfo),
    m_dbPath(dbPath),
    m_featureString(featureString),
    m_policyProvider(policyProvider),
    m_ulasProvider(ulasProvider),
    m_licensingDb(licensingDb)
    {
    if(m_licensingDb == nullptr) // either pass in a mock, or initialize here
        m_licensingDb = std::make_unique<LicensingDb>();

    m_correlationId = BeSQLite::BeGuid(true).ToString();
    m_timeRetriever = TimeRetriever::Get();

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

    if (m_applicationInfo == nullptr)
        {
        LOG.error("ClientImpl::StartApplication ERROR - Application Information object is null.");
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

    if (m_licensingDb == nullptr)
        {
        LOG.error("ClientImpl::StartApplication ERROR - Database object is null.");
        return LicenseStatus::Error;
        }

    if (SUCCESS != m_licensingDb->OpenOrCreate(m_dbPath))
        {
        LOG.error("ClientImpl::StartApplication ERROR - Database creation failed.");
        return LicenseStatus::Error;
        }

    // get policy from entitlements or database
    m_policy = GetPolicyToken();

    if (m_policy == nullptr)
        {
        LOG.error("ClientImpl::StartApplication ERROR - Policy token object is null. No policy obtained from entitlements or cached");
        return LicenseStatus::Error;
        }

    // Get product status, this will search the DB for policy
    LicenseStatus licStatus = GetLicenseStatus();

    if ((LicenseStatus::Ok == licStatus) ||
        (LicenseStatus::Offline == licStatus) ||
        (LicenseStatus::Trial == licStatus))
        {
        // spawn one new thread for each heartbeat
        // TODO: try function pointers as there is less overhead than std::function

        int64_t currentTime = m_timeRetriever->GetCurrentTimeAsUnixMillis();

        m_lastRunningUsageHeartbeatStartTime = currentTime; // ensure that StopApplication knows that this heartbeat is started
        CallOnInterval(m_stopUsageHeartbeatThread, m_usageHeartbeatThreadStopped, m_lastRunningUsageHeartbeatStartTime, HEARTBEAT_THREAD_DELAY_MS, [this]() { return UsageHeartbeat(); });

        m_lastRunningLogPostingHeartbeatStartTime = currentTime; // ensure that StopApplication knows that this heartbeat is started
        CallOnInterval(m_stopLogPostingHeartbeatThread, m_logPostingHeartbeatThreadStopped, m_lastRunningLogPostingHeartbeatStartTime, HEARTBEAT_THREAD_DELAY_MS, [this]() { return LogPostingHeartbeat(); });

        m_lastRunningPolicyHeartbeatStartTime = currentTime; // ensure that StopApplication knows that this heartbeat is started
        CallOnInterval(m_stopPolicyHeartbeatThread, m_policyHeartbeatThreadStopped, m_lastRunningPolicyHeartbeatStartTime, HEARTBEAT_THREAD_DELAY_MS, [this](){ return PolicyHeartbeat(); });
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

    m_licensingDb->Close();

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                         Jason.Wichert 5/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::CallOnInterval(std::atomic_bool& stopThread, std::atomic_bool& isFinished, std::atomic<int64_t>& lastRunStartTime, size_t interval, std::function<void(void)> func)
    {
    // run a function on a loop, after the first call delay the call by an interval of time each iteration

    // all variables used in this thread and the main thread, and modified anywhere must be atomic so there is no race condition
    std::thread th([=, &stopThread, &isFinished, &lastRunStartTime]
        {
        if (lastRunStartTime.load() == 0)
            {
            int64_t currentTime = m_timeRetriever->GetCurrentTimeAsUnixMillis();
            lastRunStartTime.store(currentTime); // to signal that the thread has started immediately, in case StopApplication is called immediately after start application
            }

        // first heartbeat without waiting
        func();

        // loop the heartbeat until stopped
        while (!stopThread.load())
            {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            if (!stopThread.load())
                {
                func();
                }
            }

        isFinished.store(true);
        });

    th.detach();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::UsageHeartbeat()
    {
    LOG.debug("ClientImpl::UsageHeartbeat");

    int64_t currentTime = m_timeRetriever->GetCurrentTimeAsUnixMillis();

    // on first heartbeat run
    if (m_startUsageHeartbeat)
        {
        // update start time first here so an immediate StopApplication call still triggers the thread to end
        m_lastRunningUsageHeartbeatStartTime.store(currentTime);
        m_startUsageHeartbeat = false;
        RecordUsage();
        }

    int64_t heartbeatInterval = m_policy->GetHeartbeatInterval(m_applicationInfo->GetProductId(), m_featureString);

    int64_t time_elapsed = currentTime - m_lastRunningUsageHeartbeatStartTime.load();

    if (time_elapsed >= heartbeatInterval)
        {
        RecordUsage();
        m_lastRunningUsageHeartbeatStartTime.store(currentTime);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::StopUsageHeartbeat()
    {
    LOG.debug("ClientImpl::StopUsageHeartbeat");

    if (m_lastRunningUsageHeartbeatStartTime.load() == 0)
        return;

    m_stopUsageHeartbeatThread.store(true);

    while (!m_usageHeartbeatThreadStopped.load())
        {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

    // Reset
    m_lastRunningUsageHeartbeatStartTime.store(0);
    m_startUsageHeartbeat = true;
    m_stopUsageHeartbeatThread.store(false);
    m_usageHeartbeatThreadStopped.store(false);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::PolicyHeartbeat()
    {
    LOG.debug("ClientImpl::PolicyHeartbeat");

    int64_t currentTime = m_timeRetriever->GetCurrentTimeAsUnixMillis();

    // on first heartbeat run
    if (m_startPolicyHeartbeat)
        {
        // TODO: try to get policy token here, or try earlier than policyInterval

        // update start time first here so an immediate StopApplication call still triggers the thread to end
        m_lastRunningPolicyHeartbeatStartTime.store(currentTime);
        m_startPolicyHeartbeat = false;
        }

    // repeat every time the heartbeat is called
    int64_t policyInterval = m_policy->GetPolicyInterval(m_applicationInfo->GetProductId(), m_featureString);

    int64_t time_elapsed = currentTime - m_lastRunningPolicyHeartbeatStartTime.load();

    if (time_elapsed >= policyInterval)
        {
        auto policyToken = GetPolicyToken();
        // check if offline grace period should start
        if (policyToken == nullptr)
            {
            if (!HasOfflineGracePeriodStarted())
                m_licensingDb->SetOfflineGracePeriodStart(DateHelper::GetCurrentTime());
            }
        // otherwise, refresh policy and check if offline grace period should be reset
        else
            {
            m_policy = policyToken;

            if (HasOfflineGracePeriodStarted())
                m_licensingDb->ResetOfflineGracePeriod();
            }
        CleanUpPolicies();
        m_lastRunningPolicyHeartbeatStartTime.store(currentTime);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::StopPolicyHeartbeat()
    {
    LOG.debug("ClientImpl::StopPolicyHeartbeat");

    if (m_lastRunningPolicyHeartbeatStartTime.load() == 0)
        return;

    m_stopPolicyHeartbeatThread.store(true);

    while (!m_policyHeartbeatThreadStopped.load())
        {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

    // Reset
    m_lastRunningPolicyHeartbeatStartTime.store(0);
    m_startPolicyHeartbeat = true;
    m_stopPolicyHeartbeatThread.store(false);
    m_policyHeartbeatThreadStopped.store(false);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::LogPostingHeartbeat()
    {
    LOG.debug("ClientImpl::LogPostingHeartbeat");

    int64_t currentTime = m_timeRetriever->GetCurrentTimeAsUnixMillis();

    // first call of LogPostingHeartbeat
    if (m_startLogPostingHeartbeat)
        {
        // update start time first here so an immediate StopApplication call still triggers the thread to end
        m_lastRunningLogPostingHeartbeatStartTime.store(currentTime);
        m_startLogPostingHeartbeat = false;

        // post logs if there are any left over from a previous session that didn't get posted
        if (m_licensingDb->GetUsageRecordCount() > 0)
            m_ulasProvider->PostUsageLogs(m_applicationInfo, m_dbPath, *m_licensingDb, m_policy); // TODO: check status of post

        if (m_licensingDb->GetFeatureRecordCount() > 0)
            m_ulasProvider->PostFeatureLogs(m_applicationInfo, m_dbPath, *m_licensingDb, m_policy); // TODO: check status of post
        }

    int64_t policyLogsPostingInterval = m_policy->GetTimeToKeepUnSentLogs(m_applicationInfo->GetProductId(), m_featureString);

    int64_t time_elapsed = currentTime - m_lastRunningLogPostingHeartbeatStartTime.load();

    // post when the shorter of the two log posting intervals elapses
    if (time_elapsed >= policyLogsPostingInterval || time_elapsed >= m_logsPostingInterval)
        {
        m_ulasProvider->PostUsageLogs(m_applicationInfo, m_dbPath, *m_licensingDb, m_policy);
        m_ulasProvider->PostFeatureLogs(m_applicationInfo, m_dbPath, *m_licensingDb, m_policy);
        m_lastRunningLogPostingHeartbeatStartTime.store(currentTime);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::StopLogPostingHeartbeat()
    {
    LOG.debug("ClientImpl::StopLogPostingHeartbeat");

    if (m_lastRunningLogPostingHeartbeatStartTime.load() == 0)
        return;

    m_stopLogPostingHeartbeatThread.store(true);

    while (!m_logPostingHeartbeatThreadStopped.load())
        {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

    // post outstanding usage logs
    if (m_licensingDb->GetUsageRecordCount() > 0)
        m_ulasProvider->PostUsageLogs(m_applicationInfo, m_dbPath, *m_licensingDb, m_policy); // TODO: check status of post

    if (m_licensingDb->GetFeatureRecordCount() > 0)
        m_ulasProvider->PostFeatureLogs(m_applicationInfo, m_dbPath, *m_licensingDb, m_policy); // TODO: check status of post

    // Reset
    m_lastRunningLogPostingHeartbeatStartTime.store(0);
    m_startLogPostingHeartbeat = true;
    m_stopLogPostingHeartbeatThread.store(false);
    m_logPostingHeartbeatThreadStopped.store(false);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClientImpl::MarkFeature(Utf8StringCR featureId, FeatureUserDataMapPtr featureUserData)
    {
    LOG.debug("ClientImpl::MarkFeature");

    if (Utf8String::IsNullOrEmpty(featureId.c_str()))
        {
        LOG.error("ClientImpl::MarkFeature: ERROR - featureId is null or empty.");
        return ERROR;
        }

    if (!m_licensingDb->IsDbOpen())
        {
        LOG.error("ClientImpl::MarkFeature ERROR - Usage DB not open.");
        return ERROR;
        }

    if (m_policy == nullptr)
        {
        LOG.error("ClientImpl::MarkFeature ERROR - StartApplication function not called.");
        return ERROR;
        }

    LicenseStatus licStatus = GetLicenseStatus();

    if ((LicenseStatus::Ok != licStatus) &&
        (LicenseStatus::Offline != licStatus) &&
        (LicenseStatus::Trial != licStatus))
        {
        LOG.errorv("ClientImpl::MarkFeature ERROR - Licenses status of product %s is %d. Feature %s cannot be tracked",
                   m_applicationInfo->GetProductId().c_str(),
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

    versionString.Sprintf("%d%.4d%.4d%.4d", m_applicationInfo->GetVersion().GetMajor(), m_applicationInfo->GetVersion().GetMinor(),
                          m_applicationInfo->GetVersion().GetSub1(), m_applicationInfo->GetVersion().GetSub2());

    LOG.debugv("ClientImpl::MarkFeature - UsageLogEntry: ultimateId:%ld, principalId:%s, userId:%s, machineName:%s, machineSID:%s, userName:%s, userSID:%s, policyId:%s, securableId:%s, "
               "productId:%ld, featureString:%s, version:%ld, projectId:%s, correlationId:%s, eventTimeZ:%s, schemaVer:%f, source:%s, country:%s, usageType:%s, featureId:%s, startTime:%s, "
               "endTime:%s, featureUserData:%s",
               m_policy->GetUltimateSAPId(),
               m_policy->GetPrincipalId().c_str(),
               m_policy->GetAppliesToUserId().c_str(),
               m_applicationInfo->GetDeviceId().c_str(),
               gsid.GetMachineSID(m_applicationInfo->GetDeviceId()).c_str(),
               m_userInfo.username.c_str(),
               gsid.GetUserSID(m_userInfo.username, m_applicationInfo->GetDeviceId()).c_str(),
               m_policy->GetPolicyId().c_str(),
               m_policy->GetSecurableId().c_str(),
               atoi(m_applicationInfo->GetProductId().c_str()),
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

    if (SUCCESS != m_licensingDb->RecordFeature(m_policy->GetUltimateSAPId(),
                                            m_policy->GetPrincipalId(),
                                            m_policy->GetAppliesToUserId(),
                                            m_applicationInfo->GetDeviceId(),
                                            gsid.GetMachineSID(m_applicationInfo->GetDeviceId()),
                                            m_userInfo.username,
                                            gsid.GetUserSID(m_userInfo.username, m_applicationInfo->GetDeviceId()),
                                            m_policy->GetPolicyId(),
                                            m_policy->GetSecurableId(),
                                            atoi(m_applicationInfo->GetProductId().c_str()),
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

    if (!m_licensingDb->IsDbOpen())
        {
        LOG.error("ClientImpl::RecordUsage ERROR - Usage DB is not open.");
        return ERROR;
        }

    Utf8String versionString;
    GenerateSID gsid;

    // Create usage record
    Utf8String eventTimeZ = DateTime::GetCurrentTimeUtc().ToString();

    versionString.Sprintf("%d%.4d%.4d%.4d", m_applicationInfo->GetVersion().GetMajor(), m_applicationInfo->GetVersion().GetMinor(),
                          m_applicationInfo->GetVersion().GetSub1(), m_applicationInfo->GetVersion().GetSub2());

    LOG.debugv("ClientImpl::RecordUsage - UsageLogEntry: ultimateId:%ld, principalId:%s, userId:%s, machineName:%s, machineSID:%s, userName:%s, userSID:%s, policyId:%s, securableId:%s, "
               "productId:%ld, featureString:%s, version:%ld, projectId:%s, correlationId:%s, eventTimeZ:%s, schemaVer:%f, source:%s, country:%s, usageType:%s",
               m_policy->GetUltimateSAPId(),
               m_policy->GetPrincipalId().c_str(),
               m_policy->GetAppliesToUserId().c_str(),
               m_applicationInfo->GetDeviceId().c_str(),
               gsid.GetMachineSID(m_applicationInfo->GetDeviceId()).c_str(),
               m_userInfo.username.c_str(),
               gsid.GetUserSID(m_userInfo.username, m_applicationInfo->GetDeviceId()).c_str(),
               m_policy->GetPolicyId().c_str(),
               m_policy->GetSecurableId().c_str(),
               atoi(m_applicationInfo->GetProductId().c_str()),
               m_featureString.c_str(),
               atoll(versionString.c_str()),
               m_projectId.c_str(),
               m_correlationId.c_str(),
               eventTimeZ.c_str(),
               LICENSE_CLIENT_SCHEMA_VERSION,
               GetLoggingPostSource().c_str(),
               m_policy->GetCountry().c_str(),
               m_policy->GetUsageType().c_str());


    if (SUCCESS != m_licensingDb->RecordUsage(m_policy->GetUltimateSAPId(),
                                          m_policy->GetPrincipalId(),
                                          m_policy->GetAppliesToUserId(),
                                          m_applicationInfo->GetDeviceId(),
                                          gsid.GetMachineSID(m_applicationInfo->GetDeviceId()),
                                          m_userInfo.username,
                                          gsid.GetUserSID(m_userInfo.username, m_applicationInfo->GetDeviceId()),
                                          m_policy->GetPolicyId(),
                                          m_policy->GetSecurableId(),
                                          atoi(m_applicationInfo->GetProductId().c_str()),
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

    // try to get policy from entitlements, fallback to database if call fails
    try {
        auto policy = m_policyProvider->GetPolicy().get();
        if (policy != nullptr)
            {
            StorePolicyInLicensingDb(policy);
            DeleteAllOtherPoliciesByUser(policy);
            }

        return policy;
        }
    catch (...)
        {
        LOG.info("ClientImpl::GetPolicyToken: Call to entitlements failed, getting policy from DB");
        
        const auto productId = m_applicationInfo->GetProductId();
        auto policy = SearchForPolicy(productId);

        // start offline grace period if there is a cached policy
        if (policy != nullptr && !HasOfflineGracePeriodStarted())
            m_licensingDb->SetOfflineGracePeriodStart(DateHelper::GetCurrentTime());

        return policy;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ILicensingDb& ClientImpl::GetLicensingDb()
    {
    return *m_licensingDb;
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
    auto qualifier = m_policy->GetQualifier("IsCheckedOut", m_applicationInfo->GetProductId().c_str(), m_featureString);

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
            m_licensingDb->DeletePolicyFile(policy->GetPolicyId());
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy> ClientImpl::GetPolicyWithId(Utf8StringCR policyId)
    {
    LOG.debug("ClientImpl::GetPolicyWithId");

    auto jsonpolicy = m_licensingDb->GetPolicyFile(policyId);
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
    auto jsonpolicies = m_licensingDb->GetPolicyFiles();
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
    auto jsonpolicies = m_licensingDb->GetPolicyFilesByUser(m_userInfo.userId);
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
        productId = m_applicationInfo->GetProductId();
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
void ClientImpl::DeleteAllOtherPoliciesByUser(std::shared_ptr<Policy> policy)
    {
    LOG.debug("ClientImpl::DeleteAllOtherPoliciesByUser");

    m_licensingDb->DeleteAllOtherPolicyFilesByUser(policy->GetPolicyId(),
        policy->GetUserData()->GetUserId());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                        Jason.Wichert 5/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::AddPolicyToDb(std::shared_ptr<Policy> policy)
    {
    //This function is for testing, allows you to put policies in the database without being entitled, etc.
    LOG.debug("ClientImpl::AddPolicyToDb");

    if (SUCCESS != m_licensingDb->OpenOrCreate(m_dbPath))
        {
        LOG.error("ClientImpl::AddPolicyToDb ERROR - Database creation failed.");
        return;
        }

    StorePolicyInLicensingDb(policy);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::StorePolicyInLicensingDb(std::shared_ptr<Policy> policy)
    {
    LOG.debug("ClientImpl::StorePolicyInLicensingDb");

    auto expiration = policy->GetPolicyExpiresOn();
    auto lastUpdate = policy->GetRequestData()->GetClientDateTime();
    auto accessKey = policy->GetRequestData()->GetAccessKey();
    m_licensingDb->AddOrUpdatePolicyFile(policy->GetPolicyId(),
        policy->GetAppliesToUserId(),
        accessKey,
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

    auto graceStartString = m_licensingDb->GetOfflineGracePeriodStart();
    return graceStartString != "";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ClientImpl::GetDaysLeftInOfflineGracePeriod(std::shared_ptr<Policy> policy, Utf8String productId, Utf8String featureString)
    {
    LOG.debug("ClientImpl::GetDaysLeftInOfflineGracePeriod");

    Utf8String graceStartString = m_licensingDb->GetOfflineGracePeriodStart();
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
LicenseStatus ClientImpl::GetLicenseStatus()
    {
    LOG.debug("ClientImpl::GetLicenseStatus");

    const auto productId = m_applicationInfo->GetProductId();

    auto policy = SearchForPolicy(productId);

    if (policy == nullptr)
        {
        return LicenseStatus::NotEntitled;
        }

    // if policy not valid, return LicenseStatus::DisabledByPolicy
    if (policy->GetPolicyStatus() != Policy::PolicyStatus::Valid)
        {
        return LicenseStatus::DisabledByPolicy;
        }

    const auto productStatus = policy->GetProductStatus(productId, m_featureString, m_applicationInfo->GetVersion());

    if (productStatus == LicenseStatus::Ok)
        {
        if (HasOfflineGracePeriodStarted())
            {
            if (!policy->IsAllowedOfflineUsage(productId, m_featureString))
                {
                return LicenseStatus::DisabledByPolicy;
                }

            if (GetDaysLeftInOfflineGracePeriod(policy, productId, m_featureString) > 0)
                {
                return LicenseStatus::Offline;
                }

            // else offline grace period has expired, return LicenseStatus::Expired
            return LicenseStatus::Expired;
            }

        return LicenseStatus::Ok;
        }

    return productStatus;
    }
