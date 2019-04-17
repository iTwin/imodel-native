/*--------------------------------------------------------------------------------------+
|    $RCSfile: ScalableMeshRDSProvider.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Richard.Bois $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ScalableMeshRDSProvider.h"
#ifndef LINUX_SCALABLEMESH_BUILD
#include <CCApi/CCPublic.h>
#endif
#include <ScalableMesh/ScalableMeshAdmin.h>
#include <ScalableMesh/ScalableMeshLib.h>  
#if !defined(DGNDB06_API)
#include <BeHttp/HttpClient.h>
#endif
#include <BeXml/BeXml.h>



BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

#define DEFAULT_BUDDI_RDS_URL "http://buddi.bentley.com/discovery.asmx/GetUrl?urlName=RealityDataServices&region="
#define SMRDSPROVIDER_LOGGER_NAME L"ScalableMesh::RDSProvider"
#define SMRDSPROVIDER_LOG (*NativeLogging::LoggingManager::GetLogger(SMRDSPROVIDER_LOGGER_NAME))

/*----------------------------------------------------------------------------+
| IScalableMeshRDSProvider Method Definition Section - Begin
+----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IScalableMeshRDSProvider::GetAzureURLAddress()
    {
    return _GetAzureURLAddress();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IScalableMeshRDSProvider::GetRDSURLAddress()
    {
    return _GetRDSURLAddress();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IScalableMeshRDSProvider::GetToken()
    {
    return _GetToken();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IScalableMeshRDSProvider::GetRootDocument()
    {
    return _GetRootDocument();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IScalableMeshRDSProvider::GetProjectID()
    {
    return _GetProjectID();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IScalableMeshRDSProviderPtr IScalableMeshRDSProvider::Create(const Utf8String& serverUrl, const Utf8String& projectGuid, const Utf8String& pwcsMeshGuid)
    {
    return new ScalableMeshRDSProvider(serverUrl, projectGuid, pwcsMeshGuid);
    }

/*----------------------------------------------------------------------------+
| IScalableMeshRDSProvider Method Definition Section - End
+----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshRDSProvider::ScalableMeshRDSProvider(const Utf8String& serverUrl, const Utf8String& projectGuid, const Utf8String& pwcsMeshGuid)
    : m_ServerUrl(serverUrl), m_ProjectGuid(projectGuid), m_PWCSMeshGuid(pwcsMeshGuid)
    {
    ScalableMeshRDSProvider::InitializeRealityDataService(this->m_ServerUrl, this->m_ProjectGuid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshRDSProvider::~ScalableMeshRDSProvider()
    {
    if (m_AzureConnection.m_handshake)
        delete m_AzureConnection.m_handshake;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ScalableMeshRDSProvider::_GetAzureURLAddress()
    {
    if (IsTokenExpired()) UpdateToken();

    return m_AzureConnection.m_url + "/" + GetRootDocumentName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ScalableMeshRDSProvider::_GetRDSURLAddress()
    {
    if(!RealityDataService::AreParametersSet())
        ScalableMeshRDSProvider::InitializeRealityDataService(this->m_ServerUrl, this->m_ProjectGuid);

    bool serverStartsWithHTTPS = RealityDataService::GetServerName().StartsWith("https://");

    Utf8String rdsUrl = (serverStartsWithHTTPS ? "" : "https://");
    rdsUrl += RealityDataService::GetServerName();
    rdsUrl += "v" + RealityDataService::GetWSGProtocol();
    rdsUrl += "/Repositories/" + RealityDataService::GetRepoName();
    rdsUrl += "/" + RealityDataService::GetSchemaName();
    rdsUrl += "/RealityData/" + m_PWCSMeshGuid;
    rdsUrl += "/" + GetRootDocumentName();

    return rdsUrl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ScalableMeshRDSProvider::_GetToken()
    {
    if (IsTokenExpired()) UpdateToken();
    return m_AzureConnection.m_token;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ScalableMeshRDSProvider::_GetRootDocument()
    {
    return GetRootDocumentName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ScalableMeshRDSProvider::_GetProjectID()
    {
    return m_ProjectGuid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshRDSProvider::InitializeRealityDataService(const Utf8String& serverUrl, const Utf8String& projectID)
    {
    auto buddiUrl = GetBuddiUrl();
    if(!serverUrl.empty() && buddiUrl != serverUrl)
        buddiUrl = serverUrl;

    RealityDataService::SetServerComponents(buddiUrl, RealityDataService::GetWSGProtocol(), RealityDataService::GetRepoName(), RealityDataService::GetSchemaName());
    RealityDataService::SetProjectId(projectID);

    ScalableMeshAdmin::ProxyInfo proxyInfo(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetProxyInfo());

    if (!proxyInfo.m_serverUrl.empty())
        {       
#ifndef VANCOUVER_API
        Utf8String proxyCreds = proxyInfo.m_user;
        proxyCreds.append(":");
        proxyCreds.append(proxyInfo.m_password);
		
        SMRDSPROVIDER_LOG.error("RealityDataService::SetProxyInfo not supported on BIM02");
        BeAssert(!"RealityDataService::SetProxyInfo not supported on BIM02");
#if 0 
        RealityDataService::SetProxyInfo(proxyInfo.m_serverUrl, proxyCreds);
#endif		
		
#else 
        SMRDSPROVIDER_LOG.error("RealityDataService::SetProxyInfo not yet implemented");
        BeAssert(!"RealityDataService::SetProxyInfo not yet implemented");
#endif
        }


#ifndef DGNDB06_API
    // Check if the connect token callback is supplied
    Utf8String token;
    if(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._SupplyAuthHeaderValue(token, Utf8String("")))
        {
        if(!token.empty())
            {
            SMRDSPROVIDER_LOG.error("Requesting token callback returns non empty token value or passing non empty token value.");
            BeAssert(false); //Previous call should not have fetched the token yet
            }

        Utf8String authTypes[3] = { Utf8String("Authorization: Token "), Utf8String("Authorization: Bearer "), Utf8String("") };

        auto authorization = authTypes[ScalableMeshLib::GetHost().GetScalableMeshAdmin()._SupplyAuthTokenType()];

        if(authorization == authTypes[1])
            {
            SMRDSPROVIDER_LOG.error("OIDC token not yet support by RealityDataService");
            BeAssert(false);
            return;
            }

        // Set the token callback in RDS (will be called before attempting to query the server)
        ConnectTokenManager::GetInstance().SetTokenCallback([authorization] (Utf8StringR token, time_t& timer)
            {
            ScalableMeshLib::GetHost().GetScalableMeshAdmin()._SupplyAuthHeaderValue(token, RealityDataService::GetServerName());
            token = authorization + token;
            timer = std::time(nullptr);
            });
        }
#endif

    // Set the error callback in RDS (will be called when errors occur)
    RealityDataService::SetErrorCallback([](Utf8String basicMessage, const RawServerResponse& rawResponse)
        {
        SMRDSPROVIDER_LOG.errorv("%s", basicMessage.c_str());
        SMRDSPROVIDER_LOG.errorv("RealityDataService request body: %s", rawResponse.body.c_str());
        });

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshRDSProvider::IsTokenExpired()
    {
    if (nullptr == m_AzureConnection.m_handshake) return true;
    int64_t currentTime;
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
    return m_AzureConnection.m_tokenTimer < currentTime;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshRDSProvider::UpdateToken()
    {
    SMRDSPROVIDER_LOG.info("Updating RealityDataService token");

    // Make sure the server names matches
    if (nullptr != m_AzureConnection.m_handshake && !m_AzureConnection.m_handshake->GetServerName().Equals(RealityDataService::GetServerName()))
        {
        // Unfortunately, AzureHandshake::SetServerName() is a protected function...
        //m_AzureConnection.m_handshake->SetServerName(RealityDataService::GetServerName());
        delete m_AzureConnection.m_handshake;
        m_AzureConnection.m_handshake = nullptr;
        }

    if (nullptr == m_AzureConnection.m_handshake)
        m_AzureConnection.m_handshake = new AzureHandshake(m_PWCSMeshGuid, false /*writeable*/);

    // Request Azure URL of the reality data
    RawServerResponse rawResponse = RealityDataService::BasicRequest((RealityDataUrl*)m_AzureConnection.m_handshake);
    if (rawResponse.status != RequestStatus::BADREQ)
        {
        // The handshake status with Azure need not be checked, if the request fails the current token will be used until it expires (or return an empty token)
        /*BentleyStatus handshakeStatus = */
        m_AzureConnection.m_handshake->ParseResponse(rawResponse.body, m_AzureConnection.m_url, m_AzureConnection.m_token, m_AzureConnection.m_tokenTimer);
        }
    else
        {
        // Try again after 50 minutes...
        DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(m_AzureConnection.m_tokenTimer);
        m_AzureConnection.m_tokenTimer += 1000 * 60 * 50;
        SMRDSPROVIDER_LOG.errorv("UpdateToken() : Request URL: %s", m_AzureConnection.m_handshake->GetHttpRequestString().c_str());
        SMRDSPROVIDER_LOG.errorv("UpdateToken() : RealityDataService failed with response code [%d]", rawResponse.responseCode);
        SMRDSPROVIDER_LOG.errorv("UpdateToken() : RealityDataService body: %s", rawResponse.body.c_str());
        BeAssert(!"ScalableMeshRDSProvider failed to update token");
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ScalableMeshRDSProvider::GetBuddiUrl()
    {
    Utf8String serverUrl;

#ifdef LINUX_SCALABLEMESH_BUILD
    SMRDSPROVIDER_LOG.error("ScalableMeshRDSProvider::GetBuddiUrl() not implemented under linux yet");
    BeAssert(!"ScalableMeshRDSProvider::GetBuddiUrl() not implemented under linux yet");
    return serverUrl;
#endif

#ifndef DGNDB06_API
    uint32_t connectRegion = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._SupplyRegionID();
    if(connectRegion != uint32_t(-1))
        {
        Utf8String buddiUrl((DEFAULT_BUDDI_RDS_URL + std::to_string(connectRegion)).c_str());
        BENTLEY_HTTP_NAMESPACE_NAME::HttpClient client;
        auto request = client.CreateRequest(buddiUrl.c_str(), "GET");
        auto response = request.PerformAsync()->GetResult();
        auto body = response.GetContent()->GetBody()->AsString();
        BeXmlStatus xmlStatus = BEXML_Success;
        BeXmlReaderPtr reader = BeXmlReader::CreateAndReadFromString(xmlStatus, body.c_str(), body.size());
        BeAssert(reader.IsValid());
        WString value, fileName;
#ifdef VANCOUVER_API
#define XML_READTO(reader, type, name, stayInCurrentElement, value) reader->ReadTo(type, name, value);
#else
#define XML_READTO(reader, type, name, stayInCurrentElement, value) reader->ReadTo(type, name, stayInCurrentElement, value);
#endif
        auto xmlResult = XML_READTO(reader, IBeXmlReader::NodeType::NODE_TYPE_Element, "string", false, &value);
        BeAssert(xmlResult == IBeXmlReader::ReadResult::READ_RESULT_Success);
        xmlResult = XML_READTO(reader, IBeXmlReader::NodeType::NODE_TYPE_Text, nullptr, false, &value);
        BeAssert(xmlResult == IBeXmlReader::ReadResult::READ_RESULT_Success);
        serverUrl = Utf8String(value.c_str());
    }
#endif

#ifndef LINUX_SCALABLEMESH_BUILD
    if(serverUrl.empty())
        {
        // Unable to retrieve valid RDS url... fall back on CCApi (Windows only)
        CallStatus status = APIERR_SUCCESS;
        try
            {
            CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
            if(!api)
                {
                SMRDSPROVIDER_LOG.error("GetBuddiUrl() : Failed to initialize CCApi");
                BeAssert(!"Failed to initialize CCApi");
                return "";
                }
            bool installed;
            status = CCApi_IsInstalled(api, &installed);
            if(!installed)
                {
                SMRDSPROVIDER_LOG.error("GetBuddiUrl() : Connection client does not seem to be installed");
                BeAssert(!"Connection client does not seem to be installed\n");
                CCApi_FreeApi(api);
                return "";
                }
            bool running = false;
            status = CCApi_IsRunning(api, &running);
            if(status != APIERR_SUCCESS || !running)
                {
                SMRDSPROVIDER_LOG.error("GetBuddiUrl() : Connection client does not seem to be running");
                BeAssert(!"Connection client does not seem to be running\n");
                CCApi_FreeApi(api);
                return "";
                }
            bool loggedIn = false;
            status = CCApi_IsLoggedIn(api, &loggedIn);
            if(status != APIERR_SUCCESS || !loggedIn)
                {
                SMRDSPROVIDER_LOG.error("GetBuddiUrl() : Connection client does not seem to be logged in");
                BeAssert(!"Connection client does not seem to be logged in\n");
                CCApi_FreeApi(api);
                return "";
                }
            bool acceptedEula = false;
            status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
            if(status != APIERR_SUCCESS || !acceptedEula)
                {
                SMRDSPROVIDER_LOG.error("GetBuddiUrl() : Connection client user does not seem to have accepted EULA");
                BeAssert(!"Connection client user does not seem to have accepted EULA\n");
                CCApi_FreeApi(api);
                return "";
                }
            bool sessionActive = false;
            status = CCApi_IsUserSessionActive(api, &sessionActive);
            if(status != APIERR_SUCCESS || !sessionActive)
                {
                SMRDSPROVIDER_LOG.error("GetBuddiUrl() : Connection client does not seem to have an active session");
                BeAssert(!"Connection client does not seem to have an active session\n");
                CCApi_FreeApi(api);
                return "";
                }

            wchar_t* buddiUrl;
            UINT32 strlen = 0;

            CCApi_GetBuddiUrl(api, L"RealityDataServices", NULL, &strlen);
            strlen += 1;
            buddiUrl = (wchar_t*)malloc((strlen) * sizeof(wchar_t));
            CCApi_GetBuddiUrl(api, L"RealityDataServices", buddiUrl, &strlen);

            serverUrl.Assign(buddiUrl);
            free(buddiUrl);
            CCApi_FreeApi(api);
            }
        catch(...)
            {
            SMRDSPROVIDER_LOG.error("GetBuddiUrl() : Unknown error while fetching RDS server url");
            BeAssert(!"Unknown error while fetching RDS server url");
            }
        }
#endif
    if(serverUrl.empty())
        {
        SMRDSPROVIDER_LOG.error("GetBuddiUrl() : RDS server URL couldn't be found");
        BeAssert(!"RDS server URL couldn't be found");
        }
    return serverUrl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshRDSProvider::IsHostedByRDS(const Utf8String& serverUrl, const Utf8String& projectGuid, const Utf8String& meshGuid)
    {
    if(!RealityDataService::AreParametersSet())
        ScalableMeshRDSProvider::InitializeRealityDataService(serverUrl, projectGuid);

    AzureHandshake handshake(meshGuid, false /*writeable*/);

    return (RequestStatus::BADREQ != RealityDataService::BasicRequest((RealityDataUrl*)(&handshake)).status);
    }

Utf8String ScalableMeshRDSProvider::GetRootDocumentName()
    {
    RawServerResponse rawResponse;
    RealityDataByIdRequest idReq = RealityDataByIdRequest(m_PWCSMeshGuid);
    RealityDataPtr entity = RealityDataService::Request(idReq, rawResponse);

    if(entity == nullptr || rawResponse.status == RequestStatus::BADREQ)
        {
        SMRDSPROVIDER_LOG.errorv("GetRootDocumentName() : RealityDataService failed with response code [%d]", rawResponse.responseCode);
        SMRDSPROVIDER_LOG.errorv("GetRootDocumentName() : RealityDataService body: %s", rawResponse.body.c_str());
        return Utf8String();
        }

    return entity->GetRootDocument();
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE

