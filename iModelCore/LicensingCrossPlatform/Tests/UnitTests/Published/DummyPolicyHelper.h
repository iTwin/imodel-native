/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/DummyPolicyHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <json/json.h>
#include <sstream>
#include <random>

#include "../../../Licensing/DateHelper.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct QualifierOverride
    {
    Utf8String qualifierName;
    Utf8String qualifierValue;
    Utf8String qualifierType;
    Utf8String qualifierPrompt;
    };

struct DummyPolicyHelper
    {
    private:
        enum class PolicyType { Full, NoSecurables, NoACLs, NoUserData, NoRequestData, OfflineNotAllowed };

        static std::string PolicyStart();
        static std::string PolicyEnd();
        static std::string PolicyNext();
        static std::string PN();
        static std::string PolicyId(Utf8String id);

        static std::string PolicyVersion();
        static std::string PolicyCreatedOn(Utf8String date);

        static std::string PolicyExpiresOn(Utf8String date);

        static std::string RequestData(int productId, Utf8String featureString, Utf8String userId);

        static std::string MachineSignature();
        static std::string AppliesToUserId(Utf8String userId);

        static std::string AppliesToSecurableIds();
        static std::string ACLs(Utf8String expiration, int accessKind, Utf8String userId, bool isTrial, bool isOfflineUsageAllowed);
        static std::string ACLsWithQualifierOverrides(Utf8String expiration, int accessKind, Utf8String userId, bool isTrial, bool isOfflineUsageAllowed, bvector<QualifierOverride>& qualifierOverrides);

        static std::string SecurableData(int productId, Utf8String featureString);
        static std::string SecurableDataWithQualifierOverrides(int productId, Utf8String featureString, bvector<QualifierOverride>& qualifierOverrides);

        static std::string UserData(Utf8String userId);

        static std::string DefaultQualifiers();

        static Utf8String GetRandomPolicyId();

        static Json::Value CreatePolicySpecific(PolicyType type, time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial);
            
    public:
        static Json::Value CreatePolicyFull(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyNoSecurables(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyNoACLs(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyNoUserData(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyNoRequestData(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyOfflineNotAllowed(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId, Utf8String featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyMissingFields();

        static Json::Value CreatePolicyQuailifierOverrides(time_t createdOn, time_t expiresOn, time_t aclExpiresOn, Utf8String userId, int productId,
                                                           Utf8String featureString, int accessKind, bool isTrial, bvector<QualifierOverride>& aclsQualifierOverrides,
                                                           bvector<QualifierOverride>& securableDataQualifierOverrides);
    };

END_BENTLEY_LICENSING_NAMESPACE
