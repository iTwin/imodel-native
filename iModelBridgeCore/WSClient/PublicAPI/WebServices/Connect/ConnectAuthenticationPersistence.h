/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectAuthenticationPersistence.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
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
        WS_EXPORT ConnectAuthenticationPersistence (MobileDgn::ILocalState* customLocalState = nullptr);
        virtual ~ConnectAuthenticationPersistence () {}

        WS_EXPORT void SetCredentials (MobileDgn::Utils::CredentialsCR credentials) override;
        WS_EXPORT MobileDgn::Utils::Credentials GetCredentials () const override;

        WS_EXPORT void SetToken (SamlTokenPtr token) override;
        WS_EXPORT SamlTokenPtr GetToken () const override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
