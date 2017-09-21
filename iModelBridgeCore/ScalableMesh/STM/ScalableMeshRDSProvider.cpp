/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshRDSProvider.cpp $
|    $RCSfile: ScalableMeshRDSProvider.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Richard.Bois $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ScalableMeshRDSProvider.h"
#include <ConnectClientWrapperNative\ConnectClientWrapper.h>

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
IScalableMeshRDSProviderPtr IScalableMeshRDSProvider::Create(const Utf8String& projectGuid, const Utf8String& pwcsMeshGuid)
    {
    return new ScalableMeshRDSProvider(projectGuid, pwcsMeshGuid);
    }

/*----------------------------------------------------------------------------+
| IScalableMeshRDSProvider Method Definition Section - End
+----------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshRDSProvider::ScalableMeshRDSProvider(const Utf8String& projectGuid, const Utf8String& pwcsMeshGuid)
    : m_ProjectGuid(projectGuid), m_PWCSMeshGuid(pwcsMeshGuid)
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

    // Get root document name
    RawServerResponse rawResponse;
    RealityDataByIdRequest idReq = RealityDataByIdRequest(m_PWCSMeshGuid); //at this point I request the RealityData's information to know the root document's path
    RealityDataPtr entity = RealityDataService::Request(idReq, rawResponse);

    if (entity == nullptr || rawResponse.status == RequestStatus::BADREQ)
        return Utf8String();

    Utf8String rootDocument = entity->GetRootDocument();
    return m_AzureConnection.m_url + "/" + rootDocument;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Richard.Bois                     09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ScalableMeshRDSProvider::_GetRDSURLAddress()
    {
    InitializeRealityDataService();

    bool serverStartsWithHTTPS = RealityDataService::GetServerName().StartsWith("https://");

    Utf8String rdsUrl = (serverStartsWithHTTPS ? "" : "https://");
    rdsUrl += RealityDataService::GetServerName();
    rdsUrl += "v" + RealityDataService::GetWSGProtocol();
    rdsUrl += "/Repositories/" + RealityDataService::GetRepoName();
    rdsUrl += "/" + RealityDataService::GetSchemaName();
    rdsUrl += "/RealityData/" + m_PWCSMeshGuid;

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
void ScalableMeshRDSProvider::InitializeRealityDataService()
    {
    //if (RealityDataService::AreParametersSet()) return SUCCESS;

    RealityDataService::SetServerComponents(GetBuddiUrl(), RealityDataService::GetWSGProtocol(), RealityDataService::GetRepoName(), RealityDataService::GetSchemaName());
    RealityDataService::SetProjectId(m_ProjectGuid);
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
    InitializeRealityDataService();

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
    WString serverUrl;

    try {
        Bentley::Connect::Wrapper::Native::ConnectClientWrapper connectClient;
        std::wstring buddiUrl;
        connectClient.GetBuddiUrl(L"RealityDataServices", buddiUrl);
        serverUrl.assign(buddiUrl.c_str());
        }
    catch (...)
        {
        BeAssert(!"Could not retrieve buddi url from Connect Client");
        }

    return Utf8String(serverUrl.c_str());
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
