/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Connect/IdentityAuthenticationPersistence.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

//#include <WebServices/WebServices.h>
#include <Licensing/Licensing.h>
#include "IConnectAuthenticationPersistence.h"
#include <BeSecurity/SecureStore.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

USING_NAMESPACE_BENTLEY_SECURITY

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IdentityAuthenticationPersistence> IdentityAuthenticationPersistencePtr;
struct EXPORT_VTABLE_ATTRIBUTE IdentityAuthenticationPersistence : public IConnectAuthenticationPersistence
    {
    private:
        mutable BeMutex m_cs;
        IJsonLocalState& m_localState;
        std::shared_ptr<ISecureStore> m_secureStore;
        mutable SamlTokenPtr m_token;

    public:
        LICENSING_EXPORT IdentityAuthenticationPersistence(IJsonLocalState* localState = nullptr, std::shared_ptr<ISecureStore> secureStore = nullptr);
        virtual ~IdentityAuthenticationPersistence() {}

        LICENSING_EXPORT void SetToken(SamlTokenPtr token) override;
        LICENSING_EXPORT SamlTokenPtr GetToken() const override;
        LICENSING_EXPORT DateTime GetTokenSetTime() const override;

        //! Does nothing
        LICENSING_EXPORT void SetCredentials(CredentialsCR credentials) override;
        //! Returns empty
        LICENSING_EXPORT Credentials GetCredentials() const override;
    };

END_BENTLEY_LICENSING_NAMESPACE
