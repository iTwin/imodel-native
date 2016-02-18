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
#include <WebServices/Connect/ConnectSessionAuthenticationPersistence.h>

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
    {
    if (ConnectAuthenticationPersistence::GetShared()->GetToken() != nullptr)
        m_persistence = ConnectAuthenticationPersistence::GetShared();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::~ConnectSignInManager()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManagerPtr ConnectSignInManager::Create()
    {
    return std::shared_ptr<ConnectSignInManager>(new ConnectSignInManager());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SignInResult> ConnectSignInManager::SignInWithToken(SamlTokenPtr token)
    {
    if (!token->IsSupported())
        {
        return CreateCompletedAsyncTask(SignInResult::Error(ConnectLocalizedString(ALERT_UnsupportedToken)));
        }

    m_persistence = std::make_shared<ConnectSessionAuthenticationPersistence>();
    m_persistence->SetToken(token);

    return CreateCompletedAsyncTask(SignInResult::Success());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SignInResult> ConnectSignInManager::SignInWithCredentials(CredentialsCR credentials)
    {
    return Connect::Login(credentials)->Then<SignInResult>([=] (SamlTokenResult result)
        {
        if (!result.IsSuccess())
            {
            // TODO: return error directly and avoid additonal localized strings
            if (HttpStatus::Unauthorized == result.GetError().GetHttpStatus())
                return SignInResult::Error(ConnectLocalizedString(ALERT_SignInFailed_Message));

            return SignInResult::Error(ConnectLocalizedString(ALERT_SignInFailed_ServerError));
            }

        SamlTokenPtr token = result.GetValue();

        m_persistence = std::make_shared<ConnectSessionAuthenticationPersistence>();
        m_persistence->SetToken(token);
        m_persistence->SetCredentials(credentials);

        return SignInResult::Success();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::FinalizeSignIn()
    {
    auto currentPersistence = m_persistence;
    m_persistence = ConnectAuthenticationPersistence::GetShared();
    m_persistence->SetToken(currentPersistence->GetToken());
    m_persistence->SetCredentials(currentPersistence->GetCredentials());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::SignOut()
    {
    m_persistence = nullptr;
    ConnectAuthenticationPersistence::GetShared()->SetToken(nullptr);
    ConnectAuthenticationPersistence::GetShared()->SetCredentials(Credentials());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSignInManager::IsSignedIn() const
    {
    return nullptr != m_persistence && m_persistence->GetToken() != nullptr;
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
    auto provider = std::make_shared<ConnectTokenProvider>(m_persistence, IsTokenBasedAuthentication(), m_tokenExpiredHandler);
    auto handler = UrlProvider::GetSecurityConfigurator(customHandler);
    return std::make_shared<ConnectAuthenticationHandler>(serverUrl, provider, handler);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSignInManager::IsTokenBasedAuthentication()
    {
    return
        m_persistence != nullptr &&
        m_persistence->GetToken() != nullptr &&
        m_persistence->GetCredentials().IsEmpty();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::SetTokenExpiredHandler(std::function<void()> handler)
    {
    m_tokenExpiredHandler = handler;
    }
