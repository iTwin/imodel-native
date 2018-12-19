/*--------------------------------------------------------------------------------------+
|
|     $Source: ECPresentationUtils.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UlasClient.h"
#include "IModelJsNative.h"
#include <Bentley/BeFileName.h>
#include <Bentley/BeVersion.h>
#include <DgnPlatform/DgnPlatformLib.h>

namespace IModelJsNative
    {

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             11/2018
//+---------------+---------------+---------------+---------------+---------------+------
//static
BeFileName* UlasClient::s_cacheDbPath = nullptr;

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             12/2018
//+---------------+---------------+---------------+---------------+---------------+------
UlasClient::~UlasClient()
    {
    if (m_client != nullptr)
        {
        m_client->StopApplication();
        m_client = nullptr;
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             11/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus UlasClient::Initialize()
    {
    BeAssert(m_client == nullptr && "May not initialize UlasClient more than once.");

    if (m_appVersion.IsEmpty())
        {
        JsInterop::GetLogger().error("Application' version has not been specified.");
        return ERROR;
        }

    if (s_cacheDbPath == nullptr)
        {
        s_cacheDbPath = new BeFileName();
        if (SUCCESS != DgnPlatformLib::GetHost().GetIKnownLocationsAdmin().GetLocalTempDirectory(*s_cacheDbPath, nullptr))
            {
            JsInterop::GetLogger().error("Could not access temp directory for the ULAS cache database.");
            return ERROR;
            }

        s_cacheDbPath->AppendToPath(L"imodeljs.usagetracking.db");
        }

    m_client = Licensing::Client::CreateFree(m_accessToken, m_appVersion, m_projectId, *s_cacheDbPath);
    if (m_client == nullptr)
        {
        JsInterop::GetLogger().error("Failed to set-up usage tracking. User is not authorized.");
        return ERROR;
        }

    return SUCCESS;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             11/2018
//+---------------+---------------+---------------+---------------+---------------+------
void UlasClient::StartTracking()
    {
    BeAssert(m_client != nullptr);
    if (m_client != nullptr)
        m_client->StartApplication();
    }
//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             11/2018
//+---------------+---------------+---------------+---------------+---------------+------
void UlasClient::StopTracking()
    {
    BeAssert(m_client != nullptr);
    if (m_client != nullptr)
        m_client->StopApplication();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             11/2018
//+---------------+---------------+---------------+---------------+---------------+------
/*WebServices::ClientInfoPtr IModelJsNative::UlasHelper::GetClientInfo()
    {
    static WebServices::ClientInfoPtr info = WebServices::ClientInfo::Create(s_appName, BeVersion(s_appVersionMajor, s_appVersionMinor, s_appVersionSub1, 0), s_appGuid, s_gprId);
    return info;
    }
    */
}