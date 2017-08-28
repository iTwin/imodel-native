/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/ChildNodeRule.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::SubCondition () : m_condition ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::SubCondition (Utf8StringCR condition) : m_condition (condition)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::SubCondition(SubConditionCR other)
    : m_condition(other.m_condition)
    {
    CommonTools::CopyRules(m_subConditions, other.m_subConditions);
    CommonTools::CloneRules(m_specifications, other.m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::~SubCondition ()
    {
    CommonTools::FreePresentationRules (m_subConditions);
    CommonTools::FreePresentationRules (m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool SubCondition::ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_condition, PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION))
        m_condition = "";

    CommonTools::LoadSpecificationsFromXmlNode<SubCondition, SubConditionList> (xmlNode, m_subConditions, SUB_CONDITION_XML_NODE_NAME);

    for (BeXmlNodeP child = xmlNode->GetFirstChild (BEXMLNODE_Element); NULL != child; child = child->GetNextSibling (BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp (child->GetName (), ALL_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<AllInstanceNodesSpecification, ChildNodeSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), ALL_RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<AllRelatedInstanceNodesSpecification, ChildNodeSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), CUSTOM_NODE_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<CustomNodeSpecification, ChildNodeSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<InstanceNodesOfSpecificClassesSpecification, ChildNodeSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<RelatedInstanceNodesSpecification, ChildNodeSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<SearchResultInstanceNodesSpecification, ChildNodeSpecificationList> (child, m_specifications);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCondition::WriteXml (BeXmlNodeP xmlNode) const
    {
    BeXmlNodeP ruleNode = xmlNode->AddEmptyElement (SUB_CONDITION_XML_NODE_NAME);

    ruleNode->AddAttributeStringValue (PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION, m_condition.c_str ());

    CommonTools::WriteRulesToXmlNode<SubCondition,           SubConditionList>           (ruleNode, m_subConditions);
    CommonTools::WriteRulesToXmlNode<ChildNodeSpecification, ChildNodeSpecificationList> (ruleNode, m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SubCondition::GetCondition (void) { return m_condition;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SubConditionList& SubCondition::GetSubConditionsR (void) { return m_subConditions;  }
SubConditionList const& SubCondition::GetSubConditions (void) const { return m_subConditions;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecificationList& SubCondition::GetSpecificationsR (void) { return m_specifications; }
ChildNodeSpecificationList const& SubCondition::GetSpecifications (void) const { return m_specifications; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule () : PresentationRule (), m_targetTree (TargetTree_MainTree), m_stopFurtherProcessing (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree)
    : PresentationRule (condition, priority, onlyIfNotHandled), m_targetTree (targetTree), m_stopFurtherProcessing (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule(ChildNodeRuleCR other)
    : PresentationRule(other), m_targetTree(other.m_targetTree), m_stopFurtherProcessing(other.m_stopFurtherProcessing)
    {
    CommonTools::CopyRules(m_subConditions, other.m_subConditions);
    CommonTools::CloneRules(m_specifications, other.m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::~ChildNodeRule ()
    {
    CommonTools::FreePresentationRules (m_subConditions);
    CommonTools::FreePresentationRules (m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP ChildNodeRule::_GetXmlElementName () const
    {
    return CHILD_NODE_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    m_targetTree = TargetTree_MainTree;
    Utf8String ruleTargetTreeString = "";
    if (BEXML_Success == xmlNode->GetAttributeStringValue (ruleTargetTreeString, CHILD_NODE_RULE_XML_ATTRIBUTE_TARGETTREE))
        m_targetTree = CommonTools::ParseTargetTreeString (ruleTargetTreeString.c_str ());

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_stopFurtherProcessing, COMMON_XML_ATTRIBUTE_STOPFURTHERPROCESSING))
        m_stopFurtherProcessing = false;

    CommonTools::LoadSpecificationsFromXmlNode<SubCondition, SubConditionList> (xmlNode, m_subConditions, SUB_CONDITION_XML_NODE_NAME);

    for (BeXmlNodeP child = xmlNode->GetFirstChild (BEXMLNODE_Element); NULL != child; child = child->GetNextSibling (BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp (child->GetName (), ALL_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<AllInstanceNodesSpecification, ChildNodeSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), ALL_RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<AllRelatedInstanceNodesSpecification, ChildNodeSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), CUSTOM_NODE_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<CustomNodeSpecification, ChildNodeSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<InstanceNodesOfSpecificClassesSpecification, ChildNodeSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<RelatedInstanceNodesSpecification, ChildNodeSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp (child->GetName (), SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<SearchResultInstanceNodesSpecification, ChildNodeSpecificationList> (child, m_specifications);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), GROUPING_RULE_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<GroupingRule, ChildNodeCustomizationRuleList>(child, m_customizationRules);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), CHECKBOX_RULE_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<CheckBoxRule, ChildNodeCustomizationRuleList>(child, m_customizationRules);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), RENAMENODE_RULE_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<RenameNodeRule, ChildNodeCustomizationRuleList>(child, m_customizationRules);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), STYLE_OVERRIDE_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<StyleOverride, ChildNodeCustomizationRuleList>(child, m_customizationRules);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), LABEL_OVERRIDE_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<LabelOverride, ChildNodeCustomizationRuleList>(child, m_customizationRules);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), SORTING_RULE_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<SortingRule, ChildNodeCustomizationRuleList>(child, m_customizationRules);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), IMAGE_ID_OVERRIDE_XML_NODE_NAME))
            CommonTools::LoadRuleFromXmlNode<ImageIdOverride, ChildNodeCustomizationRuleList>(child, m_customizationRules);
        }

    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue (CHILD_NODE_RULE_XML_ATTRIBUTE_TARGETTREE, CommonTools::FormatTargetTreeString (m_targetTree));
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_STOPFURTHERPROCESSING, m_stopFurtherProcessing);

    CommonTools::WriteRulesToXmlNode<ChildNodeSpecification, ChildNodeSpecificationList> (xmlNode, m_specifications);
    CommonTools::WriteRulesToXmlNode<CustomizationRule, ChildNodeCustomizationRuleList> (xmlNode, m_customizationRules);
    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RuleTargetTree ChildNodeRule::GetTargetTree (void) const { return m_targetTree; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SubConditionList const& ChildNodeRule::GetSubConditions (void) const { return m_subConditions;  }
SubConditionList& ChildNodeRule::GetSubConditionsR (void) { return m_subConditions;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecificationList const& ChildNodeRule::GetSpecifications (void) const { return m_specifications; }
ChildNodeSpecificationList& ChildNodeRule::GetSpecificationsR (void) { return m_specifications; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.vaiksnoras               03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeCustomizationRuleList const& ChildNodeRule::GetCustomizationRules(void) const { return m_customizationRules; }
ChildNodeCustomizationRuleList& ChildNodeRule::GetCustomizationRulesR(void) { return m_customizationRules; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::SetStopFurtherProcessing (bool stopFurtherProcessing) { m_stopFurtherProcessing = stopFurtherProcessing; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeRule::GetStopFurtherProcessing (void) const { return m_stopFurtherProcessing; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRule::RootNodeRule() 
    : ChildNodeRule(), m_autoExpand(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRule::RootNodeRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree, bool autoExpand)
    : ChildNodeRule (condition, priority, onlyIfNotHandled, targetTree), m_autoExpand (autoExpand)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP RootNodeRule::_GetXmlElementName () const
    {
    return ROOT_NODE_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas             04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootNodeRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_autoExpand, ROOT_NODE_RULE_XML_ATTRIBUTE_AUTOEXPAND))
        m_autoExpand = false;

    return ChildNodeRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas             04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void RootNodeRule::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeBooleanValue (ROOT_NODE_RULE_XML_ATTRIBUTE_AUTOEXPAND, m_autoExpand);
    ChildNodeRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas             04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootNodeRule::GetAutoExpand (void) const { return m_autoExpand; }
