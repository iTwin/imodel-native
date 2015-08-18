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

// URL configuration

#define URLNAME_ConnectEula                     "Mobile.ConnectEula"
#define URLNAME_ConnectGlobalWsg                "Mobile.ConnectGlobalWsg"
#define URLNAME_ConnectPersonalPublishingWsg    "Mobile.ConnectPersonalPublishingWsg"
#define URLNAME_ConnectProjectContentWsg        "Mobile.ConnectProjectContentWsg"
#define URLNAME_ConnectSharedContentWsg         "Mobile.ConnectSharedContentWsg"
#define URLNAME_ImsStsAuth                      "Mobile.ImsStsAuth"
#define URLNAME_Passport                        "Mobile.Passport"
#define URLNAME_PunchListWsg                    "Mobile.PunchListWsg"
#define URLNAME_UsageTracking                   "Mobile.UsageTracking"

const std::vector<Utf8String> s_urlNames = {
    URLNAME_ConnectEula,
    URLNAME_ConnectGlobalWsg,
    URLNAME_ConnectPersonalPublishingWsg,
    URLNAME_ConnectProjectContentWsg,
    URLNAME_ConnectSharedContentWsg,
    URLNAME_ImsStsAuth,
    URLNAME_Passport,
    URLNAME_PunchListWsg,
    URLNAME_UsageTracking
    };

// Region IDs from the buddi.bentley.com
uint32_t s_regionsId[3] = {
    103,    // Region "Bentley Corporate Network - DEV"
    102,    // Region "Bentley Corporate Network - QA"
    0       // No region - use BUDDI non-regional URLs
    };

// Default URLs

const Utf8String s_punchListWsgUrl[3] = {
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com"
    };

const Utf8String s_connectGlobalWsgUrl[3] = {
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com"
    };

const Utf8String s_connectSharedContentWsgUrl[3] = {
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com"
    };

const Utf8String s_connectPersonalPublishingWsgUrl[3] = {
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com"
    };

const Utf8String s_connectProjectContentWsgUrl[3] = {
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com"
    };

const Utf8String s_connectEulaUrl[3] = {
    "https://dev-agreement-eus.cloudapp.net/rest",
    "https://dev-agreement-eus.cloudapp.net/rest",
    "https://connect-agreement.bentley.com/rest"
    };

const Utf8String s_imsStsAuthUrl[3] = {
    "https://ims-testing.bentley.com/rest/ActiveSTSService/json/IssueEx",
    "https://ims-testing.bentley.com/rest/ActiveSTSService/json/IssueEx",
    "https://ims.bentley.com/rest/ActiveSTSService/json/IssueEx"
    };

const Utf8String s_usageTrackingUrl[3] = {
    "https://licenseXM.bentley.com/bss/ws/mobile",
    "https://licenseXM.bentley.com/bss/ws/mobile",
    "https://SELECTserver.bentley.com/bss/ws/mobile"
    };

const Utf8String s_passportUrl[3] = {
    "https://qa-ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport",
    "https://qa-ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport",
    "https://ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport"
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void UrlProvider::Initialize(Environment env, ILocalState* customLocalState, IBuddiClientPtr customBuddi)
    {
    s_localState = customLocalState ? customLocalState : &MobileDgnCommon::LocalState();
    s_buddi = customBuddi ? customBuddi : std::make_shared<BuddiClient>();
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetPunchlistWsgUrl()
    {
    return GetUrl(URLNAME_PunchListWsg, s_punchListWsgUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetConnectPersonalPublishingWsgUrl()
    {
    return GetUrl(URLNAME_ConnectPersonalPublishingWsg, s_connectPersonalPublishingWsgUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetConnectProjectContentWsgUrl()
    {
    return GetUrl(URLNAME_ConnectProjectContentWsg, s_connectProjectContentWsgUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetConnectGlobalWsgUrl()
    {
    return GetUrl(URLNAME_ConnectGlobalWsg, s_connectGlobalWsgUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetConnectSharedContentWsgUrl()
    {
    return GetUrl(URLNAME_ConnectSharedContentWsg, s_connectSharedContentWsgUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetConnectEulaUrl()
    {
    return GetUrl(URLNAME_ConnectEula, s_connectEulaUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetImsStsAuthUrl()
    {
    return GetUrl(URLNAME_ImsStsAuth, s_imsStsAuthUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetUsageTrackingUrl()
    {
    return GetUrl(URLNAME_UsageTracking, s_usageTrackingUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   2/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetPassportUrl()
    {
    return GetUrl(URLNAME_Passport, s_passportUrl);
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
    for (auto& ulrName : s_urlNames)
        {
        s_localState->SaveValue(LOCAL_STATE_NAMESPACE, ulrName.c_str(), Json::Value::null);
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
