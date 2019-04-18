/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include "../../../Licensing/Providers/IUlasProvider.h"

#include "../TestsHelper.h"

BEGIN_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE

struct UlasProviderMock : IUlasProvider
    {
private:
    BentleyStatus m_mockedPostUsageLogs = BentleyStatus::BSISUCCESS;
    BentleyStatus m_mockedPostFeatureLogs = BentleyStatus::BSISUCCESS;
    BentleyStatus m_mockedRealtimeTrackUsage = BentleyStatus::BSISUCCESS;
    BentleyStatus m_mockedRealtimeMarkFeature = BentleyStatus::BSISUCCESS;
    Json::Value m_mockedGetAccessKeyInfo = Json::Value::GetNull();

    int m_postUsageLogsCalls = 0;
    int m_postFeatureLogsCalls = 0;
    int m_realtimeTrackUsageCalls = 0;
    int m_realtimeMarkFeatureCalls = 0;
    int m_getAccessKeyInfoCalls = 0;

public:
    BentleyStatus PostUsageLogs(WebServices::ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy) override;
    BentleyStatus PostFeatureLogs(WebServices::ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy) override;
    folly::Future<BentleyStatus> RealtimeTrackUsage(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId) override;
    folly::Future<BentleyStatus> RealtimeMarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent, int productId, Utf8StringCR featureString, Utf8StringCR deviceId) override;
    folly::Future<Json::Value> GetAccessKeyInfo(WebServices::ClientInfoPtr clientInfo, Utf8StringCR accessKey) override;

    void MockPostUsageLogs(BentleyStatus mocked) { m_mockedPostUsageLogs = mocked; }
    void MockPostFeatureLogs(BentleyStatus mocked) { m_mockedPostFeatureLogs = mocked; }
    void MockRealtimeTrackUsage(BentleyStatus mocked) { m_mockedRealtimeTrackUsage = mocked; }
    void MockRealtimeMarkFeature(BentleyStatus mocked) { m_mockedRealtimeMarkFeature = mocked; }
    void MockGetAccessKeyInfo(Json::Value mocked) { m_mockedGetAccessKeyInfo = mocked; }

    int PostUsageLogsCalls() const { return m_postUsageLogsCalls; }
    int PostFeatureLogsCalls() const { return m_postFeatureLogsCalls; }
    int RealtimeTrackUsageCalls() const { return m_realtimeTrackUsageCalls; }
    int RealtimeMarkFeatureCalls() const { return m_realtimeMarkFeatureCalls; }
    int GetAccessKeyInfoCalls() const { return m_getAccessKeyInfoCalls; }
    };

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE
