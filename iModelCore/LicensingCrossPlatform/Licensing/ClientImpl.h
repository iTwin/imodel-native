/*--------------------------------------------------------------------------------------+
 |
 |     $Source: LicensingCrossPlatform/Licensing/ClientImpl.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "UsageDb.h"
#include "PolicyToken.h"
#include "Policy.h"
#include "PolicyHelper.h"

#include <Licensing/Utils/TimeRetriever.h>
#include <Licensing/Utils/DelayedExecutor.h>

#include <folly/BeFolly.h>
#include <folly/futures/Future.h>

#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

#define LICENSE_CLIENT_SCHEMA_VERSION       1.0

#if defined(DEBUG)
#define USAGE_HEARTBEAT_INTERVAL_MS         5*1000 
#define POLICY_HEARTBEAT_INTERVAL_MS        30*60*1000
#define LOG_POSTING_HEARTBEAT_INTERVAL_MS   30*60*1000
#else
#define USAGE_HEARTBEAT_INTERVAL_MS         60*1000
#define POLICY_HEARTBEAT_INTERVAL_MS        60*60*1000
#define LOG_POSTING_HEARTBEAT_INTERVAL_MS   60*60*1000
#endif // DEBUG)

// Log Posting Sources
#define LOGPOSTINGSOURCE_REALTIME           "RealTime"
#define LOGPOSTINGSOURCE_OFFLINE            "Offline"
#define LOGPOSTINGSOURCE_CHECKOUT           "Checkout"

BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ClientImpl> ClientImplPtr;

struct ClientImpl
{
private:
    enum LogPostingSource { RealTime, Offline, Checkout };

    ClientInfoPtr m_clientInfo;
	ConnectSignInManager::UserInfo m_userInfo;
    BeFileName m_dbPath;
    std::shared_ptr<IConnectAuthenticationProvider> m_authProvider;
    IHttpHandlerPtr m_httpHandler;
    ITimeRetrieverPtr m_timeRetriever;
    IDelayedExecutorPtr m_delayedExecutor;
    int64_t m_lastRunningUsageheartbeatStartTime = 0;
    int64_t m_lastRunningPolicyheartbeatStartTime = 0;
    int64_t m_lastRunningLogPostingheartbeatStartTime = 0;
    std::unique_ptr<UsageDb> m_usageDb;
    Utf8String m_featureString;
    Utf8String m_projectId;
    Utf8String m_correlationId;
    bool m_offlineMode;
    std::shared_ptr<PolicyToken> m_policyToken;

    void UsageHeartbeat(int64_t currentTime);
    void PolicyHeartbeat(int64_t currentTime);
    void LogPostingHeartbeat(int64_t currentTime);

    std::list<std::shared_ptr<Policy>> GetPolicies();
    std::list<std::shared_ptr<Policy>> GetUserPolicies();
    std::shared_ptr<Policy> SearchForPolicy(Utf8String requestedProductId="");

    bool HasOfflineGracePeriodStarted();
    int64_t GetDaysLeftInOfflineGracePeriod(std::shared_ptr<Policy> policy, Utf8String productId, Utf8String featureString);

    void StorePolicyTokenInUsageDb(std::shared_ptr<PolicyToken> policyToken);
    BentleyStatus RecordUsage();
    std::shared_ptr<PolicyToken> GetPolicyToken();
    BentleyStatus PostUsageLogs();
    folly::Future<Utf8String> PerformGetPolicyRequest();
    Utf8String GetLoggingPostSource(LogPostingSource lps) const;

public:
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

    LICENSING_EXPORT LicenseStatus StartApplication(); 
    LICENSING_EXPORT BentleyStatus StopApplication();
    LICENSING_EXPORT LicenseStatus GetProductStatus(int requestedProductId=-1);

    // usageSCV usage file to send
    // The company ID in SAP. // TODO: figure out where to get this one from.
    LICENSING_EXPORT folly::Future<folly::Unit> SendUsage(BeFileNameCR usageSCV, Utf8StringCR ultId);

    LICENSING_EXPORT folly::Future<Utf8String> GetCertificate();

    LICENSING_EXPORT folly::Future<std::shared_ptr<PolicyToken>> GetPolicy();

    // Used in tests
    LICENSING_EXPORT UsageDb& GetUsageDb();
    LICENSING_EXPORT void AddPolicyTokenToDb(std::shared_ptr<PolicyToken> policyToken) { StorePolicyTokenInUsageDb(policyToken); };
	LICENSING_EXPORT std::shared_ptr<Policy> GetPolicyWithId(Utf8StringCR policyId);

	// clean up policies; used internally, but also used in unit tests
	LICENSING_EXPORT void CleanUpPolicies();
};

END_BENTLEY_LICENSING_NAMESPACE
