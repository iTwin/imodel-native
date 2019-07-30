/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/NonPublished/DummyPolicyHelper.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DummyPolicyHelper.h"

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::PolicyStart()
    { 
    return "{"; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::PolicyEnd()
    { 
    return "}"; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::PolicyNext()
    { 
    return ","; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::PN()
    { 
    return PolicyNext(); 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::PolicyId(Utf8StringCR id)
        {
		Utf8String string;
		string += "\"PolicyId\":\"";
		string += id;
		string += "\"";
        return string;
        };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::PolicyVersion()
    { 
    return "\"PolicyVersion\":1.0"; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::PolicyCreatedOn(Utf8StringCR date)
    {
	Utf8String string;
	string += "\"PolicyCreatedOn\":\"";
	string += date;
	string += "\"";
    return string;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::PolicyExpiresOn(Utf8StringCR date)
    {
	Utf8String string;
	string += "\"PolicyExpiresOn\":\"";
	string += date;
	string += "\"";
    return string;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::RequestData(int productId, Utf8StringCR featureString, Utf8StringCR userId, Utf8StringCR accessKey)
    {
	Utf8String string;
    string += "\"RequestData\":{\"AccessKey\":";
    accessKey == "" ? string += "null" : string += "\"" + accessKey + "\"";
    string += ", \"AppliesTo\":\"https://entitlement-search.bentley.com/\",\"CheckedOutDate\":null,\"ClientDateTime\":\"2018-07-25T21:13:42.048Z\",\"Locale\":\"en\",\"MachineName\":\"TestDeviceId\",\"MachineSID\":null,\"RequestedSecurables\":[{\"FeatureString\":\"";
	string += featureString;
	string += "\",\"ProductId\":";
	Utf8String productIdString;
	productIdString.Sprintf("%d", productId);
	string += productIdString;
	string += ",\"Vesrion\":null}],\"UserId\":\"";
	string += userId;
	string += "\"}";
    return string;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::MachineSignature()
    { 
    return "\"MachineSignature\":\"u4o/9wtBEjZ6lfjlo8Aj+R4aaHk=\""; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::AppliesToUserId(Utf8StringCR userId)
    {
	Utf8String string;
	string += "\"AppliesToUserId\":\"";
	string += userId;
	string += "\"";
    return string;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::AppliesToSecurableIds()
    { 
    return "\"AppliesToSecurableIds\":[\"844c10c2-375d-4153-a08f-896d2e64a13f\"]"; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::ACLs(Utf8StringCR expiration, int accessKind, Utf8StringCR userId, bool isTrial, bool isOfflineUsageAllowed)
    {
	Utf8String string;
	string += "\"ACLs\":[{\"AccessKind\":";
	Utf8String accessKindString;
	accessKindString.Sprintf("%d", accessKind);
	string += accessKindString;
	string += ",\"ExpiresOn\":\"";
	string += expiration;
	string += "\",\"PrincipalId\":\"";
	string += userId;
	string += "\",\"SecurableId\":\"844c10c2-375d-4153-a08f-896d2e64a13f\"";
	string += ",\"QualifierOverrides\":[{\"Name\":\"UseAlertingService\",\"Prompt\":\"You have reached a license threshold defined by your organization's administrator. Continuing may result in additional application usage charges.\\r\\nTo proceed, you must \\\"Acknowledge\\\".\",\"Type\":\"enum\",\"Value\":\"ALERT\"},{\"Name\":\"AllowOfflineUsage\",\"Prompt\":\"You must Sign In to use this application.\",\"Type\":\"bool\",\"Value\":\"";
    if (isOfflineUsageAllowed)
        {
		string += "TRUE";
        }
    else
        {
		string += "FALSE";
        }
	string += "\"}";
    if (isTrial) // Add UsageType qualifier with value Trial
        {
		string += ",{\"Name\":\"UsageType\",\"Prompt\":null,\"Type\":\"string\",\"Value\":\"Trial\"}";
        }
	string += "]}]"; // close qualifier list, close acl, close acl list
    return string;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::ACLsWithQualifierOverrides(Utf8StringCR expiration, int accessKind, Utf8StringCR userId, bool isTrial, bool isOfflineUsageAllowed, bvector<QualifierOverride>& qualifierOverrides)
    {
	Utf8String string;
    string += "\"ACLs\":[{\"AccessKind\":";
	Utf8String accessKindString;
	accessKindString.Sprintf("%d", accessKind);
	string += accessKindString;
    string += ",\"ExpiresOn\":\"";
    string += expiration;
    string += "\",\"PrincipalId\":\"";
    string += userId;
    string += "\",\"SecurableId\":\"844c10c2-375d-4153-a08f-896d2e64a13f\"";
    string += ",\"QualifierOverrides\":[{\"Name\":\"UseAlertingService\",\"Prompt\":\"You have reached a license threshold defined by your organization's administrator. Continuing may result in additional application usage charges.\\r\\nTo proceed, you must \\\"Acknowledge\\\".\",\"Type\":\"enum\",\"Value\":\"ALERT\"},{\"Name\":\"AllowOfflineUsage\",\"Prompt\":\"You must Sign In to use this application.\",\"Type\":\"bool\",\"Value\":\"";
    if (isOfflineUsageAllowed)
        {
        string += "TRUE";
        }
    else
        {
        string += "FALSE";
        }
    string += "\"}";

    if (isTrial) // Add UsageType qualifier with value Trial
        {
        string += ",{\"Name\":\"UsageType\",\"Prompt\":null,\"Type\":\"string\",\"Value\":\"Trial\"}";
        }

    for (QualifierOverride qualifierOverride : qualifierOverrides)
        {
        string += ",{\"Name\":\"";
        string += qualifierOverride.qualifierName;
        string += "\",\"Prompt\":\"";
        string += qualifierOverride.qualifierPrompt;
        string += "\",\"Type\":\"";
        string += qualifierOverride.qualifierType;
        string += "\",\"Value\":\"";
        string += qualifierOverride.qualifierValue;
        string += "\"}";
        }

    string += "]}]"; // close qualifier list, close acl, close acl list
    return string;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::SecurableData(int productId, Utf8StringCR featureString)
    {
    Utf8String string;
    string += "\"SecurableData\":[{\"FeatureString\":\"";
    string += featureString;
    string += "\",\"ProductId\":";
	Utf8String productIdString;
	productIdString.Sprintf("%d", productId);
	string += productIdString;
    string += ",\"ProductName\":\"Bentley Navigator\",\"QualifierOverrides\":null,\"SecurableId\":\"844c10c2-375d-4153-a08f-896d2e64a13f\",\"Version\":null}]";
    return string;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::SecurableDataWithQualifierOverrides(int productId, Utf8StringCR featureString, bvector<QualifierOverride>& qualifierOverrides)
    {
    Utf8String string;
    string += "\"SecurableData\":[{\"FeatureString\":\"";
    string += featureString;
    string += "\",\"ProductId\":";
	Utf8String productIdString;
	productIdString.Sprintf("%d", productId);
	string += productIdString;
    string += ",\"ProductName\":\"Bentley Navigator\"";

    int qualifierCount = 1;
    string += ", \"QualifierOverrides\":[";
    for (QualifierOverride qualifierOverride : qualifierOverrides)
        {
        string += "{\"Name\":\"";
        string += qualifierOverride.qualifierName;
        string += "\",\"Prompt\":\"";
        string += qualifierOverride.qualifierPrompt;
        string += "\",\"Type\":\"";
        string += qualifierOverride.qualifierType;
        string += "\",\"Value\":\"";
        string += qualifierOverride.qualifierValue;
        if (qualifierCount == qualifierOverrides.size())
            string += "\"}";
        else
            string += "\"},";
        }
    string += "]";

    string += ",\"SecurableId\":\"844c10c2-375d-4153-a08f-896d2e64a13f\",\"Version\":null}]";
    return string;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::UserData(Utf8StringCR userId)
    {
    Utf8String string;
    string += "\"UserData\":{\"EntitlementGroupId\":\"00000000-0000-0000-0000-000000000000\",\"OrganizationId\":\"a0dc5a35-7a81-47a5-9380-5f54bf040c43\",\"UltimateCountryId\":\"a427cdbc-56f8-421a-829a-20dccc806660\",\"UltimateId\":\"79c5c2fd-5a32-425f-af1a-c43703ca66be\",\"UltimateSAPId\":\"1004175881\",\"UsageCountryISO\":\"US\",\"UserId\":\"";
    string += userId;
    string += "\"}";
    return string;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::DefaultQualifiers()
    {
    return "\"DefaultQualifiers\":[{\"Name\":\"FrequencyToSend\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"24\"},{\"Name\":\"UseAlertingService\",\"Prompt\":\"You have reached a license threshold defined by your organization's administrator. Continuing may result in additional application usage charges.\\r\\nTo proceed, you must \\\"Acknowledge\\\".\",\"Type\":\"enum\",\"Value\":\"Disabled\"},{\"Name\":\"IsCheckedOut\",\"Prompt\":null,\"Type\":\"bool\",\"Value\":\"false\"},{\"Name\":\"AllowOfflineUsage\",\"Prompt\":\"You must Sign In to use this application.\",\"Type\":\"bool\",\"Value\":\"TRUE\"},{\"Name\":\"HotpathRetryAttempts\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"4\"},{\"Name\":\"MaxLogFileDurationInMinutes\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"360\"},{\"Name\":\"OfflineDuration\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"7\"},{\"Name\":\"TimeToKeepUnSentLogs\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"60\"},{\"Name\":\"UsageType\",\"Prompt\":null,\"Type\":\"string\",\"Value\":\"Production\"},{\"Name\":\"LogRecordsToSendFT2\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"75000\"},{\"Name\":\"PolicyInterval\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"60\"},{\"Name\":\"LoginGracePeriod\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"7\"},{\"Name\":\"HeartbeatInterval\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"1\"}]";
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicySpecific(PolicyType type, Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial, Utf8StringCR accessKey)
    {
    bool isOnlineUsageAllowed = type != PolicyType::OfflineNotAllowed;
    // Create string
    Utf8String string;
    string += PolicyStart();
    string += PolicyId(BeSQLite::BeGuid(true).ToString()) += PN();
    string += PolicyVersion() += PN();
    string += PolicyCreatedOn(createdOn) += PN();
    string += PolicyExpiresOn(expiresOn) += PN();
    if (type != PolicyType::NoRequestData) string += RequestData(productId, featureString, userId, accessKey) += PN();
    string += MachineSignature() += PN();
    string += AppliesToUserId(userId) += PN();
    string += AppliesToSecurableIds() += PN();
    if (type != PolicyType::NoACLs) string += ACLs(aclExpiresOn, accessKind, userId, isTrial, isOnlineUsageAllowed) += PN();
    if (type != PolicyType::NoSecurables) string += SecurableData(productId, featureString) += PN();
    if (type != PolicyType::NoUserData) string += UserData(userId) += PN();
    string += DefaultQualifiers(); // no PN needed here; last subitem
    string += PolicyEnd();
    // Return json created from string
    auto json = Json::Reader::DoParse(string);
    return json;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyFull(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::Full, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial, "");
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyFullWithKey(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial, Utf8StringCR accessKey)
    {
    return CreatePolicySpecific(PolicyType::Full, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial, accessKey);
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyNoSecurables(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::NoSecurables, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial, "");
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyNoACLs(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::NoACLs, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial, "");
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyNoUserData(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::NoUserData, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial, "");
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyNoRequestData(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::NoRequestData, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial, "");
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyOfflineNotAllowed(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::OfflineNotAllowed, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial, "");
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyMissingFields()
    {
    Utf8String string;
    string += PolicyStart();
    string += PolicyId(BeSQLite::BeGuid(true).ToString());
    string += PolicyEnd();
    auto json = Json::Reader::DoParse(string);
    return json;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyQualifierOverrides(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString,
                                                               int accessKind, bool isTrial, Utf8StringCR accessKey, bvector<QualifierOverride>& aclsQualifierOverrides, bvector<QualifierOverride>& securableDataQualifierOverrides)
    {
    // Create string
    bool isOnlineUsageAllowed = true;
    Utf8String string;
    string += PolicyStart();
    string += PolicyId(BeSQLite::BeGuid(true).ToString()) += PN();
    string += PolicyVersion() += PN();
    string += PolicyCreatedOn(createdOn) += PN();
    string += PolicyExpiresOn(expiresOn) += PN();
    string += RequestData(productId, featureString, userId, accessKey) += PN();
    string += MachineSignature() += PN();
    string += AppliesToUserId(userId) += PN();
    string += AppliesToSecurableIds() += PN();

    if (aclsQualifierOverrides.size() == 0)
        {
        string += ACLs(aclExpiresOn, accessKind, userId, isTrial, isOnlineUsageAllowed) += PN();
        }
    else
        {
        string += ACLsWithQualifierOverrides(aclExpiresOn, accessKind, userId, isTrial, isOnlineUsageAllowed, aclsQualifierOverrides) += PN();
        }

    if (securableDataQualifierOverrides.size() == 0)
        {
        string += SecurableData(productId, featureString) += PN();
        }
    else
        {
        string += SecurableDataWithQualifierOverrides(productId, featureString, securableDataQualifierOverrides) += PN();
        }

    string += UserData(userId) += PN();
    string += DefaultQualifiers(); // no PN needed here; last subitem
    string += PolicyEnd();

    // Return json created from string
    auto json = Json::Reader::DoParse(string);
    return json;
    }
