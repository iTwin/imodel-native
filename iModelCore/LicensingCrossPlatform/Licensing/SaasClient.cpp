/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/SaasClient.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


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
    IHttpHandlerPtr httpHandler
    )
    {
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, httpHandler);
    return std::shared_ptr<SaasClient>(new SaasClient(std::make_shared<SaasClientImpl>(productId, featureString, ulasProvider)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> SaasClient::TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId)
    {
    return m_impl->TrackUsage(accessToken, version, projectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> SaasClient::MarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent)
    {
    return m_impl->MarkFeature(accessToken, featureEvent);
    }
