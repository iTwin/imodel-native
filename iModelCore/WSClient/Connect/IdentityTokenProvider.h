/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/UniqueTaskHolder.h>
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
        std::function<void(bool, int64_t)> m_tokenRenewHandler;
        UniqueTaskHolder<SamlTokenResult> m_tokenRetrieveTask;

        uint32_t m_tokenLifetime;
        uint32_t m_tokenRefreshRate;

    private:
        IdentityTokenProvider(IImsClientPtr client, ITokenStorePtr store, std::function<void()> tokenExpiredHandler, std::function<void(bool, int64_t)> tokenRenewHandler);
        bool ShouldRenewToken(DateTimeCR tokenSetTime);
        AsyncTaskPtr<SamlTokenResult> RenewToken();
        void RenewTokenIfNeeded();
        int64_t GetTokenExpirationTimestamp();

    public:
        WSCLIENT_EXPORT static IdentityTokenProviderPtr Create
            (
            IImsClientPtr client,
            ITokenStorePtr store,
            std::function<void()> tokenExpiredHandler = nullptr,
            std::function<void(bool, int64_t)> tokenRenewHandler = nullptr
            );

        //! Set new token lifetime and refresh rate in minutes
        WSCLIENT_EXPORT void Configure(uint32_t tokenLifetime, uint32_t tokenRefreshRate);

        WSCLIENT_EXPORT AsyncTaskPtr<ISecurityTokenPtr> UpdateToken() override;
        WSCLIENT_EXPORT ISecurityTokenPtr GetToken() override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
