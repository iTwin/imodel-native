/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <Licensing/Utils/FeatureUserDataMap.h>
#include <Licensing/UsageType.h>
#include <json/value.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct FeatureEvent
    {
private:
    Utf8String m_featureId; // must be a GUID
    BeVersion m_version;
    Utf8String m_projectId = "";
    DateTime m_startDateZ = DateTime();
    DateTime m_endDateZ = DateTime();
    FeatureUserDataMapPtr m_featureUserData;

    static Utf8String GetVersionNumberString(BeVersion version);
    static Json::Value GetFeatureUserDataJson(FeatureUserDataMapPtr featureUserData);

public:
    LICENSING_EXPORT FeatureEvent() {};
    LICENSING_EXPORT FeatureEvent(Utf8StringCR featureId, BeVersionCR version, Utf8StringCR projectId, DateTimeCR startDateZ, DateTimeCR endDateZ, FeatureUserDataMapPtr featureUserData);
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
        Utf8StringCR correlationId,
        Utf8StringCR principalId
        );
    LICENSING_EXPORT Utf8String ToDebugString();
    };

END_BENTLEY_LICENSING_NAMESPACE