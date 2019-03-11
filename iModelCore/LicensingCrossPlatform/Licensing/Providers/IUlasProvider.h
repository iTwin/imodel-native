/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Providers/IUlasProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "../LicensingDb.h"
#include "../Policy.h"
#include "../Utils/FeatureEvent.h"

#include <WebServices/Client/ClientInfo.h>

#include <memory>

BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

typedef std::shared_ptr<struct IUlasProvider> IUlasProviderPtr;

struct IUlasProvider
    {
public:
    virtual BentleyStatus PostUsageLogs(ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy) = 0;
    virtual BentleyStatus PostFeatureLogs(ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy) = 0;
    virtual folly::Future<BentleyStatus> RealtimeTrackUsage(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId) = 0;
    virtual folly::Future<BentleyStatus> RealtimeMarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent, int productId, Utf8StringCR featureString, Utf8StringCR deviceId) = 0;
    virtual folly::Future<Json::Value> GetAccessKeyInfo(ClientInfoPtr clientInfo, Utf8StringCR accessKey) = 0;
    virtual ~IUlasProvider() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
