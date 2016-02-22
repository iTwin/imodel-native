/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectTokenProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectTokenProvider.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConnectTokenProvider : public IConnectTokenProvider
    {
    private:
        IConnectAuthenticationPersistencePtr m_persistence;
        bool m_isTokenBasedAuthentication;
        std::function<void()> m_tokenExpiredHandler;

        uint64_t m_tokenLifetime;
        uint64_t m_tokenRefreshRate;

    private:
        bool ShouldRenewToken(DateTimeCR tokenSetTime);

    public:
        WSCLIENT_EXPORT ConnectTokenProvider
            (
            IConnectAuthenticationPersistencePtr customPersistence = nullptr,
            bool isTokenBasedAuthentication = false,
            std::function<void()> tokenExpiredHandler = nullptr
            );

        //! Set new token lifetime and refresh rate in minutes
        WSCLIENT_EXPORT void Configure(uint64_t tokenLifetime, uint64_t tokenRefreshRate);

        WSCLIENT_EXPORT SamlTokenPtr UpdateToken() override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken() override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
