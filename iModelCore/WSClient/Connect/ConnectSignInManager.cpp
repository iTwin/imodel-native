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
#include "IdentityTokenProvider.h"
#include "IdentityAuthenticationPersistence.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

#define LOCALSTATE_Namespace            "Connect"
#define LOCALSTATE_AuthenticationType   "AuthenticationType"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::ConnectSignInManager(ILocalState* localState, ISecureStorePtr secureStore) :
m_localState(localState ? *localState : MobileDgnCommon::LocalState()),
m_secureStore(secureStore ? secureStore : std::make_shared<SecureStore>(&m_localState))
    {
    m_persistence = GetPersistenceMatchingAuthenticationType();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::~ConnectSignInManager()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManagerPtr ConnectSignInManager::Create(ILocalState* localState, ISecureStorePtr secureStore)
    {
    return std::shared_ptr<ConnectSignInManager>(new ConnectSignInManager(localState, secureStore));
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
    BeCriticalSectionHolder lock(m_cs);

    if (nullptr == token || !token->IsSupported())
        return CreateCompletedAsyncTask(SignInResult::Error(ConnectLocalizedString(ALERT_UnsupportedToken)));

    LOG.infov("ConnectSignIn: sign-in token lifetime %d minutes", token->GetLifetime());

    return Connect::RenewToken(*token, nullptr, nullptr, m_config.identityTokenLifetime)
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

        m_authType = AuthenticationType::Token;
        m_persistence = std::make_shared<ConnectSessionAuthenticationPersistence>();
        m_persistence->SetToken(token);

        LOG.infov("ConnectSignIn: renewed token lifetime %d minutes", token->GetLifetime());
        return SignInResult::Success();
        });
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

        m_authType = AuthenticationType::Credentials;
        m_persistence = std::make_shared<ConnectSessionAuthenticationPersistence>();
        m_persistence->SetToken(token);
        m_persistence->SetCredentials(credentials);

        LOG.infov("ConnectSignIn: token lifetime %d minutes", token->GetLifetime());
        return SignInResult::Success();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::FinalizeSignIn()
    {
    auto currentPersistence = m_persistence;

    m_persistence = GetPersistenceMatchingAuthenticationType();
    m_persistence->SetToken(currentPersistence->GetToken());
    m_persistence->SetCredentials(currentPersistence->GetCredentials());

    StoreAuthenticationType(m_authType);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSignInManager::IsSignedIn()
    {
    BeCriticalSectionHolder lock(m_cs);
    return AuthenticationType::None != GetAuthenticationType();
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
    m_tokenProviders.clear();

    StoreAuthenticationType(AuthenticationType::None);

    if (m_persistence)
        {
        m_persistence->SetToken(nullptr);
        m_persistence->SetCredentials(Credentials());
        m_persistence = std::make_shared<ConnectSessionAuthenticationPersistence>();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::UserInfo ConnectSignInManager::GetUserInfo()
    {
    BeCriticalSectionHolder lock(m_cs);

    UserInfo info;

    if (!IsSignedIn())
        return info;

    SamlTokenPtr token = m_persistence->GetToken();
    bmap<Utf8String, Utf8String> attributes;
    if (nullptr == token || SUCCESS != token->GetAttributes(attributes))
        return info;

    info.username = attributes["name"];
    info.firstName = attributes["givenname"];
    info.lastName = attributes["surname"];
    info.userId = attributes["userid"];

    return info;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::SetTokenExpiredHandler(std::function<void()> handler)
    {
    BeCriticalSectionHolder lock(m_cs);
    m_tokenExpiredHandler = handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandlerPtr ConnectSignInManager::GetAuthenticationHandler(Utf8StringCR serverUrl, IHttpHandlerPtr httpHandler)
    {
    BeCriticalSectionHolder lock(m_cs);

    // TODO: Harcoded URI for first G0505 release (2016 Q1). Should match service server URL in future
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

    IConnectTokenProviderPtr baseProvider = GetBaseTokenProviderMatchingAuthenticationType();

    auto delegationProvider = std::make_shared<DelegationTokenProvider>(rpUri, baseProvider);
    delegationProvider->Configure(m_config.delegationTokenLifetime);

    m_tokenProviders[rpUri] = delegationProvider;
    return delegationProvider;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::AuthenticationType ConnectSignInManager::GetAuthenticationType()
    {
    if (AuthenticationType::None != m_authType)
        return m_authType;

    m_authType = static_cast<AuthenticationType>(m_localState.GetValue(LOCALSTATE_Namespace, LOCALSTATE_AuthenticationType).asInt());
    return m_authType;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::StoreAuthenticationType(AuthenticationType type)
    {
    m_authType = type;
    m_localState.SaveValue(LOCALSTATE_Namespace, LOCALSTATE_AuthenticationType, static_cast<int>(type));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectAuthenticationPersistencePtr ConnectSignInManager::GetPersistenceMatchingAuthenticationType()
    {
    AuthenticationType type = GetAuthenticationType();

    if (AuthenticationType::Token == type)
        return std::make_shared<IdentityAuthenticationPersistence>(m_secureStore);

    if (AuthenticationType::Credentials == type)
        return ConnectAuthenticationPersistence::GetShared();

    return std::make_shared<ConnectSessionAuthenticationPersistence>();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectTokenProviderPtr ConnectSignInManager::GetBaseTokenProviderMatchingAuthenticationType()
    {
    AuthenticationType type = GetAuthenticationType();

    if (AuthenticationType::Token == type)
        {
        auto provider = std::make_shared<IdentityTokenProvider>(m_persistence, m_tokenExpiredHandler);
        provider->Configure(m_config.identityTokenLifetime, m_config.identityTokenRefreshRate);
        return provider;
        }

    if (AuthenticationType::Credentials == type)
        {
        auto provider = std::make_shared<ConnectTokenProvider>(m_persistence);
        provider->Configure(m_config.identityTokenLifetime);
        return provider;
        }

    // Not signed in - return failing token provider
    return std::make_shared<ConnectTokenProvider>(std::make_shared<ConnectSessionAuthenticationPersistence>());
    }
