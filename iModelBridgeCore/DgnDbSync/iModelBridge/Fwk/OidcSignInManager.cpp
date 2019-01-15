/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/OidcSignInManager.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <WebServices/WebServices.h>
#include <WebServices/Configuration/UrlProvider.h>

#include "OidcSignInManager.h"

#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include "OidcTokenProvider.h"
USING_NAMESPACE_BENTLEY_DGN

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
* @bsiclass                                     John.Majerle                    01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String OidcSignInManager::_GetLastUsername() const
    {
    return IConnectSignInManager::GetLastUsername();
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     John.Majerle                    01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void OidcSignInManager::_StoreSignedInUser() 
    {
    // IConnectSignInManager::StoreSignedInUser();
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
OidcSignInManager::OidcSignInManager(Utf8StringCR callBackUrl)
    : IConnectSignInManager()
    {
    m_tokenProvider = std::make_shared< OidcTokenProvider>(OidcTokenProvider(callBackUrl));
    }


// OidcSignInManager::OidcSignInManager(Utf8StringCR callBackUrl)
//     : IConnectSignInManager(std::make_shared<RuntimeLocalState>())
//     {
//     m_tokenProvider = std::make_shared< OidcTokenProvider>(OidcTokenProvider(callBackUrl));
//     }
