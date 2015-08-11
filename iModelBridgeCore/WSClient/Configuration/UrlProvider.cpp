/*--------------------------------------------------------------------------------------+
|
|     $Source: Configuration/UrlProvider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <MobileDgn/Utils/Http/HttpConfigurationHandler.h>
#include <WebServices/Configuration/UrlProvider.h>

#define LOCAL_STATE_NAMESPACE   "UrlCache"
#define LOCAL_STATE_ENVIRONMENT "Environment"

USING_NAMESPACE_BENTLEY_WEBSERVICES

bool UrlProvider::s_isInitialized = false;
static UrlProvider::Environment s_env;
IBuddiClientPtr UrlProvider::s_buddi;
ILocalState* UrlProvider::s_localState = nullptr;

// URL configuration

#define URLNAME_PunchListWsg    "Mobile.PunchListWsg"
#define URLNAME_ConnectWsg      "Mobile.ConnectWsg"
#define URLNAME_ConnectEula     "Mobile.ConnectEula"
#define URLNAME_ImsStsAuth      "Mobile.ImsStsAuth"
#define URLNAME_UsageTracking   "Mobile.UsageTracking"
#define URLNAME_Passport        "Mobile.Passport"

const Utf8CP UrlProvider::s_urlNames[6] = {
    URLNAME_PunchListWsg,
    URLNAME_ConnectWsg,
    URLNAME_ConnectEula,
    URLNAME_ImsStsAuth,
    URLNAME_UsageTracking,
    URLNAME_Passport
    };

// Region IDs from the buddi.bentley.com
uint32_t UrlProvider::s_regionsId[3] = {
    103,    // "Bentley Corporate Network - DEV"
    102,    // "Bentley Corporate Network - QA"
    0       // No region - use BUDDI non-regional URLs
    };

// Default URLs

const Utf8String UrlProvider::s_punchListWsgUrl[3] = {
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com"
    };

const Utf8String UrlProvider::s_connectWsgUrl[3] = {
    "https://dev-wsg20-eus.cloudapp.net",
    "https://qa-wsg20-eus.cloudapp.net",
    "https://connect-wsg20.bentley.com"
    };

const Utf8String UrlProvider::s_connectEulaUrl[3] = {
    "https://dev-agreement-eus.cloudapp.net/rest",
    "https://dev-agreement-eus.cloudapp.net/rest",
    "https://connect-agreement.bentley.com/rest"
    };

const Utf8String UrlProvider::s_imsStsAuthUrl[3] = {
    "https://ims-testing.bentley.com/rest/ActiveSTSService/json/IssueEx",
    "https://ims-testing.bentley.com/rest/ActiveSTSService/json/IssueEx",
    "https://ims.bentley.com/rest/ActiveSTSService/json/IssueEx"
    };

const Utf8String UrlProvider::s_usageTrackingUrl[3] = {
    "https://licenseXM.bentley.com/bss/ws/mobile",
    "https://licenseXM.bentley.com/bss/ws/mobile",
    "https://SELECTserver.bentley.com/bss/ws/mobile"
    };

const Utf8String UrlProvider::s_passportUrl[3] = {
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
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetPunchlistWsgUrl()
    {
    return GetUrl(URLNAME_PunchListWsg, s_punchListWsgUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetConnectWsgUrl()
    {
    return GetUrl(URLNAME_ConnectWsg, s_connectWsgUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetConnectEulaUrl()
    {
    return GetUrl(URLNAME_ConnectEula, s_connectEulaUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetImsStsAuthUrl()
    {
    return GetUrl(URLNAME_ImsStsAuth, s_imsStsAuthUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   2/2015
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
    for (int i = 0; i < sizeof(s_urlNames) / sizeof(Utf8CP); i++)
        {
        s_localState->SaveValue(LOCAL_STATE_NAMESPACE, s_urlNames[i], Json::Value::null);
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
