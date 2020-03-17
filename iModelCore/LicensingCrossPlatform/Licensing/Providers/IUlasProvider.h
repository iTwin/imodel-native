/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <folly/BeFolly.h>
#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>
#include <Licensing/ApplicationInfo.h>
#include <Licensing/UsageType.h>
#include <Licensing/AuthType.h>

#include "../LicensingDb.h"
#include "../Policy.h"
#include <Licensing/Utils/FeatureEvent.h>

#include <memory>

BEGIN_BENTLEY_LICENSING_NAMESPACE

typedef std::shared_ptr<struct IUlasProvider> IUlasProviderPtr;

struct IUlasProvider
    {
public:
    virtual BentleyStatus PostUsageLogs(ApplicationInfoPtr applicationInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy) = 0;
    virtual BentleyStatus PostFeatureLogs(ApplicationInfoPtr applicationInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy) = 0;
    virtual folly::Future<folly::Unit> RealtimeTrackUsage(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId, UsageType usageType, Utf8StringCR correlationId, AuthType authType, Utf8StringCR principalId = "") = 0;
	virtual folly::Future<folly::Unit> RealtimeTrackUsage(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId, LicenseStatus licenseStatus, Utf8StringCR correlationId, AuthType authType, Utf8StringCR principalId) = 0;
    virtual folly::Future<folly::Unit> RealtimeMarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, UsageType usageType, Utf8StringCR correlationId, AuthType authType, Utf8StringCR principalId) = 0;
    virtual folly::Future<Json::Value> GetAccessKeyInfo(ApplicationInfoPtr applicationInfo, Utf8StringCR accessKey, Utf8StringCR ultimateId) = 0;
    virtual ~IUlasProvider() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
