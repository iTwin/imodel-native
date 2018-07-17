/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/PolicyToken.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PolicyToken.h"

#include <folly/BeFolly.h>
#include <folly/futures/Future.h>

#include <WebServices/Configuration/UrlProvider.h>

#include <Bentley/Base64Utilities.h>

#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PolicyToken::PolicyToken(JsonValueCR policy) :
    m_policy(policy)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<PolicyToken> PolicyToken::Create(std::shared_ptr<JWToken> jwToken)
    {
    if (jwToken == nullptr)
        return nullptr;

    Utf8String policyB64 = jwToken->GetClaim(GETCLAIM_JwtokenClaim_Url);
    if (policyB64.empty())
        return nullptr;

    Utf8String policy = Base64Utilities::Decode(policyB64);
    Json::Value policyJson;
    if (!Json::Reader::Parse(policy, policyJson))
        return nullptr;

    return std::shared_ptr<PolicyToken>(new PolicyToken(policyJson));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueCR PolicyToken::GetDefaultQualifier(Utf8StringCR qualifierName) const
    {
    auto& qualifiers = m_policy[POLICYDATA_DefaultQualifiers];

    if (!qualifiers.isArray())
        return Json::Value::GetNull();

    for (auto& qualifier : qualifiers)
        {
        if (qualifier[JSONPARAMATER_Name].asString().Equals(qualifierName.c_str()))
            return qualifier;
        }

    return Json::Value::GetNull();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PolicyToken::GetPolicyId() const
    {
    return m_policy[POLICYDATA_PolicyId].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PolicyToken::GetExpirationDate() const
    {
    return m_policy[POLICYDATA_ExpirationDate].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PolicyToken::GetLastUpdateTime() const
    {
    return m_policy[POLICYDATA_RequestData][POLICTDATA_RequestData_ClientDateTime].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t PolicyToken::GetUltimateSAPId() const
    {
    return m_policy[POLICYDATA_UserData][POLICYDATA_UserData_UltimateId].asInt64();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PolicyToken::GetPrincipalId() const
    {
    return m_policy[POLICYDATA_ACLS][0][POLICYDATA_ACLS_PrincipalId].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PolicyToken::GetSecurableId() const
    {
    return m_policy[POLICYDATA_ACLS][0][POLICYDATA_ACLS_SecurableId].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PolicyToken::GetCountry() const
    {
    return m_policy[POLICYDATA_UserData][POLICYDATA_UserData_UsageCountryISO].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PolicyToken::GetMachineName() const
    {
    return m_policy[POLICYDATA_RequestData][POLICYDATA_RequestData_MachineName].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PolicyToken::GetUsageType() const
    {
    auto& usageType = GetDefaultQualifier(POLICYDATA_DefaultQualifiers_UsageType);
    return usageType[JSONPARAMATER_Value].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PolicyToken::GetUserId() const
    {
    return m_policy[POLICYDATA_UserData][POLICYDATA_UserData_UserId].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PolicyToken::GetPolicyFile() const
    {
    return m_policy;
    }
