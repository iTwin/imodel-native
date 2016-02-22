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
#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Connect/IConnectTokenProvider.h>
#include <WebServices/Connect/SamlToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

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
        mutable BeCriticalSection m_cs;
        Configuration m_config;
        IConnectAuthenticationPersistencePtr m_persistence;
        bmap<Utf8String, IConnectTokenProviderPtr> m_tokenProviders;
        std::function<void()> m_tokenExpiredHandler;

    private:
        ConnectSignInManager();
        bool IsTokenBasedAuthentication();
        IConnectTokenProviderPtr GetTokenProvider(Utf8StringCR rpUri);
        void ClearSignInData();

    public:
        //! Must be created after mobileDgn is initialized
        WSCLIENT_EXPORT static ConnectSignInManagerPtr Create();

        WSCLIENT_EXPORT virtual ~ConnectSignInManager();

        //! Change default configuration with new one. Best called before any other calls are done.
        WSCLIENT_EXPORT void Configure(Configuration config);

        WSCLIENT_EXPORT AsyncTaskPtr<SignInResult> SignInWithToken(SamlTokenPtr token);
        WSCLIENT_EXPORT AsyncTaskPtr<SignInResult> SignInWithCredentials(CredentialsCR credentials);
        WSCLIENT_EXPORT void FinalizeSignIn();

        WSCLIENT_EXPORT void SignOut();
        WSCLIENT_EXPORT bool IsSignedIn() const;
        WSCLIENT_EXPORT UserInfo GetUserInfo() const;

        //! Will be called when token expiration is detected
        WSCLIENT_EXPORT void SetTokenExpiredHandler(std::function<void()> handler);

        //! Get authentication handler for specific server.
        //! Will configure each request to validate TLS certificate depending on UrlProvider environment.
        //! @param serverUrl should contain server URL without any directories
        //! @param httpHandler optional custom HTTP handler to send all requests trough
        WSCLIENT_EXPORT AuthenticationHandlerPtr GetAuthenticationHandler(Utf8StringCR rpUrl, IHttpHandlerPtr httpHandler = nullptr) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
