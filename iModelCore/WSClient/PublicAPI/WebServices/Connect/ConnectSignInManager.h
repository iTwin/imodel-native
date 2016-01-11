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
#include <WebServices/Connect/SamlToken.h>
#include <DgnClientFx/Utils/Http/AuthenticationHandler.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef AsyncResult<void, AsyncError> SignInResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConnectSignInManager
    {
    public:
        struct UserInfo
            {
            Utf8String username;
            Utf8String firstName;
            Utf8String lastName;
            Utf8String userId;
            };

    private:
        bool IsTokenBasedAuthentication();

    public:
        WSCLIENT_EXPORT ConnectSignInManager();
        WSCLIENT_EXPORT virtual ~ConnectSignInManager();

        WSCLIENT_EXPORT AsyncTaskPtr<SignInResult> SignInWithToken(Utf8StringCR token);
        WSCLIENT_EXPORT AsyncTaskPtr<SignInResult> SignInWithCredentials(CredentialsCR credentials);
        WSCLIENT_EXPORT void SignOut();
        WSCLIENT_EXPORT bool IsSignedIn() const;
        WSCLIENT_EXPORT UserInfo GetUserInfo() const;

        WSCLIENT_EXPORT std::shared_ptr<AuthenticationHandler> GetAuthenticationHandler
            (
            Utf8StringCR serverUrl,
            IHttpHandlerPtr customHandler = nullptr
            );
    };
END_BENTLEY_WEBSERVICES_NAMESPACE
