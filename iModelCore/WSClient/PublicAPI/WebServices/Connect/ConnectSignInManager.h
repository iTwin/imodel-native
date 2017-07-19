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
#include <BeHttp/AuthenticationHandler.h>
#include <BeSecurity/SecureStore.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SECURITY

typedef AsyncResult<void, AsyncError> SignInResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ConnectSignInManager> ConnectSignInManagerPtr;
struct ConnectSignInManager : IConnectAuthenticationProvider, std::enable_shared_from_this<ConnectSignInManager>
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

    private:
        mutable BeMutex m_cs;

        IImsClientPtr m_client;
        IJsonLocalState& m_localState;
        ISecureStorePtr m_secureStore;

        Configuration m_config;

        AuthenticationType m_authType = AuthenticationType::None;

        IConnectAuthenticationPersistencePtr m_persistence;
        bmap<Utf8String, IConnectTokenProviderPtr> m_tokenProviders;
        std::function<void()> m_tokenExpiredHandler;
        std::function<void()> m_userChangeHandler;
        std::function<void()> m_userSignInHandler;
        std::function<void()> m_userSignOutHandler;

    private:
        ConnectSignInManager(IImsClientPtr client, IJsonLocalState* localState, ISecureStorePtr secureStore);

        void CheckUserChange();
        void StoreSignedInUser();

        AuthenticationType GetAuthenticationType();
        void StoreAuthenticationType(AuthenticationType type);

        IConnectAuthenticationPersistencePtr GetPersistenceMatchingAuthenticationType();
        IConnectTokenProviderPtr GetBaseTokenProviderMatchingAuthenticationType();

        IConnectTokenProviderPtr GetCachedTokenProvider(Utf8StringCR rpUri);
        void ClearSignInData();

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
            IJsonLocalState* localState = nullptr,
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
            IJsonLocalState* localState = nullptr,
            ISecureStorePtr secureStore = nullptr
            );

        WSCLIENT_EXPORT virtual ~ConnectSignInManager();

        //! Change default configuration with new one. Best called before any other calls are done.
        WSCLIENT_EXPORT void Configure(Configuration config);

        //! Sign in using identity token.
        //! Note: Token can be retrieved from IMS sign-in page from browser, in-app web view, or other means.
        WSCLIENT_EXPORT AsyncTaskPtr<SignInResult> SignInWithToken(SamlTokenPtr token);
        //! Sign in using user credentials. Credentials will be used for token retrieval but will not be stored.
        //! Note: this only works for IMS managed users. Federated users from organisations can only use SignInWithToken().
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

        //! Get authentication handler for specific server when signed in.
        //! It will automatically authenticate all HttpRequests that is used with.
        //! Will configure each request to validate TLS certificate depending on UrlProvider environment.
        //! @param serverUrl should contain server URL without any directories
        //! @param httpHandler optional custom HTTP handler to send all requests trough
        //! @param prefix optional custom header prefix to use. Some services require different header format
        WSCLIENT_EXPORT AuthenticationHandlerPtr GetAuthenticationHandler
            (
            Utf8StringCR serverUrl, 
            IHttpHandlerPtr httpHandler = nullptr,
            HeaderPrefix prefix = HeaderPrefix::Token
            ) override;

        //! Get delegation token provider when signed in. Delegation tokens are short lived.
        //! Only use this if AuthenticationHandlerPtr cannot be used.
        //! See IConnectTokenProvider API for more info how to use it
        //! @param rpUri relying party URI to use token for
        WSCLIENT_EXPORT IConnectTokenProviderPtr GetTokenProvider(Utf8StringCR rpUri);

        //! Check if token expired and renew/handle expiration
        WSCLIENT_EXPORT void CheckAndUpdateToken();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
