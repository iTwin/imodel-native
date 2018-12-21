/*--------------------------------------------------------------------------------------+
|
|     $Source: ECPresentationUtils.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "UlasClient.h"
#include "IModelJsNative.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <WebServices/Configuration/UrlProvider.h>

// WIP_LICENSING_FOR_LINUX_AND_MACOS
#if defined(BENTLEYCONFIG_OS_APPLE_MACOS) || defined(BENTLEYCONFIG_OS_LINUX)
#include <folly/BeFolly.h>
#include <folly/futures/Future.h>

namespace Licensing
{
typedef std::shared_ptr<struct Client> ClientPtr;

struct Client
    {
    Client() {}
    static ClientPtr CreateFree() { return std::make_shared<Client>(); }
    folly::Future<BentleyStatus> TrackUsage(Utf8StringCR accessToken, BeVersionCR appVersion, Utf8StringCR projectId)
        {
        IModelJsNative::JsInterop::GetLogger().warning("Usage tracking not supported yet on Linux and MacOS.");
        return folly::makeFuture(ERROR);
        }
    };
};
#endif

namespace IModelJsNative
    {

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             12/2018
//+---------------+---------------+---------------+---------------+---------------+------
//static
UlasClient* UlasClient::s_singleton = nullptr;

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             12/2018
//+---------------+---------------+---------------+---------------+---------------+------
UlasClient::UlasClient() {}

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             12/2018
//+---------------+---------------+---------------+---------------+---------------+------
UlasClient::~UlasClient()
    {
    if (m_client != nullptr)
        m_client = nullptr;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             12/2018
//+---------------+---------------+---------------+---------------+---------------+------
void UlasClient::Initialize(Region region)
    {
    Uninitialize();

    WebServices::UrlProvider::Environment env;
    switch (region)
        {
        case Region::Dev:
            env = WebServices::UrlProvider::Dev;
            break;

        case Region::Qa:
            env = WebServices::UrlProvider::Qa;
            break;

        case Region::Perf:
            env = WebServices::UrlProvider::Perf;
            break;

        default:
        case Region::Prod:
            env = WebServices::UrlProvider::Release;
            break;
        }

    // DefaultTimeout: cache is cleared every 24 hours
    WebServices::UrlProvider::Initialize(env, WebServices::UrlProvider::DefaultTimeout, &m_localState);
    JsInterop::GetLogger().infov("Initialized iModel.js addon to region '%s'.", WebServices::UrlProvider::ToEnvironmentString(env).c_str());

    m_client = Licensing::Client::CreateFree();
    BeAssert(m_client != nullptr);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             12/2018
//+---------------+---------------+---------------+---------------+---------------+------
void UlasClient::Uninitialize()
    {
    WebServices::UrlProvider::Uninitialize();
    m_localState = RuntimeJsonLocalState();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             11/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus UlasClient::TrackUsage(Utf8StringCR accessToken, Utf8StringCR appVersionStr, Utf8StringCR projectId) const
    {
    if (m_client == nullptr)
        {
        BeAssert(false && "Must call UlasClient::Initialize first.");
        return ERROR;
        }

    if (projectId.empty())
        {
        JsInterop::GetLogger().error("Failed to set-up iModel.js usage tracking. ProjectId was not specified when opening the iModel.");
        return ERROR;
        }

    BeVersion appVersion(appVersionStr.c_str());
    if (appVersion.IsEmpty())
        {
        JsInterop::GetLogger().error("Failed to set-up iModel.js usage tracking. Application version was not specified when opening the iModel or an invalid version string was passed.");
        return ERROR;
        }

    return m_client->TrackUsage(accessToken, appVersion, projectId).onError([] (void* e) { return ERROR; }).get();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle             12/2018
//+---------------+---------------+---------------+---------------+---------------+------
//static
UlasClient& UlasClient::Get()
    {
    static std::once_flag s_onceFlag;
    std::call_once(s_onceFlag, [] ()
        {
        BeAssert(ECDb::IsInitialized() && "Licensing::Client requires BeSQLite to be initialized");
        s_singleton = new UlasClient();
        });

    return *s_singleton;
    }
}