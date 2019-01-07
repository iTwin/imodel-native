/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Connect/ConnectSessionAuthenticationPersistence.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "IConnectAuthenticationPersistence.h"
#include <BeSecurity/SecureStore.h>
#include <mutex>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ConnectSessionAuthenticationPersistence> ConnectSessionAuthenticationPersistencePtr;
struct EXPORT_VTABLE_ATTRIBUTE ConnectSessionAuthenticationPersistence : public IConnectAuthenticationPersistence
    {
    private:
        SamlTokenPtr m_token;
        Credentials m_credentials;
        DateTime m_tokenSetTime;

    public:
        ConnectSessionAuthenticationPersistence();
        
    public:
        virtual ~ConnectSessionAuthenticationPersistence() {}

        void SetCredentials(CredentialsCR credentials) override;
        Credentials GetCredentials() const override;

        void SetToken(SamlTokenPtr token) override;
        SamlTokenPtr GetToken() const override;

        DateTime GetTokenSetTime() const override;
    };

END_BENTLEY_LICENSING_NAMESPACE
