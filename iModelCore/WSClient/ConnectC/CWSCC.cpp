/*--------------------------------------------------------------------------------------+
|
|     $Source: ConnectC/CWSCC.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CWSCCInternal.h"


/*---------------------------------------------------------------------------------**//**
* Initializer.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LPCWSCC Init
(
WCharCP wTemporaryDirectory,
WCharCP wAssetsRootDirectory,
WCharCP wApplicationName,
WCharCP wApplicationVersion,
WCharCP wApplicationGUID,
WCharCP wApplicationProductId,
WCharCP wproxyUrl,
WCharCP wproxyUsername,
WCharCP wproxyPassword,
IHTTPHANDLERPTR customHandler
)
    {
    BeFileName temporaryDirectory(wTemporaryDirectory);
    BeFileName assetsRootDirectory(wAssetsRootDirectory);
    Utf8String applicationName;
    Utf8String utf8ApplicationVersion;
    Utf8String applicationGUID;
    Utf8String applicationProductId;
    Utf8String proxyUrl;
    Utf8String proxyUsername;
    Utf8String proxyPassword;

    BeStringUtilities::WCharToUtf8(applicationName, wApplicationName);
    BeStringUtilities::WCharToUtf8(utf8ApplicationVersion, wApplicationVersion);
    BeStringUtilities::WCharToUtf8(applicationGUID, wApplicationGUID);
    BeStringUtilities::WCharToUtf8(applicationProductId, wApplicationProductId);
    BeStringUtilities::WCharToUtf8(proxyUrl, wproxyUrl);
    BeStringUtilities::WCharToUtf8(proxyUsername, wproxyUsername);
    BeStringUtilities::WCharToUtf8(proxyPassword, wproxyPassword);

    BeVersion applicationVersion(utf8ApplicationVersion.c_str());

    IHttpHandlerPtr customHandlerPtr = nullptr;
#if !defined (NDEBUG)
    if (customHandler != nullptr)
        customHandlerPtr = *reinterpret_cast<shared_ptr<IHttpHandler> *>(customHandler);
#endif

    return new ConnectWebServicesClientC_internal
        (
        temporaryDirectory,
        assetsRootDirectory,
        applicationName,
        applicationVersion,
        applicationGUID,
        applicationProductId,
        &proxyUrl,
        &proxyUsername,
        &proxyPassword,
        customHandlerPtr
        );
    }

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
WCharCP wApplicationProductId,
WCharCP wproxyUrl,
WCharCP wproxyUsername,
WCharCP wproxyPassword,
IHTTPHANDLERPTR customHandler
)
    {
    Utf8String authenticatedToken;
    BeStringUtilities::WCharToUtf8(authenticatedToken, wAuthenticatedToken);

    LPCWSCC api = Init
        (
        wTemporaryDirectory,
        wAssetsRootDirectory,
        wApplicationName,
        wApplicationVersion,
        wApplicationGUID,
        wApplicationProductId,
        wproxyUrl,
        wproxyUsername,
        wproxyPassword,
        customHandler
        );

    if (api->AttemptLoginUsingToken(make_shared<SamlToken>(authenticatedToken)))
        {
        return (CWSCCHANDLE) api;
        }
    else
        {
        ConnectWebServicesClientC_FreeApi((CWSCCHANDLE) api);
        return nullptr;
        }
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
WCharCP wApplicationProductId,
WCharCP wproxyUrl,
WCharCP wproxyUsername,
WCharCP wproxyPassword,
IHTTPHANDLERPTR customHandler
)
    {
    Utf8String username;
    BeStringUtilities::WCharToUtf8(username, wUsername);

    Utf8String password;
    BeStringUtilities::WCharToUtf8(password, wPassword);

    LPCWSCC api = Init
        (
        wTemporaryDirectory,
        wAssetsRootDirectory,
        wApplicationName,
        wApplicationVersion,
        wApplicationGUID,
        wApplicationProductId,
        wproxyUrl,
        wproxyUsername,
        wproxyPassword,
        customHandler
        );

    if (api->AttemptLoginUsingCredentials(Credentials(username, password)))
        {
        return (CWSCCHANDLE) api;
        }
    else
        {
        ConnectWebServicesClientC_FreeApi((CWSCCHANDLE) api);
        return nullptr;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_FreeApi(CWSCCHANDLE apiHandle)
    {
    if (nullptr == apiHandle)
        return INVALID_PARAMETER;

    LPCWSCC api = (LPCWSCC) apiHandle;
    delete api;
    return SUCCESS;
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
* @bsimethod                                                                    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_CreateProjectFavorite
(
CWSCCHANDLE apiHandle,
WCharCP ProjectGuid
)
    {
    VERIFY_API

    Json::Value instance;

    if (ProjectGuid == nullptr)
        {
        api->SetStatusMessage ("ProjectGuid is invalid in ConnectWebServicesClientC_CreateProjectFavorite.");
        api->SetStatusDescription ("You must specify a ProjectGuid to create a ProjectFavorite instance.");
        return INVALID_PARAMETER;
        }
    instance["instanceId"] = Utf8String(ProjectGuid);
    instance["schemaName"] = "GlobalSchema";
    instance["className"] = "ProjectFavorite";

    Json::Value objectCreationJson;
    objectCreationJson["instance"] = instance;
    ObjectId objectId("GlobalSchema", "ProjectFavorite", "");


    Utf8String connectwsgglobalUrl = UrlProvider::Urls::ConnectWsgGlobal.Get();
    if (api->m_repositoryClients.find(connectwsgglobalUrl + "BentleyCONNECT.Global--CONNECT.GLOBAL") == api->m_repositoryClients.end())
        {
        api->CreateWSRepositoryClient
            (
            connectwsgglobalUrl,
            "BentleyCONNECT.Global--CONNECT.GLOBAL"
            );
        }

    auto client = api->m_repositoryClients.find(connectwsgglobalUrl + "BentleyCONNECT.Global--CONNECT.GLOBAL")->second;
    auto result = client->SendCreateObjectRequest(objectId, objectCreationJson)->GetResult();
    if (!result.IsSuccess())
        return wsresultToConnectWebServicesClientCStatus(api, result.GetError().GetId(), result.GetError().GetDisplayMessage(), result.GetError().GetDisplayDescription());

    api->SetCreatedObjectResponse(result.GetValue());
    api->SetStatusMessage("Successful operation");
    api->SetStatusDescription("ConnectWebServicesClientC_CreateProjectFavorite completed successfully.");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_CreateProjectFavorite_V2
(
CWSCCHANDLE apiHandle,
WCharCP ProjectGuid
)
    {
    VERIFY_API

    Json::Value instance;

    if (ProjectGuid == nullptr)
        {
        api->SetStatusMessage ("ProjectGuid is invalid in ConnectWebServicesClientC_CreateProjectFavorite_V2.");
        api->SetStatusDescription ("You must specify a ProjectGuid to create a ProjectFavorite_V2 instance.");
        return INVALID_PARAMETER;
        }
    instance["instanceId"] = Utf8String(ProjectGuid);
    instance["schemaName"] = "GlobalSchema";
    instance["className"] = "ProjectFavorite_V2";

    Json::Value objectCreationJson;
    objectCreationJson["instance"] = instance;
    ObjectId objectId("GlobalSchema", "ProjectFavorite_V2", "");

    Utf8String connectwsgglobalUrl = UrlProvider::Urls::ConnectWsgGlobal.Get();
    if (api->m_repositoryClients.find(connectwsgglobalUrl + "BentleyCONNECT.Global--CONNECT.GLOBAL") == api->m_repositoryClients.end())
        {
        api->CreateWSRepositoryClient
            (
            connectwsgglobalUrl,
            "BentleyCONNECT.Global--CONNECT.GLOBAL"
            );
        }

    auto client = api->m_repositoryClients.find(connectwsgglobalUrl + "BentleyCONNECT.Global--CONNECT.GLOBAL")->second;
    auto result = client->SendCreateObjectRequest(objectId, objectCreationJson)->GetResult();
    if (!result.IsSuccess())
        return wsresultToConnectWebServicesClientCStatus(api, result.GetError().GetId(), result.GetError().GetDisplayMessage(), result.GetError().GetDisplayDescription());

    api->SetCreatedObjectResponse(result.GetValue());
    api->SetStatusMessage("Successful operation");
    api->SetStatusDescription("ConnectWebServicesClientC_CreateProjectFavorite_V2 completed successfully.");
    return SUCCESS;
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

    Utf8String searchApiUrl = "https://qa-waz-search.bentley.com/token"; //UrlProvider::Urls::ImsSearch.Get()
    Utf8String collection = "IMS/User";
    if (api->m_solrClients.find(searchApiUrl + collection) == api->m_solrClients.end())
        {
        api->CreateSolrClient
            (
            searchApiUrl,
            collection
            );
        }

    auto client = api->m_solrClients.find(searchApiUrl + collection)->second;
    auto result = client->SendGetRequest()->GetResult();
    if (!result.IsSuccess())
        return httperrorToConnectWebServicesClientStatus(api, result.GetError().GetHttpStatus(), result.GetError().GetDisplayMessage(), result.GetError().GetDisplayDescription());
    
    /* TODO: How can all of this be done, yet be included in the auto gen stuff? Maybe have a manually created Free, GetBufferData...etc which either calls manually created functions, or the autoGenOne (which will proppagate accordingly)
             Can have a globally unique schema for all manually created ones, which pipes to functions which free, get...etc manually created stuff, and the other would pipe to autoGen
             Yup, all buffer functions are broken for anything buffer related which isn't autoGen. Currently that is only _GetUserInfo, but that could be a lot more, and there should only be one exposed API call for both manual and autoGen
    CWSCCBUFFER* buf = (CWSCCBUFFER*) calloc(1, sizeof(CWSCCBUFFER));
    if (buf == nullptr)
        {
        free(buf);
        api->SetStatusMessage("Memory failed to initialize interally.");
        api->SetStatusDescription("Failed to calloc memory for CWSCCBUFFER.");
        return INTERNAL_MEMORY_ERROR;
        }

    //Note this below is wrong for SolrClient. It is just a stub saying this needs to be done to properly set the out buffer
    for (WSObjectsReader::Instance instance : result.GetValue().GetInstances())
        {
        LPCWSCCORGANIZATIONBUFFER bufToFill = new CWSCCORGANIZATIONBUFFER;
        Organization_BufferStuffer(bufToFill, instance.GetProperties());
        buf->lItems.push_back(bufToFill);
        }

    buf->lCount = buf->lItems.size();
    buf->lClassType = BUFF_TYPE_ORGANIZATION;
    buf->lSchemaType = SCHEMA_TYPE_GLOBALSCHEMA;
    *organizationBuffer = (CWSCCDATABUFHANDLE) buf;
    */

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
CharCP ConnectWebServicesClientC_GetLastCreatedObjectInstanceId(CWSCCHANDLE apiHandle)
    {
    if (NULL == apiHandle)
        return "apiHandle passed into ConnectWebServicesClientC_GetLastCreatedObjectInstanceId is NULL.";
    LPCWSCC api = (LPCWSCC) apiHandle;
    auto str = api->GetLastCreatedObjectInstanceId ();
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus ConnectWebServicesClientC_ConfigureWebProxy
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

    api->CreateProxyHttpClient(proxyUrl, username, password);
    api->SetStatusMessage("Success!");
    api->SetStatusDescription("The WSRepositoryClient was successfully created.");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                    05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CallStatus wsresultToConnectWebServicesClientCStatus(LPCWSCC api, WSError::Id errorId, Utf8StringCR errorMessage, Utf8StringCR errorDescription)
    {
    api->SetStatusMessage(errorMessage.c_str());
    api->SetStatusDescription(errorDescription.c_str());
    switch(errorId)
        {
        case WSError::Id::Unknown:
            return ERROR500;
        case WSError::Id::LoginFailed:
            return LOGIN_FAILED;
        case WSError::Id::SslRequired:
            return SSL_REQUIRED;
        case WSError::Id::NotEnoughRights:
            return NOT_ENOUGH_RIGHTS;
        case WSError::Id::RepositoryNotFound:
            return REPOSITORY_NOT_FOUND;
        case WSError::Id::SchemaNotFound:
            return SCHEMA_NOT_FOUND;
        case WSError::Id::ClassNotFound:
            return CLASS_NOT_FOUND;
        case WSError::Id::PropertyNotFound:
            return PROPERTY_NOT_FOUND;
        case WSError::Id::InstanceNotFound:
            return INSTANCE_NOT_FOUND;
        case WSError::Id::FileNotFound:
            return FILE_NOT_FOUND;
        case WSError::Id::NotSupported:
            return NOT_SUPPORTED;
        case WSError::Id::NoServerLicense:
            return NO_SERVER_LICENSE;
        case WSError::Id::NoClientLicense:
            return NO_CLIENT_LICENSE;
        case WSError::Id::TooManyBadLoginAttempts:
            return TO_MANY_BAD_LOGIN_ATTEMPTS;
        case WSError::Id::ServerError:
            return ERROR500;
        case WSError::Id::BadRequest:
            return ERROR400;
        case WSError::Id::Conflict:
            return ERROR409;
        default:
            return ERROR500;
        }
    }

WSLocalState ConnectWebServicesClientC_internal::s_localState = WSLocalState();

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectWebServicesClientC_internal::ConnectWebServicesClientC_internal
(
BeFileName temporaryDirectory,
BeFileName assetsRootDirectory,
Utf8String applicationName,
BeVersion applicationVersion,
Utf8String applicationGUID,
Utf8String applicationProductId,
Utf8StringP proxyUrl,
Utf8StringP proxyUsername,
Utf8StringP proxyPassword,
IHttpHandlerPtr customHandler
)
: m_pathProvider(temporaryDirectory, assetsRootDirectory)
    {
    if (proxyUrl != nullptr)
        CreateProxyHttpClient(*proxyUrl, *proxyUsername, *proxyPassword);
    else
        m_customHandler = nullptr;

    if (customHandler != nullptr)
        m_customHandler = customHandler;

    Initialize
        (
        temporaryDirectory,
        assetsRootDirectory,
        applicationName,
        applicationVersion,
        applicationGUID,
        applicationProductId
        );
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
    DgnClientFxCommon::SetApplicationPathsProvider(&m_pathProvider);

    BeFileName::CreateNewDirectory(m_pathProvider.GetTemporaryDirectory());
    BeSQLite::BeSQLiteLib::Initialize(m_pathProvider.GetTemporaryDirectory());
    
    BeFileName dgnClientFxSqlangFile = m_pathProvider.GetAssetsRootDirectory();
    dgnClientFxSqlangFile.AppendToPath(L"sqlang");
    dgnClientFxSqlangFile.AppendToPath(L"DgnClientFx_en.sqlang.db3");
    BeSQLite::L10N::SqlangFiles sqlangFiles(dgnClientFxSqlangFile);

    DgnClientFxL10N::ReInitialize(sqlangFiles, sqlangFiles);
    auto bclient = make_shared<BuddiClient>(m_customHandler);
#if !defined (NDEBUG)
    UrlProvider::Initialize(UrlProvider::Qa, UrlProvider::DefaultTimeout, &s_localState, bclient);
#else
    UrlProvider::Initialize(UrlProvider::Release, UrlProvider::DefaultTimeout, &s_localState, bclient);
#endif
    m_clientInfo = ClientInfo::Create(applicationName, applicationVersion, applicationGUID, applicationProductId);
    m_connectSignInManager = ConnectSignInManager::Create(m_clientInfo, m_customHandler, &s_localState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectWebServicesClientC_internal::~ConnectWebServicesClientC_internal()
    {
    if (m_connectSignInManager->IsSignedIn())
        m_connectSignInManager->SignOut();
    UrlProvider::CleanUpUrlCache();
    UrlProvider::Uninitialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectWebServicesClientC_internal::AttemptLoginUsingCredentials(Credentials credentials)
    {
    auto result = m_connectSignInManager->SignInWithCredentials(credentials)->GetResult();
    bool isSuccess = result.IsSuccess();
    if (isSuccess)
        {
        SetStatusMessage("Login successful.");
        SetStatusDescription("Login using credentials performed successfully.");
        }
    return isSuccess;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConnectWebServicesClientC_internal::AttemptLoginUsingToken(SamlTokenPtr token)
    {
    auto result = m_connectSignInManager->SignInWithToken(token)->GetResult();
    bool isSuccess = result.IsSuccess();
    if (isSuccess)
        {
        SetStatusMessage("Login successful.");
        SetStatusDescription("Login using credentials performed successfully.");
        }
    return isSuccess;
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
    shared_ptr<ProxyHttpHandler> proxy = std::make_shared<ProxyHttpHandler>(proxyUrl, nullptr);
    proxy->SetProxyCredentials(Credentials(username, password));
    m_customHandler = proxy;
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
    auto authHandler = m_connectSignInManager->GetAuthenticationHandler(serverUrl, m_customHandler);
    auto clientPtr = WSRepositoryClient::Create(serverUrl, repositoryId, m_clientInfo, nullptr, authHandler);
    m_repositoryClients.insert(make_bpair(serverUrl + repositoryId, clientPtr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectWebServicesClientC_internal::CreateSolrClient
(
Utf8String serverUrl,
Utf8String collection
)
    {
    Utf8String wsgGlobalUrl = UrlProvider::Urls::ConnectWsgGlobal.Get();
    shared_ptr<AuthenticationHandler> samlAuthHandler = std::make_shared<ConnectAuthenticationHandler>
        (
        wsgGlobalUrl,
        m_connectSignInManager->GetTokenProvider(wsgGlobalUrl),
        m_customHandler,
        true
        );

    auto clientPtr = SolrClient::Create(serverUrl, collection, m_clientInfo, samlAuthHandler);
    m_solrClients.insert(make_bpair(serverUrl + collection, clientPtr));
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
void ConnectWebServicesClientC_internal::SetCreatedObjectResponse (WSCreateObjectResponse response)
    {
    m_lastCreatedObjectResponse = response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectWebServicesClientC_internal::SetObjectsResponse (WSObjectsResponse response)
    {
    m_lastObjectsResponse = response;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP ConnectWebServicesClientC_internal::GetLastCreatedObjectInstanceId ()
    {
    if (m_lastCreatedObjectResponse.GetObject ()["changedInstance"]["instanceAfterChange"]["instanceId"].isString())
        {
        return m_lastCreatedObjectResponse.GetObject ()["changedInstance"]["instanceAfterChange"]["instanceId"].asCString ();
        }
    else
        return "";
    }