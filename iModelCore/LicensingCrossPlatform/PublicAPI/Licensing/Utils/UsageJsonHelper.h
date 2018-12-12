/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Utils/UsageJsonHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/Utils/DateHelper.h>

#include <json/json.h>
#include <sstream>

#include <Bentley/BeVersion.h>
#include <BeSQLite/BeSQLite.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct UsageJsonHelper
{
private:

public:
	static Json::Value CreateJsonRandomGuids(Utf8StringCR deviceID, Utf8StringCR featureString, BeVersion version, Utf8StringCR projectID)
	{
		std::ostringstream stream;
		stream << "{";
		//stream << "\"ultID\": " << 1234 << ","; // ultimate id [NUMBER]
		//stream << "\"pid\": " << 1234 << ","; // principal id (same as IMS id?) [GUID]
		//stream << "\"imsID\": " << 1234 << ","; // ims id [GUID]
		stream << "\"hID\": \"" << deviceID << "\","; // client's machine name excluding domain info [STRING]
		//stream << "\"uID\": " << 1234 << ","; // client's login name excluding domain information (same is IMS id?) [STRING]
		stream << "\"polID\": \"" << BeSQLite::BeGuid(true).ToString() << "\","; //  FreeApplicationPolicyHelper::GetRandomGUID() << "\","; // policy file ID [GUID]
		stream << "\"secID\": \"" << BeSQLite::BeGuid(true).ToString() << "\",";// FreeApplicationPolicyHelper::GetRandomGUID() << "\","; // securable ID [STRING]
		//stream << "\"prdid\": " << 1234 << ","; // product ID (4 digits) [NUMBER]
		stream << "\"fstr\": \"" << featureString << "\","; // feature string [STRING]
		stream << "\"ver\": " << version.GetMajor() * 1000000000000 + version.GetMinor() * 100000000 + version.GetSub1() * 10000 + version.GetSub2() << ","; // application version [NUMBER]
		stream << "\"projID\": \"" << projectID << "\","; // project ID [GUID or UNDEFINED]
		stream << "\"corID\": \"" << BeSQLite::BeGuid(true).ToString() << "\","; // FreeApplicationPolicyHelper::GetRandomGUID() << "\","; // correlation GUID [GUID]
		stream << "\"evTimeZ\": \"" << DateHelper::GetCurrentTime() << "\","; // event time (current UTC time) [STRING]
		stream << "\"lVer\": " << 1 << ","; // version of scheme of this log [NUMBER]
		stream << "\"lSrc\": " << "\"RealTime\"" << ","; // source of usage log entry: RealTime, Offline, Checkout [STRING]
		stream << "\"uType\": " << "\"Production\"" << "}"; // usage type: Production, Trial, Beta, HomeUse, PreActivation [STRING]

		return Json::Reader::DoParse(Utf8String(stream.str().c_str()));
	};

};


END_BENTLEY_LICENSING_NAMESPACE