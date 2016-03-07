/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/DelegationTokenProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectTokenProvider.h>
#include <WebServices/Connect/IImsClient.h>
#include <DgnClientFx/Utils/Threading/UniqueTaskHolder.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct DelegationTokenProvider : public IConnectTokenProvider
    {
    private:
        IImsClientPtr m_client;
        Utf8String m_rpUri;
        IConnectTokenProviderPtr m_parentTokenProvider;
        SamlTokenPtr m_token;
        UniqueTaskHolder<SamlTokenResult> m_tokenRetriever;
        uint64_t m_tokenLifetime;

    private:
        AsyncTaskPtr<SamlTokenResult> RetrieveNewToken(bool updateParentTokenIfFailed = true);

    public:
        //! Create token provider for delegating service specific tokens
        //! @param client client
        //! @param rpUri Relying Party URI for token
        //! @param parentTokenProvider token provider for parent/identity token to be used to delegate new token
        WSCLIENT_EXPORT DelegationTokenProvider(IImsClientPtr client, Utf8String rpUri, IConnectTokenProviderPtr parentTokenProvider);

        //! Set new token lifetime
        WSCLIENT_EXPORT void Configure(uint64_t tokenLifetime);

        WSCLIENT_EXPORT SamlTokenPtr UpdateToken() override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken() override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
