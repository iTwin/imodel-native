/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectSignInManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Connect/IConnectTokenProvider.h>
#include <WebServices/Connect/IImsClient.h>
#include <WebServices/Connect/SamlToken.h>
#include <MobileDgn/Utils/Http/AuthenticationHandler.h>
#include <MobileDgn/Utils/SecureStore.h>

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
            uint32_t identityTokenLifetime = 7 * 24 * 60;
            //! Identity token lifetime refresh rate in minutes 
            uint32_t identityTokenRefreshRate = 60;
            //! Delegation token lifetime to be requested in minutes
            uint32_t delegationTokenLifetime = 60;
            //! Renew delegation token 5 minutes (or other) before it expires
            uint32_t delegationTokenExpirationThreshold = 5;
            };

    private:
        enum class AuthenticationType
            {
            None = 0,
            Credentials = 1,
            Token = 2
            };

        struct Authentication
            {
            AuthenticationType type = AuthenticationType::None;
            IConnectAuthenticationPersistencePtr persistence;
            IConnectTokenProviderPtr tokenProvider;
            };

    private:
        mutable BeCriticalSection m_cs;

        IImsClientPtr m_client;
        ILocalState& m_localState;
        ISecureStorePtr m_secureStore;

        Configuration m_config;

        Authentication m_auth;
        bmap<Utf8String, IConnectTokenProviderPtr> m_tokenProviders;

        std::function<void()> m_tokenExpiredHandler;
        std::function<void()> m_userChangeHandler;
        std::function<void()> m_userSignInHandler;
        std::function<void()> m_userSignOutHandler;

    private:
        ConnectSignInManager(IImsClientPtr client, ILocalState* localState, ISecureStorePtr secureStore);

        void CheckUserChange();
        void StoreSignedInUser();

        AuthenticationType ReadAuthenticationType();
        void StoreAuthenticationType(AuthenticationType type);

        Authentication CreateAuthentication(AuthenticationType type, IConnectAuthenticationPersistencePtr persistence = nullptr);
        void Configure(Authentication& auth);

        IConnectTokenProviderPtr GetCachedTokenProvider(Utf8StringCR rpUri);
        void ClearSignInData();

        void CheckAndUpdateTokenNoLock();
        bool IsSignedInNoLock();

    public:
        //! Can be created after MobileDgn is initialized.
        //! Will renew sign-in information asynchronously if needed.
        //! @param clientInfo - client applicaiton info, see ClientInfo documentation for more details
        //! @param httpHandler - custom HttpHandler to route requests trough
        //! @param localState - custom LocalState to store encrypted authentication information between sessions
        //! @param secureStore - custom encryption provider
        WSCLIENT_EXPORT static ConnectSignInManagerPtr Create
            (
            ClientInfoPtr clientInfo,
            IHttpHandlerPtr httpHandler = nullptr,
            ILocalState* localState = nullptr,
            ISecureStorePtr secureStore = nullptr
            );

        //! Can be created after MobileDgn is initialized.
        //! Will renew sign-in information asynchronously if needed.
        //! @param client - custom ImsClient for authenticating user
        //! @param localState - custom LocalState to store encrypted authentication information between sessions
        //! @param secureStore - custom encryption provider
        WSCLIENT_EXPORT static ConnectSignInManagerPtr Create
            (
            IImsClientPtr client,
            ILocalState* localState = nullptr,
            ISecureStorePtr secureStore = nullptr
            );

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
        //! Get user information stored in token
        WSCLIENT_EXPORT static UserInfo GetUserInfo(SamlTokenCR token);
        //! Get last or current user that was signed in. Returns empty if no user was signed in
        WSCLIENT_EXPORT Utf8String GetLastUsername();

        //! Will be called when token expiration is detected
        WSCLIENT_EXPORT void SetTokenExpiredHandler(std::function<void()> handler);

        //! Will be called when user change is detected. Should be set when starting application.
        WSCLIENT_EXPORT void SetUserChangeHandler(std::function<void()> handler);

        //! Will be called after finalizing user sign-in
        WSCLIENT_EXPORT void SetUserSignInHandler(std::function<void()> handler);

        //! Will be called after user sign-out
        WSCLIENT_EXPORT void SetUserSignOutHandler(std::function<void()> handler);

        //! Get authentication handler for specific server.
        //! Will automatically authenticate all HttpRequests that is used with. 
        //! Will always represent user that is signed-in when authenticating.
        //! Will configure each request to validate TLS certificate depending on UrlProvider environment.
        //! @param serverUrl should contain server URL without any directories
        //! @param httpHandler optional custom HTTP handler to send all requests trough
        WSCLIENT_EXPORT AuthenticationHandlerPtr GetAuthenticationHandler(Utf8StringCR serverUrl, IHttpHandlerPtr httpHandler = nullptr) override;

        //! Get delegation token provider when signed in. Delegation tokens are short lived.
        //! Only use this if AuthenticationHandlerPtr cannot be used.
        //! Will always represent user that is signed-in when prividing token.
        //! @param rpUri relying party URI to use token for
        WSCLIENT_EXPORT IConnectTokenProviderPtr GetTokenProvider(Utf8StringCR rpUri);

        //! Check if token expired and renew/handle expiration
        WSCLIENT_EXPORT void CheckAndUpdateToken();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
