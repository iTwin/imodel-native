/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ISaasClient.h"

#include <Licensing/Utils/TimeRetriever.h>
#include <Licensing/Utils/FeatureEvent.h>

#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

#include "Providers/IBuddiProvider.h"
#include "Providers/IUlasProvider.h"
#include "Providers/IEntitlementProvider.h"


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
    IEntitlementProviderPtr m_entitlementProvider;

public:
    LICENSING_EXPORT SaasClientImpl
        (
        int productId,
        Utf8StringCR featureString,
        IUlasProviderPtr ulasProvider,
        IEntitlementProviderPtr entitlementProvider
        );

    LICENSING_EXPORT folly::Future<TrackUsageStatus> TrackUsage
        (
        Utf8StringCR accessToken,
        BeVersionCR version,
        Utf8StringCR projectId,
        AuthType authType,
        std::vector<int> productIds,
        Utf8StringCR deviceId,
        Utf8StringCR correlationId
        );

    /**
     * @throw Http::HttpError if the request is rejected
     */
    LICENSING_EXPORT folly::Future<folly::Unit> PostUserUsage
    (
        Utf8StringCR accessToken,
        BeVersionCR version,
        Utf8StringCR projectId,
        AuthType authType,
        int productId,
        Utf8StringCR deviceId,
        UsageType usageType,
        Utf8StringCR correlationId,
        Utf8StringCR principalId
    );

    /**
     * @throw Http::HttpError if the request is rejected
     */
    LICENSING_EXPORT folly::Future<folly::Unit> PostFeatureUsage
        (
        Utf8StringCR accessToken,
        FeatureEvent featureEvent,
        AuthType authType,
        int productId,
        Utf8StringCR deviceId,
        UsageType usageType,
        Utf8StringCR correlationId,
        Utf8StringCR principalId
        );

    LICENSING_EXPORT folly::Future<EntitlementResult> CheckEntitlement
        (
        Utf8StringCR accessToken,
        BeVersionCR version,
        Utf8StringCR projectId,
        AuthType authType,
        int productId,
        Utf8StringCR deviceId,
        Utf8StringCR correlationId
        );
    };

END_BENTLEY_LICENSING_NAMESPACE
