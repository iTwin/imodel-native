/*--------------------------------------------------------------------------------------+
|
|     $Source: ConnectC/CWSCC.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CWSCCInternal.h"


/*---------------------------------------------------------------------------------**//**
* Constructor.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithToken
(
WCharCP wAuthenticatedToken,
WCharCP wTemporaryDirectory,
WCharCP wAssetsRootDirectory,
WCharCP wApplicationName,
WCharCP wApplicationVersion,
WCharCP wApplicationGUID,
WCharCP wApplicationProductId
)
    {
    Utf8String authenticatedToken;
    BeFileName temporaryDirectory(wTemporaryDirectory);
    BeFileName assetsRootDirectory(wAssetsRootDirectory);
    Utf8String applicationName;
    Utf8String utf8ApplicationVersion;
    Utf8String applicationGUID;
    Utf8String applicationProductId;
    BeStringUtilities::WCharToUtf8(authenticatedToken, wAuthenticatedToken);
    BeStringUtilities::WCharToUtf8(applicationName, wApplicationName);
    BeStringUtilities::WCharToUtf8(utf8ApplicationVersion, wApplicationVersion);
    BeStringUtilities::WCharToUtf8(applicationGUID, wApplicationGUID);
    BeStringUtilities::WCharToUtf8(applicationProductId, wApplicationProductId);

    BeVersion applicationVersion(utf8ApplicationVersion.c_str());

    LPCWSCC api = new ConnectWebServicesClientC_internal
        (
        authenticatedToken,
        temporaryDirectory,
        assetsRootDirectory,
        applicationName,
        applicationVersion,
        applicationGUID,
        applicationProductId
        );

    if (api->m_solrClientPtr == nullptr)
        return nullptr;
    return (CWSCCHANDLE) api;
    }

/*---------------------------------------------------------------------------------**//**
* Constructor.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithCredentials
(
WCharCP wUsername,
WCharCP wPassword,
WCharCP wTemporaryDirectory,
WCharCP wAssetsRootDirectory,
WCharCP wApplicationName,
WCharCP wApplicationVersion,
WCharCP wApplicationGUID,
WCharCP wApplicationProductId
)
    {
    Utf8String username;
    Utf8String password;
    BeFileName temporaryDirectory(wTemporaryDirectory);
    BeFileName assetsRootDirectory(wAssetsRootDirectory);
    Utf8String applicationName;
    Utf8String utf8ApplicationVersion;
    Utf8String applicationGUID;
    Utf8String applicationProductId;

    BeStringUtilities::WCharToUtf8(username, wUsername);
    BeStringUtilities::WCharToUtf8(password, wPassword);
    BeStringUtilities::WCharToUtf8(applicationName, wApplicationName);
    BeStringUtilities::WCharToUtf8(utf8ApplicationVersion, wApplicationVersion);
    BeStringUtilities::WCharToUtf8(applicationGUID, wApplicationGUID);
    BeStringUtilities::WCharToUtf8(applicationProductId, wApplicationProductId);

    BeVersion applicationVersion(utf8ApplicationVersion.c_str());

    LPCWSCC api = new ConnectWebServicesClientC_internal
        (
        username,
        password,
        temporaryDirectory,
        assetsRootDirectory,
        applicationName,
        applicationVersion,
        applicationGUID,
        applicationProductId
        );

    if (api->m_solrClientPtr == nullptr)
        return nullptr;
    return (CWSCCHANDLE) api;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus httperrorToConnectWebServicesClientStatus(LPCWSCC api, HttpStatus status, Utf8StringCR message, Utf8StringCR description)
    {
    api->SetStatusMessage(message);
    api->SetStatusDescription(description);
    switch (status)
        {
        case HttpStatus::InternalServerError:
            return ERROR500;
        case HttpStatus::BadRequest:
            return ERROR400;
        case HttpStatus::NotFound:
            return ERROR404;
        default:
            return ERROR500;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_GetIMSUserInfo(CWSCCHANDLE apiHandle, CWSCCDATABUFHANDLE* userBuffer)
    {
    VERIFY_API

    if (userBuffer == NULL)
        {
        api->SetStatusMessage("The userBuffer parameter is NULL.");
        api->SetStatusDescription("The userBuffer parameter passed into ConnectWebServicesClientC_GetIMSUserInfo is invalid.");
        return INVALID_PARAMETER;
        }

    auto result = api->m_solrClientPtr->SendGetRequest()->GetResult();
    if (!result.IsSuccess())
        return httperrorToConnectWebServicesClientStatus(api, result.GetError().GetHttpStatus(), result.GetError().GetDisplayMessage(), result.GetError().GetDisplayDescription());
    
    api->SetStatusMessage("Success!");
    api->SetStatusDescription("The IMS user info was successfully retreived.");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP ConnectWebServicesClientC_GetLastStatusMessage(CWSCCHANDLE apiHandle)
    {
    if (NULL == apiHandle)
        return "apiHandle passed into ConnectWebServicesClientC_GetLastStatusMessage is NULL.";
    LPCWSCC api = (LPCWSCC) apiHandle;

    return api->GetLastStatusMessage().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP ConnectWebServicesClientC_GetLastStatusDescription(CWSCCHANDLE apiHandle)
    {
    if (NULL == apiHandle)
        return "apiHandle passed into ConnectWebServicesClientC_GetLastStatusDescription is NULL.";
    LPCWSCC api = (LPCWSCC) apiHandle;

    return api->GetLastStatusDescription().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_ConfigureWebProxy(CWSCCHANDLE apiHandle, Utf8CP proxyUrl)
    {
    VERIFY_API

    if (proxyUrl == nullptr)
        {
        api->SetStatusMessage("The proxyUrl is a nullptr.");
        api->SetStatusDescription("proxyUrl cannot be a null pointer when creating a Web Proxy.");
        return INVALID_PARAMETER;
        }

    api->CreateProxyHttpClient(proxyUrl);
    api->SetStatusMessage("Success!");
    api->SetStatusDescription("The WSRepositoryClient was successfully created.");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_ConfigureWebProxyWithCredentials
(
CWSCCHANDLE apiHandle,
Utf8CP proxyUrl,
Utf8CP username,
Utf8CP password
)
    {
    VERIFY_API

    if (proxyUrl == nullptr)
        {
        api->SetStatusMessage("The proxyUrl is a nullptr.");
        api->SetStatusDescription("proxyUrl cannot be a null pointer when creating a Web Proxy.");
        return INVALID_PARAMETER;
        }

    if (username == nullptr || password == nullptr)
        {
        api->SetStatusMessage("The username of password is a nullptr.");
        api->SetStatusDescription("username and password cannot be null pointers when configuring a Web Proxy.");
        return INVALID_PARAMETER;
        }

    api->CreateProxyHttpClient(proxyUrl, username, password);
    api->SetStatusMessage("Success!");
    api->SetStatusDescription("The WSRepositoryClient was successfully created.");
    return SUCCESS;
    }

WSLocalState ConnectWebServicesClientC_internal::m_localState = WSLocalState();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectWebServicesClientC_internal::ConnectWebServicesClientC_internal
(
Utf8String authenticatedToken,
BeFileName temporaryDirectory,
BeFileName assetsRootDirectory,
Utf8String applicationName,
BeVersion applicationVersion,
Utf8String applicationGUID,
Utf8String applicationProductId
)
    {
    Initialize
        (
        temporaryDirectory,
        assetsRootDirectory,
        applicationName,
        applicationVersion,
        applicationGUID,
        applicationProductId
        );

    SamlTokenPtr token = make_shared<SamlToken>(authenticatedToken);
    auto result = m_connectSignInManager->SignInWithToken(token)->GetResult();
    if (!result.IsSuccess())
        return;

    //IMS-Search API
    shared_ptr<AuthenticationHandler> samlAuthHandler = std::make_shared<ConnectAuthenticationHandler>
        (
        UrlProvider::Urls::ConnectWsgGlobal.Get(),
        m_connectSignInManager->GetTokenProvider(UrlProvider::Urls::ConnectWsgGlobal.Get()),
        m_proxy, 
        true
        );
    Utf8String searchApiUrl = "https://qa-waz-search.bentley.com/token";
    Utf8String collection = "IMS/User";
    m_solrClientPtr = SolrClient::Create(searchApiUrl, collection, m_clientInfo, samlAuthHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectWebServicesClientC_internal::ConnectWebServicesClientC_internal
(
Utf8String username,
Utf8String password,
BeFileName temporaryDirectory,
BeFileName assetsRootDirectory,
Utf8String applicationName,
BeVersion applicationVersion,
Utf8String applicationGUID,
Utf8String applicationProductId
)
    {
    Initialize
        (
        temporaryDirectory,
        assetsRootDirectory,
        applicationName,
        applicationVersion,
        applicationGUID,
        applicationProductId
        );

    Credentials credentials(username, password);
    auto result = m_connectSignInManager->SignInWithCredentials(credentials)->GetResult();
    if (!result.IsSuccess())
        return;

    //IMS-Search API
    shared_ptr<AuthenticationHandler> samlAuthHandler = std::make_shared<ConnectAuthenticationHandler>
        (
        UrlProvider::Urls::ConnectWsgGlobal.Get(), 
        m_connectSignInManager->GetTokenProvider(UrlProvider::Urls::ConnectWsgGlobal.Get()),
        m_proxy, 
        true
        );
    Utf8String searchApiUrl = "https://qa-waz-search.bentley.com/token";
    Utf8String collection = "IMS/User";
    m_solrClientPtr = SolrClient::Create(searchApiUrl, collection, m_clientInfo, samlAuthHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectWebServicesClientC_internal::Initialize
(
BeFileName temporaryDirectory,
BeFileName assetsRootDirectory,
Utf8String applicationName,
BeVersion applicationVersion,
Utf8String applicationGUID,
Utf8String applicationProductId
)
    {

    m_lastStatusMessage = Utf8String("");
    m_lastStatusDescription = Utf8String("");

    BeFileName::CreateNewDirectory(temporaryDirectory);
    BeSQLite::BeSQLiteLib::Initialize(temporaryDirectory);
    //BeSQLite::EC::ECDb::Initialize(m_pathProv.GetTemporaryDirectory(), &m_pathProv.GetAssetsRootDirectory()); //TODO: Is this needed?

    BeFileName dgnClientFxSqlangFile = assetsRootDirectory;
    dgnClientFxSqlangFile.AppendToPath(L"sqlang");
#if !defined (NDEBUG)
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_pseudo.sqlang.db3");
#else
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_en.sqlang.db3");
#endif

    BeSQLite::L10N::SqlangFiles sqlangFiles(dgnClientFxSqlangFile); //Prob needed
    DgnClientFxL10N::ReInitialize(sqlangFiles, sqlangFiles); //needed

#if !defined (NDEBUG)
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &m_localState, nullptr);
#else
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &m_localState, nullptr);
#endif
    m_clientInfo = ClientInfo::Create(applicationName, applicationVersion, applicationGUID, applicationProductId);
    m_connectSignInManager = ConnectSignInManager::Create(m_clientInfo, nullptr, &m_localState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectWebServicesClientC_internal::CreateProxyHttpClient
(
Utf8String proxyUrl,
Utf8String username,
Utf8String password
)
    {
    m_proxy = ProxyHttpHandler::GetProxyIfReachable(proxyUrl);
    if (!Utf8String::IsNullOrEmpty(username.c_str()) && !Utf8String::IsNullOrEmpty(password.c_str()))
        m_proxy->SetProxyCredentials(Credentials(username, password));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectWebServicesClientC_internal::CreateWSRepositoryClient
(
Utf8String serverUrl,
Utf8String repositoryId
)
    {
    auto authHandler = m_connectSignInManager->GetAuthenticationHandler(serverUrl, m_proxy);
    auto clientPtr = WSRepositoryClient::Create(serverUrl, repositoryId, m_clientInfo, nullptr, authHandler);
    m_repositoryClients.insert(make_bpair(serverUrl + repositoryId, clientPtr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectWebServicesClientC_internal::SetStatusMessage(Utf8String message)
    {
    m_lastStatusMessage = message;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectWebServicesClientC_internal::SetStatusDescription(Utf8String desc)
    {
    m_lastStatusDescription = desc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ConnectWebServicesClientC_internal::GetLastStatusMessage()
    {
    return m_lastStatusMessage;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ConnectWebServicesClientC_internal::GetLastStatusDescription()
    {
    return m_lastStatusDescription;
    }