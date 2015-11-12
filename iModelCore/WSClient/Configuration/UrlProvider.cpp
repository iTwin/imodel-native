/*--------------------------------------------------------------------------------------+
|
|     $Source: Configuration/UrlProvider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <Bentley/BeTimeUtilities.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <MobileDgn/Utils/Http/HttpConfigurationHandler.h>

#define LOCAL_STATE_NAMESPACE   "UrlCache"
#define LOCAL_STATE_ENVIRONMENT "Environment"
#define RECORD_Url              "URL"
#define RECORD_TimeCached       "TimeCached"

USING_NAMESPACE_BENTLEY_WEBSERVICES

int64_t                     UrlProvider::DefaultTimeout = 24 * 3600 * 1000;
bool                        UrlProvider::s_isInitialized = false;
UrlProvider::Environment    UrlProvider::s_env = UrlProvider::Environment::Release;
int64_t                     UrlProvider::s_cacheTimeoutMs = 0;
IBuddiClientPtr             UrlProvider::s_buddi;
ILocalState*                UrlProvider::s_localState = nullptr;

// Region IDs from the buddi.bentley.com
uint32_t s_regionsId[3] = {
    103,    // Region "Bentley Corporate Network - DEV"
    102,    // Region "Bentley Corporate Network - QA"
    0       // No region for PROD - use BUDDI non-regional URLs
    };

// Managed urls
bset<UrlProvider::UrlDescriptor*> s_urlRegistry;

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectEula(
    "Mobile.ConnectEula",
    "https://dev-agreement-eus.cloudapp.net/rest",
    "https://dev-agreement-eus.cloudapp.net/rest",
    "https://connect-agreement.bentley.com/rest",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgGlobal(
    "Mobile.ConnectWsgGlobal",
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgPersonalPublishing(
    "Mobile.ConnectWsgPersonalPublishing",
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgProjectContent(
    "Mobile.ConnectWsgProjectContent",
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgPunchList(
    "Mobile.PunchListWsg",
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgSharedContent(
    "Mobile.ConnectWsgSharedContent",
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ImsStsAuth(
    "Mobile.ImsStsAuth",
    "https://ims-testing.bentley.com/rest/ActiveSTSService/json/IssueEx",
    "https://ims-testing.bentley.com/rest/ActiveSTSService/json/IssueEx",
    "https://ims.bentley.com/rest/ActiveSTSService/json/IssueEx",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::Passport(
    "Mobile.Passport",
    "https://qa-ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport",
    "https://qa-ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport",
    "https://ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::UsageTracking(
    "Mobile.UsageTracking",
    "https://licenseXM.bentley.com/bss/ws/mobile",
    "https://licenseXM.bentley.com/bss/ws/mobile",
    "https://SELECTserver.bentley.com/bss/ws/mobile",
    &s_urlRegistry
    );

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void UrlProvider::Initialize(Environment env, int64_t cacheTimeoutMs, ILocalState* customLocalState, IBuddiClientPtr customBuddi)
    {
    s_localState = customLocalState ? customLocalState : &MobileDgnCommon::LocalState();
    s_buddi = customBuddi ? customBuddi : std::make_shared<BuddiClient>();
    s_env = env;
    s_cacheTimeoutMs = cacheTimeoutMs;
    s_isInitialized = true;

    Json::Value jsonPreviousEnv = s_localState->GetValue(LOCAL_STATE_NAMESPACE, LOCAL_STATE_ENVIRONMENT);
    if (!jsonPreviousEnv.isNull() && env != jsonPreviousEnv.asUInt())
        {
        CleanUpUrlCache();
        }
    if (jsonPreviousEnv.isNull() || env != jsonPreviousEnv.asUInt())
        {
        s_localState->SaveValue(LOCAL_STATE_NAMESPACE, LOCAL_STATE_ENVIRONMENT, env);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetUrl(Utf8StringCR urlName, const Utf8String* defaultUrls)
    {
    if (!s_isInitialized)
        {
        BeAssert(false && "UrlProvider not initialized");
        return "";
        }

    Json::Value record = s_localState->GetValue(LOCAL_STATE_NAMESPACE, urlName.c_str());

    Utf8String cachedUrl;

    if (!record.isNull() && record.isObject())
        {
        cachedUrl = record[RECORD_Url].asString();
        int64_t timeCached = BeJsonUtilities::Int64FromValue(record[RECORD_TimeCached]);
        int64_t currentTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        if (!cachedUrl.empty() && ((currentTime - timeCached) < s_cacheTimeoutMs))
            {
            return cachedUrl;
            }
        }
    else if (record.isString())
        {
        // Support old cached string
        cachedUrl = record.asString();
        }

    Utf8String buddiUrl = GetBuddiUrl(urlName);
    if (!buddiUrl.empty())
        {
        record = Json::objectValue;
        record[RECORD_TimeCached] = BeJsonUtilities::StringValueFromInt64(BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        record[RECORD_Url] = buddiUrl;
        s_localState->SaveValue(LOCAL_STATE_NAMESPACE, urlName.c_str(), record);
        return buddiUrl;
        }

    if (!cachedUrl.empty())
        {
        return cachedUrl;
        }

    return defaultUrls[s_env];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetBuddiUrl(Utf8StringCR urlName)
    {
    auto result = s_buddi->GetUrl(urlName, s_regionsId[s_env])->GetResult();
    if (result.IsSuccess())
        {
        return result.GetValue();
        }
    return "";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void UrlProvider::CleanUpUrlCache()
    {
    for (auto& descriptor : s_urlRegistry)
        {
        s_localState->SaveValue(LOCAL_STATE_NAMESPACE, descriptor->GetName().c_str(), Json::Value::null);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr UrlProvider::GetSecurityConfigurator(IHttpHandlerPtr customHandler)
    {
    // TODO: maybe this could be tied to GetUrl calls - instead return HttpClient that would generate correctly configured requests
    return std::make_shared<HttpConfigurationHandler>([=] (HttpRequest& request)
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
UrlProvider::UrlDescriptor::UrlDescriptor(Utf8CP name, Utf8CP devUrl, Utf8CP qaUrl, Utf8CP prodUrl, bset<UrlDescriptor*>* registry) :
m_name(name),
m_registry(registry)
    {
    m_defaultUrls[0] = devUrl;
    m_defaultUrls[1] = qaUrl;
    m_defaultUrls[2] = prodUrl;

    if (nullptr != m_registry)
        {
        m_registry->insert(this);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UrlProvider::UrlDescriptor::~UrlDescriptor()
    {
    if (nullptr != m_registry)
        {
        m_registry->erase(this);
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
