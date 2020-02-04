/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "Policy.h"

USING_NAMESPACE_BENTLEY_LICENSING

// Policy Methods
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Policy::Policy(const Json::Value& json)
    {
    m_json = json;
    m_PolicyId = json["PolicyId"].asString();
    m_PolicyVersion = json["PolicyVersion"].asDouble();
    m_PolicyCreatedOn = json["PolicyCreatedOn"].asString();
    m_PolicyExpiresOn = json["PolicyExpiresOn"].asString();
    m_RequestData = RequestData::Create(json["RequestData"]);
    m_MachineSignature = json["MachineSignature"].asString();
    m_AppliesToUserId = json["AppliesToUserId"].asString();
    m_AppliesToSecurableIds = CreateAppliesToSecurableIds(json["AppliesToSecurableIds"]);
    m_ACLs = CreateACLs(json["ACLs"]);
    m_SecurableData = CreateSecurableData(json["SecurableData"]);
    m_UserData = UserData::Create(json["UserData"]);
    m_DefaultQualifiers = CreateDefaultQualifiers(json["DefaultQualifiers"]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy> Policy::Create(std::shared_ptr<JWToken> jwToken)
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

    return std::shared_ptr<Policy>(new Policy(policyJson));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy> Policy::Create(const Json::Value& json)
    {
    if (json.isNull() || !json.isObject())
        return nullptr;
    return std::shared_ptr<Policy>(new Policy(json));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<Utf8String> Policy::CreateAppliesToSecurableIds(const Json::Value& json)
    {
    std::list<Utf8String> items;

    if (!json.isArray())
        return items;

    for (auto& jsonitem : json)
        {
        items.push_back(jsonitem.asString());
        }

    return items;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy::ACL>> Policy::CreateACLs(const Json::Value& json)
    {
    std::list<std::shared_ptr<Policy::ACL>> items;

    if (!json.isArray())
        return items;

    for (auto& jsonitem : json)
        {
        items.push_back(Policy::ACL::Create(jsonitem));
        }

    return items;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy::SecurableData>> Policy::CreateSecurableData(const Json::Value& json)
    {
    std::list<std::shared_ptr<Policy::SecurableData>> items;

    if (!json.isArray())
        return items;

    for (auto& jsonitem : json)
        {
        items.push_back(Policy::SecurableData::Create(jsonitem));
        }

    return items;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy::Qualifier>> Policy::CreateDefaultQualifiers(const Json::Value& json)
    {
    std::list<std::shared_ptr<Policy::Qualifier>> items;

    if (!json.isArray())
        return items;

    for (auto& jsonitem : json)
        {
        items.push_back(Policy::Qualifier::Create(jsonitem));
        }

    return items;
    }

// Qualifier Methods

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Policy::Qualifier::Qualifier(Json::Value json)
    {
    m_Name = json["Name"].asString();
    m_Value = json["Value"].asString();
    m_Type = json["Type"].asString();
    m_Prompt = json["Prompt"].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy::Qualifier> Policy::Qualifier::Create(const Json::Value& json)
    {
    return std::shared_ptr<Policy::Qualifier>(new Policy::Qualifier(json));
    }

// RequestedSecurable Methods
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Policy::RequestedSecurable::RequestedSecurable(Json::Value json)
    {
    m_ProductId = json["ProductId"].asInt64();
    m_FeatureString = json["FeatureString"].asString();
    m_Version = json["Vesrion"].asString(); // (mis)spelled in Policy file this way
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy::RequestedSecurable> Policy::RequestedSecurable::Create(const Json::Value& json)
    {
    return std::shared_ptr<Policy::RequestedSecurable>(new Policy::RequestedSecurable(json));
    }

// RequestedData Methods
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Policy::RequestData::RequestData(const Json::Value& json)
    {
    m_MachineSID = json["MachineSID"].asString();
    m_AccessKey = json["AccessKey"].asString();
    m_UserId = json["UserId"].asString();
    m_RequestedSecurables = CreateRequestedSecurables(json["RequestedSecurables"]);
    m_MachineName = json["MachineName"].asString();
    m_ClientDateTime = json["ClientDateTime"].asString();
    m_Locale = json["Locale"].asString();
    m_AppliesTo = json["AppliesTo"].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy::RequestData> Policy::RequestData::Create(const Json::Value& json)
    {
    return std::shared_ptr<Policy::RequestData>(new Policy::RequestData(json));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy::RequestedSecurable>> Policy::RequestData::CreateRequestedSecurables(const Json::Value& json)
    {
    std::list<std::shared_ptr<Policy::RequestedSecurable>> reqSecItems;

    if (!json.isArray())
        return reqSecItems;

    for (auto& reqSecJson : json)
        {
        reqSecItems.push_back(Policy::RequestedSecurable::Create(reqSecJson));
        }

    return reqSecItems;
    }

// ACL Methods
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Policy::ACL::ACL(const Json::Value& json)
    {
    m_PrincipalId = json["PrincipalId"].asString();
    m_SecurableId = json["SecurableId"].asString();
    m_AccessKind = (AccessKind) json["AccessKind"].asInt();
    m_ExpiresOn = json["ExpiresOn"].asString();
    m_QualifierOverrides = CreateQualifierOverrides(json["QualifierOverrides"]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy::ACL> Policy::ACL::Create(const Json::Value& json)
    {
    return std::shared_ptr<Policy::ACL>(new Policy::ACL(json));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy::Qualifier>> Policy::ACL::CreateQualifierOverrides(const Json::Value& json)
    {
    std::list<std::shared_ptr<Policy::Qualifier>> reqSecItems;

    if (!json.isArray())
        return reqSecItems;

    for (auto& reqSecJson : json)
        {
        reqSecItems.push_back(Policy::Qualifier::Create(reqSecJson));
        }

    return reqSecItems;
    }

// SecurableData Methods
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Policy::SecurableData::SecurableData(const Json::Value& json)
    {
    m_SecurableId = json["SecurableId"].asString();
    m_ProductId = json["ProductId"].asInt64();
    m_FeatureString = json["FeatureString"].asString();
    m_Version = json["Version"].asString();
    m_QualifierOverrides = CreateQualifierOverrides(json["QualifierOverrides"]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy::SecurableData> Policy::SecurableData::Create(const Json::Value& json)
    {
    return std::shared_ptr<Policy::SecurableData>(new Policy::SecurableData(json));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy::Qualifier>> Policy::SecurableData::CreateQualifierOverrides(const Json::Value& json)
    {
    std::list<std::shared_ptr<Policy::Qualifier>> reqSecItems;

    if (!json.isArray())
        return reqSecItems;

    for (auto& reqSecJson : json)
        {
        reqSecItems.push_back(Policy::Qualifier::Create(reqSecJson));
        }

    return reqSecItems;
    }

// UserData Methods
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Policy::UserData::UserData(const Json::Value& json)
    {
    m_UserId = json["UserId"].asString();
    m_OrganizationId = json["OrganizationId"].asString();
    m_UsageCountryISO = json["UsageCountryISO"].asString();
    m_UltimateSAPId = json["UltimateSAPId"].asInt64();
    m_UltimadeId = json["UltimateId"].asString();
    m_UltimateCountryId = json["UltimateCountryId"].asString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy::UserData> Policy::UserData::Create(const Json::Value& json)
    {
    return std::shared_ptr<Policy::UserData>(new Policy::UserData(json));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Policy::IsTimeExpired(Utf8StringCR expirationTime)
    {
    int64_t timeLeft = DateHelper::diffdate(expirationTime, DateHelper::GetCurrentTime());
    return timeLeft <= 0;
    }

bool Policy::IsPastThreeWeeksExpired(Utf8StringCR expirationTime)
    {
    int64_t timeLeft = DateHelper::diffdatedays(expirationTime, DateHelper::GetCurrentTime());
    return timeLeft <= -21;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy::Qualifier> Policy::GetFirstMatchingQualifier(std::list<std::shared_ptr<Policy::Qualifier>> qualifierList, Utf8StringCR qualifierName)
    {
    std::shared_ptr<Policy::Qualifier> matchingQualifier = nullptr;
    for (auto qualifier : qualifierList)
        {
        if (qualifier->GetName() == qualifierName)
            {
            matchingQualifier = qualifier;
            break;
            }
        }
    return matchingQualifier;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::list<std::shared_ptr<Policy::ACL>> Policy::GetMatchingACLs(std::list<std::shared_ptr<Policy::ACL>> aclList, Utf8StringCR securableId)
    {
    std::list<std::shared_ptr<Policy::ACL>> matchingACLs;
    for (auto acl : aclList)
        {
        if (acl->GetSecurableId() == securableId)
            {
            matchingACLs.push_back(acl);
            }
        }
    return matchingACLs;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy::SecurableData> Policy::GetFirstMatchingSecurableData(std::list<std::shared_ptr<Policy::SecurableData>> securableList, Utf8StringCR productId, Utf8StringCR featureString)
    {
    std::shared_ptr<Policy::SecurableData> matchingSecurable = nullptr;
    for (auto securable : securableList)
        {
        if (std::to_string(securable->GetProductId()).c_str() == productId &&
            securable->GetFeatureString() == featureString)
            {
            matchingSecurable = securable;
            break;
            }
        }
    return matchingSecurable;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Policy::IsValid()
    {
    // check if policy is not empty
    if (GetJson().isNull())
        return false;
    // check if policy is expired
    if (IsExpired() && IsThreeWeeksPastExpired()) //Wait to delete expired policies 
        return false;
    // check if RequestData is present
    if (GetRequestData()->GetUserId().Equals(""))
        return false;
    // check if UserData is present
    if (GetUserData()->GetUserId().Equals(""))
        return false;
    // Todo: generate and check machine signature
    // check if ACLs are valid (Ids)
    for (auto acl : GetACLs())
        {
        if (Utf8String::IsNullOrEmpty(acl->GetPrincipalId().c_str()) ||
            (
                !acl->GetPrincipalId().Equals(GetUserData()->GetUserId()) &&
                !acl->GetPrincipalId().Equals(GetUserData()->GetOrganizationId()) &&
                !acl->GetPrincipalId().Equals(GetUserData()->GetUltimateId()) &&
                !acl->GetPrincipalId().Equals(GetUserData()->GetUltimateCountryId())
                ))
            // do nothing for now
            continue;
        }
    return true;
    }

bool Policy::ContainsProduct(Utf8StringCR productId, Utf8StringCR featureString)
	{
	for (auto securable : GetSecurableData())
		{
		if (Utf8String(std::to_string(securable->GetProductId()).c_str()).Equals(productId) &&
			securable->GetFeatureString().Equals(featureString))
			{
			// we found the correct securable
			return true;
			}
		}
	return false;
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy::Qualifier> Policy::GetQualifier(Utf8StringCR qualifierName, Utf8StringCR productId, Utf8StringCR featureString)
    {
    std::shared_ptr<Policy::Qualifier> matchingQualifier = nullptr;
    // check if policy is not empty
    if (GetJson().isNull())
        return matchingQualifier;
    // check if product is in securables
    std::shared_ptr<Policy::SecurableData> securableData = GetFirstMatchingSecurableData(GetSecurableData(), productId, featureString);

    if (securableData != nullptr)
        {
        // try to find acl with SecurableId of securableData
        auto acls = GetACLs();
        auto matchingAcls = GetMatchingACLs(acls, securableData->GetSecurableId());
        // if acl exists, look for matching qualifier
        if (matchingAcls.size() > 0)
            {
            // try to find qualifier with name
            for (auto acl : matchingAcls)
                {
                matchingQualifier = GetFirstMatchingQualifier(acl->GetQualifierOverrides(), qualifierName);
                if (matchingQualifier != nullptr)
                    break; // found a matching qualifier
                }
            }
        // if matching qualifier not found yet, search for it in securableData
        if (matchingQualifier == nullptr)
            {
            matchingQualifier = GetFirstMatchingQualifier(securableData->GetQualifierOverrides(), qualifierName);
            }
        }
    // if still no match, search for qualifier in DefaultQualifiers
    if (matchingQualifier == nullptr)
        {
        matchingQualifier = GetFirstMatchingQualifier(GetDefaultQualifiers(), qualifierName);
        }

    return matchingQualifier;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Policy::IsTrial(Utf8StringCR productId, Utf8StringCR featureString)
    {
    bool result = false;
    auto qualifier = GetQualifier("UsageType", productId, featureString);
    // check if UsageType is Trial
    if (qualifier != nullptr)
        {
        result = qualifier->GetValue() == "Trial";
        }
    return result;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Policy::EvaluationStatus Policy::GetEvalStatus(Utf8StringCR productId, Utf8StringCR featureString, BeVersionCR appVersion, std::shared_ptr<Policy::ACL>& nonEvalAcl, int64_t& daysLeft)
    {
    nonEvalAcl = nullptr;
    // NB: if we want to check for check for a policy's expiration date, we may need to return the expiration date as well as the status
    Policy::EvaluationStatus result = EvaluationStatus::NoneFound;

    auto securable = GetFirstMatchingSecurableData(GetSecurableData(), productId, featureString);
    if (securable == nullptr)
        return result;

    auto matchingAcls = GetMatchingACLs(GetACLs(), securable->GetSecurableId());
    if (matchingAcls.size() == 0)
        return result;

    bool isEval = false;
    BeVersion evalVersion = BeVersion();
    // at this point we can assume that there is a max of 2 matching ACLs
    for (auto const& acl : matchingAcls)
        {
        auto qualifierOverrides = acl->GetQualifierOverrides();
        for (auto const& qualifierOverride : qualifierOverrides)
            {
            // assuming that eval has at least: one QualifierOverride with Name:UsageType, Value:Evaluation and one with Name:Version
            if (qualifierOverride->GetName() == "UsageType" && qualifierOverride->GetValue() == "Evaluation")
                {
                isEval = true;
                }

            else if (qualifierOverride->GetName() == "Version")
                {
                evalVersion = BeVersion(qualifierOverride->GetValue().c_str());
                }
            }

        if (isEval)
            {
            // if the last two version numbers from the policy are zero, then we only care about the major and minor versions matching the application
            if (evalVersion.GetSub1() == 0 && evalVersion.GetSub2() == 0)
                {
                if (appVersion.GetMajor() == evalVersion.GetMajor() && appVersion.GetMinor() == evalVersion.GetMinor())
                    {
                    // only call it exipred if we haven't found a valid eval, and the version is good
                    if (IsTimeExpired(acl->GetExpiresOn()) && result != EvaluationStatus::Valid)
                        {
                        daysLeft = 0;
                        result = EvaluationStatus::Expired;
                        }
                    else
                        {
                        // valid eval found
                        daysLeft = DateHelper::GetDaysLeftUntilTime(acl->GetExpiresOn());
                        //set principal Id from ACL  
                        SetInUsePrincipalId(acl->GetPrincipalId());
                        result = EvaluationStatus::Valid;
                        }
                    }
                }
            else
                {
                // if the last two verison numbers aren't zero, make sure the whole version matches the application
                if (appVersion.CompareTo(evalVersion) == 0)
                    {
                    // only call it exipred if we haven't found a valid eval, and the version is good
                    if (IsTimeExpired(acl->GetExpiresOn()) && result != EvaluationStatus::Valid)
                        {
                        daysLeft = 0;
                        result = EvaluationStatus::Expired;
                        }
                    else
                        {
                        // valid eval found
                        daysLeft = DateHelper::GetDaysLeftUntilTime(acl->GetExpiresOn());
                        //set principal Id from ACL
                        SetInUsePrincipalId(acl->GetPrincipalId());
                        result = EvaluationStatus::Valid;
                        }
                    }
                }
            }
        else
            {
            nonEvalAcl = acl;  
            //set principal Id from ACL
            SetInUsePrincipalId(acl->GetPrincipalId());            
            }
        }

    return result;
    }

int64_t Policy::GetTrialDaysRemaining(Utf8StringCR productId, Utf8StringCR featureString, BeVersionCR version)
    {
    // get the time remaining in the evaluation or trial entitlements

    std::shared_ptr<Policy::ACL> nonEvalAcl;
    int64_t evalDaysLeft = -1;
    // get the eval status, and if it's not eval, use the second ACL (as of now we can assume only two ACLs max -> eval and regular, with matching securable and principal)
    auto evalStatus = GetEvalStatus(productId, featureString, version, nonEvalAcl, evalDaysLeft);
    if (evalStatus == Policy::EvaluationStatus::Valid)
        {
        if (evalDaysLeft == -1)
            {
            LOG.error("Product is evaluation, but no days left data was supplied.");
            }

        return evalDaysLeft;
        }
    else if (evalStatus == Policy::EvaluationStatus::Expired)
        {
        LOG.info("Product is evaluation, but is expired");

        return 0;
        }

    if (IsTrial(productId, featureString))
        {
        // if trial is expired, return 0, otherwise return days left
        if (IsTimeExpired(nonEvalAcl->GetExpiresOn()))
            return 0;

        return DateHelper::GetDaysLeftUntilTime(nonEvalAcl->GetExpiresOn());
        }

    LOG.info("Policy is not a trial or evaluation policy.");
    return -1;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Policy::IsAllowedOfflineUsage(Utf8StringCR productId, Utf8StringCR featureString)
    {
    bool result = false;
    auto qualifier = GetQualifier("AllowOfflineUsage", productId, featureString);
    // check if AllowOfflineUsage is TRUE
    if (qualifier != nullptr)
        {
        result = qualifier->GetValue() == "TRUE";
        }
    return result;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int Policy::GetOfflineDuration(Utf8StringCR productId, Utf8StringCR featureString)
    {
    int result = 0;
    auto qualifier = GetQualifier("OfflineDuration", productId, featureString);
    // get days of offline usage allowed
    if (qualifier != nullptr)
        {
        result = std::stoi(qualifier->GetValue().c_str());
        }
    return result;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int Policy::GetHeartbeatInterval(Utf8StringCR productId, Utf8StringCR featureString)
    {
    int result_in_ms = 0;
    auto heartbeatInterval = GetQualifier("HeartbeatInterval", productId, featureString);

    if (heartbeatInterval != nullptr)
        {
        result_in_ms = std::stoi(heartbeatInterval->GetValue().c_str()) * 60 * 1000;
        }

    return result_in_ms;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int Policy::GetPolicyInterval(Utf8StringCR productId, Utf8StringCR featureString)
    {
    int result_in_ms = 0;
    auto policyInterval = GetQualifier("PolicyInterval", productId, featureString);

    if (policyInterval != nullptr)
        {
        result_in_ms = std::stoi(policyInterval->GetValue().c_str()) * 60 * 1000;
        }

    return result_in_ms;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int Policy::GetTimeToKeepUnSentLogs(Utf8StringCR productId, Utf8StringCR featureString)
    {
    int result_in_ms = 0;
    auto sendLogsInterval = GetQualifier("TimeToKeepUnSentLogs", productId, featureString);

    if (sendLogsInterval != nullptr)
        {
        result_in_ms = std::stoi(sendLogsInterval->GetValue().c_str()) * 60 * 1000;
        }

    return result_in_ms;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Policy::GetUsageType()
    {
    Utf8String result = "";
    auto qualifier = GetFirstMatchingQualifier(GetDefaultQualifiers(), "UsageType");
    if (qualifier != nullptr)
        {
        result = qualifier->GetValue();
        }
    return result;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Licensing::LicenseStatus Policy::GetProductStatus(Utf8StringCR productId, Utf8StringCR featureString, BeVersionCR version)
    {
    if (GetJson().isNull())
        {
        return Licensing::LicenseStatus::NotEntitled;
        }
    auto securable = GetFirstMatchingSecurableData(GetSecurableData(), productId, featureString);
    if (securable == nullptr)
        {
        return Licensing::LicenseStatus::NotEntitled;
        }

    std::shared_ptr<Policy::ACL> nonEvalAcl;
    int64_t evalDaysLeft = -1;
    // get the eval status, and if it's not eval, use the second ACL (as of now we can assume only two ACLs max -> eval and regular, with matching securable and principal)
    auto evalStatus = GetEvalStatus(productId, featureString, version, nonEvalAcl, evalDaysLeft);
    if (evalStatus == Policy::EvaluationStatus::Valid)
        {
        // offline check?
        // return OK and check offline in ClientImpl
        return Licensing::LicenseStatus::Ok;
        }

    if (nonEvalAcl == nullptr)
        {
        return Licensing::LicenseStatus::NotEntitled;
        }
    if (nonEvalAcl->GetAccessKind() == Policy::ACL::AccessKind::Denied)
        {
        return Licensing::LicenseStatus::AccessDenied;
        }
    if (nonEvalAcl->GetAccessKind() == Policy::ACL::AccessKind::TrialExpired)
        {
        return Licensing::LicenseStatus::Expired;
        }
    if (IsTrial(productId, featureString))
        {
        if (IsTimeExpired(nonEvalAcl->GetExpiresOn()))
            {
            return Licensing::LicenseStatus::Expired;
            }
        return Licensing::LicenseStatus::Trial;
        }
    // offline check?
    // return OK and check offline in ClientImpl
    return Licensing::LicenseStatus::Ok;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Policy::PolicyStatus Policy::GetPolicyStatus()
    {
    if (IsExpired())
        {
        return Policy::PolicyStatus::Expired;
        }
    if (!IsValid())
        {
        return Policy::PolicyStatus::Invalid;
        }
    return Policy::PolicyStatus::Valid;
    }
