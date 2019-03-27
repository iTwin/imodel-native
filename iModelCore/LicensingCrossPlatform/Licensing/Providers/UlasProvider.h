/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Providers/UlasProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "IBuddiProvider.h"
#include "IUlasProvider.h"

#include <WebServices/Client/ClientInfo.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

struct UlasProvider : IUlasProvider
    {
protected:
    IBuddiProviderPtr m_buddiProvider;
    IHttpHandlerPtr m_httpHandler;

public:
    LICENSING_EXPORT UlasProvider
        (
        IBuddiProviderPtr buddiProvider,
        IHttpHandlerPtr httpHandler
        );

    LICENSING_EXPORT BentleyStatus PostUsageLogs(ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy);
    LICENSING_EXPORT folly::Future<folly::Unit> SendUsageLogs(ClientInfoPtr clientInfo, BeFileNameCR usageCSV, Utf8StringCR ultId);
    LICENSING_EXPORT BentleyStatus PostFeatureLogs(ClientInfoPtr clientInfo, BeFileNameCR dbPath, ILicensingDb& licensingDb, std::shared_ptr<Policy> policy);
    LICENSING_EXPORT folly::Future<folly::Unit> SendFeatureLogs(ClientInfoPtr clientInfo, BeFileNameCR featureCSV, Utf8StringCR ultId);

    LICENSING_EXPORT folly::Future<BentleyStatus> RealtimeTrackUsage(Utf8StringCR accessToken, int productId, Utf8StringCR featureString, Utf8StringCR deviceId, BeVersionCR version, Utf8StringCR projectId);
    LICENSING_EXPORT folly::Future<BentleyStatus> RealtimeMarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent, int productId, Utf8StringCR featureString, Utf8StringCR deviceId);

    LICENSING_EXPORT folly::Future<Json::Value> GetAccessKeyInfo(ClientInfoPtr clientInfo, Utf8StringCR accessKey);
    };

END_BENTLEY_LICENSING_NAMESPACE
