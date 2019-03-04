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
        // TODO convert this to use Json::Value so it doesn't take 10 minutes to read/update
        Utf8String jsonString;

        jsonString += "{";
        //stream << "\"ultID\": " << 1234 << ","; // ultimate id [NUMBER]
        //stream << "\"pid\": " << 1234 << ","; // principal id (same as IMS id?) [GUID]
        //stream << "\"imsID\": " << 1234 << ","; // ims id [GUID]
        jsonString += "\"hID\": \"" + deviceId + "\","; // client's machine name excluding domain info [STRING]
        //stream << "\"uID\": " << 1234 << ","; // client's login name excluding domain information (same is IMS id?) [STRING]
        jsonString += "\"polID\": \"" + BeSQLite::BeGuid(true).ToString() + "\","; //  FreeApplicationPolicyHelper::GetRandomGUID() << "\","; // policy file ID [GUID]
        jsonString += "\"secID\": \"" + BeSQLite::BeGuid(true).ToString() + "\","; // FreeApplicationPolicyHelper::GetRandomGUID() << "\","; // securable ID [STRING]
        jsonString += "\"prdid\": " + Utf8String(std::to_string(productId).c_str()) + ","; // product ID (4 digits) [NUMBER]
        jsonString += "\"fstr\": \"" + featureString + "\","; // feature string [STRING]
        jsonString += "\"ftrID\": \"" + featureId + "\",";

        const uint64_t versionNumber = UINT64_C(1000000000000) * version.GetMajor() + UINT64_C(100000000) * version.GetMinor() + UINT64_C(10000) * version.GetSub1() + ((uint64_t) version.GetSub2());
        Utf8String ver;
        ver.Sprintf("\"ver\": %" PRIu64 ",", versionNumber);
        jsonString += ver; // application version [NUMBER]
        jsonString += "\"projID\": \"" + projectId + "\","; // project ID [GUID or UNDEFINED]
        jsonString += "\"corID\": \"" + BeSQLite::BeGuid(true).ToString() + "\","; // FreeApplicationPolicyHelper::GetRandomGUID() << "\","; // correlation GUID [GUID]
        jsonString += "\"evTimeZ\": \"" + DateHelper::GetCurrentTime() + "\","; // event time (current UTC time) [STRING]
        jsonString += "\"lVer\": 1,"; // version of scheme of this log [NUMBER]
        jsonString += "\"lSrc\": \"RealTime\","; // source of usage log entry: RealTime, Offline, Checkout [STRING]
        jsonString += "\"uType\": \"Production\"}"; // usage type: Production, Trial, Beta, HomeUse, PreActivation [STRING]
        return jsonString;
    }
}

END_BENTLEY_LICENSING_NAMESPACE