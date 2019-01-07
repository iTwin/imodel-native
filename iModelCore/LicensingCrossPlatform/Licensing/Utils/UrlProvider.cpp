/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Utils/UrlProvider.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Licensing.h>
#include <Bentley/BeTimeUtilities.h>
//#include <Licensing/Utils/UrlProvider.h>
#include "../../PublicAPI/Licensing/Utils/UrlProvider.h"
#include <BeHttp/HttpConfigurationHandler.h>
#include <Bentley/Tasks/WorkerThread.h>

#include "../Logging.h"

#define LOCAL_STATE_NAMESPACE   "UrlCache"
#define LOCAL_STATE_ENVIRONMENT "Environment"
#define RECORD_Url              "URL"
#define RECORD_TimeCached       "TimeCached"

#define Environment_Release     "PROD"
#define Environment_Qa          "QA"
#define Environment_Dev         "DEV"
#define Environment_Perf        "PERF"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LICENSING

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
    BeAssert(nullptr != localState);

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
