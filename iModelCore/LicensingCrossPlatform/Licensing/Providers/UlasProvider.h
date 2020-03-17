/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>
#include <Licensing/UsageType.h>

#include "IBuddiProvider.h"
#include "IUlasProvider.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

inline UsageType LicenseStatusToUsageType(LicenseStatus licenseStatus)
    {
    auto usageType = licenseStatus == LicenseStatus::Trial ? UsageType::Trial : UsageType::Production;
    return usageType;
    }

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

    /**
     * @throw Http::HttpError if the request is rejected
     */
    LICENSING_EXPORT folly::Future<folly::Unit> RealtimeTrackUsage
        (
        Utf8StringCR accessToken,
        int productId,
        Utf8StringCR featureString,
        Utf8StringCR deviceId,
        BeVersionCR version,
        Utf8StringCR projectId,
        UsageType usageType,
        Utf8StringCR correlationId,
        AuthType authType,
        Utf8StringCR principalId
        );

    /**
     * @throw Http::HttpError if the request is rejected
     */
    LICENSING_EXPORT folly::Future<folly::Unit> RealtimeTrackUsage
    (
        Utf8StringCR accessToken,
        int productId,
        Utf8StringCR featureString,
        Utf8StringCR deviceId,
        BeVersionCR version,
        Utf8StringCR projectId,
        LicenseStatus licenseStatus,
        Utf8StringCR correlationId,
        AuthType authType,
        Utf8StringCR principalId
    );

    /**
     * @throw Http::HttpError if the request is rejected
     */
    LICENSING_EXPORT folly::Future<folly::Unit> RealtimeMarkFeature
        (
        Utf8StringCR accessToken,
        FeatureEvent featureEvent,
        int productId,
        Utf8StringCR featureString,
        Utf8StringCR deviceId,
        UsageType usageType,
        Utf8StringCR correlationId,
        AuthType authType,
        Utf8StringCR principalId
        );

    LICENSING_EXPORT folly::Future<Json::Value> GetAccessKeyInfo
        (
        ApplicationInfoPtr applicationInfo,
        Utf8StringCR accessKey,
        Utf8StringCR ultimateId
        );
    };

END_BENTLEY_LICENSING_NAMESPACE
