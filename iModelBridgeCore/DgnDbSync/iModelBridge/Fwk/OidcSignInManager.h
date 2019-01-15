/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/OidcSignInManager.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/Connect/IConnectSignInManager.h>
#include <iModelBridge/iModelBridgeFwkRegistry.h>

#define TOKENPREFIX_Bearer               "Bearer"
BEGIN_BENTLEY_DGN_NAMESPACE

typedef Tasks::AsyncResult<void, Tasks::AsyncError> OidcSignInResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass                           Modified from Algirdas.Mikoliunas    08/2018
+---------------+---------------+---------------+---------------+---------------+------*/

struct OidcSignInManager : WebServices::IConnectSignInManager
    {
    private:
        WebServices::IConnectTokenProviderPtr m_tokenProvider;

        Tasks::AsyncTaskPtr<WebServices::WSConnectVoidResult> _CheckAndUpdateToken() override;
        Tasks::AsyncTaskPtr<WebServices::WSConnectVoidResult> _SignOut() override;
        bool _IsSignedIn() const override;
        UserInfo _GetUserInfo() const override;
        Utf8String _GetLastUsername() const override;
        void _StoreSignedInUser() override;

        WebServices::IConnectTokenProviderPtr _GetTokenProvider(Utf8StringCR rpUri) const override;

        WebServices::AuthenticationHandlerPtr _GetAuthenticationHandler
            (
            Utf8StringCR serverUrl,
            WebServices::IHttpHandlerPtr httpHandler = nullptr,
            HeaderPrefix prefix = HeaderPrefix::Token
            ) const override;

    public:
        OidcSignInManager(Utf8StringCR callBackUrl);
    };
END_BENTLEY_DGN_NAMESPACE
