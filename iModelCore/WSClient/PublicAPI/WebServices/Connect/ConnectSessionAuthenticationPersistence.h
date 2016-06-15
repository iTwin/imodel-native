/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectSessionAuthenticationPersistence.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <DgnClientFx/DgnClientApp.h>
#include <BeSecurity/SecureStore.h>
#include <mutex>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNCLIENTFX

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

        WSCLIENT_EXPORT void SetCredentials(CredentialsCR credentials) override;
        WSCLIENT_EXPORT Credentials GetCredentials() const override;

        WSCLIENT_EXPORT void SetToken(SamlTokenPtr token) override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken() const override;

        WSCLIENT_EXPORT DateTime GetTokenSetTime() const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
