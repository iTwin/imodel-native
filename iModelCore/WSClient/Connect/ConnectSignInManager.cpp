/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectSignInManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectSignInManager.h>

#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ImsClient.h>

// These should be removed from public API in future. Currently FieldApps/MobileUtils depend on those APIs.
#include <WebServices/Connect/ConnectAuthenticationHandler.h>
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include <WebServices/Connect/ConnectTokenProvider.h>

#include "Connect.xliff.h"
#include "ConnectSessionAuthenticationPersistence.h"
#include "DelegationTokenProvider.h"
#include "IdentityTokenProvider.h"
#include "IdentityAuthenticationPersistence.h"
#include "WrapperTokenProvider.h"

#ifdef BENTLEY_WIN32
#include "ConnectionClientInterface.h"
#endif

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

#define LOCALSTATE_Namespace            "Connect"
#define LOCALSTATE_AuthenticationType   "AuthenticationType"
#define LOCALSTATE_SignedInUser         "SignedInUser"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::ConnectSignInManager(IImsClientPtr client, ILocalState* localState, ISecureStorePtr secureStore, IConnectionClientInterfacePtr connectionClient) :
m_client(client),
m_localState(localState ? *localState : MobileDgnCommon::LocalState()),
m_secureStore(secureStore ? secureStore : std::make_shared<SecureStore>(&m_localState)),
m_publicIdentityTokenProvider(std::make_shared<WrapperTokenProvider>(m_cs, m_auth.tokenProvider)),
m_connectionClient(connectionClient)
    {
    m_auth = CreateAuthentication(ReadAuthenticationType());

    CheckAndUpdateTokenNoLock();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::~ConnectSignInManager()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManagerPtr ConnectSignInManager::Create
(
ClientInfoPtr clientInfo,
IHttpHandlerPtr httpHandler,
ILocalState* localState,
ISecureStorePtr secureStore,
IConnectionClientInterfacePtr connectionClient
)
    {
    return Create(ImsClient::Create(clientInfo, httpHandler), localState, secureStore, connectionClient);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManagerPtr ConnectSignInManager::Create
(
IImsClientPtr client, 
ILocalState* localState, 
ISecureStorePtr secureStore,
IConnectionClientInterfacePtr connectionClient)
    {
    return std::shared_ptr<ConnectSignInManager>(new ConnectSignInManager(client, localState, secureStore, connectionClient));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::CheckAndUpdateToken()
    {
    BeCriticalSectionHolder lock(m_cs);
    CheckAndUpdateTokenNoLock();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::CheckAndUpdateTokenNoLock()
    {
    if (!IsSignedInNoLock())
        return;

    m_auth.tokenProvider->GetToken(); // Will renew identity token if needed
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::Configure(Configuration config)
    {
    BeCriticalSectionHolder lock(m_cs);
    m_config = config;

    for (auto provider : m_publicDelegationTokenProviders)
        Configure(*provider.second);
    
    Configure(m_auth);
    CheckAndUpdateTokenNoLock();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SignInResult> ConnectSignInManager::SignInWithToken(SamlTokenPtr token, Utf8StringCR rpUri)
    {
    BeCriticalSectionHolder lock(m_cs);

    if (nullptr == token || !token->IsSupported())
        return CreateCompletedAsyncTask(SignInResult::Error(ConnectLocalizedString(ALERT_UnsupportedToken)));

    LOG.infov("ConnectSignIn: sign-in token lifetime %d minutes", token->GetLifetime());

    return m_client->RequestToken(*token, rpUri, m_config.identityTokenLifetime)
        ->Then<SignInResult>([=] (SamlTokenResult result)
        {
        if (!result.IsSuccess())
            return SignInResult::Error(result.GetError());

        BeCriticalSectionHolder lock(m_cs);

        ClearSignInData();

        SamlTokenPtr token = result.GetValue();

        m_auth = CreateAuthentication(AuthenticationType::Token, std::make_shared<ConnectSessionAuthenticationPersistence>());
        m_auth.persistence->SetToken(token);

        CheckUserChange();
        StoreSignedInUser();

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

    return m_client->RequestToken(credentials, nullptr, m_config.identityTokenLifetime)
        ->Then<SignInResult>([=] (SamlTokenResult result)
        {
        if (!result.IsSuccess())
            return SignInResult::Error(result.GetError());

        BeCriticalSectionHolder lock(m_cs);

        ClearSignInData();

        SamlTokenPtr token = result.GetValue();

        m_auth = CreateAuthentication(AuthenticationType::Credentials, std::make_shared<ConnectSessionAuthenticationPersistence>());
        m_auth.persistence->SetToken(token);
        m_auth.persistence->SetCredentials(credentials);

        CheckUserChange();
        StoreSignedInUser();

        LOG.infov("ConnectSignIn: token lifetime %d minutes", token->GetLifetime());
        return SignInResult::Success();
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::FinalizeSignIn()
    {
    m_cs.Enter();
    auto currentPersistence = m_auth.persistence;

    m_auth = CreateAuthentication(m_auth.type);
    m_auth.persistence->SetToken(currentPersistence->GetToken());
    m_auth.persistence->SetCredentials(currentPersistence->GetCredentials());

    StoreAuthenticationType(m_auth.type);
    m_cs.Leave();

    _OnUserSignedIn();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSignInManager::IsSignedIn() const
    {
    BeCriticalSectionHolder lock(m_cs);
    return IsSignedInNoLock();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSignInManager::IsSignedInNoLock() const
    {
    return AuthenticationType::None != m_auth.type;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::SignOut()
    {
    m_cs.Enter();
    ClearSignInData();

    LOG.infov("ConnectSignOut");
    m_cs.Leave();

    _OnUserSignedOut();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::ClearSignInData()
    {
    for (auto provider : m_publicDelegationTokenProviders)
        provider.second->ClearCache();
    
    StoreAuthenticationType(AuthenticationType::None);

    m_auth.persistence->SetToken(nullptr);
    m_auth.persistence->SetCredentials(Credentials());
    m_auth = CreateAuthentication(AuthenticationType::None);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::UserInfo ConnectSignInManager::GetUserInfo() const
    {
    BeCriticalSectionHolder lock(m_cs);

    UserInfo info;

    if (!IsSignedInNoLock())
        return info;

    SamlTokenPtr token = m_auth.persistence->GetToken();
    if (nullptr == token)
        return info;

    info = GetUserInfo(*token);
    return info;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                           Andrius.Paulauskas     06/2016
//--------------------------------------------------------------------------------------
ConnectSignInManager::UserInfo ConnectSignInManager::GetUserInfo(SamlTokenCR token)
    {
    UserInfo info;

    bmap<Utf8String, Utf8String> attributes;
    if (SUCCESS != token.GetAttributes(attributes))
        return info;

    info.username = attributes["name"];
    info.firstName = attributes["givenname"];
    info.lastName = attributes["surname"];
    info.userId = attributes["userid"];
    info.organizationId = attributes["organizationid"];

    return info;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                           Andrius.Paulauskas     06/2016
//--------------------------------------------------------------------------------------
Utf8String ConnectSignInManager::GetLastUsername() const
    {
    return m_secureStore->Decrypt(m_localState.GetValue(LOCALSTATE_Namespace, LOCALSTATE_SignedInUser).asString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::RegisterListener(IListener* listener)
{
    BeCriticalSectionHolder lock(m_cs);
    if (listener == nullptr)
        return;
    m_listeners.insert(listener);
    CheckUserChange();
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::UnregisterListener(IListener* listener)
{
    BeCriticalSectionHolder lock(m_cs);
    m_listeners.erase(listener);
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::_OnUserTokenExpired() const
{
    auto listeners = m_listeners;
    for (auto listener : listeners)
        listener->_OnUserTokenExpired();

    if (m_tokenExpiredHandler)
        m_tokenExpiredHandler();
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::_OnUserChanged() const
{
    auto listeners = m_listeners;
    for (auto listener : listeners)
        listener->_OnUserChanged();

    if (m_userChangeHandler)
        m_userChangeHandler();
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::_OnUserSignedIn() const
{
    auto listeners = m_listeners;
    for (auto listener : listeners)
        listener->_OnUserSignedIn();

    if (m_userSignInHandler)
        m_userSignInHandler();
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::_OnUserSignedOut() const
{
    auto listeners = m_listeners;
    for (auto listener : listeners)
        listener->_OnUserSignedOut();

    if (m_userSignOutHandler)
        m_userSignOutHandler();
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::_OnUserSignedInViaConnectionClient() const
{
    auto listeners = m_listeners;
    for (auto listener : listeners)
        listener->_OnUserSignedInViaConnectionClient();

    if (m_connectionClientSignInHandler)
        m_connectionClientSignInHandler();
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
void ConnectSignInManager::SetUserChangeHandler(std::function<void()> handler)
    {
    BeCriticalSectionHolder lock(m_cs);
    m_userChangeHandler = handler;
    CheckUserChange();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::SetUserSignInHandler(std::function<void()> handler)
    {
    BeCriticalSectionHolder lock(m_cs);
    m_userSignInHandler = handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::SetUserSignOutHandler(std::function<void()> handler)
    {
    BeCriticalSectionHolder lock(m_cs);
    m_userSignOutHandler = handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::SetConnectionClientSignInHandler(std::function<void()> handler)
    {
    BeCriticalSectionHolder lock(m_cs);
    m_connectionClientSignInHandler = handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandlerPtr ConnectSignInManager::GetAuthenticationHandler(Utf8StringCR serverUrl, IHttpHandlerPtr httpHandler) const
    {
    BeCriticalSectionHolder lock(m_cs);

    // Harcoded URI for first G0505 release (2016 Q1). May need to match service server URL in future.
    // Update: This seems to be good enough for IMS as most of services use generic AppliesTo anyway as of 2017-04 
    Utf8String rpUri = "https://connect-wsg20.bentley.com";

    auto handler = UrlProvider::GetSecurityConfigurator(httpHandler);
    return std::make_shared<ConnectAuthenticationHandler>(serverUrl, GetTokenProvider(rpUri), handler, false);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectTokenProviderPtr ConnectSignInManager::GetTokenProvider(Utf8StringCR rpUri) const
    {
    BeCriticalSectionHolder lock(m_cs);
    return GetCachedTokenProvider(rpUri);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectTokenProviderPtr ConnectSignInManager::GetCachedTokenProvider(Utf8StringCR rpUri) const
    {
    auto it = m_publicDelegationTokenProviders.find(rpUri);
    if (it != m_publicDelegationTokenProviders.end())
        return it->second;

    auto provider = std::make_shared<DelegationTokenProvider>(m_client, rpUri, m_publicIdentityTokenProvider);
    Configure(*provider);

    m_publicDelegationTokenProviders[rpUri] = provider;
    return provider;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::AuthenticationType ConnectSignInManager::ReadAuthenticationType() const
    {
    return static_cast<AuthenticationType>(m_localState.GetValue(LOCALSTATE_Namespace, LOCALSTATE_AuthenticationType).asInt());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::StoreAuthenticationType(AuthenticationType type)
    {
    m_localState.SaveValue(LOCALSTATE_Namespace, LOCALSTATE_AuthenticationType, static_cast<int>(type));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::Authentication ConnectSignInManager::CreateAuthentication
(
AuthenticationType type,
IConnectAuthenticationPersistencePtr persistence
) const
    {
    Authentication auth;
    auth.type = type;
    auth.persistence = persistence;

    if (AuthenticationType::Token == type)
        {
        if (!persistence)
            auth.persistence = std::make_shared<IdentityAuthenticationPersistence>(&m_localState, m_secureStore);

        auth.tokenProvider = IdentityTokenProvider::Create(m_client, auth.persistence, [=]
            {
            _OnUserTokenExpired();
            });
        }
    else if (AuthenticationType::Credentials == type)
        {
        if (!persistence)
            auth.persistence = ConnectAuthenticationPersistence::GetShared();

        auth.tokenProvider = std::make_shared<ConnectTokenProvider>(m_client, auth.persistence);
        }
    else
        {
        if (!persistence)
            auth.persistence = std::make_shared<ConnectSessionAuthenticationPersistence>();

        auth.tokenProvider = std::make_shared<ConnectTokenProvider>(m_client, auth.persistence);
        }

    Configure(auth);
    return auth;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::Configure(Authentication& auth) const
    {
    if (!auth.tokenProvider)
        return;

    if (AuthenticationType::Token == auth.type)
        {
        auto provider = static_cast<IdentityTokenProvider*>(auth.tokenProvider.get());
        provider->Configure(m_config.identityTokenLifetime, m_config.identityTokenRefreshRate);
        }
    else if (AuthenticationType::Credentials == auth.type)
        {
        auto provider = static_cast<ConnectTokenProvider*>(auth.tokenProvider.get());
        provider->Configure(m_config.identityTokenLifetime);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::Configure(DelegationTokenProvider& provider) const
    {
    provider.Configure(m_config.delegationTokenLifetime, m_config.delegationTokenExpirationThreshold);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::CheckUserChange()
    {
    if (!IsSignedInNoLock())
        return;

    Utf8String storedUsername = GetLastUsername();
    if (storedUsername.empty())
        return;

    UserInfo info = GetUserInfo();
    BeAssert(!info.username.empty());
    if (info.username == storedUsername)
        return;

    _OnUserChanged();
    StoreSignedInUser();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::StoreSignedInUser()
    {
    UserInfo info = GetUserInfo();
    BeAssert(!info.username.empty());
    m_localState.SaveValue(LOCALSTATE_Namespace, LOCALSTATE_SignedInUser, m_secureStore->Encrypt(info.username.c_str()));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::InitializeConnectionClientInterface() const
    {
    if (m_connectionClient != nullptr)
        return;

#ifdef BENTLEY_WIN32
    m_connectionClient = std::make_shared<ConnectionClientInterface>();
#else
    m_connectionClient = std::make_shared<IConnectionClientInterface>();
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Mark.Uvari          09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSignInManager::IsConnectionClientInstalled()
    {
    BeCriticalSectionHolder lock(m_cs);
    InitializeConnectionClientInterface();

    return m_connectionClient->IsInstalled();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Mark.Uvari          09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<ConnectionClientTokenResult> ConnectSignInManager::GetConnectionClientToken(Utf8StringCR rpUri)
    {
    BeCriticalSectionHolder lock(m_cs);
    InitializeConnectionClientInterface();

    if (!IsConnectionClientInstalled())
        return CreateCompletedAsyncTask(ConnectionClientTokenResult::Error(ConnectLocalizedString(ALERT_ConnectionClientNotLoggedIn_Message)));

    if (!m_connectionClient->IsRunning())
        {
        m_connectionClient->StartClientApp();
        if (!m_connectionClient->IsRunning())
            return CreateCompletedAsyncTask(ConnectionClientTokenResult::Error(ConnectLocalizedString(ALERT_ConnectionClientNotLoggedIn_Message)));
        }

    if (!m_connectionClient->IsLoggedIn())
        return CreateCompletedAsyncTask(ConnectionClientTokenResult::Error(ConnectLocalizedString(ALERT_ConnectionClientNotLoggedIn_Message)));

    Utf8String errorString;
    SamlTokenPtr samlToken = m_connectionClient->GetSerializedDelegateSecurityToken(rpUri, &errorString);
    if (samlToken == nullptr)
        return CreateCompletedAsyncTask(ConnectionClientTokenResult::Error(errorString.empty() ? ConnectLocalizedString(ALERT_UnsupportedToken) : errorString));

    return  CreateCompletedAsyncTask(ConnectionClientTokenResult::Success(samlToken));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Mark.Uvari          09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::StartConnectionClientListener()
    {
    BeCriticalSectionHolder lock(m_cs);
    InitializeConnectionClientInterface();

    if (!IsConnectionClientInstalled())
        return;

    if (!m_connectionClient->IsRunning())
        {
        m_connectionClient->StartClientApp();
        if (!m_connectionClient->IsRunning())
            return;
        }

    //If current user does not match Connection Client user, sign out
    if (IsSignedInNoLock())
        {
        if (m_connectionClient->IsLoggedIn())
            {
            Utf8String ccUserId = m_connectionClient->GetUserId();
            Utf8String loggedInId = GetUserInfo().userId;
            bool equals = loggedInId.EqualsI(ccUserId.c_str());
            if (!equals)
                SignOut();                
            }
        else
            {
            SignOut();
            }
        }

    m_connectionClientListener = std::make_shared<ConnectionClientListener>(*this);
    m_connectionClient->AddClientEventListener(m_connectionClientListener->callback);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::ConnectionClientListener *ConnectSignInManager::ConnectionClientListener::s_instance = nullptr;

ConnectSignInManager::ConnectionClientListener::ConnectionClientListener(ConnectSignInManager& manager) : m_manager(manager)
    {
    s_instance = this;
    }

void ConnectSignInManager::ConnectionClientListener::callback(int eventId, WCharCP data)
    {
    s_instance->ConnectionClientCallback(eventId, data);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::ConnectionClientListener::ConnectionClientCallback(int eventId, WCharCP data)
    {
    if (IConnectionClientInterface::EVENT_TYPE::LOGIN == eventId)
        {
        LOG.infov("Connection Client: Login event");
        m_manager._OnUserSignedInViaConnectionClient();
        }
    else if (IConnectionClientInterface::EVENT_TYPE::LOGOUT == eventId)
        {
        LOG.infov("Connection Client: Logout event");
        m_manager.SignOut();
        }
    else if (IConnectionClientInterface::EVENT_TYPE::STARTUP == eventId)
        {
        LOG.infov("Connection Client: Startup event");
        }
    else if (IConnectionClientInterface::EVENT_TYPE::SHUTDOWN == eventId)
        {
        LOG.infov("Connection Client: Shutdown event");
        m_manager.SignOut();
        }
    else
        LOG.infov("Connection Client: Unknown Event");
    }
