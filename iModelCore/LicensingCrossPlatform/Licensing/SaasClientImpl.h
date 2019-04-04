/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/SaasClientImpl.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ISaasClient.h"

#include <Licensing/Utils/TimeRetriever.h>
#include <Licensing/Utils/DelayedExecutor.h>
#include <Licensing/Utils/FeatureEvent.h>

#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

#include "Providers/IBuddiProvider.h"
#include "Providers/IUlasProvider.h"


BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct SaasClientImpl> SaasClientImplPtr;

struct SaasClientImpl : ISaasClient
    {
protected:
    Utf8String m_deviceId;
    int m_productId;
    ITimeRetrieverPtr m_timeRetriever;
    IDelayedExecutorPtr m_delayedExecutor;
    Utf8String m_featureString;
    Utf8String m_correlationId;
    IUlasProviderPtr m_ulasProvider;

public:
    LICENSING_EXPORT SaasClientImpl
        (
        int productId,
        Utf8StringCR featureString,
        IUlasProviderPtr ulasProvider
        );
    LICENSING_EXPORT folly::Future<BentleyStatus> TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId);
    LICENSING_EXPORT folly::Future<BentleyStatus> MarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent);
    };

END_BENTLEY_LICENSING_NAMESPACE
