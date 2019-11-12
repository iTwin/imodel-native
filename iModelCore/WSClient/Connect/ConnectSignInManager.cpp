/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Connect/ConnectSignInManager.h>

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
USING_NAMESPACE_BENTLEY_SECURITY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_WEBSERVICES

#define LOCALSTATE_Namespace            "Connect"
#define LOCALSTATE_AuthenticationType   "AuthenticationType"
#define LOCALSTATE_SignedInUser         "SignedInUser"
#define LOCALSTATE_ConnectEnvironment   "ConnectEnvironment"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::ConnectSignInManager(IImsClientPtr client, IJsonLocalState* localState, ISecureStorePtr secureStore, IConnectionClientInterfacePtr connectionClient) :
IConnectSignInManager(std::make_shared<SecureLocalState>(localState, secureStore)),
m_client(client),
m_localState(*localState),
m_secureStore(secureStore ? secureStore : std::make_shared<SecureStore>(*localState)),
m_publicIdentityTokenProvider(std::make_shared<WrapperTokenProvider>(m_mutex, m_auth.tokenProvider)),
m_connectionClient(connectionClient)
    {
    m_auth = CreateAuthentication(ReadAuthenticationType());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManagerPtr ConnectSignInManager::Create
(
ClientInfoPtr clientInfo,
IHttpHandlerPtr httpHandler,
IJsonLocalState* localState,
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
IJsonLocalState* localState, 
ISecureStorePtr secureStore,
IConnectionClientInterfacePtr connectionClient)
    {
    return std::shared_ptr<ConnectSignInManager>(new ConnectSignInManager(client, localState, secureStore, connectionClient));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSConnectVoidResult> ConnectSignInManager::_CheckAndUpdateToken()
    {
    if (!_IsSignedIn())
        return CreateCompletedAsyncTask(WSConnectVoidResult::Success());

    m_auth.tokenProvider->GetToken(); // Will renew identity token if needed
    return CreateCompletedAsyncTask(WSConnectVoidResult::Success());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::Configure(Configuration config)
    {
    BeMutexHolder lock(m_mutex);
    m_config = config;

    for (auto provider : m_publicDelegationTokenProviders)
        Configure(*provider.second);
    
    Configure(m_auth);
    _CheckAndUpdateToken();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SignInResult> ConnectSignInManager::SignInWithToken(SamlTokenPtr token, Utf8StringCR rpUri)
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == token || !token->IsSupported())
        return CreateCompletedAsyncTask(SignInResult::Error(ConnectLocalizedString(ALERT_UnsupportedToken)));

    LOG.infov("ConnectSignIn: sign-in token lifetime %d minutes", token->GetLifetime());

    auto self(shared_from_this());
    return m_client->RequestToken(*token, rpUri, m_config.identityTokenLifetime)
        ->Then<SignInResult>([this, self] (SamlTokenResult result)
        {
        if (!result.IsSuccess())
            return SignInResult::Error(result.GetError());

        BeMutexHolder lock(m_mutex);

        _SignOut();

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
    BeMutexHolder lock(m_mutex);
    
    auto self(shared_from_this());
    return m_client->RequestToken(credentials, nullptr, m_config.identityTokenLifetime)
        ->Then<SignInResult>([this, self, credentials] (SamlTokenResult result)
        {
        if (!result.IsSuccess())
            return SignInResult::Error(result.GetError());

        BeMutexHolder lock(m_mutex);

        _SignOut();

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
    m_mutex.Enter();
    auto currentPersistence = m_auth.persistence;

    m_auth = CreateAuthentication(m_auth.type);
    m_auth.persistence->SetToken(currentPersistence->GetToken());
    m_auth.persistence->SetCredentials(currentPersistence->GetCredentials());

    StoreAuthenticationType(m_auth.type);
    StoreConnectEnvironment();
    m_mutex.Leave();

    OnUserSignedIn();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSignInManager::_IsSignedIn() const
    {
    return AuthenticationType::None != m_auth.type;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<WSConnectVoidResult> ConnectSignInManager::_SignOut()
    {
    for (auto provider : m_publicDelegationTokenProviders)
        provider.second->ClearCache();
    
    StoreAuthenticationType(AuthenticationType::None);

    m_auth.persistence->SetToken(nullptr);
    m_auth.persistence->SetCredentials(Credentials());
    m_auth = CreateAuthentication(AuthenticationType::None);

    return CreateCompletedAsyncTask(WSConnectVoidResult::Success());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           Vytautas.Barkauskas    12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectSignInManager::UserInfo ConnectSignInManager::_GetUserInfo() const
    {
    UserInfo info;

    SamlTokenPtr token = m_auth.persistence->GetToken();
    if (nullptr == token)
        return info;

    info = ReadUserInfo(*token);
    return info;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                           Andrius.Paulauskas     06/2016
//--------------------------------------------------------------------------------------
ConnectSignInManager::UserInfo ConnectSignInManager::ReadUserInfo(SamlTokenCR token)
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AuthenticationHandlerPtr ConnectSignInManager::_GetAuthenticationHandler
(
Utf8StringCR serverUrl,
IHttpHandlerPtr httpHandler,
HeaderPrefix prefix
) const
    {
    // Harcoded URI for first G0505 release (2016 Q1). May need to match service server URL in future.
    // Update: This seems to be good enough for IMS as most of services use generic AppliesTo anyway as of 2017-04 
    Utf8String rpUri = "https://connect-wsg20.bentley.com";

    auto configurationHandler = UrlProvider::GetSecurityConfigurator(httpHandler);

    auto connectHandler = ConnectAuthenticationHandler::Create
        (
        serverUrl,
        GetTokenProvider(rpUri),
        configurationHandler,
        HeaderPrefix::Saml == prefix ? TOKENPREFIX_SAML : TOKENPREFIX_Token
        );
    connectHandler->EnableExpiredTokenRefresh();
    return connectHandler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectTokenProviderPtr ConnectSignInManager::_GetTokenProvider(Utf8StringCR rpUri) const
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
    int type = 0;
    auto typeString = m_secureLocalState->GetValue(LOCALSTATE_Namespace, LOCALSTATE_AuthenticationType);
    if (!typeString.empty())
        type = std::stoi(typeString.c_str());

    return static_cast<AuthenticationType>(type);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::StoreAuthenticationType(AuthenticationType type)
    {
    char str[20];
    BeStringUtilities::FormatUInt64(str, static_cast<int>(type));

    m_secureLocalState->SaveValue(LOCALSTATE_Namespace, LOCALSTATE_AuthenticationType, str);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UrlProvider::Environment ConnectSignInManager::ReadConnectEnvironment()
    {
    auto type = "0";
    auto typeString = m_secureLocalState->GetValue(LOCALSTATE_Namespace, LOCALSTATE_ConnectEnvironment);
    if (!typeString.empty())
        type = typeString.c_str();

    return (UrlProvider::Environment) (std::stoi(type));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectSignInManager::StoreConnectEnvironment()
    {
    char str[20];
    BeStringUtilities::FormatUInt64(str, static_cast<int>(UrlProvider::GetEnvironment()));

    m_secureLocalState->SaveValue(LOCALSTATE_Namespace, LOCALSTATE_ConnectEnvironment, str);
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

        auth.tokenProvider = IdentityTokenProvider::Create(m_client, auth.persistence, [=] ()
            {
            OnUserTokenExpired();
            },
            [=] (bool success, int64_t expireTime)
            {
            OnUserTokenRenew(success, expireTime); 
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
    BeMutexHolder lock(m_mutex);
    InitializeConnectionClientInterface();

    return m_connectionClient->IsInstalled();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Mark.Uvari          09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<ConnectionClientTokenResult> ConnectSignInManager::GetConnectionClientToken(Utf8StringCR rpUri)
    {
    BeMutexHolder lock(m_mutex);
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
    BeMutexHolder lock(m_mutex);
    InitializeConnectionClientInterface();

    if (nullptr != m_connectionClientListener)
        return;

    if (!IsConnectionClientInstalled())
        return;

    if (!m_connectionClient->IsRunning())
        {
        m_connectionClient->StartClientApp();
        if (!m_connectionClient->IsRunning())
            return;
        }

    //If current user does not match Connection Client user, sign out
    if (_IsSignedIn())
        {
        if (m_connectionClient->IsLoggedIn())
            {
            Utf8String ccUserId = m_connectionClient->GetUserId();
            Utf8String loggedInId = _GetUserInfo().userId;
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
* @bsimethod                                            Giedrius.Kairys       09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectSignInManager::IsConnectionClientListenerStarted()
    {
    if (m_connectionClientListener != nullptr)
        return true;

    return false;
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
        m_manager.OnUserSignedInViaConnectionClient();
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
