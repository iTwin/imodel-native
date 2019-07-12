/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "UlasClient.h"
#include "IModelJsNative.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <WebServices/Configuration/UrlProvider.h>

// WIP_LICENSING_FOR_MACOS
#if defined(BENTLEYCONFIG_OS_APPLE_MACOS)
#include <folly/BeFolly.h>
#include <folly/futures/Future.h>

namespace Licensing
{
struct SaasClient
    {
    SaasClient(int productId = -1) {}
    static SaasClientPtr Create(int productId = -1) { return std::make_shared<SaasClient>(); }
    folly::Future<BentleyStatus> TrackUsage(Utf8StringCR accessToken, BeVersionCR appVersion, Utf8StringCR projectId, Licensing::AuthType authType = Licensing::AuthType::OIDC, int productId = -1, Utf8StringCR deviceId = "", Licensing::UsageType usageType = Licensing::UsageType::Production, Utf8StringCR correlationId = "") const
        {
        IModelJsNative::JsInterop::GetLogger().warning("Usage tracking not supported yet on MacOS.");
        return folly::makeFuture(ERROR);
        }

    folly::Future<BentleyStatus> MarkFeature(Utf8StringCR accessToken, Licensing::FeatureEvent featureEvent, Licensing::AuthType authType = Licensing::AuthType::OIDC, int productId = -1, Utf8StringCR deviceId = "", Licensing::UsageType usageType = Licensing::UsageType::Production, Utf8StringCR correlationId = "") const
        {
        IModelJsNative::JsInterop::GetLogger().warning("Feature tracking not supported yet on MacOS.");
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

    m_client = Licensing::SaasClient::Create(2686);
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
BentleyStatus UlasClient::TrackUsage(
    Utf8StringCR accessToken,
    BeVersionCR appVersion,
    Utf8StringCR projectId,
    Licensing::AuthType authType,
    int productId,
    Utf8StringCR deviceId,
    Licensing::UsageType usageType,
    Utf8StringCR correlationId) const
    {
    if (m_client == nullptr)
        {
        BeAssert(false && "Must call UlasClient::Initialize first.");
        return ERROR;
        }

    if (projectId.empty())
        {
        JsInterop::GetLogger().error("Failed to track iModel.js usage: projectId was not specified.");
        return ERROR;
        }

    if (appVersion.IsEmpty())
        {
        JsInterop::GetLogger().error("Failed to track iModel.js usage: application version was not specified or an invalid version string was passed.");
        return ERROR;
        }

    return m_client->TrackUsage(accessToken, appVersion, projectId, authType, productId, deviceId, usageType, correlationId).onError([] (void* e) { return ERROR; }).get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Evan.Preslar                     05/2019
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus UlasClient::MarkFeature(
    Utf8StringCR accessToken,
    Licensing::FeatureEvent featureEvent,
    Licensing::AuthType authType,
    int productId,
    Utf8StringCR deviceId,
    Licensing::UsageType usageType,
    Utf8StringCR correlationId) const
    {
    if (m_client == nullptr)
        {
        BeAssert(false && "Must call UlasClient::Initialize first.");
        return ERROR;
        }

    return m_client->MarkFeature(accessToken, featureEvent, authType, productId, deviceId, usageType, correlationId).onError([] (void* e) { return ERROR; }).get();
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