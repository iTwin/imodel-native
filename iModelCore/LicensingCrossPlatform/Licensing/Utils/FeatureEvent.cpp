#include <Licensing/Licensing.h>
#include <Licensing/Utils/DateHelper.h>
#include <BeSQLite/BeSQLite.h>
#include "FeatureEvent.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

FeatureEvent::FeatureEvent(Utf8StringCR featureId, BeVersionCR version, Utf8StringCR projectId, FeatureUserDataMap * featureUserData)
	: m_featureId(featureId),
	m_version(version),
	m_projectId(projectId),
	m_featureUserData(featureUserData)
{}

FeatureEvent::FeatureEvent(Utf8StringCR featureId, BeVersionCR version, FeatureUserDataMap * featureUserData)
	: m_featureId(featureId),
	m_version(version),
	m_featureUserData(featureUserData),
	m_projectId("")
{}

FeatureEvent::FeatureEvent(Utf8StringCR featureId, BeVersionCR version, Utf8StringCR projectId)
	: m_featureId(featureId),
	m_version(version),
	m_projectId(projectId),
	m_featureUserData()
{}

FeatureEvent::FeatureEvent(Utf8StringCR featureId, BeVersionCR version)
	: m_featureId(featureId),
	m_version(version),
	m_projectId(""),
	m_featureUserData()
{}

Utf8String FeatureEvent::ToJson(
	int productId,
	Utf8StringCR featureString,
	Utf8StringCR deviceId
)
    {
    const uint64_t versionNumber = UINT64_C(1000000000000) * m_version.GetMajor() + UINT64_C(100000000) * m_version.GetMinor() + UINT64_C(10000) * m_version.GetSub1() + ((uint64_t) m_version.GetSub2());
    Utf8String ver;
    ver.Sprintf("%" PRIu64 "", versionNumber);

    Json::Value requestJson(Json::objectValue);
	Json::Value userDataJson(Json::arrayValue);

	Utf8StringVector keys;
	m_featureUserData->GetKeys(keys);

	for (Utf8StringVector::iterator keys_it = keys.begin(); keys_it != keys.end(); keys_it++)
	{
		Json::Value userDataObj(Json::objectValue);
		Utf8String value;
		auto key = keys_it->c_str();
		m_featureUserData->GetValue(key, value);

		userDataObj["name"] = Utf8String(key);
		userDataObj["value"] = value;
		userDataJson.append(userDataObj);
	}

    requestJson["hID"] = deviceId;
    requestJson["polID"] = BeSQLite::BeGuid(true).ToString();
    requestJson["secID"] = BeSQLite::BeGuid(true).ToString();
    requestJson["prdid"] = Utf8String(std::to_string(productId).c_str());
    requestJson["fstr"] = featureString;
    requestJson["ftrID"] = m_featureId;
    requestJson["ver"] = ver;
    requestJson["projID"] = m_projectId;
    requestJson["corID"] = BeSQLite::BeGuid(true).ToString();
    requestJson["evTimeZ"] = DateHelper::GetCurrentTime();
    requestJson["lVer"] = 1;
    requestJson["lSrc"] = "RealTime";
    requestJson["uType"] = "Production";
	requestJson["uData"] = userDataJson;

    return Json::FastWriter().write(requestJson);
    }

END_BENTLEY_LICENSING_NAMESPACE

