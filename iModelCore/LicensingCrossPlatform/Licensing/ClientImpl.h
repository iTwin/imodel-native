/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "LicensingDb.h"
#include "Policy.h"

#include <Licensing/Utils/TimeRetriever.h>
#include <Licensing/Utils/DelayedExecutor.h>
#include <Licensing/Utils/FeatureUserDataMap.h>

#include <folly/BeFolly.h>
#include <folly/futures/Future.h>

#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

#include "Providers/IBuddiProvider.h"
#include "Providers/IPolicyProvider.h"
#include "Providers/IUlasProvider.h"
#include "ILicensingDb.h"

// Log Posting Sources
#define LOGPOSTINGSOURCE_REALTIME           "RealTime"
#define LOGPOSTINGSOURCE_OFFLINE            "Offline"
#define LOGPOSTINGSOURCE_CHECKOUT           "Checkout"

// Heartbeat thread delay (MS)
#define HEARTBEAT_THREAD_DELAY_MS           1000

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ClientImpl> ClientImplPtr;

struct ClientImpl
{
protected:
    struct Feature
        {
        Utf8String featureId;
        Utf8String featureUserData;
        };

	WebServices::ClientInfoPtr m_clientInfo;
	WebServices::ConnectSignInManager::UserInfo m_userInfo;
    BeFileName m_dbPath;
    ITimeRetrieverPtr m_timeRetriever;
    IDelayedExecutorPtr m_delayedExecutor;
    ILicensingDbPtr m_licensingDb;
    Utf8String m_featureString;
    Utf8String m_projectId;
    Utf8String m_correlationId;
    bool m_offlineMode;
    bool m_stopApplicationCalled;
    IPolicyProviderPtr m_policyProvider;
    IUlasProviderPtr m_ulasProvider;

    // Policy
	std::shared_ptr<Policy> m_policy;
    void StorePolicyInLicensingDb(std::shared_ptr<Policy> policy);
    virtual std::shared_ptr<Policy> GetPolicyToken();

    std::list<std::shared_ptr<Policy>> GetPolicies();
    virtual std::list<std::shared_ptr<Policy>> GetUserPolicies();
    std::shared_ptr<Policy> SearchForPolicy(Utf8String requestedProductId="");
    bool HasOfflineGracePeriodStarted();
    int64_t GetDaysLeftInOfflineGracePeriod(std::shared_ptr<Policy> policy, Utf8String productId, Utf8String featureString);

    // Usage heartbeat
    int64_t m_lastRunningUsageheartbeatStartTime = 0;
    bool m_startUsageHeartbeat = true;
    bool m_stopUsageHeartbeat = false;
    bool m_usageHeartbeatStopped = false;

    void UsageHeartbeat(int64_t currentTime);
    void StopUsageHeartbeat();
    BentleyStatus RecordUsage();

    // Log Posting heartbeat
    int64_t m_lastRunningLogPostingheartbeatStartTime = 0;
    bool m_startLogPostingHeartbeat = true;
    bool m_stopLogPostingHeartbeat = false;
    bool m_logPostingHeartbeatStopped = false;
    int64_t m_logsPostingInterval = 60 * 60 * 1000; // one hour in milliseconds

    void LogPostingHeartbeat(int64_t currentTime);
    void StopLogPostingHeartbeat();
    //BentleyStatus PostUsageLogs();
    //BentleyStatus PostFeatureLogs();

    // Policy heartbeat
    int64_t m_lastRunningPolicyheartbeatStartTime = 0;
    bool m_startPolicyHeartbeat = true;
    bool m_stopPolicyHeartbeat = false;
    bool m_policyHeartbeatStopped = false;

    virtual void PolicyHeartbeat(int64_t currentTime);
    void StopPolicyHeartbeat();

    // Get the logging post source as a string
    Utf8String GetLoggingPostSource() const;

public:
	LICENSING_EXPORT ClientImpl() {};

    LICENSING_EXPORT ClientImpl
        (
		const WebServices::ConnectSignInManager::UserInfo& userInfo,
        WebServices::ClientInfoPtr clientInfo,
        BeFileNameCR db_path,
        bool offlineMode,
        IPolicyProviderPtr policyProvider,
        IUlasProviderPtr ulasProvider,
        Utf8StringCR projectId,
        Utf8StringCR featureString,
        ILicensingDbPtr licensingDb
        );

    // Usages
    LICENSING_EXPORT LicenseStatus StartApplication();
    LICENSING_EXPORT BentleyStatus StopApplication();
    //LICENSING_EXPORT folly::Future<folly::Unit> SendUsageLogs(BeFileNameCR usageCSV, Utf8StringCR ultId);

    //Features
    LICENSING_EXPORT BentleyStatus MarkFeature(Utf8StringCR featureId, FeatureUserDataMapPtr featureUserData);
    //LICENSING_EXPORT folly::Future<folly::Unit> SendFeatureLogs(BeFileNameCR featureCSV, Utf8StringCR ultId);

    // Policy
    LICENSING_EXPORT folly::Future<std::shared_ptr<Policy>> GetPolicy();

    // Product status
    virtual LICENSING_EXPORT LicenseStatus GetLicenseStatus();

    // Used in tests
    LICENSING_EXPORT ILicensingDb& GetLicensingDb();
    LICENSING_EXPORT void AddPolicyToDb(std::shared_ptr<Policy> policy);
	LICENSING_EXPORT std::shared_ptr<Policy> GetPolicyWithId(Utf8StringCR policyId);

	// clean up policies; used internally, but also used in unit tests
	LICENSING_EXPORT void CleanUpPolicies();
	LICENSING_EXPORT void DeleteAllOtherPoliciesByUser(std::shared_ptr<Policy> policy);

	virtual ~ClientImpl() {}; // make sure to cleanup
};

END_BENTLEY_LICENSING_NAMESPACE
