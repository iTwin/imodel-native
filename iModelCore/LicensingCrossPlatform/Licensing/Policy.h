/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/Policy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "PolicyToken.h"
#include "DateHelper.h"

#include <Licensing/Licensing.h>
#include <list>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct Policy
{
public:
	// START defining necessary structs for Policy
	struct Qualifier
	{
	private:
		Json::Value m_qualifier;
		Utf8String m_Name;
		Utf8String m_Value;
		Utf8String m_Type;
		Utf8String m_Prompt;
		Qualifier(Json::Value json);
	public:
		static std::shared_ptr<Qualifier> Create(const Json::Value& json);
		Utf8String GetName() const { return m_Name; };
		Utf8String GetValue() const { return m_Value; };
		Utf8String GetType() const { return m_Type; };
		Utf8String GetPrompt() const { return m_Prompt; };
	};

	struct RequestedSecurable
	{
	private:
		Json::Value m_requestedSecurable;
		int64_t m_ProductId;
		Utf8String m_FeatureString;
		Utf8String m_Version;
		RequestedSecurable(Json::Value json);
	public:
		static std::shared_ptr<RequestedSecurable> Create(const Json::Value& json);
		int64_t GetProductId() const { return m_ProductId; };
		Utf8String GetFeatureString() const { return m_FeatureString; };
		Utf8String GetVersion() const { return m_Version; };
	};

	struct RequestData
	{
	private:
		Json::Value m_requestData;
		Utf8String m_MachineSID;
		Utf8String m_AccessKey;
		Utf8String m_UserId;
		time_t m_CheckedOutDate;
		std::list<std::shared_ptr<RequestedSecurable>> m_RequestedSecurables;
		Utf8String m_MachineName;
		time_t m_ClientDateTime;
		Utf8String m_Locale;
		Utf8String m_AppliesTo;
		// helper functions
		std::list<std::shared_ptr<RequestedSecurable>> CreateRequestedSecurables(Json::Value json);
		RequestData(Json::Value json);
	public:
		static std::shared_ptr<RequestData> Create(const Json::Value& json);
		Utf8String GetMachineSID() const { return m_MachineSID; };
		Utf8String GetAccessKey() const { return m_AccessKey; };
		Utf8String GetUserId() const { return m_UserId; };
		time_t GetCheckedOutDate() const { return m_CheckedOutDate; };
		std::list<std::shared_ptr<RequestedSecurable>> GetRequestedSecurables() const { return m_RequestedSecurables; };
		Utf8String GetMachineName() const { return m_MachineName; };
		time_t GetClientDateTime() const { return m_ClientDateTime; };
		Utf8String GetLocale() const { return m_Locale; };
		Utf8String GetAppliesTo() const { return m_AppliesTo; };
	};

	struct ACL
	{
	public:
		enum class AccessKind
		{
			Allowed = 1,
			Maybe = 2,
			Denied = 3,
			TrialExpired = 4
		};
	private:
		Json::Value m_acl;
		Utf8String m_PrincipalId;
		Utf8String m_SecurableId;
		AccessKind m_AccessKind;
		time_t m_ExpiresOn;
		std::list<std::shared_ptr<Qualifier>> m_QualifierOverrides;
		// helper functions
		std::list<std::shared_ptr<Qualifier>> CreateQualifierOverrides(Json::Value json);
		ACL(Json::Value json);
	public:
		static std::shared_ptr<ACL> Create(const Json::Value& json);
		Utf8String GetPrincipalId() const { return m_PrincipalId; };
		Utf8String GetSecurableId() const { return m_SecurableId; };
		AccessKind GetAccessKind() const { return m_AccessKind; };
		time_t GetExpiresOn() const { return m_ExpiresOn; };
		std::list<std::shared_ptr<Qualifier>> GetQualifierOverrides() const { return m_QualifierOverrides; };
	};

	struct SecurableData
	{
	private:
		Json::Value m_securableData;
		Utf8String m_SecurableId;
		int64_t m_ProductId;
		Utf8String m_FeatureString;
		Utf8String m_Version;
		std::list<std::shared_ptr<Qualifier>> m_QualifierOverrides;
		// helper functions
		std::list<std::shared_ptr<Qualifier>> CreateQualifierOverrides(Json::Value json);
		SecurableData(Json::Value json);
	public:
		static std::shared_ptr<SecurableData> Create(const Json::Value& json);
		Utf8String GetSecurableId() const { return m_SecurableId; };
		int64_t GetProductId() const { return m_ProductId; };
		Utf8String GetFeatureString() const { return m_FeatureString; };
		Utf8String GetVersion() const { return m_Version; };
		std::list<std::shared_ptr<Qualifier>> GetQualifierOverrides() const { return m_QualifierOverrides; };
	};

	struct UserData
	{
	private:
		Json::Value m_userData;
		Utf8String m_UserId;
		Utf8String m_OrganizationId;
		Utf8String m_UsageCountryISO;
		Utf8String m_UltimateSAPId;
		Utf8String m_UltimadeId;
		Utf8String m_UltimateCountryId;
		UserData(Json::Value json);
	public:
		static std::shared_ptr<UserData> Create(const Json::Value& json);
		Utf8String GetUserId() const { return m_UserId; };
		Utf8String GetOrganizationId() const { return m_OrganizationId; };
		Utf8String GetUsageCountryISO() const { return m_UsageCountryISO; };
		Utf8String GetUltimateSAPId() const { return m_UltimateSAPId; };
		Utf8String GetUltimateId() const { return m_UltimadeId; };
		Utf8String GetUltimateCountryId() const { return m_UltimateCountryId; };
	};
	// END
private:
	std::shared_ptr<PolicyToken> m_policyToken;
	Utf8String m_PolicyId;
	double m_PolicyVersion;
	time_t m_PolicyCreatedOn;
	time_t m_PolicyExpiresOn;
	std::shared_ptr<RequestData> m_RequestData;
	Utf8String m_MachineSignature;
	Utf8String m_AppliesToUserId;
	std::list<Utf8String> m_AppliesToSecurableIds;
	std::list<std::shared_ptr<ACL>> m_ACLs;
	std::list<std::shared_ptr<SecurableData>> m_SecurableData;
	std::shared_ptr<UserData> m_UserData;
	std::list<std::shared_ptr<Qualifier>> m_DefaultQualifiers;
	// helper functions
	std::list<Utf8String> CreateAppliesToSecurableIds(const Json::Value& json);
	std::list<std::shared_ptr<ACL>> CreateACLs(const Json::Value& json);
	std::list<std::shared_ptr<SecurableData>> CreateSecurableData(const Json::Value& json);
	std::list<std::shared_ptr<Qualifier>> CreateDefaultQualifiers(const Json::Value& json);
	Policy(std::shared_ptr<PolicyToken> policyToken);
public:
	LICENSING_EXPORT static std::shared_ptr<Policy> Create(std::shared_ptr<PolicyToken> policyToken);
	LICENSING_EXPORT Utf8String GetPolicyId() const { return m_PolicyId; };
	LICENSING_EXPORT double GetPolicyVersion() const { return m_PolicyVersion; };
	LICENSING_EXPORT time_t GetPolicyCreatedOn() const { return m_PolicyCreatedOn; };
	LICENSING_EXPORT time_t GetPolicyExpiresOn() const { return m_PolicyExpiresOn; };
	LICENSING_EXPORT std::shared_ptr<RequestData> GetRequestData() const { return m_RequestData; };
	LICENSING_EXPORT Utf8String GetMachineSignature() const { return m_MachineSignature; };
	LICENSING_EXPORT Utf8String GetAppliesToUserId() const { return m_AppliesToUserId; };
	LICENSING_EXPORT std::list<Utf8String> GetAppliesToSecurableIds() const { return m_AppliesToSecurableIds; };
	LICENSING_EXPORT std::list<std::shared_ptr<ACL>> GetACLs() const { return m_ACLs; };
	LICENSING_EXPORT std::list<std::shared_ptr<SecurableData>> GetSecurableData() const { return m_SecurableData; };
	LICENSING_EXPORT std::shared_ptr<UserData> GetUserData() const { return m_UserData; };
	LICENSING_EXPORT std::list<std::shared_ptr<Qualifier>> GetDefaultQualifiers() const { return m_DefaultQualifiers; };
};

END_BENTLEY_LICENSING_NAMESPACE
