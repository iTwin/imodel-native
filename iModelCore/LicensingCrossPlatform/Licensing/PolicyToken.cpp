/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/PolicyToken.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PolicyToken.h"

#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_LICENSING

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

    Utf8String policyB64 = jwToken->GetClaim("http://schemas.bentley.com/ws/2011/03/identity/claims/policy");
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
JsonValueCR PolicyToken::GetQualifier(Utf8StringCR qualifierName) const
    {
    auto& qulifiers = m_policy["DefaultQualifiers"];
    if (!qulifiers.isArray())
        return Json::Value::GetNull();

    for (auto& qualifier : qulifiers)
        {
        if (qualifier["Name"].asString().Equals(qualifierName.c_str()))
            return qualifier;
        }

    return Json::Value::GetNull();
    }
