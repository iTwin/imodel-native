/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/PolicyToken.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <Licensing/Utils/JWToken.h>

// AppliesTo Url
#if defined(DEBUG)
#define GETPOLICY_RequestData_AppliesTo_Url         "https://qa-entitlement-search.bentley.com/"
#else
#define GETPOLICY_RequestData_AppliesTo_Url         "https://entitlement-search.bentley.com/"
#endif // DEBUG)

// Policy Claim Url
#define GETCLAIM_JwtokenClaim_Url                   "http://schemas.bentley.com/ws/2011/03/identity/claims/policy"

// Policy Data Parameters
#define POLICYDATA_PolicyId                         "PolicyId"
#define POLICYDATA_ExpirationDate                   "PolicyExpiresOn"
#define POLICYDATA_RequestData                      "RequestData"
#define POLICTDATA_RequestData_ClientDateTime       "ClientDateTime"
#define POLICYDATA_RequestData_MachineName          "MachineName"
#define POLICYDATA_PolicyVersion                    "PolicyVersion"
#define POLICYDATA_DefaultQualifiers                "DefaultQualifiers"
#define POLICYDATA_DefaultQualifiers_UsageType      "UsageType"
#define POLICYDATA_UserData                         "UserData"
#define POLICYDATA_UserData_UltimateId              "UltimateId"
#define POLICYDATA_UserData_UsageCountryISO         "UsageCountryISO"
#define POLICYDATA_UserData_UserId                  "UserId"
#define POLICYDATA_ACLS                             "ACLs"
#define POLICYDATA_ACLS_PrincipalId                 "PrincipalId"
#define POLICYDATA_ACLS_SecurableId                 "SecurableId"

// JSON Parameters
#define JSONPARAMATER_Name                          "Name"
#define JSONPARAMATER_Value                         "Value"

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PolicyToken
{
private:
    Json::Value m_policy;
    PolicyToken(JsonValueCR policy);

public:
    LICENSING_EXPORT static std::shared_ptr<PolicyToken> Create(std::shared_ptr<JWToken> jwToken);
    LICENSING_EXPORT static std::shared_ptr<PolicyToken> PolicyToken::Create(Json::Value policy);

    LICENSING_EXPORT JsonValueCR GetDefaultQualifier(Utf8StringCR qualifierName) const;
    LICENSING_EXPORT Json::Value GetPolicyFile() const;
    LICENSING_EXPORT Utf8String GetPolicyId() const;
    LICENSING_EXPORT Utf8String GetExpirationDate() const;
    LICENSING_EXPORT Utf8String GetLastUpdateTime() const;
    LICENSING_EXPORT int64_t GetUltimateSAPId() const;
    LICENSING_EXPORT Utf8String GetPrincipalId() const;
    LICENSING_EXPORT Utf8String GetSecurableId() const;
    LICENSING_EXPORT Utf8String GetCountry() const;
    LICENSING_EXPORT Utf8String GetMachineName() const;
    LICENSING_EXPORT Utf8String GetUsageType() const;
    LICENSING_EXPORT Utf8String GetUserId() const;
};

END_BENTLEY_LICENSING_NAMESPACE
