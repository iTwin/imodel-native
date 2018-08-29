/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/DummyJsonHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <sstream>
#include <random>

#include "DateHelper.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct DummyJsonHelper
{
private:
	enum class PolicyType { Full, NoSecurables, NoACLs, NoUserData, NoRequestData, OfflineNotAllowed };
	static std::string PolicyStart() { return "{"; };
	static std::string PolicyEnd() { return "}"; };
	static std::string PolicyNext() { return ","; };
	static std::string PN() { return PolicyNext(); };
	static std::string PolicyId(Utf8String id)
		{
		std::ostringstream stream;
		stream << "\"PolicyId\":\"";
		stream << id.c_str();
		stream << "\"";
		return stream.str();
		};

	static std::string PolicyVersion() { return "\"PolicyVersion\":1.0"; };
	static std::string PolicyCreatedOn(Utf8String date)
		{
		std::ostringstream stream;
		stream << "\"PolicyCreatedOn\":\"";
		stream << date.c_str();
		stream << "\"";
		return stream.str();
		};

	static std::string PolicyExpiresOn(Utf8String date)
		{
		std::ostringstream stream;
		stream << "\"PolicyExpiresOn\":\"";
		stream << date.c_str();
		stream << "\"";
		return stream.str();
		};

	static std::string RequestData(int productId, Utf8String featureString, Utf8String userId)
		{
		std::ostringstream stream;
		stream << "\"RequestData\":{\"AccessKey\":null,\"AppliesTo\":\"https://entitlement-search.bentley.com/\",\"CheckedOutDate\":null,\"ClientDateTime\":\"2018-07-25T21:13:42.048Z\",\"Locale\":\"en\",\"MachineName\":\"TestDeviceId\",\"MachineSID\":null,\"RequestedSecurables\":[{\"FeatureString\":\"";
		stream << featureString.c_str();
		stream << "\",\"ProductId\":";
		stream << productId;
		stream << ",\"Vesrion\":null}],\"UserId\":\"";
		stream << userId.c_str();
		stream << "\"}";
		return stream.str();
		};

	static std::string MachineSignature() { return "\"MachineSignature\":\"u4o/9wtBEjZ6lfjlo8Aj+R4aaHk=\""; };
	static std::string AppliesToUserId(Utf8String userId)
		{
		std::ostringstream stream;
		stream << "\"AppliesToUserId\":\"";
		stream << userId.c_str();
		stream << "\"";
		return stream.str();
		};

	static std::string AppliesToSecurableIds() { return "\"AppliesToSecurableIds\":[\"844c10c2-375d-4153-a08f-896d2e64a13f\"]"; };
	static std::string ACLs(Utf8String expiration, int accessKind, Utf8String userId, bool isTrial, bool isOfflineUsageAllowed)
		{
		std::ostringstream stream;
		stream << "\"ACLs\":[{\"AccessKind\":";
		stream << accessKind;
		stream << ",\"ExpiresOn\":\"";
		stream << expiration.c_str();
		stream << "\",\"PrincipalId\":\"";
		stream << userId.c_str();
		stream << "\",\"SecurableId\":\"844c10c2-375d-4153-a08f-896d2e64a13f\"";
		stream << ",\"QualifierOverrides\":[{\"Name\":\"UseAlertingService\",\"Prompt\":\"You have reached a license threshold defined by your organization's administrator. Continuing may result in additional application usage charges.\\r\\nTo proceed, you must \\\"Acknowledge\\\".\",\"Type\":\"enum\",\"Value\":\"ALERT\"},{\"Name\":\"AllowOfflineUsage\",\"Prompt\":\"You must Sign In to use this application.\",\"Type\":\"bool\",\"Value\":\"";
		if (isOfflineUsageAllowed)
			{
			stream << "TRUE";
			}
		else
			{
			stream << "FALSE";
			}
		stream << "\"}";
		if (isTrial) // Add UsageType qualifier with value Trial
			{
			stream << ",{\"Name\":\"UsageType\",\"Prompt\":null,\"Type\":\"string\",\"Value\":\"Trial\"}";
			}
		stream << "]}]"; // close qualifier list, close acl, close acl list
		return stream.str();
		};

	static std::string SecurableData(int productId, Utf8String featureString)
		{
		std::ostringstream stream;
		stream << "\"SecurableData\":[{\"FeatureString\":\"";
		stream << featureString.c_str();
		stream << "\",\"ProductId\":";
		stream << productId;
		stream << ",\"ProductName\":\"Bentley Navigator\",\"QualifierOverrides\":null,\"SecurableId\":\"844c10c2-375d-4153-a08f-896d2e64a13f\",\"Version\":null}]";
		return stream.str();
		};

	static std::string UserData(Utf8String userId)
		{
		std::ostringstream stream;
		stream << "\"UserData\":{\"EntitlementGroupId\":\"00000000-0000-0000-0000-000000000000\",\"OrganizationId\":\"a0dc5a35-7a81-47a5-9380-5f54bf040c43\",\"UltimateCountryId\":\"a427cdbc-56f8-421a-829a-20dccc806660\",\"UltimateId\":\"79c5c2fd-5a32-425f-af1a-c43703ca66be\",\"UltimateSAPId\":\"1004175881\",\"UsageCountryISO\":\"US\",\"UserId\":\"";
		stream << userId.c_str();
		stream << "\"}";
		return stream.str();
		};

	static std::string DefaultQualifiers()
		{
		return "\"DefaultQualifiers\":[{\"Name\":\"FrequencyToSend\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"24\"},{\"Name\":\"UseAlertingService\",\"Prompt\":\"You have reached a license threshold defined by your organization's administrator. Continuing may result in additional application usage charges.\\r\\nTo proceed, you must \\\"Acknowledge\\\".\",\"Type\":\"enum\",\"Value\":\"Disabled\"},{\"Name\":\"IsCheckedOut\",\"Prompt\":null,\"Type\":\"bool\",\"Value\":\"false\"},{\"Name\":\"AllowOfflineUsage\",\"Prompt\":\"You must Sign In to use this application.\",\"Type\":\"bool\",\"Value\":\"TRUE\"},{\"Name\":\"HotpathRetryAttempts\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"4\"},{\"Name\":\"MaxLogFileDurationInMinutes\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"360\"},{\"Name\":\"OfflineDuration\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"7\"},{\"Name\":\"TimeToKeepUnSentLogs\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"60\"},{\"Name\":\"UsageType\",\"Prompt\":null,\"Type\":\"string\",\"Value\":\"Production\"},{\"Name\":\"LogRecordsToSendFT2\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"75000\"},{\"Name\":\"PolicyInterval\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"60\"},{\"Name\":\"LoginGracePeriod\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"7\"},{\"Name\":\"HeartbeatInterval\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"1\"}]";
		};

	static Utf8String GetRandomPolicyId()
		{
		// set up random number generator
		const std::string alphanum("abcdefghijklmnopqrstuvwxyz0123456789");
		std::random_device r;
		std::default_random_engine e(r());
		std::uniform_int_distribution<int> u(0, int(alphanum.size()-1));
		// generate string in format "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
		std::string idstring("");
		for (int i = 0; i < 8; i++) // create "xxxxxxxx-" portion
			{
			idstring += alphanum[u(e)];
			}
		idstring += "-";
		for (int j = 0; j < 3; j++) // create 3 "xxxx-" portions
			{
			for (int i = 0; i < 4; i++)
				{
					idstring += alphanum[u(e)];
				}
			idstring += "-";
			}
		for (int i = 0; i < 12; i++) // create "xxxxxxxxxxxx" portion
			{
			idstring += alphanum[u(e)];
			}
		return Utf8String(idstring.c_str());
		};

	static Json::Value CreatePolicySpecific(PolicyType type, time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
		{
		bool isOnlineUsageAllowed = type != PolicyType::OfflineNotAllowed;
		// Create string
		std::ostringstream stream;
		stream << PolicyStart();
		stream << PolicyId(GetRandomPolicyId()) << PN();
		stream << PolicyVersion() << PN();
		stream << PolicyCreatedOn(DateHelper::TimeToString(createdOn).c_str()) << PN();
		stream << PolicyExpiresOn(DateHelper::TimeToString(expiresOn).c_str()) << PN();
		if (type != PolicyType::NoRequestData) stream << RequestData(productId, featureString, userId) << PN();
		stream << MachineSignature() << PN();
		stream << AppliesToUserId(userId) << PN();
		stream << AppliesToSecurableIds() << PN();
		if (type != PolicyType::NoACLs) stream << ACLs(DateHelper::TimeToString(aclExpiresOn).c_str(), accessKind, userId, isTrial, isOnlineUsageAllowed) << PN();
		if (type != PolicyType::NoSecurables) stream << SecurableData(productId, featureString) << PN();
		if (type != PolicyType::NoUserData) stream << UserData(userId) << PN();
		stream << DefaultQualifiers(); // no PN needed here; last subitem
		stream << PolicyEnd();
		// Return json created from string
		auto json = Json::Reader::DoParse(Utf8String(stream.str().c_str()));
		return json;
		}

public:
	static Json::Value CreatePolicyFull(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
	{
		return CreatePolicySpecific(PolicyType::Full, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
	};

	static Json::Value CreatePolicyNoSecurables(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
	{
		return CreatePolicySpecific(PolicyType::NoSecurables, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
	};

	static Json::Value CreatePolicyNoACLs(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
	{
		return CreatePolicySpecific(PolicyType::NoACLs, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
	};

	static Json::Value CreatePolicyNoUserData(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
	{
		return CreatePolicySpecific(PolicyType::NoUserData, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
	};

	static Json::Value CreatePolicyNoRequestData(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
	{
		return CreatePolicySpecific(PolicyType::NoRequestData, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
	};

	static Json::Value CreatePolicyOfflineNotAllowed(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
	{
		return CreatePolicySpecific(PolicyType::OfflineNotAllowed, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
	};

	static Json::Value CreatePolicyMissingFields()
	{
		std::ostringstream stream;
		stream << PolicyStart();
		stream << PolicyId(GetRandomPolicyId());
		stream << PolicyEnd();
		auto json = Json::Reader::DoParse(Utf8String(stream.str().c_str()));
		return json;
	};
};

END_BENTLEY_LICENSING_NAMESPACE
