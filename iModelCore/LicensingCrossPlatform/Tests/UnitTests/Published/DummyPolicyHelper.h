/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/DummyPolicyHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/Utils/DateHelper.h>
#include <BeSQLite/BeSQLite.h>
#include <json/json.h>

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

        static Utf8String PolicyStart();
        static Utf8String PolicyEnd();
        static Utf8String PolicyNext();
        static Utf8String PN();
        static Utf8String PolicyId(Utf8StringCR id);

        static Utf8String PolicyVersion();
        static Utf8String PolicyCreatedOn(Utf8StringCR date);

        static Utf8String PolicyExpiresOn(Utf8StringCR date);

        static Utf8String RequestData(int productId, Utf8StringCR featureString, Utf8StringCR userId);

        static Utf8String MachineSignature();
        static Utf8String AppliesToUserId(Utf8StringCR userId);

        static Utf8String AppliesToSecurableIds();
        static Utf8String ACLs(Utf8StringCR expiration, int accessKind, Utf8StringCR userId, bool isTrial, bool isOfflineUsageAllowed);
        static Utf8String ACLsWithQualifierOverrides(Utf8StringCR expiration, int accessKind, Utf8StringCR userId, bool isTrial, bool isOfflineUsageAllowed, bvector<QualifierOverride>& qualifierOverrides);

        static Utf8String SecurableData(int productId, Utf8StringCR featureString);
        static Utf8String SecurableDataWithQualifierOverrides(int productId, Utf8StringCR featureString, bvector<QualifierOverride>& qualifierOverrides);

        static Utf8String UserData(Utf8StringCR userId);

        static Utf8String DefaultQualifiers();

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
