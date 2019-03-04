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
#include "UsageDb.h"
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
    IBuddiProviderPtr buddiProvider,
    IPolicyProviderPtr policyProvider,
    IUlasProviderPtr ulasProvider,
    Utf8StringCR projectId,
    Utf8StringCR featureString,
    IHttpHandlerPtr httpHandler,
    IUsageDbPtr usageDb
    )
    {
    m_userInfo = ConnectSignInManager::UserInfo();

    m_accessKey = accessKey;
    m_clientInfo = clientInfo;
    m_dbPath = db_path;
    m_offlineMode = offlineMode;
    m_buddiProvider = buddiProvider;
    m_policyProvider = policyProvider;
    m_ulasProvider = ulasProvider;
    m_projectId = projectId;
    m_featureString = featureString;
    m_httpHandler = httpHandler;

    if(m_usageDb == nullptr) // either pass in a mock, or initialize here
        m_usageDb = std::make_unique<UsageDb>(); // should this be make shared?

    m_correlationId = BeGuid(true).ToString();
    m_timeRetriever = TimeRetriever::Get();
    m_delayedExecutor = DelayedExecutor::Get();

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
    LOG.trace("ClientWithKey::StartApplication");

    if (SUCCESS != m_usageDb->OpenOrCreate(m_dbPath))
        {
        LOG.error("ClientWithKeyImpl::StartApplication ERROR - Database creation failed.");
        return LicenseStatus::Error;
        }

    // TODO: implement UsageDb caching, for now just get each time
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
        //UsageHeartbeat(currentTimeUnixMs);      // Record usage to database
        //LogPostingHeartbeat(currentTimeUnixMs); // calls post usage logs and post feature logs
        PolicyHeartbeat(currentTimeUnixMs);     // refresh policy, check if offline grace period should start
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

    m_lastRunningPolicyheartbeatStartTime = 0;      // This will stop Policy heartbeat
    m_lastRunningUsageheartbeatStartTime = 0;       // This will stop Usage heartbeat
    m_lastRunningLogPostingheartbeatStartTime = 0;  // This will stop log posting heartbeat

    if (m_usageDb->GetUsageRecordCount() > 0)
        m_ulasProvider->PostUsageLogs(*m_usageDb, m_policy);

    if (m_usageDb->GetFeatureRecordCount() > 0)
        m_ulasProvider->PostFeatureLogs(*m_usageDb, m_policy);

    m_usageDb->Close();

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy> ClientWithKeyImpl::GetPolicyToken()
    {
    // TODO: rename this method? (and ClientImpl::GetPolicyToken) we are getting policy, not the token, and are storing the policy in the db...
    LOG.debug("ClientWithKeyImpl::GetPolicyToken");

    auto policy = m_policyProvider->GetPolicyWithKey(m_accessKey).get();

    // TODO: implement usageDb for policies for accessKey (right now uses UserId)
    //if (policy != nullptr)
    //    {
    //    StorePolicyInUsageDb(policy);
    //    DeleteAllOtherUserPolicies(policy);
    //    }

    return policy;
    }

LicenseStatus ClientWithKeyImpl::GetProductStatus(int requestedProductId)
    {
    // NOTE: This override is temporary, we will implement DB caching soon
    LOG.debug("ClientWithKeyImpl::GetProductStatus");

    // productId to look for; allow custom product to be searched for (for testing purposes)
    Utf8String productId;
    if (requestedProductId < 0)
        productId = m_clientInfo->GetApplicationProductId();
    else
        productId = Utf8String(std::to_string(requestedProductId).c_str());
    // get valid policy for user
    
    //auto policy = SearchForPolicy(productId);
    auto policy = GetPolicyToken(); // for now: get from entitlements instead of caching
    
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


//folly::Future<folly::Unit> ClientWithKeyImpl::SendUsageRealtimeWithKey()
//    {
//    LOG.trace("ClientWithKeyImpl::SendUsageRealtimeWithKey");
//
//    auto url = m_buddiProvider->UlasRealtimeLoggingBaseUrl();
//
//    url += "/" + m_accessKey;
//
//    std::ofstream logfile("D:/performSendUsageRealtimeWithKey.txt");
//    logfile << url << std::endl;
//    logfile.close();
//
//    HttpClient client(nullptr, m_httpHandler);
//    auto uploadRequest = client.CreateRequest(url, "POST");
//    //uploadRequest.GetHeaders().SetValue("authorization", "Bearer " + m_accessTokenString);
//    uploadRequest.GetHeaders().SetValue("content-type", "application/json; charset=utf-8");
//
//    // create Json body
//    auto jsonBody = UsageJsonHelper::CreateJsonRandomGuids
//        (
//        m_clientInfo->GetDeviceId(),
//        m_featureString,
//        m_clientInfo->GetApplicationVersion(),
//        m_projectId,
//        std::stoi(m_clientInfo->GetApplicationProductId().c_str())
//        );
//
//    uploadRequest.SetRequestBody(HttpStringBody::Create(jsonBody));
//
//    return uploadRequest.Perform().then(
//        [=](Response response)
//        {
//        if (!response.IsSuccess())
//            {
//            std::ofstream logfile("D:/performSendUsageRealtimeWithKeyResponse.txt");
//            logfile << response.GetBody().AsString() << std::endl;
//            logfile << (int)response.GetHttpStatus() << std::endl;
//            logfile.close();
//            throw HttpError(response);
//            }
//        std::ofstream logfile("D:/performSendUsageRealtimeWithKeyResponse.txt");
//        logfile << response.GetBody().AsString() << std::endl;
//        logfile << (int)response.GetHttpStatus() << std::endl;
//        logfile.close();
//        return folly::makeFuture();
//        });
//    }
