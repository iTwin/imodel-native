/*--------------------------------------------------------------------------------------+
|
|     $Source: Configuration/UrlProvider.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <Bentley/BeTimeUtilities.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <BeHttp/HttpConfigurationHandler.h>
#include "../Client/Logging.h"

#define LOCAL_STATE_NAMESPACE   "UrlCache"
#define LOCAL_STATE_ENVIRONMENT "Environment"
#define RECORD_Url              "URL"
#define RECORD_TimeCached       "TimeCached"

#define Environment_Release     "PROD"
#define Environment_Qa          "QA"
#define Environment_Dev         "DEV"
#define Environment_Perf        "PERF"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_WEBSERVICES

int64_t                     UrlProvider::DefaultTimeout = 24 * 3600 * 1000;
bool                        UrlProvider::s_isInitialized = false;
UrlProvider::Environment    UrlProvider::s_env = UrlProvider::Environment::Release;
int64_t                     UrlProvider::s_cacheTimeoutMs = 0;
IBuddiClientPtr             UrlProvider::s_buddi;
IJsonLocalState*            UrlProvider::s_localState = nullptr;
ITaskSchedulerPtr           UrlProvider::s_thread;

IHttpHandlerPtr UrlProvider::s_customHandler;

// Region IDs from the buddi.bentley.com
uint32_t s_regionsId[4] = {
    103,    // Region "Bentley Corporate Network - DEV"
    102,    // Region "Bentley Corporate Network - QA"
    0,      // No region for PROD - use BUDDI non-regional URLs
    294     // Region "Performance Testing"
    };

// Managed urls.
// ADDING NEW URL:
//  Submit help request "I Need Something"-> "Be Connect" -> "Buddi" on help.bentley.com.
//  Request should contain 4 URLs and their maching regions. See s_regionsId for using proper region.
//  Got to buddi.bentley.com to dobule check if correct URLs were added.
// Will log errors if buddi.bentley.com does not have required URL for given environment (region).
UrlProvider::UrlDescriptor::Registry s_urlRegistry;

const UrlProvider::UrlDescriptor UrlProvider::Urls::BIMReviewShare(
    "BIMReviewShare",
    "https://qa-bimreviewshare.bentley.com",
    "https://qa-bimreviewshare.bentley.com",
    "https://bimreviewshare.bentley.com",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectEula(
    "Mobile.ConnectEula",
    "https://dev-agreement-eus.cloudapp.net/rest",
    "https://qa-connect-agreement.bentley.com/rest",
    "https://connect-agreement.bentley.com/rest",
    "https://perf-agreement-eus.cloudapp.net/rest",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectTermsOfServiceUrl(
    "ConnectTermsOfServiceUrl",
    "https://dev-agreementportal-eus.cloudapp.net/AgreementApp/Home/Eula/View/ReadOnly/BentleyConnect",
    "https://qa-connect-agreementportal.bentley.com/AgreementApp/Home/Eula/View/ReadOnly/BentleyConnect",
    "https://connect-agreementportal.bentley.com/AgreementApp/Home/Eula/view/readonly/BentleyConnect",
    "https://perf-agreementportal-eus.cloudapp.net/AgreementApp/Home/Eula/view/readonly/BentleyConnect",
    &s_urlRegistry
     );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectProjectUrl(
    "Mobile.ConnectProjectUrl",
    "https://dev-webportal-eus.cloudapp.net/project/index?projectId=",
    "https://qa-connect-webportal.bentley.com/project/index?projectId=",
    "https://connect.bentley.com/project/index?projectId=",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgGlobal(
    "Mobile.ConnectWsgGlobal",
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-connect-wsg20.bentley.com",
    "https://connect-wsg20.bentley.com",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectedContext(
    "CONNECTEDContextService.URL",
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-connect-wsg20.bentley.com",
    "https://connect-wsg20.bentley.com",
    "https://perf-wsg20-eus.cloudapp.net",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgPersonalPublishing(
    "Mobile.ConnectWsgPersonalPublishing",
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-connect-wsg20.bentley.com",
    "https://connect-wsg20.bentley.com",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgProjectContent(
    "Mobile.ConnectWsgProjectContent",
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-connect-wsg20.bentley.com",
    "https://connect-wsg20.bentley.com",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgProjectShare(
    "ProjectShare.Url",
    "https://dev-projectsharestorage-eus.cloudapp.net",
    "https://qa-connect-projectsharestorage.bentley.com",
    "https://connect-projectsharestorage.bentley.com",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgPunchList(
    "Mobile.PunchListWsg",
    "https://dev-punchlist-eus.cloudapp.net",
    "https://qa-connect-punchlist.bentley.com",
    "https://connect-punchlist.bentley.com",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgClashIssues(
    "Mobile.ClashIssuesWsg",
    "https://dev-punchlist-eus.cloudapp.net",
    "https://qa-connect-punchlist.bentley.com",
    "https://connect-punchlist.bentley.com",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgSharedContent(
    "Mobile.ConnectWsgSharedContent",
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-connect-wsg20.bentley.com",
    "https://connect-wsg20.bentley.com",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgRepositoryFederation(
    "RepositoryFederationService.URL",
    "https://dev-bcsf.bentley.com/ProjectGateway/Wsg",
    "https://qa-bcsf.bentley.com/ProjectGateway/Wsg",
    "https://bcsf.bentley.com/ProjectGateway/Wsg",
    "https://perf-bcsf.bentley.com/ProjectGateway/Wsg",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectForms(
    "Mobile.ConnectForms",
    "https://dev-formswsg-eus.cloudapp.net",
    "https://qa-connect-formswsg.bentley.com",
    "https://connect-formswsg.bentley.com",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::iModelHubApi(
    "iModelHubApi",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ImsStsAuth(
    "Mobile.ImsStsAuth",
    "https://qa-ims.bentley.com/rest/ActiveSTSService/json/IssueEx",
    "https://qa-ims.bentley.com/rest/ActiveSTSService/json/IssueEx",
    "https://ims.bentley.com/rest/ActiveSTSService/json/IssueEx",
    "https://qa-ims.bentley.com/rest/ActiveSTSService/json/IssueEx",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ImsActiveStsDelegationService(
    "ActiveSTSDelegationServiceUrl",
    "https://qa-ims.bentley.com/rest/DelegationSTSService",
    "https://qa-ims.bentley.com/rest/DelegationSTSService",
    "https://ims.bentley.com/rest/DelegationSTSService",
    "https://qa-ims.bentley.com/rest/DelegationSTSService",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ImsFederatedAuth(
    "IMS.FederatedAuth.Url",
    "https://qa-ims.bentley.com/",
    "https://qa-ims.bentley.com/",
    "https://ims.bentley.com/",
    "https://qa-ims.bentley.com/",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::Passport(
    "Mobile.Passport",
    "https://qa-ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport",
    "https://qa-ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport",
    "https://ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::UsageAndFeatureTrackingAPI(
    "Mobile.UsageAndFeatureTrackingAPI",
    "https://qa-selectserver.bentley.com/LicensingProxy",
    "https://qa-selectserver.bentley.com/LicensingProxy",
    "https://SELECTserver.bentley.com/LicensingProxy",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::UsageTracking(
    "Mobile.UsageTracking",
    "https://licenseXM.bentley.com/bss/ws/mobile",
    "https://licenseXM.bentley.com/bss/ws/mobile",
    "https://SELECTserver.bentley.com/bss/ws/mobile",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::UsageLoggingServices(
    "UsageLoggingServices.Url",
    "https://dev-ulas-eus2-sfc01.bentley.com/",
    "https://qa-ulas-eus2-sfc01.bentley.com/",
    "https://ulas.bentley.com/",
    "https://perf-ulas-eus2-sfc01.bentley.com/",
    &s_urlRegistry
);

const UrlProvider::UrlDescriptor UrlProvider::Urls::UsageLoggingServicesLocation(
    "UsageLoggingServices.Location.Url",
    "https://dev-ulas-eus2-sfc01.bentley.com/api/v1/location",
    "https://qa-ulas-eus2-sfc01.bentley.com/api/v1/location",
    "https://ulas.bentley.com/api/v1/location",
    "https://perf-ulas-eus2-sfc01.bentley.com/Bentley.ULAS.LocationService/LocationSvcWebApi",
    &s_urlRegistry
);

const UrlProvider::UrlDescriptor UrlProvider::Urls::EntitlementPolicyService(
    "EntitlementPolicyService.Url",
    "https://dev-ulas-eus2-sfc01.bentley.com/v1/policyservice/api",
    "https://qa-ulas-eus2-sfc01.bentley.com/v1/policyservice/api",
    "https://entitlement-search.bentley.com/PolicyService/api",
    "https://qa-ulas-eus2-sfc01.bentley.com/v1/PolicyService/api",
    &s_urlRegistry
);

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectXmpp(
    "BeXMPP",
    "dev-xmppcollab-eus.cloudapp.net",
    "qa-connect-xmppcollab.bentley.com",
    "connect-xmppcollab.bentley.com",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ImsActiveSTSHelperService(
    "ActiveSTSHelperServiceUrl",
    "https://qa-ims.bentley.com/rest/STSHelperService",
    "https://qa-ims.bentley.com/rest/STSHelperService",
    "https://ims.bentley.com/rest/STSHelperService",
    "https://qa-ims.bentley.com/rest/STSHelperService",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ImsPassiveAuthUrl(
    "PassiveAuthUrl",
    "https://qa-ims.bentley.com/SignIn/K",
    "https://qa-ims.bentley.com/SignIn/K",
    "https://ims.bentley.com/Account/LoginK",
    nullptr,
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::iModelBridgeConfiguration(
    "iModelBridgeConfiguration",
    "https://dev-connect-iModelBridgeConfiguration.bentley.com",
    "https://qa-connect-iModelBridgeConfiguration.bentley.com",
    "https://connect-iModelBridgeConfiguration.bentley.com",
    "https://perf-connect-iModelBridgeConfiguration.bentley.com",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::RecommendationServiceUrl(
    "RecommendationService.URL",
    "https://dev-recommendation-eus.cloudapp.net",
    "https://qa-connect-recommendation.bentley.com",
    "https://connect-recommendation.bentley.com",
    "https://dev-recommendation-eus.cloudapp.net",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ProjectSharedFederatedUIURL(
    "ProjectSharedFederatedUI.URL",
    "https://dev-projectshareportal.bentley.com/",
    "https://qa-projectshareportal.bentley.com/",
    "https://projectshareportal.bentley.com/",
    "https://perf-projectshareportal.bentley.com/",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::IMSOpenID(
    "IMSOpenID",
    "https://qa-imsoidc.bentley.com/",
    "https://qa-imsoidc.bentley.com/",
    "https://qa-imsoidc.bentley.com/",
    "https://imsoidc.bentley.com",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectProductSettingsService(
    "ProductSettingsService.RP",
    "https://dev-connect-productsettingsservice.bentley.com",
    "https://qa-connect-productsettingsservice.bentley.com",
    "https://connect-productsettingsservice.bentley.com",
    "https://qa-connect-productsettingsservice.bentley.com",
    &s_urlRegistry
    );

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bset<const UrlProvider::UrlDescriptor*> UrlProvider::GetUrlRegistry()
    {
    bset<const UrlDescriptor*> registry;
    for (const auto& pair : s_urlRegistry)
        registry.insert(pair.second);

    return registry;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
const UrlProvider::UrlDescriptor* UrlProvider::ResolveUrlDescriptor(Utf8StringCR uriString)
    {
    BeUri uri(uriString);
    if (uri.GetScheme() != "buddi")
        return nullptr;

    if (uri.GetHost() != "resolve")
        return nullptr;

    Utf8String path = uri.GetPath().substr(1);
    if (path.empty())
        return nullptr;

    auto urlDescriptor = s_urlRegistry.find(path);
    if (s_urlRegistry.end() == urlDescriptor)
        return nullptr;

    return urlDescriptor->second;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void UrlProvider::Initialize
(
Environment env,
int64_t cacheTimeoutMs,
IJsonLocalState* localState,
IBuddiClientPtr customBuddi,
IHttpHandlerPtr customHandler,
ITaskSchedulerPtr customScheduler
)
    {
    BeAssert (nullptr != localState);

    s_thread = customScheduler;
    if (nullptr == s_thread)
        s_thread = WorkerThread::Create("UrlProvider");

    s_localState = localState;
    s_customHandler = customHandler;
    s_buddi = customBuddi ? customBuddi : std::make_shared<BuddiClient>(s_customHandler, nullptr, s_thread);
    s_cacheTimeoutMs = cacheTimeoutMs;
    s_isInitialized = true;

    SetEnvironment(env);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UrlProvider::Uninitialize()
    {
    s_isInitialized = false;
    s_buddi = nullptr;
    s_customHandler = nullptr;
    s_localState = nullptr;
    s_thread = nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetUrl(Utf8StringCR urlName, const Utf8String* defaultUrls)
    {
    Utf8String cachedUrl;

    if (!s_isInitialized)
        {
        BeAssert(false && "UrlProvider not initialized");
        return cachedUrl;
        }

    Json::Value record = s_localState->GetJsonValue(LOCAL_STATE_NAMESPACE, urlName.c_str());
    if (!record.isNull() && record.isObject())
        {
        cachedUrl = record[RECORD_Url].asString();
        int64_t timeCached = BeJsonUtilities::Int64FromValue(record[RECORD_TimeCached]);
        int64_t currentTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        if (!cachedUrl.empty() && ((currentTime - timeCached) >= s_cacheTimeoutMs))
            {
            // Refresh in background
            s_thread->ExecuteAsyncWithoutAttachingToCurrentTask([=]
                {
                CacheBuddiUrl(urlName);
                });
            }
        }

    if (cachedUrl.empty())
        cachedUrl = CacheBuddiUrl(urlName)->GetResult();

    if (cachedUrl.empty())
        cachedUrl = defaultUrls[s_env];

    return cachedUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Utf8String> UrlProvider::CacheBuddiUrl(Utf8StringCR urlName)
    {
    uint32_t region = s_regionsId[s_env];
    return s_buddi->GetUrl(urlName, region)->Then<Utf8String>(s_thread, [=] (BuddiUrlResult result)
        {
        Utf8String url = result.GetValue();
        if (!result.IsSuccess() || url.empty())
            {
            LOG.errorv("URL '%s' is not configured on buddi service for region '%d'. Will use fallback URL.", urlName.c_str(), region);
            return url;
            }
        Json::Value record;
        record[RECORD_TimeCached] = BeJsonUtilities::StringValueFromInt64(BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        record[RECORD_Url] = url;
        s_localState->SaveJsonValue(LOCAL_STATE_NAMESPACE, urlName.c_str(), record);
        return url;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void UrlProvider::CleanUpUrlCache()
    {
    for (auto& descriptor : s_urlRegistry)
        {
        s_localState->SaveJsonValue(LOCAL_STATE_NAMESPACE, descriptor.second->GetName().c_str(), Json::Value::GetNull());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr UrlProvider::GetSecurityConfigurator(IHttpHandlerPtr customHandler)
    {
    // TODO: maybe this could be tied to GetUrl calls - instead return HttpClient that would generate correctly configured requests
    return std::make_shared<HttpConfigurationHandler>([=] (Http::Request& request)
        {
        if (Environment::Release == s_env)
            {
            request.SetValidateCertificate(true);
            }
        }, customHandler);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UrlProvider::SetHttpHandler(IHttpHandlerPtr customHandler)
    {
    s_customHandler = customHandler;
    s_buddi = std::make_shared<BuddiClient>(s_customHandler, nullptr, s_thread);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UrlProvider::Environment UrlProvider::GetEnvironment()
    {
    return s_env;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UrlProvider::SetEnvironment(UrlProvider::Environment env)
    {
    s_env = env;

    Json::Value jsonPreviousEnv = s_localState->GetJsonValue(LOCAL_STATE_NAMESPACE, LOCAL_STATE_ENVIRONMENT);
    if (!jsonPreviousEnv.isNull() && env != jsonPreviousEnv.asUInt())
        CleanUpUrlCache();

    if (jsonPreviousEnv.isNull() || env != jsonPreviousEnv.asUInt())
        s_localState->SaveJsonValue(LOCAL_STATE_NAMESPACE, LOCAL_STATE_ENVIRONMENT, env);

    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::ToEnvironmentString(UrlProvider::Environment env)
    {
    switch (env)
        {
        case UrlProvider::Environment::Release:
            return Environment_Release;
        case UrlProvider::Environment::Qa:
            return Environment_Qa;
        case UrlProvider::Environment::Dev:
            return Environment_Dev;
        case UrlProvider::Environment::Perf:
            return Environment_Perf;
        }

    return Utf8String();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UrlProvider::UrlDescriptor::UrlDescriptor(Utf8CP name, Utf8CP devUrl, Utf8CP qaUrl, Utf8CP prodUrl, Utf8CP perfUrl, Registry* registry) :
m_name(name),
m_registry(registry)
    {
    m_defaultUrls[0] = devUrl;
    m_defaultUrls[1] = qaUrl;
    m_defaultUrls[2] = prodUrl;
    m_defaultUrls[3] = perfUrl;

    if (nullptr != m_registry)
        {
        BeAssert(m_registry->find(m_name) == m_registry->end());
        (*m_registry)[m_name] = this;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UrlProvider::UrlDescriptor::~UrlDescriptor()
    {
    if (nullptr != m_registry)
        {
        m_registry->erase(m_name);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UrlProvider::UrlDescriptor::GetName() const
    {
    return m_name;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::UrlDescriptor::Get() const
    {
    return UrlProvider::GetUrl(m_name, m_defaultUrls);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Robert.Lukasonok    07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::UrlDescriptor::GetBuddiUri() const
    {
    if (m_name.empty())
        return "";

    return "buddi://resolve/" + m_name;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Utf8String> UrlProvider::UrlDescriptor::GetAsync() const
    {
    return CreateCompletedAsyncTask(UrlProvider::GetUrl(m_name, m_defaultUrls));
    }
