/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Configuration/UrlProvider.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/WString.h>
#include <MobileDgn/MobileDgnCommon.h>
#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Configuration/BuddiClient.h>

USING_NAMESPACE_BENTLEY_MOBILEDGN
BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 Julija.Semenenko    07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct UrlData
    {
    private:
        Utf8String m_defaultUrl;
        //Check URL IDs in the buddi.bentley.com
        //For all DEV URLs: Bentley Corporate Network – DEV (ID: 103)
        //For all QA URLs: Bentley Corporate Network – QA (ID: 102)
        //Release URLs without regions have their own unique IDs
        uint32_t m_regionId;

    public:
        UrlData(Utf8String defaultUrl, uint32_t regionId) : m_defaultUrl(defaultUrl), m_regionId(regionId) {}

        Utf8String GetDefaultUrl() const
            {
            return m_defaultUrl;
            }

        uint32_t GetId() const
            {
            return m_regionId;
            }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Brad.Hadden    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct UrlProvider
    {
    public:
        // Used for array indexes! Values must increment from 0
        enum Environment
            {
            Dev = 0,
            Qa,
            Release
            };

    private:
        static bool s_isInitialized;

        static ILocalState* s_localState;
        static IBuddiClientPtr s_buddi;

        static const Utf8CP s_urlNames[6];

        static const UrlData s_punchListWsgUrl[3];
        static const UrlData s_connectWsgUrl[3];
        static const UrlData s_connectEulaUrl[3];
        static const UrlData s_connectLearnStsAuthUri[3];
        static const UrlData s_usageTrackingUrl[3];
        static const UrlData s_passportUrl[3];

    private:
        static Utf8String GetBuddiUrl(Utf8StringCR urlName, uint32_t regionId);
        static Utf8String GetUrl(Utf8CP urlName, const UrlData* defaultUrls);

    public:
        WSCLIENT_EXPORT static void Initialize(Environment env,
                                               ILocalState* customlocalState = nullptr,
                                               IBuddiClientPtr customBuddi = nullptr);

        WSCLIENT_EXPORT static Utf8String GetPunchlistWsgUrl();
        WSCLIENT_EXPORT static Utf8String GetConnectWsgUrl();
        WSCLIENT_EXPORT static Utf8String GetConnectEulaUrl();
        WSCLIENT_EXPORT static Utf8String GetConnectLearnStsAuthUri();
        WSCLIENT_EXPORT static Utf8String GetUsageTrackingUrl();
        WSCLIENT_EXPORT static Utf8String GetPassportUrl();

        WSCLIENT_EXPORT static void CleanUpUrlCache();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
