/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/DummyPolicyHelper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DummyPolicyHelper.h"

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::PolicyStart()
    { 
    return "{"; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::PolicyEnd()
    { 
    return "}"; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::PolicyNext() 
    { 
    return ","; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::PN()
    { 
    return PolicyNext(); 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::PolicyId(Utf8String id)
        {
        std::ostringstream stream;
        stream << "\"PolicyId\":\"";
        stream << id.c_str();
        stream << "\"";
        return stream.str();
        };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::PolicyVersion()
    { 
    return "\"PolicyVersion\":1.0"; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::PolicyCreatedOn(Utf8String date)
    {
    std::ostringstream stream;
    stream << "\"PolicyCreatedOn\":\"";
    stream << date.c_str();
    stream << "\"";
    return stream.str();
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::PolicyExpiresOn(Utf8String date)
    {
    std::ostringstream stream;
    stream << "\"PolicyExpiresOn\":\"";
    stream << date.c_str();
    stream << "\"";
    return stream.str();
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::RequestData(int productId, Utf8String featureString, Utf8String userId)
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

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::MachineSignature()
    { 
    return "\"MachineSignature\":\"u4o/9wtBEjZ6lfjlo8Aj+R4aaHk=\""; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::AppliesToUserId(Utf8String userId)
    {
    std::ostringstream stream;
    stream << "\"AppliesToUserId\":\"";
    stream << userId.c_str();
    stream << "\"";
    return stream.str();
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::AppliesToSecurableIds()
    { 
    return "\"AppliesToSecurableIds\":[\"844c10c2-375d-4153-a08f-896d2e64a13f\"]"; 
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::ACLs(Utf8String expiration, int accessKind, Utf8String userId, bool isTrial, bool isOfflineUsageAllowed)
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

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::ACLsWithQualifierOverrides(Utf8String expiration, int accessKind, Utf8String userId, bool isTrial, bool isOfflineUsageAllowed, bvector<QualifierOverride>& qualifierOverrides)
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

    for (QualifierOverride qualifierOverride : qualifierOverrides)
        {
        stream << ",{\"Name\":\"";
        stream << qualifierOverride.qualifierName;
        stream << "\",\"Prompt\":\"";
        stream << qualifierOverride.qualifierPrompt;
        stream << "\",\"Type\":\"";
        stream << qualifierOverride.qualifierType;
        stream << "\",\"Value\":\"";
        stream << qualifierOverride.qualifierValue;
        stream << "\"}";
        }

    stream << "]}]"; // close qualifier list, close acl, close acl list
    return stream.str();
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::SecurableData(int productId, Utf8String featureString)
    {
    std::ostringstream stream;
    stream << "\"SecurableData\":[{\"FeatureString\":\"";
    stream << featureString.c_str();
    stream << "\",\"ProductId\":";
    stream << productId;
    stream << ",\"ProductName\":\"Bentley Navigator\",\"QualifierOverrides\":null,\"SecurableId\":\"844c10c2-375d-4153-a08f-896d2e64a13f\",\"Version\":null}]";
    return stream.str();
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::SecurableDataWithQualifierOverrides(int productId, Utf8String featureString, bvector<QualifierOverride>& qualifierOverrides)
    {
    std::ostringstream stream;
    stream << "\"SecurableData\":[{\"FeatureString\":\"";
    stream << featureString.c_str();
    stream << "\",\"ProductId\":";
    stream << productId;
    stream << ",\"ProductName\":\"Bentley Navigator\"";

    int qualifierCount = 1;
    stream << ", \"QualifierOverrides\":[";
    for (QualifierOverride qualifierOverride : qualifierOverrides)
        {
        stream << "{\"Name\":\"";
        stream << qualifierOverride.qualifierName;
        stream << "\",\"Prompt\":\"";
        stream << qualifierOverride.qualifierPrompt;
        stream << "\",\"Type\":\"";
        stream << qualifierOverride.qualifierType;
        stream << "\",\"Value\":\"";
        stream << qualifierOverride.qualifierValue;
        if (qualifierCount == qualifierOverrides.size())
            stream << "\"}";
        else
            stream << "\"},";
        }
    stream << "]";

    stream << ",\"SecurableId\":\"844c10c2-375d-4153-a08f-896d2e64a13f\",\"Version\":null}]";
    return stream.str();
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::UserData(Utf8String userId)
    {
    std::ostringstream stream;
    stream << "\"UserData\":{\"EntitlementGroupId\":\"00000000-0000-0000-0000-000000000000\",\"OrganizationId\":\"a0dc5a35-7a81-47a5-9380-5f54bf040c43\",\"UltimateCountryId\":\"a427cdbc-56f8-421a-829a-20dccc806660\",\"UltimateId\":\"79c5c2fd-5a32-425f-af1a-c43703ca66be\",\"UltimateSAPId\":\"1004175881\",\"UsageCountryISO\":\"US\",\"UserId\":\"";
    stream << userId.c_str();
    stream << "\"}";
    return stream.str();
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::string DummyPolicyHelper::DefaultQualifiers()
    {
    return "\"DefaultQualifiers\":[{\"Name\":\"FrequencyToSend\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"24\"},{\"Name\":\"UseAlertingService\",\"Prompt\":\"You have reached a license threshold defined by your organization's administrator. Continuing may result in additional application usage charges.\\r\\nTo proceed, you must \\\"Acknowledge\\\".\",\"Type\":\"enum\",\"Value\":\"Disabled\"},{\"Name\":\"IsCheckedOut\",\"Prompt\":null,\"Type\":\"bool\",\"Value\":\"false\"},{\"Name\":\"AllowOfflineUsage\",\"Prompt\":\"You must Sign In to use this application.\",\"Type\":\"bool\",\"Value\":\"TRUE\"},{\"Name\":\"HotpathRetryAttempts\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"4\"},{\"Name\":\"MaxLogFileDurationInMinutes\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"360\"},{\"Name\":\"OfflineDuration\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"7\"},{\"Name\":\"TimeToKeepUnSentLogs\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"60\"},{\"Name\":\"UsageType\",\"Prompt\":null,\"Type\":\"string\",\"Value\":\"Production\"},{\"Name\":\"LogRecordsToSendFT2\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"75000\"},{\"Name\":\"PolicyInterval\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"60\"},{\"Name\":\"LoginGracePeriod\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"7\"},{\"Name\":\"HeartbeatInterval\",\"Prompt\":null,\"Type\":\"int\",\"Value\":\"1\"}]";
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DummyPolicyHelper::GetRandomPolicyId()
    {
    // set up random number generator
    const std::string alphanum("abcdefghijklmnopqrstuvwxyz0123456789");
    std::random_device r;
    std::default_random_engine e(r());
    std::uniform_int_distribution<int> u(0, int(alphanum.size() - 1));
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

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicySpecific(PolicyType type, time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
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

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyFull(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::Full, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyNoSecurables(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::NoSecurables, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyNoACLs(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::NoACLs, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyNoUserData(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::NoUserData, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyNoRequestData(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::NoRequestData, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyOfflineNotAllowed(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial)
    {
    return CreatePolicySpecific(PolicyType::OfflineNotAllowed, createdOn, expiresOn, aclExpiresOn, userId, productId, featureString, accessKind, isTrial);
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyMissingFields()
    {
    std::ostringstream stream;
    stream << PolicyStart();
    stream << PolicyId(GetRandomPolicyId());
    stream << PolicyEnd();
    auto json = Json::Reader::DoParse(Utf8String(stream.str().c_str()));
    return json;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value DummyPolicyHelper::CreatePolicyQuailifierOverrides(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString,
                                                               int accessKind, bool isTrial, bvector<QualifierOverride>& aclsQualifierOverrides, bvector<QualifierOverride>& securableDataQualifierOverrides)
    {
    // Create string
    bool isOnlineUsageAllowed = true;
    std::ostringstream stream;
    stream << PolicyStart();
    stream << PolicyId(GetRandomPolicyId()) << PN();
    stream << PolicyVersion() << PN();
    stream << PolicyCreatedOn(DateHelper::TimeToString(createdOn).c_str()) << PN();
    stream << PolicyExpiresOn(DateHelper::TimeToString(expiresOn).c_str()) << PN();
    stream << RequestData(productId, featureString, userId) << PN();
    stream << MachineSignature() << PN();
    stream << AppliesToUserId(userId) << PN();
    stream << AppliesToSecurableIds() << PN();

    if (aclsQualifierOverrides.size() == 0)
        {
        stream << ACLs(DateHelper::TimeToString(aclExpiresOn).c_str(), accessKind, userId, isTrial, isOnlineUsageAllowed) << PN();
        }
    else
        {
        stream << ACLsWithQualifierOverrides(DateHelper::TimeToString(aclExpiresOn).c_str(), accessKind, userId, isTrial, isOnlineUsageAllowed, aclsQualifierOverrides) << PN();
        }

    if (securableDataQualifierOverrides.size() == 0)
        {
        stream << SecurableData(productId, featureString) << PN();
        }
    else
        {
        stream << SecurableDataWithQualifierOverrides(productId, featureString, securableDataQualifierOverrides) << PN();
        }

    stream << UserData(userId) << PN();
    stream << DefaultQualifiers(); // no PN needed here; last subitem
    stream << PolicyEnd();

    // Return json created from string
    auto json = Json::Reader::DoParse(Utf8String(stream.str().c_str()));
    return json;
    }
