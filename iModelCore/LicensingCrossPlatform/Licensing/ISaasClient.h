/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Bentley/BeVersion.h>
#include <folly/BeFolly.h>
#include <Licensing/Utils/FeatureEvent.h>
#include <Licensing/UsageType.h>
#include <Licensing/AuthType.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct ISaasClient
    {
public:
    virtual folly::Future<BentleyStatus> TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId, AuthType authType, int productId, Utf8StringCR deviceId, UsageType usageType, Utf8StringCR correlationId) { return BentleyStatus::SUCCESS; };
    virtual folly::Future<BentleyStatus> MarkFeature(Utf8StringCR accessToken, FeatureEvent featureEvent, AuthType authType, int productId, Utf8StringCR deviceId, UsageType usageType, Utf8StringCR correlationId) { return BentleyStatus::SUCCESS; };
    virtual ~ISaasClient() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
