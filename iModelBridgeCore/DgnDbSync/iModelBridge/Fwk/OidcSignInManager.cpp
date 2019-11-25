/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <WebServices/WebServices.h>
#include <WebServices/Configuration/UrlProvider.h>

#include "OidcSignInManager.h"

#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/iModelHub/Client/OidcTokenProvider.h>
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_IMODELHUB

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
OidcSignInManagerPtr OidcSignInManager::FromCallBack(Utf8StringCR callBackUrl)
    {
    return std::make_shared < OidcSignInManager> (std::make_shared< OidcTokenProvider>(callBackUrl));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
OidcSignInManager::OidcSignInManager(WebServices::IConnectTokenProviderPtr provider)
    : IConnectSignInManager(std::make_shared<RuntimeLocalState>())
    {
    m_tokenProvider = provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
OidcSignInManagerPtr OidcSignInManager::FromAccessToken(Utf8StringCR accesToken)
    {
    return std::make_shared < OidcSignInManager> (std::make_shared< OidcStaticTokenProvider>(accesToken));
    }