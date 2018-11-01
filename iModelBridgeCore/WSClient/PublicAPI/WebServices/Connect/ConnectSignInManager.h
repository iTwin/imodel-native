/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectSignInManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/IConnectSignInManager.h>
#include <WebServices/Connect/IImsClient.h>
#include <WebServices/Connect/SamlToken.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <BeSecurity/SecureStore.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SECURITY

typedef AsyncResult<void, AsyncError> SignInResult;
typedef AsyncResult<SamlTokenPtr, AsyncError> ConnectionClientTokenResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ConnectSignInManager> ConnectSignInManagerPtr;
struct ConnectSignInManager : IConnectSignInManager, std::enable_shared_from_this<ConnectSignInManager>
    {
    public:
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

        struct ConnectionClientListener
            {
            private:
                ConnectSignInManager& m_manager;
                static ConnectionClientListener* s_instance;

            public:
                ConnectionClientListener(ConnectSignInManager& manager);
                static void callback(int eventId, WCharCP data);
                void ConnectionClientCallback(int eventId, WCharCP data);
            };

    private:
        IImsClientPtr m_client;
        IJsonLocalState& m_localState;
        ISecureStorePtr m_secureStore;

        Configuration m_config;

        Authentication m_auth;

        IConnectTokenProviderPtr m_publicIdentityTokenProvider;
        mutable bmap<Utf8String, std::shared_ptr<struct DelegationTokenProvider>> m_publicDelegationTokenProviders;


        mutable IConnectionClientInterfacePtr m_connectionClient;
        std::shared_ptr<ConnectionClientListener> m_connectionClientListener;

    private:
        void _CheckAndUpdateToken() override;
        void _SignOut() override;
        bool _IsSignedIn() const override;
        UserInfo _GetUserInfo() const override;
        Utf8String _GetLastUsername() const override;
        IConnectTokenProviderPtr _GetTokenProvider(Utf8StringCR rpUri) const override;
        AuthenticationHandlerPtr _GetAuthenticationHandler
            (
            Utf8StringCR serverUrl,
            IHttpHandlerPtr httpHandler = nullptr,
            HeaderPrefix prefix = HeaderPrefix::Token
            ) const override;
        void _StoreSignedInUser() override;

    private:
        ConnectSignInManager(IImsClientPtr client, IJsonLocalState* localState, ISecureStorePtr secureStore, IConnectionClientInterfacePtr connectionClient);

        AuthenticationType ReadAuthenticationType() const;
        void StoreAuthenticationType(AuthenticationType type);
        void StoreConnectEnvironment();

        Authentication CreateAuthentication(AuthenticationType type, IConnectAuthenticationPersistencePtr persistence = nullptr) const;
        void Configure(Authentication& auth) const;
        void Configure(struct DelegationTokenProvider& provider) const;

        void InitializeConnectionClientInterface() const;

    public:
        //! Create manager. Call CheckAndUpdateToken() to update sign-in information seperately.
        //! @param clientInfo - client applicaiton info, see ClientInfo documentation for more details
        //! @param httpHandler - custom HttpHandler to route requests trough
        //! @param localState - custom LocalState to store encrypted authentication information between sessions
        //! @param secureStore - custom encryption provider
        //! @param connectionClient - Connection Client API
        WSCLIENT_EXPORT static ConnectSignInManagerPtr Create
            (
            ClientInfoPtr clientInfo,
            IHttpHandlerPtr httpHandler = nullptr,
            IJsonLocalState* localState = nullptr,
            ISecureStorePtr secureStore = nullptr,
            IConnectionClientInterfacePtr connectionClient = nullptr
            );

        //! Create manager. Call CheckAndUpdateToken() to update sign-in information seperately.
        //! @param client - custom ImsClient for authenticating user
        //! @param localState - custom LocalState to store encrypted authentication information between sessions
        //! @param secureStore - custom encryption provider
        //! @param connectionClient - Connection Client API
        WSCLIENT_EXPORT static ConnectSignInManagerPtr Create
            (
            IImsClientPtr client,
            IJsonLocalState* localState = nullptr,
            ISecureStorePtr secureStore = nullptr,
            IConnectionClientInterfacePtr connectionClient = nullptr
            );


        //! Change default configuration with new one. Best called before any other calls are done.
        WSCLIENT_EXPORT void Configure(Configuration config);

        //! Sign in using identity token.
        //! Note: Token can be retrieved from IMS sign-in page from browser, in-app web view, or other means.
        WSCLIENT_EXPORT AsyncTaskPtr<SignInResult> SignInWithToken(SamlTokenPtr token, Utf8StringCR rpUri = nullptr);
        //! Sign in using user credentials. Credentials will be used for token retrieval but will not be stored.
        //! Note: this only works for IMS managed users. Federated users from organisations can only use SignInWithToken().
        WSCLIENT_EXPORT AsyncTaskPtr<SignInResult> SignInWithCredentials(CredentialsCR credentials);
        //! Store sign-in information on disk so user would stay signed-in. Call after successful sign-in.
        WSCLIENT_EXPORT void FinalizeSignIn();

        //! Get user information stored in token
        WSCLIENT_EXPORT static UserInfo ReadUserInfo(SamlTokenCR token);

        //! Check if Connection Client is installed
        //! Should be called first to make sure the API will work correctly
        WSCLIENT_EXPORT bool IsConnectionClientInstalled();

        //! Uses Connection Client API to get a token to sign in with
        WSCLIENT_EXPORT AsyncTaskPtr<ConnectionClientTokenResult> GetConnectionClientToken(Utf8StringCR rpUri);

        //! Uses Connection Client API to listen to events fired by Connection Client
        WSCLIENT_EXPORT void StartConnectionClientListener();

        //! Get token's connect environment from local state
        WSCLIENT_EXPORT UrlProvider::Environment ReadConnectEnvironment();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
