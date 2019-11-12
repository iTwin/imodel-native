/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/


#include <Licensing/SaasClient.h>
#include "SaasClientImpl.h"
#include "Providers/BuddiProvider.h"
#include "Providers/UlasProvider.h"

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SaasClient::SaasClient
    (
    std::shared_ptr<struct ISaasClient> implementation
    )
    {
    m_impl = implementation;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SaasClientPtr SaasClient::Create
    (
    int productId,
    Utf8StringCR featureString,
    Http::IHttpHandlerPtr httpHandler
    )
    {
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, httpHandler);
    return std::shared_ptr<SaasClient>(new SaasClient(std::make_shared<SaasClientImpl>(productId, featureString, ulasProvider)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> SaasClient::TrackUsage
    (
    Utf8StringCR accessToken,
    BeVersionCR version,
    Utf8StringCR projectId,
    AuthType authType,
    int productId,
    Utf8StringCR deviceId,
    UsageType usageType,
    Utf8StringCR correlationId
    )
    {
    return m_impl->TrackUsage(accessToken, version, projectId, authType, productId, deviceId, usageType, correlationId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> SaasClient::MarkFeature
    (
    Utf8StringCR accessToken,
    FeatureEvent featureEvent,
    AuthType authType,
    int productId,
    Utf8StringCR deviceId,
    UsageType usageType,
    Utf8StringCR correlationId
    )
    {
    return m_impl->MarkFeature(accessToken, featureEvent, authType, productId, deviceId, usageType, correlationId);
    }
