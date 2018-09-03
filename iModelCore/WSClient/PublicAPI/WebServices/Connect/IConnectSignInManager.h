/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/IConnectSignInManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Connect/IConnectTokenProvider.h>
#include <WebServices/Connect/IConnectionClientInterface.h>
#include <BeHttp/AuthenticationHandler.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IConnectSignInManager> IConnectSignInManagerPtr;
struct IConnectSignInManager : IConnectAuthenticationProvider
    {
    public:
        struct IListener
            {
            virtual ~IListener() {};
            //! Will be called when token expiration is detected
            virtual void _OnUserTokenExpired() {};
            //! Will be called when token expiration is detected
            virtual void _OnUserTokenRenew(bool success, int64_t tokenExpireTimestamp) {};
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

    protected:
        mutable BeMutex m_mutex;

        bset<IListener*> m_listeners;
        std::function<void()> m_tokenExpiredHandler;
        std::function<void()> m_userChangeHandler;
        std::function<void()> m_userSignInHandler;
        std::function<void()> m_userSignOutHandler;
        std::function<void()> m_connectionClientSignInHandler;

    protected:
        void OnUserTokenExpired() const;
        void OnUserTokenRenew(bool success, int64_t expireTime) const;
        void OnUserChanged() const;
        void OnUserSignedIn() const;
        void OnUserSignedOut() const;
        void OnUserSignedInViaConnectionClient() const;

        void CheckUserChange();

    protected:
        //! Will be called thread safe by CheckAndUpdateToken
        virtual void _CheckAndUpdateToken() = 0;
        //! Will be called thread safe by SignOut
        virtual void _SignOut() = 0;
        //! Will be called thread safe by IsSignedIn
        virtual bool _IsSignedIn() const = 0;
        //! Will be called thread safe by GetUserInfo
        virtual UserInfo _GetUserInfo() const = 0;
        //! Will be called thread safe by GetLastUsername
        virtual Utf8String _GetLastUsername() const = 0;
        //! Will be called thread safe by GetTokenProvider
        virtual IConnectTokenProviderPtr _GetTokenProvider(Utf8StringCR rpUri) const = 0;
        //! Will be called thread safe by GetAuthenticationHandler
        virtual AuthenticationHandlerPtr _GetAuthenticationHandler
            (
            Utf8StringCR serverUrl,
            IHttpHandlerPtr httpHandler = nullptr,
            HeaderPrefix prefix = HeaderPrefix::Token
            ) const = 0;
        //! Will be called by CheckUserChange
        virtual void _StoreSignedInUser() = 0;

    public:
        virtual ~IConnectSignInManager() {}

        //! Check if token expired and renew/handle expiration
        WSCLIENT_EXPORT void CheckAndUpdateToken();

        //! Sign-out user and remove all user information from disk
        WSCLIENT_EXPORT void SignOut();

        //! Check if user is signed-in
        //! Uses IsSignedInNoLock, GetLastUsername, StoreSignedInUser
        //! Calls GetUserInfo and listener function
        WSCLIENT_EXPORT bool IsSignedIn() const;

        //! Get user information stored in identity token
        WSCLIENT_EXPORT UserInfo GetUserInfo() const;

        //! Get last or current user that was signed in. Returns empty if no user was signed in
        WSCLIENT_EXPORT Utf8String GetLastUsername() const;

        //! Get authentication handler for specific server.
        //! Will automatically authenticate all HttpRequests that is used with. 
        //! Will always represent user that is signed-in when authenticating.
        //! Will configure each request to validate TLS certificate depending on UrlProvider environment.
        //! @param serverUrl should contain server URL without any directories
        //! @param httpHandler optional custom HTTP handler to send all given server authenticated requests trough. It will not be used for secure/sensitive token retrieval service.
        //! @param prefix optional custom header prefix to use. Some services require different header format
        WSCLIENT_EXPORT AuthenticationHandlerPtr GetAuthenticationHandler
            (
            Utf8StringCR serverUrl,
            IHttpHandlerPtr httpHandler = nullptr,
            HeaderPrefix prefix = HeaderPrefix::Token
            ) const override;

        //! Get delegation token provider when signed in. Delegation tokens are short lived.
        //! Only use this if AuthenticationHandlerPtr cannot be used.
        //! Will always represent user that is signed-in when prividing token.
        //! @param rpUri relying party URI to use token for
        WSCLIENT_EXPORT IConnectTokenProviderPtr GetTokenProvider(Utf8StringCR rpUri) const;

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
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
