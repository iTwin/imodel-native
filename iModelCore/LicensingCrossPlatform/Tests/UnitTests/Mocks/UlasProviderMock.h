/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Mocks/UlasProviderMock.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include "../../../Licensing/Providers/IUlasProvider.h"

#include "../TestsHelper.h"

BEGIN_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE

struct UlasProviderMock : IUlasProvider
    {
public:
    MOCK_METHOD4(PostUsageLogs, BentleyStatus(ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy));
    MOCK_METHOD4(PostFeatureLogs, BentleyStatus(ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy));
    MOCK_METHOD6(RealtimeTrackUsage, folly::Future<BentleyStatus>(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId));
    MOCK_METHOD5(RealtimeMarkFeature, folly::Future<BentleyStatus>(Utf8StringCR accessToken, FeatureEvent featureEvent, int productId, Utf8StringCR featureString, Utf8StringCR deviceId));
    MOCK_METHOD2(GetAccessKeyInfo, folly::Future<Json::Value>(ClientInfoPtr clientInfo, Utf8StringCR accessKey));
    };

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE
