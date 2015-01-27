/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/Connect/ConnectAuthenticationPersistence.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/Connect/IConnectAuthenticationPersistence.h>
#include <MobileDgn/MobileDgnApplication.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ConnectAuthenticationPersistence : public IConnectAuthenticationPersistence
    {
    public:
        MobileDgn::ILocalState& m_localState;

        mutable SamlTokenPtr m_token;

    public:
        WSCLIENT_EXPORT ConnectAuthenticationPersistence (MobileDgn::ILocalState* customLocalState = nullptr);
        virtual ~ConnectAuthenticationPersistence () {}

        WSCLIENT_EXPORT void SetCredentials (MobileDgn::Utils::CredentialsCR credentials) override;
        WSCLIENT_EXPORT MobileDgn::Utils::Credentials GetCredentials () const override;

        WSCLIENT_EXPORT void SetToken (SamlTokenPtr token) override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken () const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
