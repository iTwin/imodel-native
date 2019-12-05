/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "UlasProviderMock.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

BentleyStatus UlasProviderMock::PostUsageLogs(ApplicationInfoPtr applicationInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy)
    {
    m_postUsageLogsCalls++;
    return m_mockedPostUsageLogs;
    }

BentleyStatus UlasProviderMock::PostFeatureLogs(ApplicationInfoPtr applicationInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy)
    {
    m_postFeatureLogsCalls++;
    return m_mockedPostFeatureLogs;
    }

folly::Future<BentleyStatus> UlasProviderMock::RealtimeTrackUsage(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId, UsageType usageType, Utf8StringCR correlationId, AuthType authType, Utf8StringCR principalId)
    {
    m_realtimeTrackUsageCalls++;
    return folly::makeFuture(m_mockedRealtimeTrackUsage);
    }

folly::Future<BentleyStatus> UlasProviderMock::RealtimeTrackUsage(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId, LicenseStatus licenseStatus, Utf8StringCR correlationId, AuthType authType, Utf8StringCR principalId)
{
    m_realtimeTrackUsageCalls++;
    return folly::makeFuture(m_mockedRealtimeTrackUsage);
}

folly::Future<BentleyStatus> UlasProviderMock::RealtimeMarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, UsageType usageType, Utf8StringCR correlationId, AuthType authType)
    {
    m_realtimeMarkFeatureCalls++;
    return folly::makeFuture(m_mockedRealtimeMarkFeature);
    }

folly::Future<Json::Value> UlasProviderMock::GetAccessKeyInfo(ApplicationInfoPtr applicationInfo, Utf8StringCR accessKey, Utf8StringCR ultimateId)
    {
    m_getAccessKeyInfoCalls++;
    return folly::makeFuture(m_mockedGetAccessKeyInfo);
    }
