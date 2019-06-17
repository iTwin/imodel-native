/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <Licensing/Utils/FeatureUserDataMap.h>
#include <Licensing/UsageType.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct FeatureEvent
    {
private:
    Utf8String m_featureId; // must be a GUID
    BeVersion m_version;
    Utf8String m_projectId = "";
    FeatureUserDataMapPtr m_featureUserData;

public:
    LICENSING_EXPORT FeatureEvent() {};
    LICENSING_EXPORT FeatureEvent(Utf8StringCR featureId, BeVersionCR version, Utf8StringCR projectId, FeatureUserDataMapPtr featureUserData);
    LICENSING_EXPORT FeatureEvent(Utf8StringCR featureId, BeVersionCR version, Utf8StringCR projectId);
    LICENSING_EXPORT FeatureEvent(Utf8StringCR featureId, BeVersionCR version, FeatureUserDataMapPtr featureUserData);
    LICENSING_EXPORT FeatureEvent(Utf8StringCR featureId, BeVersionCR version);
    LICENSING_EXPORT Utf8String ToJson
        (
        int productId,
        Utf8StringCR featureString,
        Utf8StringCR deviceId,
        UsageType usageType,
        Utf8StringCR correlationId
        );
    };

END_BENTLEY_LICENSING_NAMESPACE