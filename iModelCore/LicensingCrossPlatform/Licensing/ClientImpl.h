/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>
#include <Licensing/ApplicationInfo.h>

#include "LicensingDb.h"
#include "Policy.h"

#include <Licensing/Utils/TimeRetriever.h>
#include <Licensing/Utils/FeatureUserDataMap.h>

#include <folly/BeFolly.h>
#include <folly/futures/Future.h>

#include <WebServices/Connect/IConnectAuthenticationProvider.h>
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

    ApplicationInfoPtr m_applicationInfo;
    WebServices::ConnectSignInManager::UserInfo m_userInfo;
    BeFileName m_dbPath;
    ITimeRetrieverPtr m_timeRetriever;
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
    void StorePolicyInLicensingDb(std::shared_ptr<Policy> policy, bool IsCheckout = false, Utf8StringCR projectId = "");
    void StoreProjectPolicyInLicensingDb(std::shared_ptr<Policy> policy, Utf8StringCR projectId);
    virtual std::shared_ptr<Policy> GetPolicyToken();
    virtual std::shared_ptr<Policy> GetProjectPolicyToken(Utf8StringCR projectId);

    virtual std::list<std::shared_ptr<Policy>> GetValidUserPolicies();
	virtual std::list<std::shared_ptr<Policy>> GetValidCheckouts();
    std::shared_ptr<Policy> SearchForPolicy(Utf8String requestedProductId);
	std::shared_ptr<Policy> SearchForCheckout(Utf8String productId, Utf8String featureString);
    bool HasOfflineGracePeriodStarted();
    int64_t GetDaysLeftInOfflineGracePeriod(std::shared_ptr<Policy> policy, Utf8String productId, Utf8String featureString);

    // Usage heartbeat
    std::atomic<int64_t> m_lastRunningUsageHeartbeatStartTime{ 0 };
    bool m_startUsageHeartbeat = true;
    std::atomic_bool m_stopUsageHeartbeatThread{ false };
    std::atomic_bool m_usageHeartbeatThreadStopped{ false };

    void UsageHeartbeat();
	void StartUsageHeartbeat(); 
	void StopUsageHeartbeat();
    BentleyStatus RecordUsage();
    Utf8String m_usageStartTime;

    // Log Posting heartbeat
    std::atomic<int64_t> m_lastRunningLogPostingHeartbeatStartTime{ 0 };
    bool m_startLogPostingHeartbeat = true;
    std::atomic_bool m_stopLogPostingHeartbeatThread{ false };
    std::atomic_bool m_logPostingHeartbeatThreadStopped{ false };
    int64_t m_logsPostingInterval = 60 * 60 * 1000; // one hour in milliseconds

    void LogPostingHeartbeat();
	void StartLogPostingHeartbeat();
    void StopLogPostingHeartbeat();

    // Policy heartbeat
    std::atomic<int64_t> m_lastRunningPolicyHeartbeatStartTime{ 0 };
    bool m_startPolicyHeartbeat = true;
    std::atomic_bool m_stopPolicyHeartbeatThread{ false };
    std::atomic_bool m_policyHeartbeatThreadStopped{ false };

    virtual void PolicyHeartbeat();
	void StartPolicyHeartbeat();
	void StopPolicyHeartbeat();

    // Get the logging post source as a string
    Utf8String GetLoggingPostSource() const;

    void CallOnInterval(std::atomic_bool& stopThread, std::atomic_bool& isFinished, std::atomic<int64_t>& lastRunStartTime, size_t interval, std::function<void(void)> func);

	LicenseStatus StartApplicationGeneric(std::function<std::shared_ptr<Policy>()> getPolicy);

public:
    LICENSING_EXPORT ClientImpl() {};

    LICENSING_EXPORT ClientImpl
        (
        const WebServices::ConnectSignInManager::UserInfo& userInfo,
        ApplicationInfoPtr applicationInfo,
        BeFileNameCR db_path,
        bool offlineMode,
        IPolicyProviderPtr policyProvider,
        IUlasProviderPtr ulasProvider,
        Utf8StringCR projectId,
        Utf8StringCR featureString,
        ILicensingDbPtr licensingDb
        );

	bool ValidateParamsAndDB();

    //Validation of params and check for valid checkout policy
    BentleyM0200::Licensing::LicenseStatus CheckoutCheck(bool &retflag);

    // Usages
    LICENSING_EXPORT LicenseStatus StartApplication();
    LICENSING_EXPORT LicenseStatus StartApplicationForProject(Utf8StringCR projectId);
    LICENSING_EXPORT BentleyStatus StopApplication();

    //Features
    LICENSING_EXPORT BentleyStatus MarkFeature(Utf8StringCR featureId, FeatureUserDataMapPtr featureUserData);

    // Policy
    LICENSING_EXPORT folly::Future<std::shared_ptr<Policy>> GetPolicy();
    LICENSING_EXPORT folly::Future<std::shared_ptr<Policy>> GetPolicy(Utf8StringCR projectId);

    // Product status
    virtual LICENSING_EXPORT LicenseStatus GetLicenseStatus();
    LICENSING_EXPORT int64_t GetTrialDaysRemaining();

	//Import .belic files
	virtual LICENSING_EXPORT int64_t ImportCheckout(BeFileNameCR filepath);
    virtual LICENSING_EXPORT BentleyStatus DeleteLocalCheckout(Utf8StringCR productId);

    // Used in tests
    LICENSING_EXPORT ILicensingDb& GetLicensingDb();
    LICENSING_EXPORT void AddPolicyToDb(std::shared_ptr<Policy> policy, bool IsCheckout = false);
    LICENSING_EXPORT std::shared_ptr<Policy> GetPolicyWithId(Utf8StringCR policyId);

    // clean up policies; used internally, but also used in unit tests
    LICENSING_EXPORT void CleanUpPolicies();
    LICENSING_EXPORT void DeleteAllOtherPoliciesByUser(std::shared_ptr<Policy> policy);
    LICENSING_EXPORT void DeleteAllOtherPoliciesByProject(std::shared_ptr<Policy> policy, Utf8StringCR projectId);

    virtual ~ClientImpl() {}; // make sure to cleanup
    };

END_BENTLEY_LICENSING_NAMESPACE
