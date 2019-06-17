/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "IBuddiProvider.h"
#include "IUlasProvider.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct UlasProvider : IUlasProvider
    {
protected:
    IBuddiProviderPtr m_buddiProvider;
    Http::IHttpHandlerPtr m_httpHandler;

public:
    LICENSING_EXPORT UlasProvider
        (
        IBuddiProviderPtr buddiProvider,
        Http::IHttpHandlerPtr httpHandler
        );

    LICENSING_EXPORT BentleyStatus PostUsageLogs
        (
        ApplicationInfoPtr applicationInfo,
        BeFileNameCR dbPath,
        ILicensingDb& licensingDb,
        std::shared_ptr<Policy> policy
        );
    LICENSING_EXPORT folly::Future<folly::Unit> SendUsageLogs
        (
        ApplicationInfoPtr applicationInfo,
        BeFileNameCR usageCSV,
        Utf8StringCR ultId
        );
    LICENSING_EXPORT BentleyStatus PostFeatureLogs
        (
        ApplicationInfoPtr applicationInfo,
        BeFileNameCR dbPath,
        ILicensingDb& licensingDb,
        std::shared_ptr<Policy> policy
        );
    LICENSING_EXPORT folly::Future<folly::Unit> SendFeatureLogs
        (
        ApplicationInfoPtr applicationInfo,
        BeFileNameCR featureCSV,
        Utf8StringCR ultId
        );

    LICENSING_EXPORT folly::Future<BentleyStatus> RealtimeTrackUsage
        (
        Utf8StringCR accessToken,
        int productId,
        Utf8StringCR featureString,
        Utf8StringCR deviceId,
        BeVersionCR version,
        Utf8StringCR projectId,
        UsageType usageType,
        Utf8StringCR correlationId
        );
    LICENSING_EXPORT folly::Future<BentleyStatus> RealtimeMarkFeature
        (
        Utf8StringCR accessToken,
        FeatureEvent featureEvent,
        int productId,
        Utf8StringCR featureString,
        Utf8StringCR deviceId,
        UsageType usageType,
        Utf8StringCR correlationId
        );

    LICENSING_EXPORT folly::Future<Json::Value> GetAccessKeyInfo
        (
        ApplicationInfoPtr applicationInfo,
        Utf8StringCR accessKey
        );
    };

END_BENTLEY_LICENSING_NAMESPACE
