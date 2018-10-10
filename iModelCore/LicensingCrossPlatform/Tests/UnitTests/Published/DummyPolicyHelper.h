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
        static std::string PolicyId(Utf8StringCR id);

        static std::string PolicyVersion();
        static std::string PolicyCreatedOn(Utf8StringCR date);

        static std::string PolicyExpiresOn(Utf8StringCR date);

        static std::string RequestData(int productId, Utf8StringCR featureString, Utf8StringCR userId);

        static std::string MachineSignature();
        static std::string AppliesToUserId(Utf8StringCR userId);

        static std::string AppliesToSecurableIds();
        static std::string ACLs(Utf8StringCR expiration, int accessKind, Utf8StringCR userId, bool isTrial, bool isOfflineUsageAllowed);
        static std::string ACLsWithQualifierOverrides(Utf8StringCR expiration, int accessKind, Utf8StringCR userId, bool isTrial, bool isOfflineUsageAllowed, bvector<QualifierOverride>& qualifierOverrides);

        static std::string SecurableData(int productId, Utf8StringCR featureString);
        static std::string SecurableDataWithQualifierOverrides(int productId, Utf8StringCR featureString, bvector<QualifierOverride>& qualifierOverrides);

        static std::string UserData(Utf8StringCR userId);

        static std::string DefaultQualifiers();

        static Utf8String GetRandomPolicyId();

        static Json::Value CreatePolicySpecific(PolicyType type, Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial);
            
    public:
        static Json::Value CreatePolicyFull(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyNoSecurables(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyNoACLs(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyNoUserData(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyNoRequestData(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyOfflineNotAllowed(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId, Utf8StringCR featureString, int accessKind, bool isTrial);

        static Json::Value CreatePolicyMissingFields();

        static Json::Value CreatePolicyQualifierOverrides(Utf8StringCR createdOn, Utf8StringCR expiresOn, Utf8StringCR aclExpiresOn, Utf8StringCR userId, int productId,
                                                           Utf8StringCR featureString, int accessKind, bool isTrial, bvector<QualifierOverride>& aclsQualifierOverrides,
                                                           bvector<QualifierOverride>& securableDataQualifierOverrides);
    };

END_BENTLEY_LICENSING_NAMESPACE
