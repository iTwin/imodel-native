/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
        IConnectTokenProviderPtr _GetTokenProvider(Utf8StringCR rpUri) const override;

        AuthenticationHandlerPtr _GetAuthenticationHandler
            (
            Utf8StringCR serverUrl,
            IHttpHandlerPtr httpHandler = nullptr,
            HeaderPrefix prefix = HeaderPrefix::Token
            ) const override;

    public:
        AsyncTaskPtr<OidcSignInResult> SignInWithCredentials(CredentialsCR credentials);

        OidcSignInManager() : IConnectSignInManager(std::make_shared<RuntimeLocalState>()) {};
    };

END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
