/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectAuthenticationPersistence.h $
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
USING_NAMESPACE_BENTLEY_SECURITY

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ConnectAuthenticationPersistence> ConnectAuthenticationPersistencePtr;
struct EXPORT_VTABLE_ATTRIBUTE ConnectAuthenticationPersistence : public IConnectAuthenticationPersistence
    {
    private:
        static ConnectAuthenticationPersistencePtr s_shared;
        static std::once_flag s_shared_once;

        mutable BeMutex m_cs;
        IJsonLocalState& m_localState;
        std::shared_ptr<ISecureStore> m_secureStore;

        mutable SamlTokenPtr m_token;

        size_t m_onUserChangedKey;
        bmap<size_t, std::function<void()>> m_onUserChangedListeners;

    private:
        ConnectAuthenticationPersistence
            (
            IJsonLocalState* customLocalState = nullptr,
            std::shared_ptr<ISecureStore> customSecureStore = nullptr
            );

        void UpgradeIfNeeded() const;

    public:
        virtual ~ConnectAuthenticationPersistence() {}

        //! Optional singleton instance initialization with custom parameters. This is not thread safe initialization so should be done
        //! once in application lifecycle. Default behavior does not require initialization.
        WSCLIENT_EXPORT static void CustomInitialize
            (
            IJsonLocalState* customLocalState = nullptr,
            std::shared_ptr<ISecureStore> customSecureStore = nullptr
            );

        //! Get shared thread-safe persistence instance. Data stored in this instance may be available
        //! seperate different apps if SecureStore implmenetation allows it
        WSCLIENT_EXPORT static ConnectAuthenticationPersistencePtr GetShared();

        WSCLIENT_EXPORT void SetCredentials(CredentialsCR credentials) override;
        WSCLIENT_EXPORT Credentials GetCredentials() const override;

        WSCLIENT_EXPORT void SetToken(SamlTokenPtr token) override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken() const override;

        WSCLIENT_EXPORT DateTime GetTokenSetTime() const override;

        //! @param onUserChangedCallback - is executed when new user successfully logins thus replacing old user if there was one.
        //! @return key that can be used to unregister listener
        WSCLIENT_EXPORT size_t RegisterUserChangedListener(std::function<void()> onUserChangedCallback);

        //! @param key - unregister listener that was registered with given key
        WSCLIENT_EXPORT void UnregisterUserChangedListener(size_t key);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
