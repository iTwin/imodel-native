/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/OidcSignInManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/WebServices.h>
#include <WebServices/Configuration/UrlProvider.h>

#include "OidcSignInManager.h"

#include <WebServices/Connect/ConnectAuthenticationHandler.h>

USING_NAMESPACE_BENTLEY_DGN

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Tasks::AsyncTaskPtr<OidcSignInResult> OidcSignInManager::SignInWithCallBack()
    {
    m_tokenProvider = std::make_shared<OidcTokenProvider>();
    return CreateCompletedAsyncTask(OidcSignInResult::Success());
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Tasks::AsyncTaskPtr<WebServices::WSConnectVoidResult> OidcSignInManager::_CheckAndUpdateToken()
    {
    m_tokenProvider->GetToken();

    return CreateCompletedAsyncTask(WebServices::WSConnectVoidResult::Success());
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Tasks::AsyncTaskPtr<WebServices::WSConnectVoidResult> OidcSignInManager::_SignOut()
    {
    m_tokenProvider = nullptr;

    return CreateCompletedAsyncTask(WebServices::WSConnectVoidResult::Success());
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool OidcSignInManager::_IsSignedIn() const
    {
    return nullptr != m_tokenProvider;
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WebServices::IConnectSignInManager::UserInfo OidcSignInManager::_GetUserInfo() const
    {
    return IConnectSignInManager::UserInfo();
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String OidcSignInManager::_GetLastUsername() const
    {
    return "";
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WebServices::IConnectTokenProviderPtr OidcSignInManager::_GetTokenProvider(Utf8StringCR rpUri) const
    {
    return m_tokenProvider;
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void OidcSignInManager::_StoreSignedInUser()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WebServices::AuthenticationHandlerPtr OidcSignInManager::_GetAuthenticationHandler(Utf8StringCR serverUrl, WebServices::IHttpHandlerPtr httpHandler, HeaderPrefix prefix) const
    {
    auto configurationHandler = WebServices::UrlProvider::GetSecurityConfigurator(httpHandler);

    auto connectHandler = WebServices::ConnectAuthenticationHandler::Create
        (
        serverUrl,
        GetTokenProvider(""),
        configurationHandler,
        TOKENPREFIX_Bearer
        );
    connectHandler->EnableExpiredTokenRefresh();
    return connectHandler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OidcSignInManager::OidcSignInManager(WebServices::IConnectTokenProviderPtr m_tokenProvider)
    :m_tokenProvider(m_tokenProvider)
    {

    }