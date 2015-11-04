/*--------------------------------------------------------------------------------------+
|
|     $Source: Configuration/UrlProvider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Configuration/UrlProvider.h>
#include <MobileDgn/Utils/Http/HttpConfigurationHandler.h>

#define LOCAL_STATE_NAMESPACE   "UrlCache"
#define LOCAL_STATE_ENVIRONMENT "Environment"

USING_NAMESPACE_BENTLEY_WEBSERVICES

bool UrlProvider::s_isInitialized = false;
static UrlProvider::Environment s_env;
IBuddiClientPtr UrlProvider::s_buddi;
ILocalState* UrlProvider::s_localState = nullptr;
IHttpHandlerPtr UrlProvider::s_customHandler;

// Region IDs from the buddi.bentley.com
uint32_t s_regionsId[3] = {
    103,    // Region "Bentley Corporate Network - DEV"
    102,    // Region "Bentley Corporate Network - QA"
    0       // No region - use BUDDI non-regional URLs
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

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectProjectUrl(
    "Mobile.ConnectProjectUrl",
    "https://dev-webportal-eus.cloudapp.net/project/index?projectId=",
    "https://qa-webportal-eus.cloudapp.net/project/index?projectId=",
    "https://connect.bentley.com/project/index?projectId=",
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
    "https://dev-punchlist-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com",
    &s_urlRegistry
    );

const UrlProvider::UrlDescriptor UrlProvider::Urls::ConnectWsgClashIssues(
    "Mobile.ClashIssuesWsg",
    "https://dev-punchlist-eus.cloudapp.net",
    "https://qa-punchlist-eus.cloudapp.net",
    "",
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
void UrlProvider::Initialize(Environment env, ILocalState* customLocalState, IBuddiClientPtr customBuddi, IHttpHandlerPtr customHandler)
    {
    s_localState = customLocalState ? customLocalState : &MobileDgnCommon::LocalState();
    s_customHandler = customHandler;
    s_buddi = customBuddi ? customBuddi : std::make_shared<BuddiClient>(s_customHandler);
    s_env = env;
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
Utf8String UrlProvider::GetUrl(Utf8CP urlName, const Utf8String* defaultUrls)
    {
    if (!s_isInitialized)
        {
        BeAssert(false && "UrlProvider not initialized");
        return "";
        }

    Json::Value jsonUrl = s_localState->GetValue(LOCAL_STATE_NAMESPACE, urlName);

    Utf8String url = jsonUrl.asString();
    if (!url.empty())
        {
        return url;
        }

    url = GetBuddiUrl(urlName);
    if (!url.empty())
        {
        s_localState->SaveValue(LOCAL_STATE_NAMESPACE, urlName, url);
        return url;
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
    return UrlProvider::GetUrl(m_name.c_str(), m_defaultUrls);
    }
