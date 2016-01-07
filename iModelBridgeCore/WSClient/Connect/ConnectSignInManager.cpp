/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectSignInManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// TODO: review unneccesary includes
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectSignInManager.h>

#include <Bentley/Base64Utilities.h>
#include <MobileDgn/MobileDgnUi.h>
#include <WebServices/Connect/Connect.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectSpaces.h>
#include <WebServices/Connect/Authentication.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectTokenProvider.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>

#include "Connect.xliff.h"
#include "AuthenticationData.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::ConnectSignInManager()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::~ConnectSignInManager()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::SignInWithToken(Utf8StringCR tokenStr)
    {
    ConnectAuthenticationPersistence::GetShared()->SetToken(std::make_shared<SamlToken>(tokenStr));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SignInResult> ConnectSignInManager::SignInWithCredentials(CredentialsCR credentials)
    {
    auto token = std::make_shared<SamlToken>();
    StatusInt result = Connect::Login(credentials, *token);
    if (Connect::BC_LOGIN_ERROR == result)
        {
        return CreateCompletedAsyncTask(SignInResult::Error(ConnectLocalizedString(ALERT_SignInFailed_Message)));
        }
    else if (Connect::BC_SUCCESS != result)
        {
        return CreateCompletedAsyncTask(SignInResult::Error(ConnectLocalizedString(ALERT_SignInFailed_ServerError)));
        }

    ConnectAuthenticationPersistence::GetShared()->SetToken(token);
    ConnectAuthenticationPersistence::GetShared()->SetCredentials(credentials);
    return CreateCompletedAsyncTask(SignInResult::Success());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::SignOut()
    {
    ConnectAuthenticationPersistence::GetShared()->SetToken(nullptr);
    ConnectAuthenticationPersistence::GetShared()->SetCredentials(Credentials());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSignInManager::IsSignedIn() const
    {
    return ConnectAuthenticationPersistence::GetShared()->GetToken() != nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::UserInfo ConnectSignInManager::GetUserInfo() const
    {
    UserInfo userInfo;

    if (!IsSignedIn())
        return userInfo;

    SamlTokenPtr token = ConnectAuthenticationPersistence::GetShared()->GetToken();

    bmap<Utf8String, Utf8String> attributes;
    if (SUCCESS != token->GetAttributes(attributes))
        return userInfo;

    userInfo.username = attributes["name"];
    userInfo.firstName = attributes["givenname"];
    userInfo.lastName = attributes["surname"];
    userInfo.userId = attributes["userid"];
    return userInfo;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<AuthenticationHandler> ConnectSignInManager::GetAuthenticationHandler(Utf8StringCR serverUrl, IHttpHandlerPtr customHandler)
    {
    auto persistence = ConnectAuthenticationPersistence::GetShared();
    auto provider = std::make_shared<ConnectTokenProvider>(persistence);
    auto handler = UrlProvider::GetSecurityConfigurator(customHandler);
    return std::make_shared<ConnectAuthenticationHandler>(serverUrl, provider, handler);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSignInManager::IsTokenBasedAuthentication()
    {
    return
        ConnectAuthenticationPersistence::GetShared()->GetToken() != nullptr &&
        ConnectAuthenticationPersistence::GetShared()->GetCredentials().IsEmpty();
    }
