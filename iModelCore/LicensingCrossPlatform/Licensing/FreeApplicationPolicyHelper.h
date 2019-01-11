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

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct FreeApplicationPolicyHelper
{
	private:
		static Utf8String PolicyStart()
			{
				return "{";
			};

		static Utf8String PolicyEnd()
			{
				return "}";
			};

		static Utf8String PolicyNext()
			{
				return ",";
			};

		static Utf8String PN()
			{
				return PolicyNext();
			};

		static Utf8String PolicyId(Utf8StringCR id)
			{
				Utf8String string;
				string += "\"PolicyId\":\"";
				string += id;
				string += "\"";
				return string;
			};

		static Utf8String AppliesToUserId(Utf8StringCR userId)
		{
			Utf8String string;
			string += "\"AppliesToUserId\":\"";
			string += userId;
			string += "\"";
			return string;
		};

		static Utf8String UserData(Utf8StringCR userId)
			{
				Utf8String string;
				string += "\"UserData\":{\"EntitlementGroupId\":\"00000000-0000-0000-0000-000000000000\",\"OrganizationId\":\"00000000-0000-0000-0000-000000000000\",\"UltimateCountryId\":\"00000000-0000-0000-0000-000000000000\",\"UltimateId\":\"00000000-0000-0000-0000-000000000000\",\"UltimateSAPId\":\"0000000000\",\"UsageCountryISO\":\"US\",\"UserId\":\"";
				string += userId;
				string += "\"}";
				return string;
			};

		static Utf8String DefaultQualifiers()
			{
				return "\"DefaultQualifiers\":[{\"Name\":\"FrequencyToSend\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"24\"},{\"Name\":\"UseAlertingService\",\"Prompt\":\"You have reached a license threshold defined by your organization's administrator. Continuing may result in additional application usage charges.\\r\\nTo proceed, you must \\\"Acknowledge\\\".\",\"Type\":\"enum\",\"Value\":\"Disabled\"},{\"Name\":\"IsCheckedOut\",\"Prompt\":null,\"Type\":\"bool\",\"Value\":\"false\"},{\"Name\":\"AllowOfflineUsage\",\"Prompt\":\"You must Sign In to use this application.\",\"Type\":\"bool\",\"Value\":\"TRUE\"},{\"Name\":\"HotpathRetryAttempts\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"4\"},{\"Name\":\"MaxLogFileDurationInMinutes\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"360\"},{\"Name\":\"OfflineDuration\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"7\"},{\"Name\":\"TimeToKeepUnSentLogs\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"60\"},{\"Name\":\"UsageType\",\"Prompt\":null,\"Type\":\"string\",\"Value\":\"Production\"},{\"Name\":\"LogRecordsToSendFT2\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"75000\"},{\"Name\":\"PolicyInterval\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"60\"},{\"Name\":\"LoginGracePeriod\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"7\"},{\"Name\":\"HeartbeatInterval\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"1\"}]";
			};

	public:
		static std::shared_ptr<Policy> CreatePolicy()
			{
				Utf8String string;
				string += PolicyStart();
				string += PolicyId("00000000-0000-0000-0000-000000000000") += PN();
				string += UserData("00000000-0000-0000-0000-000000000000") += PN();
				string += DefaultQualifiers();
				string += PolicyEnd();
				auto json = Json::Reader::DoParse(Utf8String(string));
				return Policy::Create(json);
			};
};


END_BENTLEY_LICENSING_NAMESPACE