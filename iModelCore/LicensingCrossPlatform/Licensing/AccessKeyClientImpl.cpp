/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "AccessKeyClientImpl.h"
#include "GenerateSID.h"
#include "Logging.h"
#include "LicensingDb.h"

#include <Licensing/Utils/LogFileHelper.h>
#include <Licensing/Utils/UsageJsonHelper.h>
#include <fstream>

#include <BeHttp/HttpError.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

AccessKeyClientImpl::AccessKeyClientImpl
    (
    Utf8StringCR accessKey,
    ApplicationInfoPtr applicationInfo,
    BeFileNameCR db_path,
    bool offlineMode,
    IPolicyProviderPtr policyProvider,
    IUlasProviderPtr ulasProvider,
    Utf8StringCR projectId,
    Utf8StringCR featureString,
    ILicensingDbPtr licensingDb
    )
    {
    m_userInfo = ConnectSignInManager::UserInfo();

    m_accessKey = accessKey;
    m_applicationInfo = applicationInfo;
    m_dbPath = db_path;
    m_offlineMode = offlineMode;
    m_policyProvider = policyProvider;
    m_ulasProvider = ulasProvider;
    m_projectId = projectId;
    m_featureString = featureString;
    m_licensingDb = licensingDb;

    if(m_licensingDb == nullptr) // either pass in a mock, or initialize here
        m_licensingDb = std::make_unique<LicensingDb>(); // should this be make shared?

    m_correlationId = BeSQLite::BeGuid(true).ToString();
    m_timeRetriever = TimeRetriever::Get();

    m_isAccessKeyValid = false; // assume invalid access key to start

    if (Utf8String::IsNullOrEmpty(projectId.c_str()))
        m_projectId = "00000000-0000-0000-0000-000000000000";

    if (m_offlineMode)
        {
        // Fake use m_offlineMode in order to avoid unused variable warning 
        // that is treated as error on clang compilers for iOS.
        }
    }

LicenseStatus AccessKeyClientImpl::StartApplication()
    {
    LOG.trace("AccessKeyClientImpl::StartApplication");

    if (SUCCESS != m_licensingDb->OpenOrCreate(m_dbPath))
        {
        LOG.error("AccessKeyClientImpl::StartApplication ERROR - Database creation failed.");
        return LicenseStatus::Error;
        }

    m_policy = GetPolicyToken();

    if (m_policy == nullptr)
        {
        LOG.error("AccessKeyClientImpl::StartApplication ERROR - Policy token object is null.");
        return LicenseStatus::Error;
        }

    // Get product status
    LicenseStatus licStatus = GetLicenseStatus();

    if ((LicenseStatus::Ok == licStatus) ||
        (LicenseStatus::Offline == licStatus) ||
        (LicenseStatus::Trial == licStatus))
        {
        // Begin heartbeat
        CallOnInterval(m_stopPolicyHeartbeatThread, m_policyHeartbeatThreadStopped, m_lastRunningPolicyHeartbeatStartTime, HEARTBEAT_THREAD_DELAY_MS, [this]() { return PolicyHeartbeat(); });
        }
    else
        {
        LOG.errorv("ClientImpl::StartApplication - LicenseStatus Returned: %d", licStatus);
        }

    return licStatus;
    }

BentleyStatus AccessKeyClientImpl::StopApplication()
    {
    LOG.trace("AccessKeyClientImpl::StopApplication");
    StopPolicyHeartbeat(); // This will stop Policy heartbeat

    m_licensingDb->Close();

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AccessKeyClientImpl::ValidateAccessKey()
    {
    LOG.debug("AccessKeyClientImpl::ValidateAccessKey");

    // if we are offline with a valid cached policy
    if (HasOfflineGracePeriodStarted())
        {
        if (m_policy->GetRequestData()->GetAccessKey().Equals(m_accessKey))
            {
            m_isAccessKeyValid = true;
            return m_isAccessKeyValid;
            }
        }

    auto responseJson = m_ulasProvider->GetAccessKeyInfo(m_applicationInfo, m_accessKey).get();

    if (Json::Value::GetNull() == responseJson)
        {
        // call failed, so assume access key is unchanged since the last check
        LOG.error("AccessKeyClientImpl::ValidateAccessKey - Call to AccessKey service failed");
        return m_isAccessKeyValid;
        }

    if ("Success" != responseJson["status"].asString())
        {
        // unsuccessful if AccessKey is expired, not active, or not found - clear out policy from database
        LOG.errorv("AccessKeyClientImpl::ValidateAccessKey - %s", responseJson["msg"].asString().c_str());
        m_isAccessKeyValid = false;
        }
    else
        {
        m_isAccessKeyValid = true;
        }

    return m_isAccessKeyValid;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy> AccessKeyClientImpl::GetPolicyToken()
    {
    // TODO: rename this method? (and ClientImpl::GetPolicyToken) we are getting policy, not the token, and are storing the policy in the db...
    LOG.debug("AccessKeyClientImpl::GetPolicyToken");

    // try to get policy from entitlements, fallback to database if call fails to support offline workflows
    try
        {
        // call to entitlements will fail if accesskey is inactive or expired
        auto policy = m_policyProvider->GetPolicyWithKey(m_accessKey).get();

        if (policy != nullptr)
            {
            StorePolicyInLicensingDb(policy);
            DeleteAllOtherPoliciesByKey(policy);
            }

        return policy;
        }
    catch (...)
        {
        // http call failed, offline support below
        LOG.info("AccessKeyClientImpl::GetPolicyToken: Call to entitlements failed, getting policy from DB");

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
LicenseStatus AccessKeyClientImpl::GetLicenseStatus()
    {
    LOG.debug("AccessKeyClientImpl::GetLicenseStatus");

    if (!ValidateAccessKey())
        {
        LOG.info("AccessKey missing from portal(deleted), expired, or revoked");
        return LicenseStatus::NotEntitled;
        }

    return ClientImpl::GetLicenseStatus();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AccessKeyClientImpl::DeleteAllOtherPoliciesByKey(std::shared_ptr<Policy> policy)
    {
    LOG.debug("AccessKeyClientImpl::DeleteAllOtherPoliciesByKey");

    m_licensingDb->DeleteAllOtherPolicyFilesByKey(policy->GetPolicyId(),
        policy->GetRequestData()->GetAccessKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy>> AccessKeyClientImpl::GetUserPolicies()
    {
    LOG.debug("AccessKeyClientImpl::GetUserPolicies");

    std::list<std::shared_ptr<Policy>> policyList;
    auto jsonpolicies = m_licensingDb->GetPolicyFilesByKey(m_accessKey);
    for (auto json : jsonpolicies)
        {
        auto policy = Policy::Create(json);
        if (!policy->IsValid())
            continue;
        if (policy->GetRequestData()->GetAccessKey().Equals(m_accessKey))
            {
            policyList.push_back(policy);
            }
        }

    return policyList;
    }
