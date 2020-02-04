/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ClientImpl.h"
#include "GenerateSID.h"
#include <Licensing/Utils/LogFileHelper.h>
#include <Licensing/Utils/UsageJsonHelper.h>
#include "Logging.h"
#include "LicensingDb.h"
#include <BeHttp/HttpError.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <Bentley/BeTextFile.h>

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
    if (m_licensingDb == nullptr) // either pass in a mock, or initialize here
        m_licensingDb = std::make_unique<LicensingDb>();

    m_correlationId = BeSQLite::BeGuid(true).ToString();
    m_timeRetriever = TimeRetriever::Get();

    if (Utf8String::IsNullOrEmpty(projectId.c_str()))
        {
        m_projectId = "00000000-0000-0000-0000-000000000000";
        }
    else
        {
        m_projectId = projectId;
        }

    m_offlineMode = offlineMode;
    }

bool ClientImpl::ValidateParamsAndDB()
    {
    // start time for offline usage logs
    m_usageStartTime = DateTime::GetCurrentTimeUtc().ToString();

    if (m_applicationInfo == nullptr)
        {
        LOG.error("ClientImpl::StartApplication ERROR - Application Information object is null.");
        return false;
        }

    if (BeFileName::IsNullOrEmpty(m_dbPath))
        {
        LOG.error("ClientImpl::StartApplication ERROR - Database path string is null or empty.");
        return false;
        }

    if (Utf8String::IsNullOrEmpty(m_correlationId.c_str()))
        {
        LOG.error("ClientImpl::StartApplication ERROR - Correlation ID (Usage ID) string is null or empty.");
        return false;
        }

    if (m_timeRetriever == nullptr)
        {
        LOG.error("ClientImpl::StartApplication ERROR - Time retriever object is null.");
        return false;
        }

    if (m_licensingDb == nullptr)
        {
        LOG.error("ClientImpl::StartApplication ERROR - Database object is null.");
        return false;
        }

    if (SUCCESS != m_licensingDb->OpenOrCreate(m_dbPath))
        {
        LOG.error("ClientImpl::StartApplication ERROR - Database creation failed.");
        return false;
        }
    return true;
    }

LicenseStatus ClientImpl::StartApplicationGeneric(std::function<std::shared_ptr<Policy>()> getPolicy)
    {
    LOG.debug("ClientImpl::StartApplicationGeneric");

    if (!ValidateParamsAndDB())
        {
        return LicenseStatus::Error;
        }

    const auto productId = m_applicationInfo->GetProductId();
    auto policy = SearchForCheckout(productId, m_featureString);
    if (policy != nullptr)
        {
        LOG.info("Checkout found returning LicenseStatus OK");
        m_policy = policy;
        StartLogPostingHeartbeat();
        return LicenseStatus::Ok;
        }

    m_policy = getPolicy();

    if (m_policy == nullptr)
        {
        LOG.error("ClientImpl::StartApplicationGeneric ERROR - Policy token object is null. No policy obtained from entitlements or cached");
        return LicenseStatus::Error;
        }

    // Get product status, this will search the DB for policy
    LicenseStatus licStatus = GetLicenseStatus();

    if ((LicenseStatus::Ok == licStatus) ||
        (LicenseStatus::Offline == licStatus) ||
        (LicenseStatus::Trial == licStatus))
        {
        StartUsageHeartbeat();
        StartLogPostingHeartbeat();
        StartPolicyHeartbeat();
        }
    else
        {
        LOG.errorv("ClientImpl::StartApplicationGeneric - LicenseStatus Returned: %d", licStatus);
        }

    return licStatus;
    }


