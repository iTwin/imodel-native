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

    private:
        static Utf8String GetBuddiUrl(Utf8StringCR urlName);
        static Utf8String GetUrl(Utf8CP urlName, const Utf8String* defaultUrls);

    public:
        WSCLIENT_EXPORT static void Initialize(Environment env,
                                               ILocalState* customlocalState = nullptr,
                                               IBuddiClientPtr customBuddi = nullptr);

        //! Should be used with all requests to provided URLs!
        //! Returns handler that will configure requests depending on environment.
        //! Will setup certificate validation appropriately.
        WSCLIENT_EXPORT static IHttpHandlerPtr GetSecurityConfigurator(IHttpHandlerPtr customHandler = nullptr);

        WSCLIENT_EXPORT static Utf8String GetPunchlistWsgUrl();
        WSCLIENT_EXPORT static Utf8String GetConnectGlobalWsgUrl();
        WSCLIENT_EXPORT static Utf8String GetConnectSharedContentWsgUrl();
        WSCLIENT_EXPORT static Utf8String GetConnectPersonalPublishingWsgUrl();
        WSCLIENT_EXPORT static Utf8String GetConnectProjectContentWsgUrl();
        WSCLIENT_EXPORT static Utf8String GetConnectEulaUrl();
        WSCLIENT_EXPORT static Utf8String GetImsStsAuthUrl();
        WSCLIENT_EXPORT static Utf8String GetUsageTrackingUrl();
        WSCLIENT_EXPORT static Utf8String GetPassportUrl();

        WSCLIENT_EXPORT static void CleanUpUrlCache();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
