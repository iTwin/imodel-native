#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/Utils/FeatureUserDataMap.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct FeatureEvent {
	public:
		Utf8StringCR m_featureId;
		BeVersionCR m_version;
		Utf8StringCR m_projectId;
		FeatureUserDataMap* m_featureUserData;
		FeatureEvent(Utf8StringCR featureId, BeVersionCR version, Utf8StringCR projectId, FeatureUserDataMap* featureUserData);
		FeatureEvent(Utf8StringCR featureId, BeVersionCR version, Utf8StringCR projectId);
		FeatureEvent(Utf8StringCR featureId, BeVersionCR version, FeatureUserDataMap* featureUserData);
		FeatureEvent(Utf8StringCR featureId, BeVersionCR version);
		LICENSING_EXPORT Utf8String ToJson(
			int productId,
			Utf8StringCR featureString,
			Utf8StringCR deviceId
		);
};

END_BENTLEY_LICENSING_NAMESPACE