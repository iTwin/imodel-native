/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/ClientWithKeyImpl.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientWithKeyImpl.h"
#include "GenerateSID.h"
#include "Logging.h"
#include "LicensingDb.h"
#include "FreeApplicationPolicyHelper.h"

#include <Licensing/Utils/LogFileHelper.h>
#include <Licensing/Utils/UsageJsonHelper.h>
#include <fstream>

#include <BeHttp/HttpError.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_LICENSING

ClientWithKeyImpl::ClientWithKeyImpl
    (
    Utf8StringCR accessKey,
    ClientInfoPtr clientInfo,
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
    m_clientInfo = clientInfo;
    m_dbPath = db_path;
    m_offlineMode = offlineMode;
    m_policyProvider = policyProvider;
    m_ulasProvider = ulasProvider;
    m_projectId = projectId;
    m_featureString = featureString;
    m_licensingDb = licensingDb;

    if(m_licensingDb == nullptr) // either pass in a mock, or initialize here
        m_licensingDb = std::make_unique<LicensingDb>(); // should this be make shared?

    m_correlationId = BeGuid(true).ToString();
    m_timeRetriever = TimeRetriever::Get();
    m_delayedExecutor = DelayedExecutor::Get();

    m_isAccessKeyValid = false; // assume invalid access key to start

    if (Utf8String::IsNullOrEmpty(projectId.c_str()))
        m_projectId = "00000000-0000-0000-0000-000000000000";

    if (m_offlineMode)
        {
        // Fake use m_offlineMode in order to avoid unused variable warning 
        // that is treated as error on clang compilers for iOS.
        }
    }

LicenseStatus ClientWithKeyImpl::StartApplication()
    {
    LOG.trace("ClientWithKeyImpl::StartApplication");

    if (SUCCESS != m_licensingDb->OpenOrCreate(m_dbPath))
        {
        LOG.error("ClientWithKeyImpl::StartApplication ERROR - Database creation failed.");
        return LicenseStatus::Error;
        }

    m_policy = GetPolicyToken();

    if (m_policy == nullptr)
        {
        LOG.error("ClientWithKeyImpl::StartApplication ERROR - Policy token object is null.");
        return LicenseStatus::Error;
        }

    // Get product status
    LicenseStatus licStatus = GetProductStatus();

    if ((LicenseStatus::Ok == licStatus) ||
        (LicenseStatus::Offline == licStatus) ||
        (LicenseStatus::Trial == licStatus))
        {
        // Begin heartbeats
        int64_t currentTimeUnixMs = m_timeRetriever->GetCurrentTimeAsUnixMillis();
        PolicyHeartbeat(currentTimeUnixMs);     // refresh policy
        }
    else
        {
        LOG.errorv("ClientImpl::StartApplication - LicenseStatus Returned: %d", licStatus);
        }

    return licStatus;
    }

BentleyStatus ClientWithKeyImpl::StopApplication()
    {
    LOG.trace("ClientWithKeyImpl::StopApplication");
    StopPolicyHeartbeat(); // This will stop Policy heartbeat

    m_licensingDb->Close();

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientWithKeyImpl::PolicyHeartbeat(int64_t currentTime)
    {
    LOG.debug("ClientWithKeyImpl::PolicyHeartbeat");

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
            // refresh policy
            auto policyToken = GetPolicyToken();
            if (policyToken != nullptr)
                {
                m_policy = policyToken;
                }

            CleanUpPolicies(); // if policy is expired or invalid, this will result in GetProductStatus() returning NotEntitled
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
bool ClientWithKeyImpl::ValidateAccessKey()
    {
    LOG.debug("ClientWithKeyImpl::ValidateAccessKey");

    auto responseJson = m_ulasProvider->GetAccessKeyInfo(m_accessKey).get();

    if (Json::Value::GetNull() == responseJson)
        {
        // call failed, so assume access key is unchanged since the last check
        LOG.error("ClientWithKeyImpl::ValidateAccessKey - Call to AccessKey service failed");
        return m_isAccessKeyValid;
        }

    if ("Success" != responseJson["status"].asString())
        {
        // unsuccessful if AccessKey is expired, not active, or not found - clear out policy from database
        LOG.errorv("ClientWithKeyImpl::ValidateAccessKey - %s", responseJson["msg"].asString().c_str());
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
std::shared_ptr<Policy> ClientWithKeyImpl::GetPolicyToken()
    {
    // TODO: rename this method? (and ClientImpl::GetPolicyToken) we are getting policy, not the token, and are storing the policy in the db...
    LOG.debug("ClientWithKeyImpl::GetPolicyToken");

    // returns nullptr if accesskey is inactive or expired
    auto policy = m_policyProvider->GetPolicyWithKey(m_accessKey).get();

    if (policy != nullptr)
        {
        StorePolicyInLicensingDb(policy);
        DeleteAllOtherPoliciesByKey(policy);
        }

    return policy;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus ClientWithKeyImpl::GetProductStatus(int requestedProductId)
    {
    LOG.debug("ClientWithKeyImpl::GetProductStatus");

    if (!ValidateAccessKey())
        {
        LOG.info("AccessKey missing from portal(deleted), expired, or revoked");
        return LicenseStatus::NotEntitled;
        }

    return ClientImpl::GetProductStatus(requestedProductId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientWithKeyImpl::DeleteAllOtherPoliciesByKey(std::shared_ptr<Policy> policy)
    {
    LOG.debug("ClientWithKeyImpl::DeleteAllOtherPoliciesByKey");

    m_licensingDb->DeleteAllOtherPolicyFilesByKey(policy->GetPolicyId(),
        policy->GetRequestData()->GetAccessKey());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy>> ClientWithKeyImpl::GetUserPolicies()
    {
    LOG.debug("ClientWithKeyImpl::GetUserPolicies");

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
