/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
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
bool SubCondition::_ShallowEqual(PresentationKeyCR other) const
    {
    SubCondition const* otherRule = dynamic_cast<SubCondition const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_condition == otherRule->m_condition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SubCondition::_GetXmlElementName() const {return SUB_CONDITION_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubCondition::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PresentationKey::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_condition, PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION))
        m_condition = "";

    CommonToolsInternal::LoadSpecificationsFromXmlNode<SubCondition, SubConditionList>(xmlNode, m_subConditions, SUB_CONDITION_XML_NODE_NAME, this);

    for (BeXmlNodeP child = xmlNode->GetFirstChild(BEXMLNODE_Element); NULL != child; child = child->GetNextSibling(BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp(child->GetName(), ALL_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<AllInstanceNodesSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), ALL_RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<AllRelatedInstanceNodesSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), CUSTOM_NODE_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<CustomNodeSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<InstanceNodesOfSpecificClassesSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<RelatedInstanceNodesSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<SearchResultInstanceNodesSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCondition::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue(PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION, m_condition.c_str());
    CommonToolsInternal::WriteRulesToXmlNode<SubCondition, SubConditionList>(xmlNode, m_subConditions);
    CommonToolsInternal::WriteRulesToXmlNode<ChildNodeSpecification, ChildNodeSpecificationList>(xmlNode, m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SubCondition::_GetJsonElementType() const {return "SubCondition";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubCondition::_ReadJson(JsonValueCR json)
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
void SubCondition::_WriteJson(JsonValueR json) const
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
ChildNodeRule::ChildNodeRule() : m_targetTree(TargetTree_MainTree), m_stopFurtherProcessing(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule(Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree)
    : ConditionalPresentationRule(condition, priority, onlyIfNotHandled), m_targetTree(targetTree), m_stopFurtherProcessing(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule(ChildNodeRuleCR other)
    : ConditionalPresentationRule(other), m_targetTree(other.m_targetTree), m_stopFurtherProcessing(other.m_stopFurtherProcessing)
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
bool ChildNodeRule::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ConditionalPresentationRule::_ShallowEqual(other))
        return false;

    ChildNodeRuleCP otherRule = dynamic_cast<ChildNodeRuleCP>(&other);
    if (nullptr == otherRule)
        return false;

    // cannot decide if rules are similar without comparing customization rules
    if (!m_customizationRules.empty() || !otherRule->m_customizationRules.empty())
        return false;

    return m_stopFurtherProcessing == otherRule->m_stopFurtherProcessing
        && m_targetTree == otherRule->m_targetTree;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ChildNodeRule::_GetXmlElementName() const
    {
    return CHILD_NODE_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeRule::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!ConditionalPresentationRule::_ReadXml(xmlNode))
        return false;

    m_targetTree = TargetTree_MainTree;
    Utf8String ruleTargetTreeString = "";
    if (BEXML_Success == xmlNode->GetAttributeStringValue(ruleTargetTreeString, CHILD_NODE_RULE_XML_ATTRIBUTE_TARGETTREE))
        m_targetTree = CommonToolsInternal::ParseTargetTreeString(ruleTargetTreeString.c_str(), _GetXmlElementName());

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_stopFurtherProcessing, COMMON_XML_ATTRIBUTE_STOPFURTHERPROCESSING))
        m_stopFurtherProcessing = false;

    CommonToolsInternal::LoadSpecificationsFromXmlNode<SubCondition, SubConditionList>(xmlNode, m_subConditions, SUB_CONDITION_XML_NODE_NAME, this);

    for (BeXmlNodeP child = xmlNode->GetFirstChild(BEXMLNODE_Element); NULL != child; child = child->GetNextSibling(BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp(child->GetName(), ALL_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<AllInstanceNodesSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), ALL_RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<AllRelatedInstanceNodesSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), CUSTOM_NODE_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<CustomNodeSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<InstanceNodesOfSpecificClassesSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<RelatedInstanceNodesSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<SearchResultInstanceNodesSpecification, ChildNodeSpecificationList>(child, m_specifications, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), GROUPING_RULE_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<GroupingRule, ChildNodeCustomizationRuleList>(child, m_customizationRules, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), CHECKBOX_RULE_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<CheckBoxRule, ChildNodeCustomizationRuleList>(child, m_customizationRules, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), STYLE_OVERRIDE_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<StyleOverride, ChildNodeCustomizationRuleList>(child, m_customizationRules, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), LABEL_OVERRIDE_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<LabelOverride, ChildNodeCustomizationRuleList>(child, m_customizationRules, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), SORTING_RULE_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<SortingRule, ChildNodeCustomizationRuleList>(child, m_customizationRules, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), IMAGE_ID_OVERRIDE_XML_NODE_NAME))
            CommonToolsInternal::LoadRuleFromXmlNode<ImageIdOverride, ChildNodeCustomizationRuleList>(child, m_customizationRules, this);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::_WriteXml(BeXmlNodeP xmlNode) const
    {
    ConditionalPresentationRule::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(CHILD_NODE_RULE_XML_ATTRIBUTE_TARGETTREE, CommonToolsInternal::FormatTargetTreeString(m_targetTree));
    xmlNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_STOPFURTHERPROCESSING, m_stopFurtherProcessing);
    if (!m_subConditions.empty())
        CommonToolsInternal::WriteRulesToXmlNode<SubCondition, SubConditionList>(xmlNode, m_subConditions);
    if (!m_specifications.empty())
        CommonToolsInternal::WriteRulesToXmlNode<ChildNodeSpecification, ChildNodeSpecificationList>(xmlNode, m_specifications);
    if (!m_customizationRules.empty())
        CommonToolsInternal::WriteRulesToXmlNode<CustomizationRule, ChildNodeCustomizationRuleList>(xmlNode, m_customizationRules);
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
bool ChildNodeRule::_ReadJson(JsonValueCR json)
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
void ChildNodeRule::_WriteJson(JsonValueR json) const
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
RuleTargetTree ChildNodeRule::GetTargetTree(void) const { return m_targetTree; }

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
    if (m_targetTree != TargetTree_MainTree)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CHILD_NODE_RULE_XML_ATTRIBUTE_TARGETTREE, m_targetTree);
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
RootNodeRule::RootNodeRule()
    : ChildNodeRule(), m_autoExpand(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRule::RootNodeRule(Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree, bool autoExpand)
    : ChildNodeRule(condition, priority, onlyIfNotHandled, targetTree), m_autoExpand(autoExpand)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootNodeRule::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ChildNodeRule::_ShallowEqual(other))
        return false;

    RootNodeRuleCP otherRule = dynamic_cast<RootNodeRuleCP>(&other);
    if (nullptr == otherRule)
        return false;

    return m_autoExpand == otherRule->m_autoExpand;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RootNodeRule::_GetXmlElementName() const
    {
    return ROOT_NODE_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootNodeRule::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!ChildNodeRule::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_autoExpand, COMMON_XML_ATTRIBUTE_AUTOEXPAND))
        m_autoExpand = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RootNodeRule::_WriteXml(BeXmlNodeP xmlNode) const
    {
    ChildNodeRule::_WriteXml(xmlNode);
    xmlNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_AUTOEXPAND, m_autoExpand);
    }

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
bool RootNodeRule::_ReadJson(JsonValueCR json)
    {
    if (!ChildNodeRule::_ReadJson(json))
        return false;

    m_autoExpand = json[COMMON_JSON_ATTRIBUTE_AUTOEXPAND].asBool(false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RootNodeRule::_WriteJson(JsonValueR json) const
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
