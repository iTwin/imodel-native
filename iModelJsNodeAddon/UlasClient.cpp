/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "UlasClient.h"
#include "IModelJsNative.h"
#include <DgnPlatform/DgnPlatformLib.h>
#include <WebServices/Configuration/UrlProvider.h>

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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Evan.Preslar                     03/2020
//+---------------+---------------+---------------+---------------+---------------+------
folly::Future<BentleyStatus> UlasClient::TrackUsage(
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
        Utf8String message = "Must call UlasClient::Initialize first.";
        JsInterop::GetLogger().error(message.c_str());
        return BentleyStatus::ERROR;
        }

    if (projectId.empty())
        {
        Utf8String message = "Failed to track iModel.js usage: projectId was not specified.";
        JsInterop::GetLogger().error(message.c_str());
        return BentleyStatus::ERROR;
        }

    if (appVersion.IsEmpty())
        {
        Utf8String message = "Failed to track iModel.js usage: application version was not specified or an invalid version string was passed.";
        JsInterop::GetLogger().error(message.c_str());
        return BentleyStatus::ERROR;
        }

    if (Licensing::UsageType::Production > usageType || Licensing::UsageType::Academic < usageType)
        {
        Utf8String message = "Failed to track iModel.js usage: usage type was not specified or an invalid usage type was specfied.";
        JsInterop::GetLogger().error(message.c_str());
        return BentleyStatus::ERROR;
        }

    return m_client->TrackUsage(accessToken, appVersion, projectId, authType, productId, deviceId, usageType, correlationId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Evan.Preslar                     03/2020
//+---------------+---------------+---------------+---------------+---------------+------
folly::Future<folly::Unit> UlasClient::PostUserUsage(
    Utf8StringCR accessToken,
    BeVersionCR appVersion,
    Utf8StringCR projectId,
    Licensing::AuthType authType,
    int productId,
    Utf8StringCR deviceId,
    Licensing::UsageType usageType,
    Utf8StringCR correlationId,
    Utf8StringCR principalId) const
    {
    if (m_client == nullptr)
        {
        Utf8String message = "Must call UlasClient::Initialize first.";
        JsInterop::GetLogger().error(message.c_str());
        throw std::runtime_error(message.c_str());
        }

    if (projectId.empty())
        {
        Utf8String message = "Failed to track iModel.js usage: projectId was not specified.";
        JsInterop::GetLogger().error(message.c_str());
        throw std::runtime_error(message.c_str());
        }

    if (appVersion.IsEmpty())
        {
        Utf8String message = "Failed to track iModel.js usage: application version was not specified or an invalid version string was passed.";
        JsInterop::GetLogger().error(message.c_str());
        throw std::runtime_error(message.c_str());
        }

    if (Licensing::UsageType::Production > usageType || Licensing::UsageType::Academic < usageType)
        {
        Utf8String message = "Failed to track iModel.js usage: usage type was not specified or an invalid usage type was specfied.";
        JsInterop::GetLogger().error(message.c_str());
        throw std::runtime_error(message.c_str());
        }

    return m_client->PostUserUsage(accessToken, appVersion, projectId, authType, productId, deviceId, usageType, correlationId, principalId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Evan.Preslar                     03/2020
//+---------------+---------------+---------------+---------------+---------------+------
folly::Future<BentleyStatus> UlasClient::MarkFeature(
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
        Utf8String message = "Must call UlasClient::Initialize first.";
        JsInterop::GetLogger().error(message.c_str());
        return BentleyStatus::ERROR;
        }

    return m_client->MarkFeature(accessToken, featureEvent, authType, productId, deviceId, usageType, correlationId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Evan.Preslar                     03/2020
//+---------------+---------------+---------------+---------------+---------------+------
folly::Future<folly::Unit> UlasClient::PostFeatureUsage(
    Utf8StringCR accessToken,
    Licensing::FeatureEvent featureEvent,
    Licensing::AuthType authType,
    int productId,
    Utf8StringCR deviceId,
    Licensing::UsageType usageType,
    Utf8StringCR correlationId,
    Utf8StringCR principalId) const
    {
    if (m_client == nullptr)
        {
        Utf8String message = "Must call UlasClient::Initialize first.";
        JsInterop::GetLogger().error(message.c_str());
        throw std::runtime_error(message.c_str());
        }

    return m_client->PostFeatureUsage(accessToken, featureEvent, authType, productId, deviceId, usageType, correlationId, principalId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Zain.Ulabidin                     11/2019
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus UlasClient::CheckEntitlement(
    Utf8StringCR accessToken,
    BeVersionCR appVersion,
    Utf8StringCR projectId,
    Licensing::AuthType authType,
    int productId,
    Utf8StringCR deviceId,
    Utf8StringCR correlationId,
    Licensing::EntitlementResult &entitlementResult) const
    {
    if (m_client == nullptr)
        {
        BeAssert(false && "Must call UlasClient::Initialize first.");
        return ERROR;
        }

    if (projectId.empty())
        {
        JsInterop::GetLogger().error("Failed to validate entitlements: projectId was not specified.");
        return ERROR;
        }

    if (appVersion.IsEmpty())
        {
        JsInterop::GetLogger().error("Failed to validate entitlements: application version was not specified or an invalid version string was passed.");
        return ERROR;
        }

    BentleyStatus status = BentleyStatus::ERROR;
    auto result = m_client->CheckEntitlement(accessToken, appVersion, projectId, authType, productId, deviceId,correlationId)
    .then([&entitlementResult, &status](Licensing::EntitlementResult value) {
        entitlementResult = value;
        status = BentleyStatus::SUCCESS;
            }).onError([&status](std::exception& ex) {
                status = BentleyStatus::ERROR;
                });
    // Without this check the function exit without returning the result
    while (result.isReady() != true){}

    return status;
    }

    //-------------------------------------------------------------------------------------
    // @bsimethod                                                         Matt Yale 4/2020
    //-------------------------------------------------------------------------------------
    folly::Future<TrackUsageStatus> UlasClient::EntitlementWorkflow
    (
        Utf8StringCR accessToken,
        BeVersionCR version,
        Utf8StringCR projectId,
        Licensing::AuthType authType,
        std::vector<int> productIds,
        Utf8StringCR deviceId,
        Utf8StringCR correlationId
    ) const
    {
    //Param Validation 
    TrackUsageStatus status = TrackUsageStatus::NotEntitled;
    //Implementation
    auto result = m_client->EntitlementWorkflow(accessToken, version, projectId, authType, productIds, deviceId, correlationId)
    .then([&status](TrackUsageStatus value) {
        status = value;
                }).onError([&status](std::exception& ex) {
                status = TrackUsageStatus::Error;
                });
    // Without this check the function exit without returning the result
    while (result.isReady() != true){}

    return status;
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