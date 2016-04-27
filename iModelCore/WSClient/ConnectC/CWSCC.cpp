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
CWSCC_EXPORT CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithToken(WCharCP wAuthenticatedToken, uint32_t productId)
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
CWSCC_EXPORT CWSCCHANDLE ConnectWebServicesClientC_InitializeApiWithCredentials(WCharCP wUsername, WCharCP wPassword, uint32_t productId)
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
CALLSTATUS httperrorToConnectWebServicesClientStatus(HttpStatus status, Utf8StringCR message, Utf8StringCR description)
    {
    switch (status)
        {
            case HttpStatus::InternalServerError:
                return CALLSTATUS{ ERROR500, message.c_str(), description.c_str() };
            case HttpStatus::BadRequest:
                return CALLSTATUS{ ERROR400, message.c_str(), description.c_str() };
            case HttpStatus::NotFound:
                return CALLSTATUS{ ERROR404, message.c_str(), description.c_str() };
            default:
                return CALLSTATUS{ ERROR500, message.c_str(), description.c_str() };
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CWSCC_EXPORT CALLSTATUS ConnectWebServicesClientC_GetIMSUserInfo(CWSCCHANDLE apiHandle, CWSCCDATABUFHANDLE* userBuffer)
    {
    VERIFY_API

    if (userBuffer == NULL)
        return CALLSTATUS{ INVALID_PARAMETER, "The userBuffer parameter is NULL." };

    auto result = api->m_solrClientPtr->SendGetRequest()->GetResult();
    if (!result.IsSuccess())
        return httperrorToConnectWebServicesClientStatus( result.GetError().GetHttpStatus(), result.GetError().GetDisplayMessage(), result.GetError().GetDisplayDescription());
    

    return CALLSTATUS{ SUCCESS, "Success!" };
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
#if defined (DEBUG)
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_pseudo.sqlang.db3");
#else
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_en.sqlang.db3");
#endif

    BeSQLite::L10N::SqlangFiles sqlangFiles(dgnClientFxSqlangFile); //Prob needed
    DgnClientFxL10N::ReInitialize(sqlangFiles, sqlangFiles); //needed

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    ConnectAuthenticationPersistence::CustomInitialize(&m_localState);
    auto bclient = make_shared<BuddiClient>(proxy);
#if defined (DEBUG)
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
#if defined (DEBUG)
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_pseudo.sqlang.db3");
#else
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_en.sqlang.db3");
#endif

    BeSQLite::L10N::SqlangFiles sqlangFiles(dgnClientFxSqlangFile);
    DgnClientFxL10N::ReInitialize(sqlangFiles, sqlangFiles);

    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    ConnectAuthenticationPersistence::CustomInitialize(&m_localState);
    auto bclient = make_shared<BuddiClient>(proxy);
#if defined (DEBUG)
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