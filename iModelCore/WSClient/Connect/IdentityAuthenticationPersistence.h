/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/IdentityAuthenticationPersistence.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <MobileDgn/MobileDgnApplication.h>
#include <MobileDgn/Utils/SecureStore.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IdentityAuthenticationPersistence> IdentityAuthenticationPersistencePtr;
struct EXPORT_VTABLE_ATTRIBUTE IdentityAuthenticationPersistence : public IConnectAuthenticationPersistence
    {
    private:
        mutable BeCriticalSection m_cs;
        ILocalState& m_localState;
        std::shared_ptr<ISecureStore> m_secureStore;
        mutable SamlTokenPtr m_token;

    public:
        WSCLIENT_EXPORT IdentityAuthenticationPersistence(ILocalState* localState = nullptr, std::shared_ptr<ISecureStore> secureStore = nullptr);
        virtual ~IdentityAuthenticationPersistence() {}

        WSCLIENT_EXPORT void SetToken(SamlTokenPtr token) override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken() const override;
        WSCLIENT_EXPORT DateTime GetTokenSetTime() const override;

        //! Does nothing
        WSCLIENT_EXPORT void SetCredentials(CredentialsCR credentials) override;
        //! Returns empty
        WSCLIENT_EXPORT Credentials GetCredentials() const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
