/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Helpers/OIDC/OidcSignInManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../../Common.h"
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>
#include "OidcTokenProvider.h"

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

#define TOKENPREFIX_Bearer               "Bearer"

typedef AsyncResult<void, AsyncError> OidcSignInResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct OidcSignInManager> OidcSignInManagerPtr;
struct OidcSignInManager : IConnectSignInManager
    {
    private:
        IConnectTokenProviderPtr m_tokenProvider;

        AsyncTaskPtr<WSConnectVoidResult> _CheckAndUpdateToken() override;
        AsyncTaskPtr<WSConnectVoidResult> _SignOut() override;
        bool _IsSignedIn() const override;
        UserInfo _GetUserInfo() const override;
        Utf8String _GetLastUsername() const override;
        IConnectTokenProviderPtr _GetTokenProvider(Utf8StringCR rpUri) const override;
        //! Will be called by CheckUserChange
        void _StoreSignedInUser() override;

        AuthenticationHandlerPtr _GetAuthenticationHandler
            (
            Utf8StringCR serverUrl,
            IHttpHandlerPtr httpHandler = nullptr,
            HeaderPrefix prefix = HeaderPrefix::Token
            ) const override;

    public:
        AsyncTaskPtr<OidcSignInResult> SignInWithCredentials(CredentialsCR credentials);
    };

END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
