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
CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithToken(WCharCP wAuthenticatedToken, uint32_t productId)
    {
    Utf8String authenticatedToken;
    BeStringUtilities::WCharToUtf8(authenticatedToken, wAuthenticatedToken);
    LPCWSCC api = new ConnectWebServicesClientC_internal(authenticatedToken, productId);
    if (api->m_wsRepositoryClientPtr == nullptr || api->m_solrClientPtr == nullptr)
        return nullptr;
    return (CWSCCHANDLE) api;
    }

/*---------------------------------------------------------------------------------**//**
* Constructor.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithCredentials(WCharCP wUsername, WCharCP wPassword, uint32_t productId)
    {
    Utf8String username, password;
    BeStringUtilities::WCharToUtf8(username, wUsername);
    BeStringUtilities::WCharToUtf8(password, wPassword);
    LPCWSCC api = new ConnectWebServicesClientC_internal(username, password, productId);
    if (api->m_wsRepositoryClientPtr == nullptr || api->m_solrClientPtr == nullptr)
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

WSLocalState ConnectWebServicesClientC_internal::m_localState = WSLocalState();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectWebServicesClientC_internal::ConnectWebServicesClientC_internal(Utf8String tokenXml, uint32_t productId)
    {
    DgnClientFxCommon::SetApplicationPathsProvider(&m_pathProv);

    BeFileName::CreateNewDirectory(m_pathProv.GetTemporaryDirectory());
    BeSQLite::BeSQLiteLib::Initialize(m_pathProv.GetTemporaryDirectory()); //Prob needed
    BeSQLite::EC::ECDb::Initialize(m_pathProv.GetTemporaryDirectory(), &m_pathProv.GetAssetsRootDirectory()); //TODO: Is this needed?

    BeFileName dgnClientFxSqlangFile = m_pathProv.GetAssetsRootDirectory();
    dgnClientFxSqlangFile.AppendToPath(L"sqlang");
#if !defined (NDEBUG)
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_pseudo.sqlang.db3");
#else
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_en.sqlang.db3");
#endif

    BeSQLite::L10N::SqlangFiles sqlangFiles(dgnClientFxSqlangFile); //Prob needed
    DgnClientFxL10N::ReInitialize(sqlangFiles, sqlangFiles); //needed

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    ConnectAuthenticationPersistence::CustomInitialize(&m_localState);
    auto bclient = make_shared<BuddiClient>(proxy);
#if !defined (NDEBUG)
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &m_localState, bclient);
#else
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &m_localState, bclient);
#endif
    ClientInfoPtr clientInfo = shared_ptr<ClientInfo>(new ClientInfo("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", Utf8PrintfString("%i", productId)));
    auto manager = ConnectSignInManager::Create(clientInfo, proxy, &m_localState);
    SamlTokenPtr token = make_shared<SamlToken>(tokenXml);
    auto result = manager->SignInWithToken(token)->GetResult();
    if (!result.IsSuccess())
        return;

    //WSG
    Utf8String serverUrl = UrlProvider::Urls::ConnectWsgGlobal.Get();
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);
    Utf8String repositoryId = "BentleyCONNECT.Global--CONNECT.GLOBAL";
    m_wsRepositoryClientPtr = WSRepositoryClient::Create(serverUrl, repositoryId, clientInfo, nullptr, authHandler);

    //IMS-Search API
    shared_ptr<AuthenticationHandler> samlAuthHandler = std::make_shared<ConnectAuthenticationHandler>(serverUrl, manager->GetTokenProvider(serverUrl), proxy, true);
    Utf8String searchApiUrl = "https://qa-waz-search.bentley.com/token";
    Utf8String collection = "IMS/User";
    m_solrClientPtr = SolrClient::Create(searchApiUrl, collection, clientInfo, samlAuthHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectWebServicesClientC_internal::ConnectWebServicesClientC_internal(Utf8String username, Utf8String password, uint32_t productId)
    {
    DgnClientFxCommon::SetApplicationPathsProvider(&m_pathProv);
    
    BeFileName::CreateNewDirectory(m_pathProv.GetTemporaryDirectory());
    BeSQLite::BeSQLiteLib::Initialize(m_pathProv.GetTemporaryDirectory());
    BeSQLite::EC::ECDb::Initialize(m_pathProv.GetTemporaryDirectory(), &m_pathProv.GetAssetsRootDirectory());
    
    BeFileName dgnClientFxSqlangFile = m_pathProv.GetAssetsRootDirectory();
    dgnClientFxSqlangFile.AppendToPath(L"sqlang");
#if !defined (NDEBUG)
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_pseudo.sqlang.db3");
#else
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_en.sqlang.db3");
#endif

    BeSQLite::L10N::SqlangFiles sqlangFiles(dgnClientFxSqlangFile);
    DgnClientFxL10N::ReInitialize(sqlangFiles, sqlangFiles);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    ConnectAuthenticationPersistence::CustomInitialize(&m_localState);
    auto bclient = make_shared<BuddiClient>(proxy);
#if !defined (NDEBUG)
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &m_localState, bclient);
#else
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &m_localState, bclient);
#endif
    ClientInfoPtr clientInfo = shared_ptr<ClientInfo>(new ClientInfo("Bentley-Test", BeVersion(1, 0), "TestAppGUID", "TestDeviceId", "TestSystem", Utf8PrintfString("%i", productId)));
    auto manager = ConnectSignInManager::Create(clientInfo, proxy, &m_localState);
    Credentials credentials(username, password);
    auto result = manager->SignInWithCredentials(credentials)->GetResult();
    if (!result.IsSuccess())
        return;

    //WSG
    Utf8String serverUrl = UrlProvider::Urls::ConnectWsgGlobal.Get();
    auto authHandler = manager->GetAuthenticationHandler(serverUrl, proxy);
    Utf8String repositoryId = "BentleyCONNECT.Global--CONNECT.GLOBAL";
    m_wsRepositoryClientPtr = WSRepositoryClient::Create(serverUrl, repositoryId, clientInfo, nullptr, authHandler);

    //IMS-Search API
    shared_ptr<AuthenticationHandler> samlAuthHandler = std::make_shared<ConnectAuthenticationHandler>(serverUrl, manager->GetTokenProvider(serverUrl), proxy, true);
    Utf8String searchApiUrl = "https://qa-waz-search.bentley.com/token";
    Utf8String collection = "IMS/User";
    m_solrClientPtr = SolrClient::Create(searchApiUrl, collection, clientInfo, samlAuthHandler);
    }

void ConnectWebServicesClientC_internal::SetStatusMessage(Utf8String message)
    {
    m_lastStatusMessage = message;
    }

void ConnectWebServicesClientC_internal::SetStatusDescription(Utf8String desc)
    {
    m_lastStatusDescription = desc;
    }

Utf8StringCR ConnectWebServicesClientC_internal::GetLastStatusMessage()
    {
    return m_lastStatusMessage;
    }

Utf8StringCR ConnectWebServicesClientC_internal::GetLastStatusDescription()
    {
    return m_lastStatusDescription;
    }