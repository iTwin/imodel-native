/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshRDSProvider.cpp $
|    $RCSfile: ScalableMeshRDSProvider.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Richard.Bois $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ScalableMeshRDSProvider.h"
#ifndef LINUX_SCALABLEMESH_BUILD
#include <CCApi/CCPublic.h>
#endif
#include <ScalableMesh/ScalableMeshAdmin.h>
#include <ScalableMesh/ScalableMeshLib.h>
#include <BeHttp/HttpClient.h>
#include <BeXml/BeXml.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

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
    {}

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
    if (RealityDataService::AreParametersSet()) return;
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
		
        assert(!"RealityDataService::SetProxyInfo not supported on BIM02");
#if 0 
        RealityDataService::SetProxyInfo(proxyInfo.m_serverUrl, proxyCreds);
#endif		
		
#else 
        assert(!"RealityDataService::SetProxyInfo not yet available");
#endif
        }

    // Check if the connect token callback is supplied
    Utf8String token;
    if(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._SupplyAuthHeaderValue(token, Utf8String("")))
        {
        BeAssert(token.empty()); // Previous call should not have fetched the token yet

        Utf8String authTypes[3] = { Utf8String("Authorization: Token "), Utf8String("Authorization: Bearer "), Utf8String("") };

        auto authorization = authTypes[ScalableMeshLib::GetHost().GetScalableMeshAdmin()._SupplyAuthTokenType()];

        if(authorization == authTypes[1])
            {
            BeAssert(false); // OIDC not yet supported
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
    ScalableMeshRDSProvider::InitializeRealityDataService(this->m_ServerUrl, this->m_ProjectGuid);

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
        assert(!"Problem with the handshake");
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ScalableMeshRDSProvider::GetBuddiUrl()
    {
    Utf8String serverUrl;
    uint32_t connectRegion = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._SupplyRegionID();
    if(connectRegion > 0)
        {
#define DEFAULT_BUDDI_RDS_URL "http://buddi.bentley.com/discovery.asmx/GetUrl?urlName=RealityDataServices&region="
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
    if(serverUrl.empty())
        {
#ifndef LINUX_SCALABLEMESH_BUILD
        // Unable to retrieve valid RDS url... fall back on CCApi (Windows only)
        CallStatus status = APIERR_SUCCESS;
        try
            {
            CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
            if(!api)
                {
                BeAssert(!"Couldn't initialize Connection client COM API");
                return "";
                }
            bool installed;
            status = CCApi_IsInstalled(api, &installed);
            if(!installed)
                {
                BeAssert(!"Connection client does not seem to be installed\n");
                CCApi_FreeApi(api);
                return "";
                }
            bool running = false;
            status = CCApi_IsRunning(api, &running);
            if(status != APIERR_SUCCESS || !running)
                {
                BeAssert(!"Connection client does not seem to be running\n");
                CCApi_FreeApi(api);
                return "";
                }
            bool loggedIn = false;
            status = CCApi_IsLoggedIn(api, &loggedIn);
            if(status != APIERR_SUCCESS || !loggedIn)
                {
                BeAssert(!"Connection client does not seem to be logged in\n");
                CCApi_FreeApi(api);
                return "";
                }
            bool acceptedEula = false;
            status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
            if(status != APIERR_SUCCESS || !acceptedEula)
                {
                BeAssert(!"Connection client user does not seem to have accepted EULA\n");
                CCApi_FreeApi(api);
                return "";
                }
            bool sessionActive = false;
            status = CCApi_IsUserSessionActive(api, &sessionActive);
            if(status != APIERR_SUCCESS || !sessionActive)
                {
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
            BeAssert(!"Error thrown while fetching RDS server url");
            }
#endif
        }
    BeAssert(!serverUrl.empty()); // RDS server URL couldn't be found
    return serverUrl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshRDSProvider::IsHostedByRDS(const Utf8String& serverUrl, const Utf8String& projectGuid, const Utf8String& meshGuid)
    {
    ScalableMeshRDSProvider::InitializeRealityDataService(serverUrl, projectGuid);

    AzureHandshake handshake(meshGuid, false /*writeable*/);

    return (RequestStatus::BADREQ != RealityDataService::BasicRequest((RealityDataUrl*)(&handshake)).status);
    }

Utf8String ScalableMeshRDSProvider::GetRootDocumentName()
    {
    RawServerResponse rawResponse;
    RealityDataByIdRequest idReq = RealityDataByIdRequest(m_PWCSMeshGuid);
    RealityDataPtr entity = RealityDataService::Request(idReq, rawResponse);

    if (entity == nullptr || rawResponse.status == RequestStatus::BADREQ)
        return Utf8String();

    return entity->GetRootDocument();
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE

