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
#include <mutex>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ConnectAuthenticationPersistence> ConnectAuthenticationPersistencePtr;
struct EXPORT_VTABLE_ATTRIBUTE ConnectAuthenticationPersistence : public IConnectAuthenticationPersistence
    {
    private:
        static ConnectAuthenticationPersistencePtr s_shared;
        static std::once_flag s_shared_once;

        mutable BeCriticalSection m_cs;
        ILocalState& m_localState;
        std::shared_ptr<ISecureStore> m_secureStore;

        mutable SamlTokenPtr m_token;

        bvector<std::function<void ()>> m_onUserChangedCallbacks;

    private:
        ConnectAuthenticationPersistence
            (
            ILocalState* customLocalState = nullptr,
            std::shared_ptr<ISecureStore> customSecureStore = nullptr
            );

        void UpgradeIfNeeded () const;

    public:
        virtual ~ConnectAuthenticationPersistence () {}

        //! Optional singleton instance initialization with custom parameters. This is not thread safe initialization so should be done 
        //! once in application lifecycle. Default behavior does not require initialization.
        WSCLIENT_EXPORT static void CustomInitialize
            (
            ILocalState* customLocalState = nullptr,
            std::shared_ptr<ISecureStore> customSecureStore = nullptr
            );

        //! Get shared thread-safe persistence instance
        WSCLIENT_EXPORT static ConnectAuthenticationPersistencePtr GetShared ();

        //! Connect credentials are shared between apps if SecureStore implementation allows it
        WSCLIENT_EXPORT void SetCredentials (CredentialsCR credentials) override;
        //! Connect credentials are shared between apps if SecureStore implementation allows it
        WSCLIENT_EXPORT Credentials GetCredentials () const override;

        WSCLIENT_EXPORT void SetToken (SamlTokenPtr token) override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken () const override;

        //! Chould only be called when app initializes.
        //! Callback is executed when new user successfully logins thus replacing old user if there was one.
        WSCLIENT_EXPORT void RegisterUserChangedListener (std::function<void ()> onUserChangedCallback);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
