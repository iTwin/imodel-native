/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::SubCondition() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::SubCondition(Utf8StringCR condition) : m_condition(condition) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::SubCondition(SubConditionCR other)
    : m_condition(other.m_condition)
    {
    CommonToolsInternal::CopyRules(m_subConditions, other.m_subConditions, this);
    CommonToolsInternal::CloneRules(m_specifications, other.m_specifications, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::~SubCondition()
    {
    CommonToolsInternal::FreePresentationRules(m_subConditions);
    CommonToolsInternal::FreePresentationRules(m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubCondition::ShallowEqual(SubConditionCR other) const
    {
    return m_condition == other.m_condition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubCondition::ReadXml(BeXmlNodeP xmlNode)
    {
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
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCondition::WriteXml(BeXmlNodeP xmlNode) const
    {
    BeXmlNodeP ruleNode = xmlNode->AddEmptyElement(SUB_CONDITION_XML_NODE_NAME);

    ruleNode->AddAttributeStringValue(PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION, m_condition.c_str());

    CommonToolsInternal::WriteRulesToXmlNode<SubCondition, SubConditionList>(ruleNode, m_subConditions);
    CommonToolsInternal::WriteRulesToXmlNode<ChildNodeSpecification, ChildNodeSpecificationList>(ruleNode, m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubCondition::ReadJson(JsonValueCR json)
    {
    m_condition = json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION].asCString("");
    CommonToolsInternal::LoadFromJson(json[SUB_CONDITION_JSON_ATTRIBUTE_SUBCONDITIONS], m_subConditions, CommonToolsInternal::LoadRuleFromJson<SubCondition>, this);
    CommonToolsInternal::LoadFromJsonByPriority(json[SUB_CONDITION_JSON_ATTRIBUTE_SPECIFICATIONS], m_specifications, ChildNodeSpecification::Create, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value SubCondition::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    if (!m_condition.empty())
        json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION] = m_condition;
    if (!m_subConditions.empty())
        CommonToolsInternal::WriteRulesToJson<SubCondition, SubConditionList>(json[SUB_CONDITION_JSON_ATTRIBUTE_SUBCONDITIONS], m_subConditions);
    if (!m_specifications.empty())
        CommonToolsInternal::WriteRulesToJson<ChildNodeSpecification, ChildNodeSpecificationList>(json[SUB_CONDITION_JSON_ATTRIBUTE_SPECIFICATIONS], m_specifications);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SubCondition::GetCondition(void) { return m_condition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SubConditionList const& SubCondition::GetSubConditions(void) const { return m_subConditions; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCondition::AddSubCondition(SubConditionR subCondition)
    {
    InvalidateHash();
    subCondition.SetParent(this);
    m_subConditions.push_back(&subCondition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecificationList const& SubCondition::GetSpecifications(void) const { return m_specifications; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCondition::AddSpecification(ChildNodeSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_specifications.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 SubCondition::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(m_condition.c_str(), m_condition.size());
    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));

    Utf8String currentHash = md5.GetHashString();
    for (SubConditionP condition : m_subConditions)
        {
        Utf8StringCR conditionHash = condition->GetHash(currentHash.c_str());
        md5.Add(conditionHash.c_str(), conditionHash.size());
        }
    for (ChildNodeSpecificationP spec : m_specifications)
        {
        Utf8StringCR conditionHash = spec->GetHash(currentHash.c_str());
        md5.Add(conditionHash.c_str(), conditionHash.size());
        }

    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule() : m_targetTree(TargetTree_MainTree), m_stopFurtherProcessing(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule(Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree)
    : ConditionalPresentationRule(condition, priority, onlyIfNotHandled), m_targetTree(targetTree), m_stopFurtherProcessing(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule(ChildNodeRuleCR other)
    : ConditionalPresentationRule(other), m_targetTree(other.m_targetTree), m_stopFurtherProcessing(other.m_stopFurtherProcessing)
    {
    CommonToolsInternal::CopyRules(m_subConditions, other.m_subConditions, this);
    CommonToolsInternal::CloneRules(m_specifications, other.m_specifications, this);
    CommonToolsInternal::CloneRules(m_customizationRules, other.m_customizationRules, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::~ChildNodeRule()
    {
    CommonToolsInternal::FreePresentationRules(m_subConditions);
    CommonToolsInternal::FreePresentationRules(m_specifications);
    CommonToolsInternal::FreePresentationRules(m_customizationRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2019
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
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ChildNodeRule::_GetXmlElementName() const
    {
    return CHILD_NODE_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeRule::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!ConditionalPresentationRule::_ReadXml(xmlNode))
        return false;

    m_targetTree = TargetTree_MainTree;
    Utf8String ruleTargetTreeString = "";
    if (BEXML_Success == xmlNode->GetAttributeStringValue(ruleTargetTreeString, CHILD_NODE_RULE_XML_ATTRIBUTE_TARGETTREE))
        m_targetTree = CommonToolsInternal::ParseTargetTreeString(ruleTargetTreeString.c_str());

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
* @bsimethod                                    Eligijus.Mauragas               10/2012
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
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ChildNodeRule::_GetJsonElementType() const
    {
    return CHILD_NODE_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeRule::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalPresentationRule::_ReadJson(json))
        return false;

    m_stopFurtherProcessing = json[COMMON_JSON_ATTRIBUTE_STOPFURTHERPROCESSING].asBool(false);
    CommonToolsInternal::LoadFromJson(json[CHILD_NODE_RULE_JSON_ATTRIBUTE_SUBCONDITIONS], m_subConditions, CommonToolsInternal::LoadRuleFromJson<SubCondition>, this);
    CommonToolsInternal::LoadFromJsonByPriority(json[CHILD_NODE_RULE_JSON_ATTRIBUTE_SPECIFICATIONS], m_specifications, ChildNodeSpecification::Create, this);
    CommonToolsInternal::LoadFromJsonByPriority(json[CHILD_NODE_RULE_JSON_ATTRIBUTE_CUSTOMIZATIONRULES], m_customizationRules, CustomizationRule::Create, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
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
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RuleTargetTree ChildNodeRule::GetTargetTree(void) const { return m_targetTree; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SubConditionList const& ChildNodeRule::GetSubConditions(void) const { return m_subConditions; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::AddSubCondition(SubConditionR subCondition)
    {
    InvalidateHash();
    subCondition.SetParent(this);
    m_subConditions.push_back(&subCondition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecificationList const& ChildNodeRule::GetSpecifications(void) const { return m_specifications; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::AddSpecification(ChildNodeSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_specifications.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.vaiksnoras               03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeCustomizationRuleList const& ChildNodeRule::GetCustomizationRules(void) const { return m_customizationRules; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::AddCustomizationRule(CustomizationRuleR customizationRule)
    {
    InvalidateHash();
    customizationRule.SetParent(this);
    m_customizationRules.push_back(&customizationRule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::SetStopFurtherProcessing(bool stopFurtherProcessing) { m_stopFurtherProcessing = stopFurtherProcessing; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeRule::GetStopFurtherProcessing(void) const { return m_stopFurtherProcessing; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ChildNodeRule::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ConditionalPresentationRule::_ComputeHash(parentHash);
    md5.Add(&m_targetTree, sizeof(m_targetTree));
    md5.Add(&m_stopFurtherProcessing, sizeof(m_stopFurtherProcessing));

    Utf8String currentHash = md5.GetHashString();
    for (SubConditionP condition : m_subConditions)
        {
        Utf8StringCR conditionHash = condition->GetHash(currentHash.c_str());
        md5.Add(conditionHash.c_str(), conditionHash.size());
        }
    for (ChildNodeSpecificationP spec : m_specifications)
        {
        Utf8StringCR conditionHash = spec->GetHash(currentHash.c_str());
        md5.Add(conditionHash.c_str(), conditionHash.size());
        }
    for (CustomizationRuleP custRule : m_customizationRules)
        {
        Utf8StringCR custRuleHash = custRule->GetHash(currentHash.c_str());
        md5.Add(custRuleHash.c_str(), custRuleHash.size());
        }

    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRule::RootNodeRule()
    : ChildNodeRule(), m_autoExpand(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRule::RootNodeRule(Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree, bool autoExpand)
    : ChildNodeRule(condition, priority, onlyIfNotHandled, targetTree), m_autoExpand(autoExpand)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2019
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
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RootNodeRule::_GetXmlElementName() const
    {
    return ROOT_NODE_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas             04/2013
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
* @bsimethod                                    dmitrijus.tiazlovas             04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void RootNodeRule::_WriteXml(BeXmlNodeP xmlNode) const
    {
    ChildNodeRule::_WriteXml(xmlNode);
    xmlNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_AUTOEXPAND, m_autoExpand);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RootNodeRule::_GetJsonElementType() const
    {
    return ROOT_NODE_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootNodeRule::_ReadJson(JsonValueCR json)
    {
    if (!ChildNodeRule::_ReadJson(json))
        return false;

    m_autoExpand = json[COMMON_JSON_ATTRIBUTE_AUTOEXPAND].asBool(false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RootNodeRule::_WriteJson(JsonValueR json) const
    {
    ChildNodeRule::_WriteJson(json);
    if (m_autoExpand)
        json[COMMON_JSON_ATTRIBUTE_AUTOEXPAND] = m_autoExpand;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas             04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootNodeRule::GetAutoExpand(void) const { return m_autoExpand; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RootNodeRule::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ChildNodeRule::_ComputeHash(parentHash);
    md5.Add(&m_autoExpand, sizeof(m_autoExpand));
    return md5;
    }
