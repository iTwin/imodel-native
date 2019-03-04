#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/Utils/DateHelper.h>
#include <BeSQLite/BeSQLite.h>
#include <WebServices/Client/ClientInfo.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

namespace FeatureEvent {
    Utf8String ToJson(
        Utf8StringCR featureId,
        int productId,
        Utf8StringCR featureString, 
        BeVersionCR version,
        Utf8StringCR deviceId,
        Utf8StringCR projectId
    ) {
        const uint64_t versionNumber = UINT64_C(1000000000000) * version.GetMajor() + UINT64_C(100000000) * version.GetMinor() + UINT64_C(10000) * version.GetSub1() + ((uint64_t) version.GetSub2());
        Utf8String ver;
        ver.Sprintf("%" PRIu64 "", versionNumber);

        Json::Value requestJson(Json::objectValue);

        requestJson["hID"] = deviceId;
        requestJson["polID"] = BeSQLite::BeGuid(true).ToString();
        requestJson["secID"] = BeSQLite::BeGuid(true).ToString();
        requestJson["prdid"] = Utf8String(std::to_string(productId).c_str());
        requestJson["fstr"] = featureString;
        requestJson["ftrID"] = featureId;
        requestJson["ver"] = ver;
        requestJson["projID"] = projectId;
        requestJson["corID"] = BeSQLite::BeGuid(true).ToString();
        requestJson["evTimeZ"] = DateHelper::GetCurrentTime();
        requestJson["lVer"] = 1;
        requestJson["lSrc"] = "RealTime";
        requestJson["uType"] = "Production";

        return Json::FastWriter().write(requestJson);
    }
}

END_BENTLEY_LICENSING_NAMESPACE