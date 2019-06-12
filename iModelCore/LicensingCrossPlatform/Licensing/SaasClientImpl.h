/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ISaasClient.h"

#include <Licensing/Utils/TimeRetriever.h>
#include <Licensing/Utils/FeatureEvent.h>

#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

#include "Providers/IBuddiProvider.h"
#include "Providers/IUlasProvider.h"


BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct SaasClientImpl> SaasClientImplPtr;

struct SaasClientImpl : ISaasClient
    {
protected:
    Utf8String m_deviceId;
    int m_productId;
    Utf8String m_featureString;
    IUlasProviderPtr m_ulasProvider;

public:
    LICENSING_EXPORT SaasClientImpl
        (
        int productId,
        Utf8StringCR featureString,
        IUlasProviderPtr ulasProvider
        );
    LICENSING_EXPORT folly::Future<BentleyStatus> TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId, Utf8StringCR deviceId, UsageType usageType, Utf8StringCR correlationId);
    LICENSING_EXPORT folly::Future<BentleyStatus> MarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent, Utf8StringCR deviceId, UsageType usageType, Utf8StringCR correlationId);
    };

END_BENTLEY_LICENSING_NAMESPACE
