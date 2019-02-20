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
    ClientInfoPtr m_clientInfo;
    BeFileName m_dbPath;
    IHttpHandlerPtr m_httpHandler;

public:
    UlasProvider
        (
        IBuddiProviderPtr buddiProvider,
        ClientInfoPtr clientInfo,
        BeFileNameCR dbPath,
        IHttpHandlerPtr httpHandler
        );

    BentleyStatus PostUsageLogs(UsageDb& usageDb, std::shared_ptr<Policy> policy);
    folly::Future<folly::Unit> SendUsageLogs(BeFileNameCR usageCSV, Utf8StringCR ultId);
    BentleyStatus PostFeatureLogs(UsageDb& usageDb, std::shared_ptr<Policy> policy);
    folly::Future<folly::Unit> SendFeatureLogs(BeFileNameCR featureCSV, Utf8StringCR ultId);
    };

END_BENTLEY_LICENSING_NAMESPACE
