/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
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
    virtual folly::Future<BentleyStatus> RealtimeTrackUsage(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId, UsageType usageType, Utf8StringCR correlationId, AuthType authType) = 0;
    virtual folly::Future<BentleyStatus> RealtimeMarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, UsageType usageType, Utf8StringCR correlationId, AuthType authType) = 0;
    virtual folly::Future<Json::Value> GetAccessKeyInfo(ApplicationInfoPtr applicationInfo, Utf8StringCR accessKey) = 0;
    virtual ~IUlasProvider() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
