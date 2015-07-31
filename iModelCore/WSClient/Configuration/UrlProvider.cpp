/*--------------------------------------------------------------------------------------+
|
|     $Source: Configuration/UrlProvider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Configuration/UrlProvider.h>

#define LOCAL_STATE_NAMESPACE   "UrlCache"
#define LOCAL_STATE_ENVIRONMENT "Environment"

USING_NAMESPACE_BENTLEY_WEBSERVICES

static UrlProvider::Environment s_env;
IBuddiClientPtr UrlProvider::s_buddi;
ILocalState* UrlProvider::s_localState = nullptr;

bool UrlProvider::s_isInitialized = false;

const Utf8CP UrlProvider::s_urlNames[6] = {
    "Mobile.PunchListWsg",
    "Mobile.ConnectWsg",
    "Mobile.ConnectEula",
    "Mobile.ImsStsAuth",       //ConnectLearnStsAuth
    "Mobile.UsageTracking",
    "Mobile.Passport"
    };


const UrlData UrlProvider::s_punchListWsgUrl[3] = {
        {"https://dev-wsg20-eus.cloudapp.net", 103},
        {"https://qa-wsg20-eus.cloudapp.net", 102},
        {"https://connect-wsg20.bentley.com", 1230}
    };

// CONNECT 1.1
//const Utf8String UrlProvider::s_connectWsgUrl[3] = {
//    "https://qa-connectgateway-eus.cloudapp.net",
//    "https://qa-connectgateway-eus.cloudapp.net",
//    "https://prod-connectgateway-eus.cloudapp.net"
//    };

const UrlData UrlProvider::s_connectWsgUrl[3] = {
        {"https://dev-wsg20-eus.cloudapp.net", 103},
        {"https://qa-wsg20-eus.cloudapp.net", 102},
        {"https://connect-wsg20.bentley.com", 1231}
    };

const UrlData UrlProvider::s_connectEulaUrl[3] = {
        {"https://dev-agreement-eus.cloudapp.net/rest", 103},
        {"https://dev-agreement-eus.cloudapp.net/rest", 102},
        {"https://connect-agreement.bentley.com/rest", 1232}
    };

const UrlData UrlProvider::s_connectLearnStsAuthUri[3] = {
        {"https://ims-testing.bentley.com/rest/ActiveSTSService/json/IssueEx", 103},
        {"https://ims-testing.bentley.com/rest/ActiveSTSService/json/IssueEx", 102},
        {"https://ims.bentley.com/rest/ActiveSTSService/json/IssueEx", 1233}
    };

const UrlData UrlProvider::s_usageTrackingUrl[3] = {
        {"https://licenseXM.bentley.com/bss/ws/mobile", 103},
        {"https://licenseXM.bentley.com/bss/ws/mobile", 102},
        {"https://SELECTserver.bentley.com/bss/ws/mobile", 1234}
    };

const UrlData UrlProvider::s_passportUrl[3] = {
        {"https://qa-ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport", 103},
        {"https://qa-ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport", 102},
        {"https://ims.bentley.com/services/bentleyconnectservice/rest/json/HasUserPassport", 1235}
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

    //TODO: fix, app crashes on start
    /*Json::Value jsonPreviousEnv = s_localState->GetValue(LOCAL_STATE_NAMESPACE, LOCAL_STATE_ENVIRONMENT);
    if (!jsonPreviousEnv.isNull() && env != jsonPreviousEnv.asUInt())
        {
        CleanUpUrlCache();
        }
    if (jsonPreviousEnv.isNull() || env != jsonPreviousEnv.asUInt())
        {    
        s_localState->SaveValue(LOCAL_STATE_NAMESPACE, LOCAL_STATE_ENVIRONMENT, env);
        }*/
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetPunchlistWsgUrl()
    {
    return GetUrl("Mobile.PunchListWsg", s_punchListWsgUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetConnectWsgUrl()
    {
    return GetUrl("Mobile.ConnectWsg", s_connectWsgUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetConnectEulaUrl()
    {
    return GetUrl("Mobile.ConnectEula", s_connectEulaUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetConnectLearnStsAuthUri()
    {
    return GetUrl("Mobile.ImsStsAuth", s_connectLearnStsAuthUri);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   2/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetUsageTrackingUrl()
    {
    return GetUrl("Mobile.UsageTracking", s_usageTrackingUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   2/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetPassportUrl()
    {
    return GetUrl("Mobile.Passport", s_passportUrl);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetUrl(Utf8CP urlName, const UrlData* defaultUrls)
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

    url = GetBuddiUrl(urlName, defaultUrls[s_env].GetId());
    if (!url.empty())
        {
        s_localState->SaveValue(LOCAL_STATE_NAMESPACE, urlName, url);
        return url;
        }

    return defaultUrls[s_env].GetDefaultUrl();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Julija.Semenenko   06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UrlProvider::GetBuddiUrl(Utf8StringCR urlName, uint32_t regionId)
    {
    auto result = s_buddi->GetUrl(urlName, regionId)->GetResult();
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
