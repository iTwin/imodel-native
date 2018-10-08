/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectSignInManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
#include <WebServices/Connect/IConnectionClientInterface.h>
#include <MobileDgn/Utils/Http/AuthenticationHandler.h>
#include <MobileDgn/Utils/SecureStore.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

typedef AsyncResult<void, AsyncError> SignInResult;
typedef AsyncResult<SamlTokenPtr, AsyncError> ConnectionClientTokenResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ConnectSignInManager> ConnectSignInManagerPtr;
struct ConnectSignInManager : IConnectAuthenticationProvider, std::enable_shared_from_this<ConnectSignInManager>
    {
    public:
        struct IListener
            {
            virtual ~IListener() {};
            //! Will be called when token expiration is detected
            virtual void _OnUserTokenExpired() {};
            //! Will be called when user change is detected
            virtual void _OnUserChanged() {};
            //! Will be called after finalizing user sign-in
            virtual void _OnUserSignedIn() {};
            //! Will be called after user sign-out
            virtual void _OnUserSignedOut() {};
            //! Will be called after user signs in to Connection Client
            virtual void _OnUserSignedInViaConnectionClient() {};
            };
        struct UserInfo
            {
            Utf8String username;
            Utf8String firstName;
            Utf8String lastName;
            Utf8String userId;
            Utf8String organizationId;
            bool IsComplete() const
                {
                return !(username.Equals("") || firstName.Equals("") || lastName.Equals("") || userId.Equals("") || organizationId.Equals(""));
                };
            };

        struct Configuration
            {
            //! Identity token lifetime to be requested in minutes. Defaults to 1 week.
            //! If application is offline for given lifetime, signed-in state still persist because user would not be able to sign-in anyway.
            //! First connection to network after lifetime will reject expired token and session expiration will be forced.
            //! Session expiration will require user to be re-signed-in.
            uint32_t identityTokenLifetime = 7 * 24 * 60;
            //! Identity token lifetime refresh rate in minutes. Renews token to keep maximum lifetime available.
            uint32_t identityTokenRefreshRate = 60;
            //! Delegation token lifetime to be requested in minutes. Delegation tokens with limited lifetime are meant for services.
            uint32_t delegationTokenLifetime = 60;
            //! Renew delegation token 5 minutes (or other) before it expires. Gets new token before old one expires.
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

        class ConnectionClientListener
            {
            ConnectSignInManager& m_manager;
            static ConnectionClientListener* s_instance;

            public:
                ConnectionClientListener(ConnectSignInManager& manager);
                static void callback(int eventId, WCharCP data);
                void ConnectionClientCallback(int eventId, WCharCP data);
            };

    private:
        mutable BeCriticalSection m_cs;

        IImsClientPtr m_client;
        ILocalState& m_localState;
        ISecureStorePtr m_secureStore;

        Configuration m_config;

        Authentication m_auth;

        IConnectTokenProviderPtr m_publicIdentityTokenProvider;
        mutable bmap<Utf8String, std::shared_ptr<struct DelegationTokenProvider>> m_publicDelegationTokenProviders;

        bset<IListener*> m_listeners;
        std::function<void()> m_tokenExpiredHandler;
        std::function<void()> m_userChangeHandler;
        std::function<void()> m_userSignInHandler;
        std::function<void()> m_userSignOutHandler;
        std::function<void()> m_connectionClientSignInHandler;

        mutable IConnectionClientInterfacePtr m_connectionClient;
        std::shared_ptr<ConnectionClientListener> m_connectionClientListener;

    private:
        ConnectSignInManager(IImsClientPtr client, ILocalState* localState, ISecureStorePtr secureStore, IConnectionClientInterfacePtr connectionClient);

        void CheckUserChange();
        void StoreSignedInUser();

        AuthenticationType ReadAuthenticationType() const;
        void StoreAuthenticationType(AuthenticationType type);

        Authentication CreateAuthentication(AuthenticationType type, IConnectAuthenticationPersistencePtr persistence = nullptr) const;
        void Configure(Authentication& auth) const;
        void Configure(struct DelegationTokenProvider& provider) const;

        IConnectTokenProviderPtr GetCachedTokenProvider(Utf8StringCR rpUri) const;
        void ClearSignInData();

        void CheckAndUpdateTokenNoLock();
        bool IsSignedInNoLock()  const;

        void InitializeConnectionClientInterface() const;

        void _OnUserTokenExpired() const;
        void _OnUserChanged() const;
        void _OnUserSignedIn() const;
        void _OnUserSignedOut() const;
        void _OnUserSignedInViaConnectionClient() const;

    public:
        //! Can be created after MobileDgn is initialized.
        //! Will renew sign-in information asynchronously if needed.
        //! @param clientInfo - client applicaiton info, see ClientInfo documentation for more details
        //! @param httpHandler - custom HttpHandler to route requests trough
        //! @param localState - custom LocalState to store encrypted authentication information between sessions
        //! @param secureStore - custom encryption provider
        //! @param connectionClient - Connection Client API
        WSCLIENT_EXPORT static ConnectSignInManagerPtr Create
            (
            ClientInfoPtr clientInfo,
            IHttpHandlerPtr httpHandler = nullptr,
            ILocalState* localState = nullptr,
            ISecureStorePtr secureStore = nullptr,
            IConnectionClientInterfacePtr connectionClient = nullptr
            );

        //! Can be created after MobileDgn is initialized.
        //! Will renew sign-in information asynchronously if needed.
        //! @param client - custom ImsClient for authenticating user
        //! @param localState - custom LocalState to store encrypted authentication information between sessions
        //! @param secureStore - custom encryption provider
        //! @param connectionClient - Connection Client API
        WSCLIENT_EXPORT static ConnectSignInManagerPtr Create
            (
            IImsClientPtr client,
            ILocalState* localState = nullptr,
            ISecureStorePtr secureStore = nullptr,
            IConnectionClientInterfacePtr connectionClient = nullptr
            );

        WSCLIENT_EXPORT virtual ~ConnectSignInManager();

        //! Change default configuration with new one. Best called before any other calls are done.
        WSCLIENT_EXPORT void Configure(Configuration config);
        
        //! Sign in using identity token. 
        WSCLIENT_EXPORT AsyncTaskPtr<SignInResult> SignInWithToken(SamlTokenPtr token, Utf8StringCR rpUri = nullptr);
        //! Sign in using user credentials. Credentials will be used for future token retrieval.
        WSCLIENT_EXPORT AsyncTaskPtr<SignInResult> SignInWithCredentials(CredentialsCR credentials);
        //! Store sign-in information on disk so user would stay signed-in. Call after successful sign-in.
        WSCLIENT_EXPORT void FinalizeSignIn();

        //! Sign-out user and remove all user information from disk
        WSCLIENT_EXPORT void SignOut();
        //! Check if user is signed-in
        WSCLIENT_EXPORT bool IsSignedIn() const;
        //! Get user information stored in identity token
        WSCLIENT_EXPORT UserInfo GetUserInfo() const;
        //! Get user information stored in token
        WSCLIENT_EXPORT static UserInfo GetUserInfo(SamlTokenCR token);
        //! Get last or current user that was signed in. Returns empty if no user was signed in
        WSCLIENT_EXPORT Utf8String GetLastUsername() const;

        //! Register listener to get user state change events
        WSCLIENT_EXPORT void RegisterListener(IListener* listener);
        //! Unregister listener registered with RegisterListener()
        WSCLIENT_EXPORT void UnregisterListener(IListener* listener);
        
        //! DEPRECATED, Use RegisterListener()! Will be called when token expiration is detected
        WSCLIENT_EXPORT void SetTokenExpiredHandler(std::function<void()> handler);
        //! DEPRECATED, Use RegisterListener()! Will be called when user change is detected. Should be set when starting application.
        WSCLIENT_EXPORT void SetUserChangeHandler(std::function<void()> handler);
        //! DEPRECATED, Use RegisterListener()! Will be called after finalizing user sign-in
        WSCLIENT_EXPORT void SetUserSignInHandler(std::function<void()> handler);
        //! DEPRECATED, Use RegisterListener()! Will be called after user sign-out
        WSCLIENT_EXPORT void SetUserSignOutHandler(std::function<void()> handler);
        //! DEPRECATED, Use RegisterListener()! Will be called after user signs in to Connection Client
        WSCLIENT_EXPORT void SetConnectionClientSignInHandler(std::function<void()> handler);

        //! Get authentication handler for specific server.
        //! Will automatically authenticate all HttpRequests that is used with. 
        //! Will always represent user that is signed-in when authenticating.
        //! Will configure each request to validate TLS certificate depending on UrlProvider environment.
        //! @param serverUrl should contain server URL without any directories
        //! @param httpHandler optional custom HTTP handler to send all given server authenticated requests trough. It will not be used for secure/sensitive token retrieval service.
        WSCLIENT_EXPORT AuthenticationHandlerPtr GetAuthenticationHandler(Utf8StringCR serverUrl, IHttpHandlerPtr httpHandler = nullptr) const override;

        //! Get delegation token provider when signed in. Delegation tokens are short lived.
        //! Only use this if AuthenticationHandlerPtr cannot be used.
        //! Will always represent user that is signed-in when prividing token.
        //! @param rpUri relying party URI to use token for
        WSCLIENT_EXPORT IConnectTokenProviderPtr GetTokenProvider(Utf8StringCR rpUri) const;

        //! Check if token expired and renew/handle expiration
        WSCLIENT_EXPORT void CheckAndUpdateToken();

        //! Check if Connection Client is installed
        //! Should be called first to make sure the API will work correctly
        WSCLIENT_EXPORT bool IsConnectionClientInstalled();

        //! Uses Connection Client API to get a token to sign in with
        WSCLIENT_EXPORT AsyncTaskPtr<ConnectionClientTokenResult> GetConnectionClientToken(Utf8StringCR rpUri);

        //! Uses Connection Client API to listen to events fired by Connection Client
        WSCLIENT_EXPORT void StartConnectionClientListener();

        //! Check if Connection Client listener is started
        WSCLIENT_EXPORT bool IsConnectionClientListenerStarted();

    };

END_BENTLEY_WEBSERVICES_NAMESPACE
