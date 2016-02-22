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
#include <WebServices/Connect/Connect.h>
#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectTokenProvider.h>
#include <MobileDgn/Utils/Threading/UniqueTaskHolder.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct DelegationTokenProvider : public IConnectTokenProvider
    {
    private:
        Utf8String m_rpUri;
        IConnectTokenProviderPtr m_baseTokenProvider;
        SamlTokenPtr m_token;
        UniqueTaskHolder<SamlTokenResult> m_tokenRetriever;
        uint64_t m_tokenLifetime;

    private:
        AsyncTaskPtr<SamlTokenResult> RetrieveNewToken();

    public:
        //! Create token provider for delegating service specific tokens
        //! @param rpUri Relying Party URI for token
        //! @param baseTokenProvider token provider for base/identity token to be used to delegate new token
        WSCLIENT_EXPORT DelegationTokenProvider(Utf8String rpUri, IConnectTokenProviderPtr baseTokenProvider);

        //! Set new token lifetime
        WSCLIENT_EXPORT void Configure(uint64_t tokenLifetime);

        WSCLIENT_EXPORT SamlTokenPtr UpdateToken() override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken() override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
