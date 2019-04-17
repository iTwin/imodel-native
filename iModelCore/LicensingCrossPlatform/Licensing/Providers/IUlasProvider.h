/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <folly/BeFolly.h>
#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "../LicensingDb.h"
#include "../Policy.h"
#include <Licensing/Utils/FeatureEvent.h>

#include <WebServices/Client/ClientInfo.h>

#include <memory>

BEGIN_BENTLEY_LICENSING_NAMESPACE

typedef std::shared_ptr<struct IUlasProvider> IUlasProviderPtr;

struct IUlasProvider
    {
public:
    virtual BentleyStatus PostUsageLogs(WebServices::ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy) = 0;
    virtual BentleyStatus PostFeatureLogs(WebServices::ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy) = 0;
    virtual folly::Future<BentleyStatus> RealtimeTrackUsage(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId) = 0;
    virtual folly::Future<BentleyStatus> RealtimeMarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent, int productId, Utf8StringCR featureString, Utf8StringCR deviceId) = 0;
    virtual folly::Future<Json::Value> GetAccessKeyInfo(WebServices::ClientInfoPtr clientInfo, Utf8StringCR accessKey) = 0;
    virtual ~IUlasProvider() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
