/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Connect/DelegationTokenProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

//#include <WebServices/Client/WebServicesClient.h>
#include <Licensing/Licensing.h>
#include "IConnectAuthenticationPersistence.h"
#include "IConnectTokenProvider.h"
#include "IImsClient.h"
#include <Bentley/Tasks/UniqueTaskHolder.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct DelegationTokenProvider : public IConnectTokenProvider
    {
    private:
        IImsClientPtr m_client;
        Utf8String m_rpUri;
        IConnectTokenProviderPtr m_parentTokenProvider;
        UniqueTaskHolder<SamlTokenResult> m_tokenRetrieveTask;

        SamlTokenPtr m_token;
        uint32_t m_tokenLifetime;
        DateTime m_tokenUpdateDate;

        uint32_t m_tokenRequestLifetime;
        uint32_t m_tokenExpirationThreshold;

    private:
        void ValidateToken();
        AsyncTaskPtr<SamlTokenResult> RetrieveNewToken(bool updateParentTokenIfFailed = true);

    public:
        //! Create token provider for delegating service specific tokens
        //! @param client client
        //! @param rpUri Relying Party URI for token
        //! @param parentTokenProvider token provider for parent/identity token to be used to delegate new token
        LICENSING_EXPORT DelegationTokenProvider(IImsClientPtr client, Utf8String rpUri, IConnectTokenProviderPtr parentTokenProvider);

        //! Set new token lifetime and expiration threshold in minutes
        LICENSING_EXPORT void Configure(uint32_t tokenLifetime, uint32_t tokenExpirationThreshold);

        LICENSING_EXPORT AsyncTaskPtr<ISecurityTokenPtr> UpdateToken() override;
        LICENSING_EXPORT ISecurityTokenPtr GetToken() override;
        LICENSING_EXPORT void ClearCache();
    };

END_BENTLEY_LICENSING_NAMESPACE
