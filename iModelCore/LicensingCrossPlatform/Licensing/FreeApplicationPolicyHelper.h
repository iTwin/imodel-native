/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/FreeApplicationPolicyHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "Policy.h"

#include <Licensing/Licensing.h>
#include <json/json.h>
#include <sstream>

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct FreeApplicationPolicyHelper
{
	private:
		static std::string PolicyStart()
			{
				return "{";
			};

		static std::string PolicyEnd()
			{
				return "}";
			};

		static std::string PolicyNext()
			{
				return ",";
			};

		static std::string PN()
			{
				return PolicyNext();
			};

		static std::string PolicyId(Utf8StringCR id)
			{
				std::ostringstream stream;
				stream << "\"PolicyId\":\"";
				stream << id.c_str();
				stream << "\"";
				return stream.str();
			};

		static std::string AppliesToUserId(Utf8StringCR userId)
		{
			std::ostringstream stream;
			stream << "\"AppliesToUserId\":\"";
			stream << userId.c_str();
			stream << "\"";
			return stream.str();
		};

		static std::string UserData(Utf8StringCR userId)
			{
				std::ostringstream stream;
				stream << "\"UserData\":{\"EntitlementGroupId\":\"00000000-0000-0000-0000-000000000000\",\"OrganizationId\":\"00000000-0000-0000-0000-000000000000\",\"UltimateCountryId\":\"00000000-0000-0000-0000-000000000000\",\"UltimateId\":\"00000000-0000-0000-0000-000000000000\",\"UltimateSAPId\":\"0000000000\",\"UsageCountryISO\":\"US\",\"UserId\":\"";
				stream << userId.c_str();
				stream << "\"}";
				return stream.str();
			};

		static std::string DefaultQualifiers()
			{
				return "\"DefaultQualifiers\":[{\"Name\":\"FrequencyToSend\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"24\"},{\"Name\":\"UseAlertingService\",\"Prompt\":\"You have reached a license threshold defined by your organization's administrator. Continuing may result in additional application usage charges.\\r\\nTo proceed, you must \\\"Acknowledge\\\".\",\"Type\":\"enum\",\"Value\":\"Disabled\"},{\"Name\":\"IsCheckedOut\",\"Prompt\":null,\"Type\":\"bool\",\"Value\":\"false\"},{\"Name\":\"AllowOfflineUsage\",\"Prompt\":\"You must Sign In to use this application.\",\"Type\":\"bool\",\"Value\":\"TRUE\"},{\"Name\":\"HotpathRetryAttempts\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"4\"},{\"Name\":\"MaxLogFileDurationInMinutes\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"360\"},{\"Name\":\"OfflineDuration\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"7\"},{\"Name\":\"TimeToKeepUnSentLogs\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"60\"},{\"Name\":\"UsageType\",\"Prompt\":null,\"Type\":\"string\",\"Value\":\"Production\"},{\"Name\":\"LogRecordsToSendFT2\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"75000\"},{\"Name\":\"PolicyInterval\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"60\"},{\"Name\":\"LoginGracePeriod\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"7\"},{\"Name\":\"HeartbeatInterval\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"1\"}]";
			};

	public:
		static std::shared_ptr<Policy> CreatePolicy()
			{
				std::ostringstream stream;
				stream << PolicyStart();
				stream << PolicyId("00000000-0000-0000-0000-000000000000") << PN();
				stream << UserData("00000000-0000-0000-0000-000000000000") << PN();
				stream << DefaultQualifiers();
				stream << PolicyEnd();
				auto json = Json::Reader::DoParse(Utf8String(stream.str().c_str()));
				return Policy::Create(json);
			};
};


END_BENTLEY_LICENSING_NAMESPACE