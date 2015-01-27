/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/UrlProvider.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

static UrlProvider::Environment s_env;
bool UrlProvider::s_isInitialized = false;

const Utf8String UrlProvider::s_punchListWsgUrl[3] = {
    "https://qa-punchlist-eus.cloudapp.net",
    "https://qa-punchlist-eus.cloudapp.net",
    "https://prod-punchlist-eus.cloudapp.net"
    };

const Utf8String UrlProvider::s_connectWsgUrl[3] = {
    "https://qa-connectgateway-eus.cloudapp.net",
    "https://qa-connectgateway-eus.cloudapp.net",
    "https://prod-connectgateway-eus.cloudapp.net"
    };

const Utf8String UrlProvider::s_connectEulaUrl[3] = {
    "https://dev-agreement-eus.cloudapp.net/rest",
    "https://dev-agreement-eus.cloudapp.net/rest",
    "https://agreement-eus.cloudapp.net/rest"
    };

const Utf8String UrlProvider::s_connectLearnStsAuthUri[3] = {
    "https://ims-testing.bentley.com/rest/ActiveSTSService/json/IssueEx",
    "https://ims-testing.bentley.com/rest/ActiveSTSService/json/IssueEx",
    "https://ims.bentley.com/rest/ActiveSTSService/json/IssueEx"
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void UrlProvider::Initialize (Environment env)
    {
    s_env = env;
    s_isInitialized = true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool UrlProvider::IsInitialized ()
    {
    return s_isInitialized;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UrlProvider::GetPunchlistWsgUrl ()
    {
    BeAssert (s_isInitialized && "UrlProvider not initialized");
    return s_punchListWsgUrl[s_env];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UrlProvider::GetConnectWsgUrl ()
    {
    BeAssert (s_isInitialized && "UrlProvider not initialized");
    return s_connectWsgUrl[s_env];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UrlProvider::GetConnectEulaUrl ()
    {
    BeAssert (s_isInitialized && "UrlProvider not initialized");
    return s_connectEulaUrl[s_env];
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Brad.Hadden   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UrlProvider::GetConnectLearnStsAuthUri ()
    {
    BeAssert (s_isInitialized && "UrlProvider not initialized");
    return s_connectLearnStsAuthUri[s_env];
    }
