/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Utils/UrlProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

#include <Bentley/WString.h>
#include <Bentley/Tasks/TaskScheduler.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include "UrlProvider.h"
#include "BuddiClient.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

USING_NAMESPACE_BENTLEY_TASKS 

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Brad.Hadden    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
//! UrlProvider is a class that allows getting remotely configured service URLs based on
//! initialized environment (DEV, QA or PROD/RELEASE). Environment is set of services working
//! in isolated system. Switching between environments is only needed for testing.
//! All released software should point to PROD/RELEASE environment services by default.
struct UrlProvider
    {
    public:
        // Used for array indexes! Values must increment from 0
        enum Environment
            {
            Dev = 0,
            Qa,
            Release,
            Perf
            };

        struct UrlDescriptor
            {
            public:
                using Registry = bmap<Utf8String, const UrlDescriptor*>;

            private:
                Utf8String m_name;
                Utf8String m_defaultUrls[4];
                Registry* m_registry;

            public:
                //! Constructur for internal use
                LICENSING_EXPORT UrlDescriptor(Utf8CP name, Utf8CP devUrl, Utf8CP qaUrl, Utf8CP prodUrl, Utf8CP perfUrl, Registry* registry);
                LICENSING_EXPORT ~UrlDescriptor();

                //! Get URL name used to identify it
                LICENSING_EXPORT Utf8StringCR GetName() const;

                //! Retrieve cached, server configured or default configured URL.
                //! Will update URL in cache periodically in background.
                LICENSING_EXPORT Utf8String Get() const;

                //! Returns a BUDDI URI which can be resolved back into this descriptor
                LICENSING_EXPORT Utf8String GetBuddiUri() const;
            };

        LICENSING_EXPORT static int64_t DefaultTimeout;

    private:
        static bool s_isInitialized;
        static UrlProvider::Environment s_env;
        static int64_t s_cacheTimeoutMs;
        static IJsonLocalState* s_localState;
        static IBuddiClientPtr s_buddi;
        static IHttpHandlerPtr s_customHandler;
        static ITaskSchedulerPtr s_thread;

    private:
        static AsyncTaskPtr<Utf8String> CacheBuddiUrl(Utf8StringCR urlName);
        static Utf8String GetUrl(Utf8StringCR urlName, const Utf8String* defaultUrls);

    public:
        //! Initialize UrlProvider for current session.
        //! @param env - environment to get URLs for. Changing environment will clear URL cache
        //! @param cacheTimeoutMs - maximum time URL is cached in milliseconds. Defaults to 24 hours.
        //! @param customlocalState - custom local state for caching URLs
        //! @param customBuddi - custom buddi client for requesting URLs
        //! @param customHandler - custom handler
        //! @param customScheduler - custom scheduler for async code
        LICENSING_EXPORT static void Initialize(
            Environment env,
            int64_t cacheTimeoutMs = DefaultTimeout,
            IJsonLocalState* customlocalState = nullptr,
            IBuddiClientPtr customBuddi = nullptr,
            IHttpHandlerPtr customHandler = nullptr,
            ITaskSchedulerPtr customScheduler = nullptr
        );

        LICENSING_EXPORT static void Uninitialize();
        LICENSING_EXPORT static void SetHttpHandler(IHttpHandlerPtr customHandler);
        //! Should be used with all requests to provided URLs!
        //! Returns handler that will configure requests depending on environment.
        //! Will setup certificate validation appropriately.
        LICENSING_EXPORT static IHttpHandlerPtr GetSecurityConfigurator(IHttpHandlerPtr customHandler = nullptr);

        //! Clear local URL cache
        LICENSING_EXPORT static void CleanUpUrlCache();

        //! Get available UrlDescriptors
        LICENSING_EXPORT static bset<const UrlProvider::UrlDescriptor*> GetUrlRegistry();

        //! Get connect environment
        LICENSING_EXPORT static Environment GetEnvironment();

        //! Set connect environment
        //! @param env environment
        LICENSING_EXPORT static void SetEnvironment(Environment env);

        //! Get connect environment as string
        //! @param env environment
        LICENSING_EXPORT static Utf8String ToEnvironmentString(Environment env);

        //! Resolve UrlDescriptor by its BUDDI URI.
        //! @param uri - BUDDI URI retrieved from UrlDescriptor::GetBuddiUri call
        //! @return If succeeds, returns a pointer to UrlDescriptor. Otherwise, nullptr.
        LICENSING_EXPORT static const UrlDescriptor* ResolveUrlDescriptor(Utf8StringCR uri);

        //! Available URLs
        struct Urls
            {
            LICENSING_EXPORT static const UrlDescriptor BIMReviewShare;
            LICENSING_EXPORT static const UrlDescriptor ConnectedContext;
            LICENSING_EXPORT static const UrlDescriptor ConnectEula;
            LICENSING_EXPORT static const UrlDescriptor ConnectTermsOfServiceUrl;
            LICENSING_EXPORT static const UrlDescriptor ConnectProjectUrl;
            LICENSING_EXPORT static const UrlDescriptor ConnectWsgGlobal;
            LICENSING_EXPORT static const UrlDescriptor ConnectWsgPersonalPublishing;
            LICENSING_EXPORT static const UrlDescriptor ConnectWsgProjectContent;
            LICENSING_EXPORT static const UrlDescriptor ConnectWsgProjectShare;
            LICENSING_EXPORT static const UrlDescriptor ConnectWsgPunchList;
            LICENSING_EXPORT static const UrlDescriptor ConnectWsgClashIssues;
            LICENSING_EXPORT static const UrlDescriptor ConnectWsgSharedContent;
            LICENSING_EXPORT static const UrlDescriptor ConnectWsgRepositoryFederation;
            LICENSING_EXPORT static const UrlDescriptor ConnectForms;
            LICENSING_EXPORT static const UrlDescriptor iModelHubApi;
            LICENSING_EXPORT static const UrlDescriptor ImsStsAuth;
            LICENSING_EXPORT static const UrlDescriptor ImsActiveStsDelegationService;
            LICENSING_EXPORT static const UrlDescriptor ImsFederatedAuth;
            LICENSING_EXPORT static const UrlDescriptor Passport;
            LICENSING_EXPORT static const UrlDescriptor UsageAndFeatureTrackingAPI;
            LICENSING_EXPORT static const UrlDescriptor UsageTracking;
            LICENSING_EXPORT static const UrlDescriptor UsageLoggingServices;
            LICENSING_EXPORT static const UrlDescriptor UsageLoggingServicesLocation;
            LICENSING_EXPORT static const UrlDescriptor EntitlementPolicyService;
            LICENSING_EXPORT static const UrlDescriptor ConnectXmpp;
            LICENSING_EXPORT static const UrlDescriptor ImsActiveSTSHelperService;
            LICENSING_EXPORT static const UrlDescriptor ImsPassiveAuthUrl;
            LICENSING_EXPORT static const UrlDescriptor iModelBridgeConfiguration;
            LICENSING_EXPORT static const UrlDescriptor RecommendationServiceUrl;
            LICENSING_EXPORT static const UrlDescriptor ProjectSharedFederatedUIURL;
            LICENSING_EXPORT static const UrlDescriptor IMSOpenID;
            LICENSING_EXPORT static const UrlDescriptor ConnectProductSettingsService;
            };
    };

    END_BENTLEY_LICENSING_NAMESPACE
