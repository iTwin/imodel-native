/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Licensing/Licensing.h>
#include <Licensing/Utils/DateHelper.h>
#include <BeSQLite/BeSQLite.h>
#include <Licensing/Utils/FeatureEvent.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

FeatureEvent::FeatureEvent(Utf8StringCR featureId, BeVersionCR version, Utf8StringCR projectId, FeatureUserDataMapPtr featureUserData) :
    m_featureId(featureId),
    m_version(version),
    m_projectId(projectId),
    m_featureUserData(featureUserData)
    {}

FeatureEvent::FeatureEvent(Utf8StringCR featureId, BeVersionCR version, FeatureUserDataMapPtr featureUserData) :
    m_featureId(featureId),
    m_version(version),
    m_featureUserData(featureUserData)
    {}

FeatureEvent::FeatureEvent(Utf8StringCR featureId, BeVersionCR version, Utf8StringCR projectId) :
    m_featureId(featureId),
    m_version(version),
    m_projectId(projectId)
    {
    m_featureUserData = std::make_shared<FeatureUserDataMap>();
    }

FeatureEvent::FeatureEvent(Utf8StringCR featureId, BeVersionCR version) :
    m_featureId(featureId),
    m_version(version)
    {
    m_featureUserData = std::make_shared<FeatureUserDataMap>();
    }

Utf8String FeatureEvent::ToJson
    (
    int productId,
    Utf8StringCR featureString,
    Utf8StringCR deviceId,
    UsageType usageType,
    Utf8StringCR correlationId
    )
    {
    const uint64_t versionNumber = UINT64_C(1000000000000) * m_version.GetMajor() + UINT64_C(100000000) * m_version.GetMinor() + UINT64_C(10000) * m_version.GetSub1() + ((uint64_t) m_version.GetSub2());
    Utf8String ver;
    ver.Sprintf("%" PRIu64 "", versionNumber);

    Json::Value requestJson(Json::arrayValue);
    Json::Value featureLogJson(Json::objectValue);
    Json::Value userDataJson(Json::arrayValue);

    if (!m_featureUserData->Empty())
        {
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
        }
    else
        {
        userDataJson = Json::Value::GetNull();
        }

    featureLogJson["hID"] = deviceId; // client's machine name [STRING]
    featureLogJson["polID"] = BeSQLite::BeGuid(true).ToString(); // policy file ID [GUID
    featureLogJson["secID"] = BeSQLite::BeGuid(true).ToString(); // securable ID [STRING]
    featureLogJson["prdid"] = Utf8String(std::to_string(productId).c_str()); // product ID (4 digits) [NUMBER]
    featureLogJson["fstr"] = featureString;  // feature string [STRING]
    featureLogJson["ftrID"] = m_featureId; // feature Id [STRING]
    featureLogJson["ver"] = ver; // application version [NUMBER]
    featureLogJson["projID"] = m_projectId; // project ID [GUID or UNDEFINED]
    featureLogJson["corID"] = correlationId == "" ? BeSQLite::BeGuid(true).ToString() : correlationId; // correlation GUID [GUID] new GUID if correleationId not specified
    featureLogJson["evTimeZ"] = DateHelper::GetCurrentTime(); // event time (current UTC time) [STRING]
    featureLogJson["lVer"] = 1; // version of scheme of this log [NUMBER]
    featureLogJson["lSrc"] = "RealTime"; // source of usage log entry: RealTime, Offline, Checkout [STRING]

    Utf8String usageTypeString;
    switch (usageType)
        {
        case UsageType::Production: {usageTypeString = "Production"; } break;
        case UsageType::Trial: {usageTypeString = "Trial"; } break;
        case UsageType::Beta: {usageTypeString = "Beta"; } break;
        case UsageType::HomeUse: {usageTypeString = "HomeUse"; } break;
        case UsageType::PreActivation: {usageTypeString = "PreActivation"; } break;
        case UsageType::Evaluation: {usageTypeString = "Evaluation"; } break;
        case UsageType::Academic: {usageTypeString = "Academic"; } break;
        default: {usageTypeString = "Production"; } break;
        }
    featureLogJson["uType"] = usageTypeString; // usage type: Production, Trial, Beta, HomeUse, PreActivation, Evaluation, Academic [STRING]
    featureLogJson["uData"] = userDataJson;

    requestJson.append(featureLogJson); // the input to ULAS is an array of FeatureLogEntries

    return Json::FastWriter().write(requestJson);
    }

END_BENTLEY_LICENSING_NAMESPACE

