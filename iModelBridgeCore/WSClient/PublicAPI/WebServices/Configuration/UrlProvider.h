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

        struct UrlDescriptor
            {
            private:
                Utf8String m_name;
                Utf8String m_defaultUrls[3];
                bset<UrlDescriptor*>* m_registry;

            public:
                // Constructur for internal use
                UrlDescriptor(Utf8CP name, Utf8CP devUrl, Utf8CP qaUrl, Utf8CP prodUrl, bset<UrlDescriptor*>* registry);
                ~UrlDescriptor();

                Utf8StringCR GetName() const;

                //! Retrieve cached or server configured URL depending on UrlProvider configuration
                WSCLIENT_EXPORT Utf8String Get() const;
            };

    private:
        static bool s_isInitialized;

        static ILocalState* s_localState;
        static IBuddiClientPtr s_buddi;
        static IHttpHandlerPtr s_customHandler;

    private:
        static Utf8String GetBuddiUrl(Utf8StringCR urlName);
        static Utf8String GetUrl(Utf8CP urlName, const Utf8String* defaultUrls);

    public:
        WSCLIENT_EXPORT static void Initialize(Environment env,
                                               ILocalState* customlocalState = nullptr,
                                               IBuddiClientPtr customBuddi = nullptr,
                                               IHttpHandlerPtr customHandler = nullptr);

        //! Should be used with all requests to provided URLs!
        //! Returns handler that will configure requests depending on environment.
        //! Will setup certificate validation appropriately.
        WSCLIENT_EXPORT static IHttpHandlerPtr GetSecurityConfigurator(IHttpHandlerPtr customHandler = nullptr);

        WSCLIENT_EXPORT static void CleanUpUrlCache();

        struct Urls
            {
            WSCLIENT_EXPORT static const UrlDescriptor ConnectEula;
            WSCLIENT_EXPORT static const UrlDescriptor ConnectWsgGlobal;
            WSCLIENT_EXPORT static const UrlDescriptor ConnectWsgPersonalPublishing;
            WSCLIENT_EXPORT static const UrlDescriptor ConnectWsgProjectContent;
            WSCLIENT_EXPORT static const UrlDescriptor ConnectWsgPunchList;
            WSCLIENT_EXPORT static const UrlDescriptor ConnectWsgSharedContent;
            WSCLIENT_EXPORT static const UrlDescriptor ImsStsAuth;
            WSCLIENT_EXPORT static const UrlDescriptor Passport;
            WSCLIENT_EXPORT static const UrlDescriptor UsageTracking;
            };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
