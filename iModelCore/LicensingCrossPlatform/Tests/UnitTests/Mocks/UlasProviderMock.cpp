
#include "UlasProviderMock.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

BentleyStatus UlasProviderMock::PostUsageLogs(ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy)
    {
    m_postUsageLogsCalls++;
    return m_mockedPostUsageLogs;
    }

BentleyStatus UlasProviderMock::PostFeatureLogs(ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy)
    {
    m_postFeatureLogsCalls++;
    return m_mockedPostFeatureLogs;
    }

folly::Future<BentleyStatus> UlasProviderMock::RealtimeTrackUsage(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId)
    {
    m_realtimeTrackUsageCalls++;
    return folly::makeFuture(m_mockedRealtimeTrackUsage);
    }

folly::Future<BentleyStatus> UlasProviderMock::RealtimeMarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent, int productId, Utf8StringCR featureString, Utf8StringCR deviceId)
    {
    m_realtimeMarkFeatureCalls++;
    return folly::makeFuture(m_mockedRealtimeMarkFeature);
    }

folly::Future<Json::Value> UlasProviderMock::GetAccessKeyInfo(ClientInfoPtr clientInfo, Utf8StringCR accessKey)
    {
    m_getAccessKeyInfoCalls++;
    return folly::makeFuture(m_mockedGetAccessKeyInfo);
    }
