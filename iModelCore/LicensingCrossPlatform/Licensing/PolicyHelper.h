/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/PolicyHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "Policy.h"
#include "DateHelper.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct PolicyHelper
{
public:
	// enumeration for GetProductStatus return value
	enum class ProductStatus
		{
		Allowed,
		Denied,
		NoLicense,
		TrialExpired
		};
	// enumeration for GetPolicyStatus return value
	enum class PolicyStatus
		{
		Error = -1,
		Valid = 0,
		Expired = 1,
		NotFound = 2,
		Invalid = 3
		};

private:
	static bool IsTimeExpired(const time_t& expirationTime)
		{
			double timeLeft = difftime(expirationTime, DateHelper::GetCurrentTime());
			return timeLeft <= 0;
		}

	static std::shared_ptr<Policy::Qualifier> GetFirstMatchingQualifier(std::list<std::shared_ptr<Policy::Qualifier>> qualifierList, Utf8String qualifierName)
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
		};

	static std::shared_ptr<Policy::ACL> GetFirstMatchingACL(std::list<std::shared_ptr<Policy::ACL>> aclList, Utf8String securableId)
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

	static std::shared_ptr<Policy::SecurableData> GetFirstMatchingSecurableData(std::list<std::shared_ptr<Policy::SecurableData>> securableList, Utf8String productId, Utf8String featureString)
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

public:
	static bool IsValid(std::shared_ptr<Policy> policy) {
		// check if policy is a nullptr
		if (policy == nullptr)
			return false;
		// check if policy is expired
		if (IsExpired(policy))
			return false;
		// check if RequestData is present
		if (policy->GetRequestData() == nullptr)
			return false;
		// check if UserData is present
		if (policy->GetUserData() == nullptr)
			return false;
		// Todo: generate and check machine signature
		// check if ACLs are valid (Ids)
		for (auto acl : policy->GetACLs())
			{
			if (Utf8String::IsNullOrEmpty(acl->GetPrincipalId().c_str()) ||
				(
					!acl->GetPrincipalId().Equals(policy->GetUserData()->GetUserId()) &&
					!acl->GetPrincipalId().Equals(policy->GetUserData()->GetOrganizationId()) &&
					!acl->GetPrincipalId().Equals(policy->GetUserData()->GetUltimateId()) &&
					!acl->GetPrincipalId().Equals(policy->GetUserData()->GetUltimateCountryId())
					))
				// do nothing for now
				continue;
			}
		return true;
	}

	static bool IsExpired(std::shared_ptr<Policy> policy)
		{
		return IsTimeExpired(policy->GetPolicyExpiresOn());
		};

	static std::shared_ptr<Policy::Qualifier> GetQualifier(std::shared_ptr<Policy> policy, Utf8String qualifierName, Utf8String productId, Utf8String featureString)
		{
		std::shared_ptr<Policy::Qualifier> matchingQualifier = nullptr;
		// check if policy is not nullptr
		if (policy == nullptr)
			return matchingQualifier;
		// check if product is in securables
		std::shared_ptr<Policy::SecurableData> securableData = GetFirstMatchingSecurableData(policy->GetSecurableData(), productId, featureString);

		if (securableData != nullptr)
			{
			// try to find acl with SecurableId of securableData
			std::shared_ptr<Policy::ACL> acl = nullptr;
			auto acls = policy->GetACLs();
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
			matchingQualifier = GetFirstMatchingQualifier(policy->GetDefaultQualifiers(), qualifierName);
		}

		return matchingQualifier;
		}

	static bool IsTrial(std::shared_ptr<Policy> policy, Utf8String productId, Utf8String featureString)
		{
		bool result = false;
		auto qualifier = GetQualifier(policy, "UsageType", productId, featureString);
		// check if UsageType is Trial
		if (qualifier != nullptr)
			{
			result = qualifier->GetValue() == "Trial";
			}
		return result;
		};

	static bool IsAllowedOfflineUsage(std::shared_ptr<Policy> policy, Utf8String productId, Utf8String featureString)
		{
		bool result = false;
		auto qualifier = GetQualifier(policy, "AllowOfflineUsage", productId, featureString);
		// check if AllowOfflineUsage is TRUE
		if (qualifier != nullptr)
		{
			result = qualifier->GetValue() == "TRUE";
		}
		return result;
		}

	static int GetOfflineDuration(std::shared_ptr<Policy> policy, Utf8String productId, Utf8String featureString)
		{
		int result = 0;
		auto qualifier = GetQualifier(policy, "OfflineDuration", productId, featureString);
		// get days of offline usage allowed
		if (qualifier != nullptr)
		{
			result = std::stoi(qualifier->GetValue().c_str());
		}
		return result;
		}

	static PolicyHelper::ProductStatus GetProductStatus(std::shared_ptr<Policy> policy, Utf8String productId, Utf8String featureString)
		{
		if (policy == nullptr)
			{
			return PolicyHelper::ProductStatus::NoLicense;
			}
		auto securable = GetFirstMatchingSecurableData(policy->GetSecurableData(), productId, featureString);
		if (securable == nullptr)
			{
			return PolicyHelper::ProductStatus::NoLicense;
			}
		auto acl = GetFirstMatchingACL(policy->GetACLs(), securable->GetSecurableId());
		if (acl == nullptr)
			{
			return PolicyHelper::ProductStatus::NoLicense;
			}
		if (acl->GetAccessKind() == Policy::ACL::AccessKind::Denied)
			{
			return PolicyHelper::ProductStatus::Denied;
			}
		if (acl->GetAccessKind() == Policy::ACL::AccessKind::TrialExpired)
			{
			return PolicyHelper::ProductStatus::TrialExpired;
			}
		if (IsTrial(policy, productId, featureString))
			{
			if (IsTimeExpired(acl->GetExpiresOn()))
				{
				return PolicyHelper::ProductStatus::TrialExpired;
				}
			}
		return PolicyHelper::ProductStatus::Allowed;
		}

	static PolicyHelper::PolicyStatus GetPolicyStatus(std::shared_ptr<Policy> policy)
		{
		if (IsExpired(policy))
			{
			return PolicyHelper::PolicyStatus::Expired;
			}
		if (!IsValid(policy))
			{
			return PolicyHelper::PolicyStatus::Invalid;
			}
		return PolicyHelper::PolicyStatus::Valid;
		}
};

END_BENTLEY_LICENSING_NAMESPACE
