/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Licensing/ClientImpl.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "ClientInterface.h"
#include "UsageDb.h"
#include "Policy.h"

#include <Licensing/Utils/TimeRetriever.h>
#include <Licensing/Utils/DelayedExecutor.h>
#include <Licensing/Utils/FeatureUserDataMap.h>

#include <folly/BeFolly.h>
#include <folly/futures/Future.h>

#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

// Log Posting Sources
#define LOGPOSTINGSOURCE_REALTIME           "RealTime"
#define LOGPOSTINGSOURCE_OFFLINE            "Offline"
#define LOGPOSTINGSOURCE_CHECKOUT           "Checkout"

// Heartbeat thread delay (MS)
#define HEARTBEAT_THREAD_DELAY_MS           1000

BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ClientImpl> ClientImplPtr;

struct ClientImpl : ClientInterface
{
protected:
    struct Feature
        {
        Utf8String featureId;
        Utf8String featureUserData;
        };

    ClientInfoPtr m_clientInfo;
	ConnectSignInManager::UserInfo m_userInfo;
    BeFileName m_dbPath;
    std::shared_ptr<IConnectAuthenticationProvider> m_authProvider;
    IHttpHandlerPtr m_httpHandler;
    ITimeRetrieverPtr m_timeRetriever;
    IDelayedExecutorPtr m_delayedExecutor;
    std::unique_ptr<UsageDb> m_usageDb;
    Utf8String m_featureString;
    Utf8String m_projectId;
    Utf8String m_correlationId;
    bool m_offlineMode;
    Utf8String m_accessToken;
    bool m_stopApplicationCalled;

    // Policy
	std::shared_ptr<Policy> m_policy;
    void StorePolicyInUsageDb(std::shared_ptr<Policy> policy);
    std::shared_ptr<Policy> GetPolicyToken();

    std::list<std::shared_ptr<Policy>> GetPolicies();
    std::list<std::shared_ptr<Policy>> GetUserPolicies();
    std::shared_ptr<Policy> SearchForPolicy(Utf8String requestedProductId="");
    folly::Future<Utf8String> PerformGetPolicyRequest();
    bool HasOfflineGracePeriodStarted();
    int64_t GetDaysLeftInOfflineGracePeriod(std::shared_ptr<Policy> policy, Utf8String productId, Utf8String featureString);

    // Usage heartbeat
    int64_t m_lastRunningUsageheartbeatStartTime = 0;
    bool m_startUsageHeartbeat = true;
    bool m_stopUsageHeartbeat = false;
    bool m_usageHeartbeatStopped = false;

    void UsageHeartbeat(int64_t currentTime);
    void UsageHeartbeatRealTime(int64_t currentTime);
    void StopUsageHeartbeat();
    BentleyStatus RecordUsage();

    // Log Posting heartbeat
    int64_t m_lastRunningLogPostingheartbeatStartTime = 0;
    bool m_startLogPostingHeartbeat = true;
    bool m_stopLogPostingHeartbeat = false;
    bool m_logPostingHeartbeatStopped = false;

    void LogPostingHeartbeat(int64_t currentTime);
    void StopLogPostingHeartbeat();
    BentleyStatus PostUsageLogs();
    BentleyStatus PostFeatureLogs();

    // Policy heartbeat
    int64_t m_lastRunningPolicyheartbeatStartTime = 0;
    bool m_startPolicyHeartbeat = true;
    bool m_stopPolicyHeartbeat = false;
    bool m_policyHeartbeatStopped = false;

    void PolicyHeartbeat(int64_t currentTime);
    void StopPolicyHeartbeat();

    // Get the logging post source as a string
    Utf8String GetLoggingPostSource() const;

public:
	LICENSING_EXPORT ClientImpl() {};

    LICENSING_EXPORT ClientImpl
        (
		const ConnectSignInManager::UserInfo& userInfo,
        ClientInfoPtr clientInfo,
        std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
        BeFileNameCR db_path,
        bool offlineMode,
        Utf8String projectId,
        Utf8String featureString,
        IHttpHandlerPtr httpHandler
        );

    // Usages
    LICENSING_EXPORT LicenseStatus StartApplication();
    LICENSING_EXPORT BentleyStatus StopApplication();
    LICENSING_EXPORT folly::Future<folly::Unit> SendUsageRealtime();
    LICENSING_EXPORT folly::Future<folly::Unit> SendUsageLogs(BeFileNameCR usageCSV, Utf8StringCR ultId);

    //Features
    LICENSING_EXPORT BentleyStatus MarkFeature(Utf8String featureId, FeatureUserDataMap* featureUserData);
    LICENSING_EXPORT folly::Future<folly::Unit> SendFeatureLogs(BeFileNameCR featureCSV, Utf8StringCR ultId);

    // Policy
    LICENSING_EXPORT folly::Future<Utf8String> GetCertificate();
    LICENSING_EXPORT folly::Future<std::shared_ptr<Policy>> GetPolicy();
    
    // Product status
    LICENSING_EXPORT LicenseStatus GetProductStatus(int requestedProductId = -1);

    // Used in tests
    LICENSING_EXPORT UsageDb& GetUsageDb();
	LICENSING_EXPORT void AddPolicyToDb(std::shared_ptr<Policy> policy) { StorePolicyInUsageDb(policy); };
	LICENSING_EXPORT std::shared_ptr<Policy> GetPolicyWithId(Utf8StringCR policyId);

	// clean up policies; used internally, but also used in unit tests
	LICENSING_EXPORT void CleanUpPolicies();
	LICENSING_EXPORT void DeleteAllOtherUserPolicies(std::shared_ptr<Policy> policy);
};

END_BENTLEY_LICENSING_NAMESPACE