LicenseStatus ClientImpl::StartApplicationForProject(Utf8StringCR projectId)
    {
    LOG.debug("ClientImpl::StartApplicationForProject");

    auto getPolicy = [this, projectId]() { return GetProjectPolicyToken(projectId); };

    return StartApplicationGeneric(getPolicy);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus ClientImpl::StartApplication()
    {
    LOG.debug("ClientImpl::StartApplication");

    auto getPolicy = [this]() { return GetPolicyToken(); };

    return StartApplicationGeneric(getPolicy);
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
    // TODO: try function pointers as there is less overhead than std::function

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

void ClientImpl::StartUsageHeartbeat()
    {
    m_lastRunningUsageHeartbeatStartTime = m_timeRetriever->GetCurrentTimeAsUnixMillis(); // ensure that StopApplication knows that this heartbeat is started
    CallOnInterval(m_stopUsageHeartbeatThread, m_usageHeartbeatThreadStopped, m_lastRunningUsageHeartbeatStartTime, HEARTBEAT_THREAD_DELAY_MS, [this]() { return UsageHeartbeat(); });
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

void ClientImpl::StartPolicyHeartbeat()
    {
    m_lastRunningPolicyHeartbeatStartTime = m_timeRetriever->GetCurrentTimeAsUnixMillis(); // ensure that StopApplication knows that this heartbeat is started
    CallOnInterval(m_stopPolicyHeartbeatThread, m_policyHeartbeatThreadStopped, m_lastRunningPolicyHeartbeatStartTime, HEARTBEAT_THREAD_DELAY_MS, [this]() { return PolicyHeartbeat(); });
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

void ClientImpl::StartLogPostingHeartbeat()
    {
    m_lastRunningLogPostingHeartbeatStartTime = m_timeRetriever->GetCurrentTimeAsUnixMillis(); // ensure that StopApplication knows that this heartbeat is started
    CallOnInterval(m_stopLogPostingHeartbeatThread, m_logPostingHeartbeatThreadStopped, m_lastRunningLogPostingHeartbeatStartTime, HEARTBEAT_THREAD_DELAY_MS, [this]() { return LogPostingHeartbeat(); });
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
    bool durationTracked = false; // TODO: where to get this?

    Utf8String startTimeUtc = m_usageStartTime; // gets usage time from start of StartApplication call, is this right?
    Utf8String endTimeUtc = DateTime::GetCurrentTimeUtc().ToString();
    Utf8String entryDate = endTimeUtc;

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

    LOG.debugv("ClientImpl::MarkFeature - "
        "FeatureLogEntry: "
        "ultimateId:%ld, "
        "usageCountryIso:%s, "
        "productId:%ld, "
        "featureString:%s, "
        "productVersion:%ld, "
        "machineName:%s, "
        "machineSID:%s, "
        "userName:%s, "
        "userSID:%s, "
        "imsId:%s, "
        "projectId:%s, "
        "correlationId:%s, "
        "featureId:%s, "
        "startTime:%s, "
        "endTime:%s, "
        "durationTracked:%s"
        "userData:%s ",
        m_policy->GetUltimateSAPId(),
        m_policy->GetCountry().c_str(),
        atoi(m_applicationInfo->GetProductId().c_str()),
        m_featureString.c_str(),
        atoll(versionString.c_str()),
        m_applicationInfo->GetDeviceId().c_str(),
        gsid.GetMachineSID(m_applicationInfo->GetDeviceId()).c_str(),
        m_userInfo.username.c_str(),
        gsid.GetUserSID(m_userInfo.username, m_applicationInfo->GetDeviceId()).c_str(),
        m_policy->GetAppliesToUserId().c_str(),
        m_projectId.c_str(),
        m_correlationId.c_str(),
        featureId.c_str(),
        startTimeUtc.c_str(),
        endTimeUtc.c_str(),
        durationTracked ? "true" : "false",
        userDataString.c_str());

    if (SUCCESS != m_licensingDb->RecordFeature(m_policy->GetUltimateSAPId(),
        m_policy->GetCountry(),
        atoi(m_applicationInfo->GetProductId().c_str()),
        m_featureString,
        atoll(versionString.c_str()),
        m_applicationInfo->GetDeviceId(),
        gsid.GetMachineSID(m_applicationInfo->GetDeviceId()),
        m_userInfo.username,
        gsid.GetUserSID(m_userInfo.username, m_applicationInfo->GetDeviceId()),
        m_policy->GetAppliesToUserId(),
        m_projectId,
        m_correlationId,
        featureId,
        startTimeUtc,
        endTimeUtc,
        durationTracked ? "true" : "false",
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
    Utf8String startTimeUtc = m_usageStartTime;
    Utf8String endTimeUtc = DateTime::GetCurrentTimeUtc().ToString();
    Utf8String entryDate = endTimeUtc;

    versionString.Sprintf("%d%.4d%.4d%.4d", m_applicationInfo->GetVersion().GetMajor(), m_applicationInfo->GetVersion().GetMinor(),
        m_applicationInfo->GetVersion().GetSub1(), m_applicationInfo->GetVersion().GetSub2());

    LOG.debugv("ClientImpl::RecordUsage - "
        "UsageLogEntry: "
        "ultimateId:%ld, "
        "principalId:%s, "
        "productId:%ld, "
        "usageCountryIso:%s, "
        "featureString:%s, "
        "imsId:%s, "
        "machineName:%s, "
        "machineSID:%s, "
        "userName:%s, "
        "userSID:%s, "
        "policyId:%s, "
        "productVersion:%ld, "
        "projectId:%s, "
        "correlationId:%s, "
        "logVersion:%ld, "
        "logPostingSource:%s, "
        "usageType:%s, "
        "startTimeUtc:%s, "
        "endTimeUtc:%s, "
        "entryDate:%s, "
        "partitionId:%i ",
        m_policy->GetUltimateSAPId(),
        m_policy->GetInUsePrincipalId().c_str(),
        atoi(m_applicationInfo->GetProductId().c_str()),
        m_policy->GetCountry().c_str(),
        m_featureString.c_str(),
        m_policy->GetAppliesToUserId().c_str(),
        m_applicationInfo->GetDeviceId().c_str(),
        gsid.GetMachineSID(m_applicationInfo->GetDeviceId()).c_str(),
        m_userInfo.username.c_str(),
        gsid.GetUserSID(m_userInfo.username, m_applicationInfo->GetDeviceId()).c_str(),
        m_policy->GetPolicyId().c_str(),
        atoll(versionString.c_str()),
        m_projectId.c_str(),
        m_correlationId.c_str(),
        LOG_VERSION,
        GetLoggingPostSource().c_str(),
        m_policy->GetUsageType().c_str(),
        startTimeUtc.c_str(),
        endTimeUtc.c_str(),
        entryDate.c_str(),
        PARTITION_ID);

    if (SUCCESS != m_licensingDb->RecordUsage(m_policy->GetUltimateSAPId(),
        m_policy->GetPrincipalId(),
        atoi(m_applicationInfo->GetProductId().c_str()),
        m_policy->GetCountry(),
        m_featureString,
        m_policy->GetAppliesToUserId(),
        m_applicationInfo->GetDeviceId(),
        gsid.GetMachineSID(m_applicationInfo->GetDeviceId()),
        m_userInfo.username,
        gsid.GetUserSID(m_userInfo.username, m_applicationInfo->GetDeviceId()),
        m_policy->GetPolicyId(),
        atoll(versionString.c_str()),
        m_projectId,
        m_correlationId,
        LOG_VERSION,
        GetLoggingPostSource(),
        m_policy->GetUsageType(),
        startTimeUtc,
        endTimeUtc,
        entryDate,
        PARTITION_ID))
        {
        LOG.error("ClientImpl::RecordUsage - ERROR: Usage failed to be recorded.");
        return ERROR;
        }

    return SUCCESS;
    }

std::shared_ptr<Policy> ClientImpl::GetProjectPolicyToken(Utf8StringCR projectId)
    {
    LOG.debug("ClientImpl::GetProjectPolicyToken");

    try
        {
        auto policy = m_policyProvider->GetPolicy(projectId).get();
        if (policy != nullptr)
            {
            StoreProjectPolicyInLicensingDb(policy, projectId);
            DeleteAllOtherPoliciesByProject(policy, projectId);
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
std::shared_ptr<Policy> ClientImpl::GetPolicyToken()
    {
    LOG.debug("ClientImpl::GetPolicyToken");

    // try to get policy from entitlements, fallback to database if call fails
    try
        {
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

folly::Future<std::shared_ptr<Policy>> ClientImpl::GetPolicy(Utf8StringCR projectId)
    {
    return m_policyProvider->GetPolicy(projectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClientImpl::GetLoggingPostSource() const
    {
    LOG.debug("ClientImpl::GetLoggingPostSource");

    // TEMPORARY: only allow offline for this value, we do not support realtime at the moment
    return "Offline";

    //// Check for checked out license
    //auto qualifier = m_policy->GetQualifier("IsCheckedOut", m_applicationInfo->GetProductId().c_str(), m_featureString);

    //if (qualifier->GetValue().Equals("true"))
    //    return "Checkout";

    //if (m_offlineMode)
    //    return "Offline";
    //else
    //    return "RealTime";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::CleanUpPolicies()
    {
    LOG.debug("ClientImpl::CleanUpPolicies");

    auto policies = m_licensingDb->GetPolicyFiles();
    for (auto policy : policies)
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

    auto jsonPolicy = m_licensingDb->GetPolicyFile(policyId);

    if (jsonPolicy == Json::Value::GetNull())
        {
        return nullptr;
        }

    return Policy::Create(jsonPolicy);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy>> ClientImpl::GetValidUserPolicies()
    {
    LOG.debug("ClientImpl::GetValidUserPolicies");

    auto policies = m_licensingDb->GetValidPolicyFilesForUser(m_userInfo.userId);
    return policies;
    }

std::list<std::shared_ptr<Policy>> ClientImpl::GetValidCheckouts()
    {
    LOG.debug("ClientImpl::GetValidCheckouts");

    // TODO more useful logging
    std::list<std::shared_ptr<Policy>> policyList;
    auto jsonpolicies = m_licensingDb->GetAllCheckouts();
    for (auto json : jsonpolicies)
        {
        auto policy = Policy::Create(json);
        if (!policy->IsValid())
            continue;
        if (policy->IsExpired())
            continue;
        policyList.push_back(policy);
        }
    return policyList;
    }

std::shared_ptr<Policy> ClientImpl::SearchForCheckout(Utf8String productId, Utf8String featureString) // TODO productId should be int
    {
    LOG.debug("ClientImpl::SearchForCheckout");

    auto policies = GetValidCheckouts();

    for (auto policy : policies)
        {
        // look for a securable that matches productId and featureString
        if (policy->ContainsProduct(productId, featureString))
            {
            LOG.info("ClientImpl::SearchForCheckout - found checkout from db");
            return policy;
            }
        }
    LOG.debug("ClientImpl::SearchForCheckout - no checkout found");
    return nullptr; // not found
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy> ClientImpl::SearchForPolicy(Utf8String productId)
    {
    LOG.debug("ClientImpl::SearchForPolicy");

    // get all of user's policies
    auto policies = GetValidUserPolicies();

    for (auto policy : policies)
        {
        // policy assumed to be valid due to IsValid check in GetValidUserPolicies()
        // look for a securable that matches productId and featureString
        for (auto securable : policy->GetSecurableData())
            {
            if (policy->ContainsProduct(productId, m_featureString))
                {
                LOG.info("ClientImpl::SearchForPolicy - found policy from db");
                return policy;
                }
            }
        }

    LOG.debug("ClientImpl::SearchForPolicy - policy not found");
    return nullptr; // not found
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

void ClientImpl::DeleteAllOtherPoliciesByProject(std::shared_ptr<Policy> policy, Utf8StringCR projectId)
    {
    LOG.debug("ClientImpl::DeleteAllOtherPoliciesByProject");
    m_licensingDb->DeleteAllOtherPolicyFilesByProject(policy->GetPolicyId(),
        policy->GetUserData()->GetUserId(),
        projectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                        Jason.Wichert 5/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::AddPolicyToDb(std::shared_ptr<Policy> policy, bool IsCheckout)
    {
    //This function is for testing, allows you to put policies in the database without being entitled, etc.
    LOG.debug("ClientImpl::AddPolicyToDb");

    if (SUCCESS != m_licensingDb->OpenOrCreate(m_dbPath))
        {
        LOG.error("ClientImpl::AddPolicyToDb ERROR - Database creation failed.");
        return;
        }

    StorePolicyInLicensingDb(policy, IsCheckout);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientImpl::StorePolicyInLicensingDb(std::shared_ptr<Policy> policy, bool IsCheckout, Utf8StringCR projectId)
    {
    LOG.debug("ClientImpl::StorePolicyInLicensingDb");

    auto expiration = policy->GetPolicyExpiresOn();
    auto lastUpdate = policy->GetRequestData()->GetClientDateTime();
    auto accessKey = policy->GetRequestData()->GetAccessKey();

    if (IsCheckout)
        {
        m_licensingDb->AddOrUpdateCheckout(policy->GetPolicyId(),
            policy->GetAppliesToUserId(),
            accessKey,
            expiration,
            lastUpdate,
            policy->GetJson());
        return;
        }

    m_licensingDb->AddOrUpdatePolicyFile(policy->GetPolicyId(),
        policy->GetAppliesToUserId(),
        accessKey,
        expiration,
        lastUpdate,
        policy->GetJson(),
        projectId);
    }

void ClientImpl::StoreProjectPolicyInLicensingDb(std::shared_ptr<Policy> policy, Utf8StringCR projectId)
    {
    LOG.debug("ClientImpl::StoreProjectPolicyInLicensingDb");

    const bool isCheckout = false;
    StorePolicyInLicensingDb(policy, isCheckout, projectId);
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

    if (policy->GetPolicyStatus() == Policy::PolicyStatus::Expired)
        {
        return LicenseStatus::Expired;
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                     Jason.Wichert   7/2019
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ClientImpl::GetTrialDaysRemaining()
    {
    LOG.debug("ClientImpl::GetTrialDaysRemaining");

    const auto productId = m_applicationInfo->GetProductId();

    auto policy = SearchForPolicy(productId);
    if (policy == nullptr)
        {
        LOG.error("Policy not found");
        return -1;
        }
    if (policy->GetPolicyStatus() != Policy::PolicyStatus::Valid)
        {
        LOG.error("Policy not valid");
        return -1;
        }

    int64_t daysLeft = policy->GetTrialDaysRemaining(productId, m_featureString, m_applicationInfo->GetVersion());
    return daysLeft;
    }

int64_t ClientImpl::ImportCheckout(BeFileNameCR filepath)
    {
    LOG.debug("ClientImpl::ImportCheckout");
    //Verify File Exists   
    if (!filepath.DoesPathExist())
        {
        LOG.debugv("File does not exist, or user lacks permissions to open %s", filepath.c_str());
        return -1; // Error 
        }

    if (!filepath.EndsWithI(L"belic"))
        {
        LOG.debug("not a belic file");
        return -1;
        }

    //Read in policy file string 
    std::shared_ptr<Policy> policy = nullptr;
    try
        {
        BeFileStatus status;
        BeTextFilePtr infileReadPtr;
        WString policyToken;
        WString policyCert;
        infileReadPtr = BeTextFile::Open(status, filepath.c_str(), TextFileOpenType::Read, TextFileOptions::None);
        if (!(BeFileStatus::Success == status))
            {
            LOG.debug("File open failed");
            return -1;
            }
        TextFileReadStatus read_status = infileReadPtr->GetLine(policyToken);
        if (!(TextFileReadStatus::Success == read_status))
            {
            LOG.debug("Failed to read from file");
            return -1;
            }
        read_status = infileReadPtr->GetLine(policyCert);
        if (!(TextFileReadStatus::Success == read_status))
            {
            LOG.debug("Failed to read from file");
            return -1;
            }
        policyToken.DropQuotes();
        policyCert.DropQuotes();
        Utf8String Token(policyToken);
        Utf8String Cert(policyCert);
        infileReadPtr->Close();

        //Add policy to DB 
        policy = Policy::Create(JWToken::Create(Token.c_str(), Cert.c_str()));
        if (policy == nullptr)
            {
            LOG.debug("Could not create policy from supplied policy token and cert");
            return -1; //error occured in JWTToken creation or in Policy creation 
            }
        }
    catch (...)
        {
        LOG.debug("Exception occured attempting to import the belic file");
        return -1;
        }
    GenerateSID gsid;
    Utf8String localDeviceId = m_applicationInfo->GetDeviceId();
    Utf8String policyDeviceId = policy->GetRequestData()->GetMachineName();
    //Verify policy is for this device ID before putting in DB
    auto localSig = gsid.GetMachineSID(localDeviceId);
    auto policySig = gsid.GetMachineSID(policyDeviceId);
    if (localSig != policySig)
        {
        LOG.debug("Device ID does not match provided policy device id");
        return -2; // policy and machine signature mismatch, policy not for this device. 
        }
    AddPolicyToDb(policy, true);
    return 0;//success 
    }
