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
