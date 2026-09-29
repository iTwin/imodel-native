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
SubCondition::SubCondition() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::SubCondition(Utf8StringCR condition) : m_condition(condition) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::SubCondition(SubConditionCR other)
    : PresentationKey(other), m_condition(other.m_condition)
    {
    CommonToolsInternal::CopyRules(m_subConditions, other.m_subConditions, this);
    CommonToolsInternal::CloneRules(m_specifications, other.m_specifications, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::~SubCondition()
    {
    CommonToolsInternal::FreePresentationRules(m_requiredSchemas);
    CommonToolsInternal::FreePresentationRules(m_subConditions);
    CommonToolsInternal::FreePresentationRules(m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SubCondition::_GetJsonElementType() const {return "SubCondition";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubCondition::_ReadJson(BeJsConst json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    m_condition = json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION].asCString("");
    if (json.isMember(COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS))
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS, json[COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS], m_requiredSchemas, CommonToolsInternal::LoadRuleFromJson<RequiredSchemaSpecification>, this);
    if (json.isMember(SUB_CONDITION_JSON_ATTRIBUTE_SUBCONDITIONS))
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), SUB_CONDITION_JSON_ATTRIBUTE_SUBCONDITIONS, json[SUB_CONDITION_JSON_ATTRIBUTE_SUBCONDITIONS], m_subConditions, CommonToolsInternal::LoadRuleFromJson<SubCondition>, this);
    if (json.isMember(SUB_CONDITION_JSON_ATTRIBUTE_SPECIFICATIONS))
        CommonToolsInternal::LoadFromJsonByPriority(_GetJsonElementType(), SUB_CONDITION_JSON_ATTRIBUTE_SPECIFICATIONS, json[SUB_CONDITION_JSON_ATTRIBUTE_SPECIFICATIONS], m_specifications, ChildNodeSpecification::Create, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCondition::_WriteJson(BeJsValue json) const
    {
    if (!m_condition.empty())
        json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION] = m_condition;
    if (!m_requiredSchemas.empty())
        CommonToolsInternal::WriteRulesToJson<RequiredSchemaSpecification, RequiredSchemaSpecificationsList>(json[COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS], m_requiredSchemas);
    if (!m_subConditions.empty())
        CommonToolsInternal::WriteRulesToJson<SubCondition, SubConditionList>(json[SUB_CONDITION_JSON_ATTRIBUTE_SUBCONDITIONS], m_subConditions);
    if (!m_specifications.empty())
        CommonToolsInternal::WriteRulesToJson<ChildNodeSpecification, ChildNodeSpecificationList>(json[SUB_CONDITION_JSON_ATTRIBUTE_SPECIFICATIONS], m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SubCondition::GetCondition(void) { return m_condition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCondition::ClearRequiredSchemaSpecifications()
    {
    CommonToolsInternal::FreePresentationRules(m_requiredSchemas);
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCondition::AddRequiredSchemaSpecification(RequiredSchemaSpecificationR spec) {ADD_HASHABLE_CHILD(m_requiredSchemas, spec);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SubConditionList const& SubCondition::GetSubConditions(void) const { return m_subConditions; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCondition::AddSubCondition(SubConditionR subCondition)
    {
    ADD_HASHABLE_CHILD(m_subConditions, subCondition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecificationList const& SubCondition::GetSpecifications(void) const { return m_specifications; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCondition::AddSpecification(ChildNodeSpecificationR specification)
    {
    ADD_HASHABLE_CHILD(m_specifications, specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 SubCondition::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_condition.empty())
        ADD_STR_VALUE_TO_HASH(md5, PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION, m_condition);
    ADD_RULES_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS, m_requiredSchemas);
    ADD_RULES_TO_HASH(md5, SUB_CONDITION_JSON_ATTRIBUTE_SUBCONDITIONS, m_subConditions);
    ADD_RULES_TO_HASH(md5, SUB_CONDITION_JSON_ATTRIBUTE_SPECIFICATIONS, m_specifications);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCondition::_SetIndex(int& index)
    {
    T_Super::_SetIndex(index);
    SET_RULES_INDEX(m_subConditions, index);
    SET_RULES_INDEX(m_specifications, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule()
    : m_stopFurtherProcessing(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule(Utf8StringCR condition, int priority, bool onlyIfNotHandled)
    : ConditionalPresentationRule(condition, priority, onlyIfNotHandled), m_stopFurtherProcessing(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule(ChildNodeRuleCR other)
    : ConditionalPresentationRule(other), m_stopFurtherProcessing(other.m_stopFurtherProcessing)
    {
    CommonToolsInternal::CopyRules(m_subConditions, other.m_subConditions, this);
    CommonToolsInternal::CloneRules(m_specifications, other.m_specifications, this);
    CommonToolsInternal::CloneRules(m_customizationRules, other.m_customizationRules, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::~ChildNodeRule()
    {
    CommonToolsInternal::FreePresentationRules(m_subConditions);
    CommonToolsInternal::FreePresentationRules(m_specifications);
    CommonToolsInternal::FreePresentationRules(m_customizationRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ChildNodeRule::_GetJsonElementType() const
    {
    return CHILD_NODE_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeRule::_ReadJson(BeJsConst json)
    {
    if (!ConditionalPresentationRule::_ReadJson(json))
        return false;

    m_stopFurtherProcessing = json[COMMON_JSON_ATTRIBUTE_STOPFURTHERPROCESSING].asBool(false);
    if (json.isMember(CHILD_NODE_RULE_JSON_ATTRIBUTE_SUBCONDITIONS))
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), CHILD_NODE_RULE_JSON_ATTRIBUTE_SUBCONDITIONS, json[CHILD_NODE_RULE_JSON_ATTRIBUTE_SUBCONDITIONS], m_subConditions, CommonToolsInternal::LoadRuleFromJson<SubCondition>, this);
    if (json.isMember(CHILD_NODE_RULE_JSON_ATTRIBUTE_SPECIFICATIONS))
        CommonToolsInternal::LoadFromJsonByPriority(_GetJsonElementType(), CHILD_NODE_RULE_JSON_ATTRIBUTE_SPECIFICATIONS, json[CHILD_NODE_RULE_JSON_ATTRIBUTE_SPECIFICATIONS], m_specifications, ChildNodeSpecification::Create, this);
    if (json.isMember(CHILD_NODE_RULE_JSON_ATTRIBUTE_CUSTOMIZATIONRULES))
        CommonToolsInternal::LoadFromJsonByPriority(_GetJsonElementType(), CHILD_NODE_RULE_JSON_ATTRIBUTE_CUSTOMIZATIONRULES, json[CHILD_NODE_RULE_JSON_ATTRIBUTE_CUSTOMIZATIONRULES], m_customizationRules, CustomizationRule::Create, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::_WriteJson(BeJsValue json) const
    {
    ConditionalPresentationRule::_WriteJson(json);
    if (m_stopFurtherProcessing)
        json[COMMON_JSON_ATTRIBUTE_STOPFURTHERPROCESSING] = m_stopFurtherProcessing;
    if (!m_subConditions.empty())
        CommonToolsInternal::WriteRulesToJson<SubCondition, SubConditionList>(json[CHILD_NODE_RULE_JSON_ATTRIBUTE_SUBCONDITIONS], m_subConditions);
    if (!m_specifications.empty())
        CommonToolsInternal::WriteRulesToJson<ChildNodeSpecification, ChildNodeSpecificationList>(json[CHILD_NODE_RULE_JSON_ATTRIBUTE_SPECIFICATIONS], m_specifications);
    if (!m_customizationRules.empty())
        CommonToolsInternal::WriteRulesToJson<CustomizationRule, ChildNodeCustomizationRuleList>(json[CHILD_NODE_RULE_JSON_ATTRIBUTE_CUSTOMIZATIONRULES], m_customizationRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SubConditionList const& ChildNodeRule::GetSubConditions(void) const { return m_subConditions; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::AddSubCondition(SubConditionR subCondition)
    {
    ADD_HASHABLE_CHILD(m_subConditions, subCondition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecificationList const& ChildNodeRule::GetSpecifications(void) const { return m_specifications; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::AddSpecification(ChildNodeSpecificationR specification)
    {
    ADD_HASHABLE_CHILD(m_specifications, specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeCustomizationRuleList const& ChildNodeRule::GetCustomizationRules(void) const { return m_customizationRules; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::AddCustomizationRule(CustomizationRuleR customizationRule)
    {
    ADD_HASHABLE_CHILD(m_customizationRules, customizationRule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::SetStopFurtherProcessing(bool stopFurtherProcessing) { m_stopFurtherProcessing = stopFurtherProcessing; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeRule::GetStopFurtherProcessing(void) const { return m_stopFurtherProcessing; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ChildNodeRule::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_stopFurtherProcessing)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_STOPFURTHERPROCESSING, m_stopFurtherProcessing);
    ADD_RULES_TO_HASH(md5, CHILD_NODE_RULE_JSON_ATTRIBUTE_SUBCONDITIONS, m_subConditions);
    ADD_RULES_TO_HASH(md5, CHILD_NODE_RULE_JSON_ATTRIBUTE_SPECIFICATIONS, m_specifications);
    ADD_RULES_TO_HASH(md5, CHILD_NODE_RULE_JSON_ATTRIBUTE_CUSTOMIZATIONRULES, m_customizationRules);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::_SetIndex(int& index)
    {
    T_Super::_SetIndex(index);
    SET_RULES_INDEX(m_subConditions, index);
    SET_RULES_INDEX(m_specifications, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRule::RootNodeRule()
    : ChildNodeRule(), m_autoExpand(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRule::RootNodeRule(Utf8StringCR condition, int priority, bool onlyIfNotHandled, bool autoExpand)
    : ChildNodeRule(condition, priority, onlyIfNotHandled), m_autoExpand(autoExpand)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RootNodeRule::_GetJsonElementType() const
    {
    return ROOT_NODE_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootNodeRule::_ReadJson(BeJsConst json)
    {
    if (!ChildNodeRule::_ReadJson(json))
        return false;

    m_autoExpand = json[COMMON_JSON_ATTRIBUTE_AUTOEXPAND].asBool(false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RootNodeRule::_WriteJson(BeJsValue json) const
    {
    ChildNodeRule::_WriteJson(json);
    if (m_autoExpand)
        json[COMMON_JSON_ATTRIBUTE_AUTOEXPAND] = m_autoExpand;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootNodeRule::GetAutoExpand(void) const { return m_autoExpand; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RootNodeRule::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_autoExpand)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_AUTOEXPAND, m_autoExpand);
    return md5;
    }
