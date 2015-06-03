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
#include <MobileDgn/Utils/SecureStore.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ConnectAuthenticationPersistence : public IConnectAuthenticationPersistence
    {
    private:
        static bvector<std::function<void ()>> s_onUserChangedCallbacks;

        ILocalState& m_localState;
        std::shared_ptr<ISecureStore> m_secureStore;

        mutable SamlTokenPtr m_token;

    public:
        WSCLIENT_EXPORT ConnectAuthenticationPersistence 
            (
            ILocalState* customLocalState = nullptr,
            std::shared_ptr<ISecureStore> customSecureStore = nullptr
            );
        virtual ~ConnectAuthenticationPersistence () {}

        WSCLIENT_EXPORT void SetCredentials (CredentialsCR credentials) override;
        WSCLIENT_EXPORT Credentials GetCredentials () const override;

        WSCLIENT_EXPORT void SetToken (SamlTokenPtr token) override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken () const override;

        //! Chould only be called when app initializes.
        //! Callback is executed when new user successfully logins thus replacing old user if there was one.
        WSCLIENT_EXPORT static void RegisterUserChangedListener (std::function<void ()> onUserChangedCallback);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
