/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectSignInManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectSignInManager.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/Connect.h>
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectSessionAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectTokenProvider.h>
#include <WebServices/Connect/DelegationTokenProvider.h>
#include "Connect.xliff.h"

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
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::Configure(Configuration config)
    {
    BeCriticalSectionHolder lock(m_cs);
    m_config = config;
    m_tokenProviders.clear();
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

    BeCriticalSectionHolder lock(m_cs);

    ClearSignInData();

    m_persistence = std::make_shared<ConnectSessionAuthenticationPersistence>();
    m_persistence->SetToken(token);

    return CreateCompletedAsyncTask(SignInResult::Success());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SignInResult> ConnectSignInManager::SignInWithCredentials(CredentialsCR credentials)
    {
    BeCriticalSectionHolder lock(m_cs);
    return Connect::Login(credentials, nullptr, nullptr, m_config.identityTokenLifetime)
        ->Then<SignInResult>([=] (SamlTokenResult result)
        {
        if (!result.IsSuccess())
            {
            // TODO: return error directly and avoid additonal localized strings
            if (HttpStatus::Unauthorized == result.GetError().GetHttpStatus())
                return SignInResult::Error(ConnectLocalizedString(ALERT_SignInFailed_Message));

            return SignInResult::Error(ConnectLocalizedString(ALERT_SignInFailed_ServerError));
            }

        BeCriticalSectionHolder lock(m_cs);

        ClearSignInData();

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
    BeCriticalSectionHolder lock(m_cs);
    ClearSignInData();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::ClearSignInData()
    {
    m_persistence = nullptr;
    m_tokenProviders.clear();

    ConnectAuthenticationPersistence::GetShared()->SetToken(nullptr);
    ConnectAuthenticationPersistence::GetShared()->SetCredentials(Credentials());
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
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSignInManager::IsSignedIn() const
    {
    BeCriticalSectionHolder lock(m_cs);
    return nullptr != m_persistence && m_persistence->GetToken() != nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::UserInfo ConnectSignInManager::GetUserInfo() const
    {
    BeCriticalSectionHolder lock(m_cs);

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
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandlerPtr ConnectSignInManager::GetAuthenticationHandler(Utf8StringCR serverUrl, IHttpHandlerPtr httpHandler)
    {
    BeCriticalSectionHolder lock(m_cs);

    // Harcoded URI for first G0505 release (2016 Q1). Should match service server URL in future
    Utf8String rpUri = "https://connect-wsg20.bentley.com";

    auto handler = UrlProvider::GetSecurityConfigurator(httpHandler);
    return std::make_shared<ConnectAuthenticationHandler>(serverUrl, GetTokenProvider(rpUri), handler);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectTokenProviderPtr ConnectSignInManager::GetTokenProvider(Utf8StringCR rpUri)
    {
    auto it = m_tokenProviders.find(rpUri);
    if (it != m_tokenProviders.end())
        return it->second;

    auto baseProvider = std::make_shared<ConnectTokenProvider>(m_persistence, IsTokenBasedAuthentication(), m_tokenExpiredHandler);
    baseProvider->Configure(m_config.identityTokenLifetime, m_config.identityTokenRefreshRate);

    auto delegationProvider = std::make_shared<DelegationTokenProvider>(rpUri, baseProvider);
    delegationProvider->Configure(m_config.delegationTokenLifetime);

    m_tokenProviders[rpUri] = delegationProvider;
    return delegationProvider;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::SetTokenExpiredHandler(std::function<void()> handler)
    {
    m_tokenExpiredHandler = handler;
    }
