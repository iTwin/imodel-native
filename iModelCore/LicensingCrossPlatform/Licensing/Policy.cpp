/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Policy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
	m_PolicyCreatedOn = DateHelper::StringToTime(json["PolicyCreatedOn"].asCString());
	m_PolicyExpiresOn = DateHelper::StringToTime(json["PolicyExpiresOn"].asCString());
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
/*std::shared_ptr<Policy> Policy::Create(std::shared_ptr<PolicyToken> policyToken)
	{
	if (policyToken == nullptr)
		return nullptr;
	return std::shared_ptr<Policy>(new Policy(policyToken));
	}*/

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
	//if (json.isNull()) return nullptr;
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
	//if (json.isNull()) return nullptr;
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
	m_CheckedOutDate = DateHelper::StringToTime(json["CheckedOutDate"].asCString());
	m_RequestedSecurables = CreateRequestedSecurables(json["RequestedSecurables"]);
	m_MachineName = json["MachineName"].asString();
	m_ClientDateTime = DateHelper::StringToTime(json["ClientDateTime"].asCString());
	m_Locale = json["Locale"].asString();
	m_AppliesTo = json["AppliesTo"].asString();
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy::RequestData> Policy::RequestData::Create(const Json::Value& json)
	{
	//if (json.isNull()) return nullptr;
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
	m_AccessKind = (AccessKind)json["AccessKind"].asInt();
	m_ExpiresOn = DateHelper::StringToTime(json["ExpiresOn"].asCString());
	m_QualifierOverrides = CreateQualifierOverrides(json["QualifierOverrides"]);
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<Policy::ACL> Policy::ACL::Create(const Json::Value& json)
	{
	//if (json.isNull()) return nullptr;
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
	//if (json.isNull()) return nullptr;
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
	//if (json.isNull()) return nullptr;
	return std::shared_ptr<Policy::UserData>(new Policy::UserData(json));
	}

bool Policy::IsTimeExpired(const time_t& expirationTime)
	{
	double timeLeft = difftime(expirationTime, DateHelper::GetCurrentTime());
	return timeLeft <= 0;
	}

std::shared_ptr<Policy::Qualifier> Policy::GetFirstMatchingQualifier(std::list<std::shared_ptr<Policy::Qualifier>> qualifierList, Utf8String qualifierName)
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

std::shared_ptr<Policy::ACL> Policy::GetFirstMatchingACL(std::list<std::shared_ptr<Policy::ACL>> aclList, Utf8String securableId)
	{
	std::shared_ptr<Policy::ACL> matchingACL = nullptr;
	for (auto acl : aclList)
		{
		if (acl->GetSecurableId() == securableId)
			{
			matchingACL = acl;
			break;
			}
		}
	return matchingACL;
	}

std::shared_ptr<Policy::SecurableData> Policy::GetFirstMatchingSecurableData(std::list<std::shared_ptr<Policy::SecurableData>> securableList, Utf8String productId, Utf8String featureString)
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

bool Policy::IsValid()
	{
	// check if policy is not empty
	if (GetJson().isNull())
		return false;
	// check if policy is expired
	if (IsExpired())
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

std::shared_ptr<Policy::Qualifier> Policy::GetQualifier(Utf8String qualifierName, Utf8String productId, Utf8String featureString)
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
		std::shared_ptr<Policy::ACL> acl = nullptr;
		auto acls = GetACLs();
		acl = GetFirstMatchingACL(acls, securableData->GetSecurableId());
		// if acl exists, look for matching qualifier
		if (acl != nullptr)
			{
			// try to find qualifier with name
			matchingQualifier = GetFirstMatchingQualifier(acl->GetQualifierOverrides(), qualifierName);
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

bool Policy::IsTrial(Utf8String productId, Utf8String featureString)
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

bool Policy::IsAllowedOfflineUsage(Utf8String productId, Utf8String featureString)
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

int Policy::GetOfflineDuration(Utf8String productId, Utf8String featureString)
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

Policy::ProductStatus Policy::GetProductStatus(Utf8String productId, Utf8String featureString)
	{
	if (GetJson().isNull())
		{
		return Policy::ProductStatus::NoLicense;
		}
	auto securable = GetFirstMatchingSecurableData(GetSecurableData(), productId, featureString);
	if (securable == nullptr)
		{
		return Policy::ProductStatus::NoLicense;
		}
	auto acl = GetFirstMatchingACL(GetACLs(), securable->GetSecurableId());
	if (acl == nullptr)
		{
		return Policy::ProductStatus::NoLicense;
		}
	if (acl->GetAccessKind() == Policy::ACL::AccessKind::Denied)
		{
		return Policy::ProductStatus::Denied;
		}
	if (acl->GetAccessKind() == Policy::ACL::AccessKind::TrialExpired)
		{
		return Policy::ProductStatus::TrialExpired;
		}
	if (IsTrial(productId, featureString))
		{
		if (IsTimeExpired(acl->GetExpiresOn()))
			{
			return Policy::ProductStatus::TrialExpired;
			}
		}
	return Policy::ProductStatus::Allowed;
	}

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