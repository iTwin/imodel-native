/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR HashableBase::GetHash() const
    {
    BeMutexHolder lock(m_mutex);
    if (m_hash.empty())
        {
        const_cast<HashableBase*>(this)->ComputeHash();
        if (m_hash.empty())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Rules, "Failed to calculate rule's hash");
        }
    return m_hash;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void HashableBase::InvalidateHash()
    {
    BeMutexHolder lock(m_mutex);
    if (nullptr != m_parent)
        m_parent->InvalidateHash();
    m_hash.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void HashableBase::ComputeHash()
    {
    BeMutexHolder lock(m_mutex);
    m_hash = _ComputeHash().GetHashString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationKey::ReadJson(BeJsConst json)
    {
    Utf8CP jsonRuleTypeAttributeName = _GetJsonElementTypeAttributeName();
    if (nullptr != jsonRuleTypeAttributeName)
        {
        Utf8CP jsonRuleType = json[jsonRuleTypeAttributeName].asCString("");
        Utf8CP thisRuleType = _GetJsonElementType();
        if (CommonToolsInternal::CheckRuleIssue(!*jsonRuleType, thisRuleType ? thisRuleType : "", jsonRuleTypeAttributeName, json[jsonRuleTypeAttributeName], "string value"))
            return false;

        if (thisRuleType && 0 != strcmp(jsonRuleType, thisRuleType))
            return false;
        }

    return _ReadJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationKey::WriteJson(BeJsValue json) const
    {
    if (nullptr != _GetJsonElementTypeAttributeName() && nullptr != _GetJsonElementType())
        json[_GetJsonElementTypeAttributeName()] = _GetJsonElementType();
    _WriteJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeJsDocument PresentationKey::WriteJson() const
    {
    BeJsDocument json;
    json.SetEmptyObject();
    WriteJson(json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PresentationKey::_ComputeHash() const
    {
    MD5 md5;

    Utf8CP name = _GetJsonElementType();
    if (name)
        md5.Add(name, strlen(name));
    if (m_index.IsValid())
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, "index", m_index.Value());
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PrioritizedPresentationKey::_ReadJson(BeJsConst json)
    {
    m_priority = json[COMMON_JSON_ATTRIBUTE_PRIORITY].asInt(1000);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PrioritizedPresentationKey::_WriteJson(BeJsValue json) const
    {
    PresentationKey::_WriteJson(json);
    if (1000 != m_priority)
        json[COMMON_JSON_ATTRIBUTE_PRIORITY] = m_priority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PrioritizedPresentationKey::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_priority != 1000)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_PRIORITY, m_priority);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRule::PresentationRule()
    : m_onlyIfNotHandled(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRule::PresentationRule(PresentationRule const& other)
    : PrioritizedPresentationKey(other), m_onlyIfNotHandled(other.m_onlyIfNotHandled)
    {
    CommonToolsInternal::CopyRules(m_requiredSchemas, other.m_requiredSchemas, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRule::PresentationRule(PresentationRule&& other)
    : PrioritizedPresentationKey(std::move(other)), m_onlyIfNotHandled(other.m_onlyIfNotHandled)
    {
    CommonToolsInternal::SwapRules(m_requiredSchemas, other.m_requiredSchemas, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRule::PresentationRule(int priority, bool onlyIfNotHandled)
    : PrioritizedPresentationKey(priority), m_onlyIfNotHandled(onlyIfNotHandled)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRule::~PresentationRule()
    {
    CommonToolsInternal::FreePresentationRules(m_requiredSchemas);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PresentationRule::_GetJsonElementTypeAttributeName() const {return COMMON_JSON_ATTRIBUTE_RULETYPE;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRule::_ReadJson(BeJsConst json)
    {
    if (!PrioritizedPresentationKey::_ReadJson(json))
        return false;

    m_onlyIfNotHandled = json[COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED].asBool(false);

    if (json.isMember(COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS))
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS, json[COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS], m_requiredSchemas, CommonToolsInternal::LoadRuleFromJson<RequiredSchemaSpecification>, this);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRule::_WriteJson(BeJsValue json) const
    {
    PrioritizedPresentationKey::_WriteJson(json);

    if (m_onlyIfNotHandled)
        json[COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED] = m_onlyIfNotHandled;

    if (!m_requiredSchemas.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RequiredSchemaSpecification, RequiredSchemaSpecificationsList>
            (json[COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS], m_requiredSchemas);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PresentationRule::GetOnlyIfNotHandled(void) const { return m_onlyIfNotHandled; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRule::ClearRequiredSchemaSpecifications()
    {
    CommonToolsInternal::FreePresentationRules(m_requiredSchemas);
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRule::AddRequiredSchemaSpecification(RequiredSchemaSpecificationR spec) {ADD_HASHABLE_CHILD(m_requiredSchemas, spec);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PresentationRule::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_onlyIfNotHandled)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED, m_onlyIfNotHandled);
    ADD_RULES_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS, m_requiredSchemas);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ConditionalPresentationRule::GetCondition() const { return m_condition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalPresentationRule::SetCondition(Utf8String value) { m_condition = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConditionalPresentationRule::_ReadJson(BeJsConst json)
    {
    if (!PresentationRule::_ReadJson(json))
        return false;

    m_condition = json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION].asCString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalPresentationRule::_WriteJson(BeJsValue json) const
    {
    PresentationRule::_WriteJson(json);
    if (!m_condition.empty())
        json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION] = m_condition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ConditionalPresentationRule::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_condition.empty())
        ADD_STR_VALUE_TO_HASH(md5, PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION, m_condition);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PresentationRuleSpecification::Accept(PresentationRuleSpecificationVisitor& visitor) const {_Accept(visitor);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PresentationRuleSpecification::_GetJsonElementTypeAttributeName() const {return COMMON_JSON_ATTRIBUTE_SPECTYPE;}
