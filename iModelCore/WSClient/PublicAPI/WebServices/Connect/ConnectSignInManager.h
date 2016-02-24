/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectSignInManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <MobileDgn/Utils/Http/AuthenticationHandler.h>
#include <MobileDgn/Utils/SecureStore.h>
#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Connect/IConnectTokenProvider.h>
#include <WebServices/Connect/SamlToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

typedef AsyncResult<void, AsyncError> SignInResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ConnectSignInManager> ConnectSignInManagerPtr;
struct ConnectSignInManager : IConnectAuthenticationProvider
    {
    public:
        struct UserInfo
            {
            Utf8String username;
            Utf8String firstName;
            Utf8String lastName;
            Utf8String userId;
            };

        struct Configuration
            {
            //! Identity token lifetime to be requested in minutes 
            uint64_t identityTokenLifetime = 7 * 24 * 60;
            //! Identity token lifetime refresh rate in minutes 
            uint64_t identityTokenRefreshRate = 60;
            //! Delegation token lifetime to be requested in minutes
            uint64_t delegationTokenLifetime = 60;
            };

    private:
        enum class AuthenticationType
            {
            None = 0,
            Credentials = 1,
            Token = 2
            };

    private:
        mutable BeCriticalSection m_cs;

        ILocalState& m_localState;
        ISecureStorePtr m_secureStore;

        Configuration m_config;

        AuthenticationType m_authType = AuthenticationType::None;

        IConnectAuthenticationPersistencePtr m_persistence;
        bmap<Utf8String, IConnectTokenProviderPtr> m_tokenProviders;
        std::function<void()> m_tokenExpiredHandler;

    private:
        ConnectSignInManager(ILocalState* localState, ISecureStorePtr secureStore);

        AuthenticationType GetAuthenticationType();
        void StoreAuthenticationType(AuthenticationType type);

        IConnectAuthenticationPersistencePtr GetPersistenceMatchingAuthenticationType();
        IConnectTokenProviderPtr GetBaseTokenProviderMatchingAuthenticationType();

        IConnectTokenProviderPtr GetTokenProvider(Utf8StringCR rpUri);
        void ClearSignInData();

    public:
        //! Can be created after MobileDgn is initialized
        WSCLIENT_EXPORT static ConnectSignInManagerPtr Create(ILocalState* localState = nullptr, ISecureStorePtr secureStore = nullptr);
        WSCLIENT_EXPORT virtual ~ConnectSignInManager();

        //! Change default configuration with new one. Best called before any other calls are done.
        WSCLIENT_EXPORT void Configure(Configuration config);

        //! Sign in using identity token. 
        WSCLIENT_EXPORT AsyncTaskPtr<SignInResult> SignInWithToken(SamlTokenPtr token);
        //! Sign in using user credentials. Credentials will be used for future token retrieval.
        WSCLIENT_EXPORT AsyncTaskPtr<SignInResult> SignInWithCredentials(CredentialsCR credentials);
        //! Store sign-in information on disk so user would stay signed-in. Call after successful sign-in.
        WSCLIENT_EXPORT void FinalizeSignIn();

        //! Sign-out user and remove all user information from disk
        WSCLIENT_EXPORT void SignOut();
        //! Check if user is signed-in
        WSCLIENT_EXPORT bool IsSignedIn();
        //! Get user information stored in identity token
        WSCLIENT_EXPORT UserInfo GetUserInfo();

        //! Will be called when token expiration is detected
        WSCLIENT_EXPORT void SetTokenExpiredHandler(std::function<void()> handler);

        //! Get authentication handler for specific server when signed in.
        //! Will configure each request to validate TLS certificate depending on UrlProvider environment.
        //! @param serverUrl should contain server URL without any directories
        //! @param httpHandler optional custom HTTP handler to send all requests trough
        WSCLIENT_EXPORT AuthenticationHandlerPtr GetAuthenticationHandler(Utf8StringCR rpUrl, IHttpHandlerPtr httpHandler = nullptr) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
