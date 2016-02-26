/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/IdentityTokenProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectTokenProvider.h>
#include <WebServices/Connect/IImsClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IdentityTokenProvider> IdentityTokenProviderPtr;
struct IdentityTokenProvider : IConnectTokenProvider, std::enable_shared_from_this<IdentityTokenProvider>
    {
    private:
        IImsClientPtr m_client;
        ITokenStorePtr m_store;
        std::function<void()> m_tokenExpiredHandler;

        uint64_t m_tokenLifetime;
        uint64_t m_tokenRefreshRate;

    private:
        IdentityTokenProvider(IImsClientPtr client, ITokenStorePtr store, std::function<void()> tokenExpiredHandler);
        bool ShouldRenewToken(DateTimeCR tokenSetTime);

    public:
        WSCLIENT_EXPORT static IdentityTokenProviderPtr Create
            (
            IImsClientPtr client,
            ITokenStorePtr store,
            std::function<void()> tokenExpiredHandler = nullptr
            );

        //! Set new token lifetime and refresh rate in minutes
        WSCLIENT_EXPORT void Configure(uint64_t tokenLifetime, uint64_t tokenRefreshRate);

        WSCLIENT_EXPORT SamlTokenPtr UpdateToken() override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken() override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
