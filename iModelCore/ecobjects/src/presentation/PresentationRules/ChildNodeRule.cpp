/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ChildNodeRule.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "CommonTools.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::SubCondition () : m_condition (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SubCondition::SubCondition (WStringCR condition) : m_condition (condition)
    {
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
        m_condition = L"";

    CommonTools::LoadRulesFromXmlNode <SubCondition, SubConditionList> (xmlNode, m_subConditions, SUB_CONDITION_XML_NODE_NAME);

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
void SubCondition::WriteXml (BeXmlNodeP xmlNode)
    {
    BeXmlNodeP ruleNode = xmlNode->AddEmptyElement (SUB_CONDITION_XML_NODE_NAME);

    ruleNode->AddAttributeStringValue (PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION, m_condition.c_str ());

    CommonTools::WriteRulesToXmlNode<SubCondition,           SubConditionList>           (ruleNode, m_subConditions);
    CommonTools::WriteRulesToXmlNode<ChildNodeSpecification, ChildNodeSpecificationList> (ruleNode, m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR SubCondition::GetCondition (void) { return m_condition;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
SubConditionList& SubCondition::GetSubConditions (void) { return m_subConditions;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecificationList& SubCondition::GetSpecifications (void) { return m_specifications; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule () : PresentationRule ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::ChildNodeRule (WStringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree)
    : PresentationRule (condition, priority, onlyIfNotHandled), m_targetTree (targetTree)
    {
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
CharCP ChildNodeRule::_GetXmlElementName ()
    {
    return CHILD_NODE_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    m_targetTree = TargetTree_MainTree;
    WString ruleTargetTreeString = L"";
    if (BEXML_Success == xmlNode->GetAttributeStringValue (ruleTargetTreeString, CHILD_NODE_RULE_XML_ATTRIBUTE_TARGETTREE))
        m_targetTree = CommonTools::ParseTargetTreeString (ruleTargetTreeString.c_str ());

    CommonTools::LoadRulesFromXmlNode <SubCondition, SubConditionList> (xmlNode, m_subConditions, SUB_CONDITION_XML_NODE_NAME);

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

    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeRule::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue (CHILD_NODE_RULE_XML_ATTRIBUTE_TARGETTREE, CommonTools::FormatTargetTreeString (m_targetTree));

    CommonTools::WriteRulesToXmlNode<ChildNodeSpecification, ChildNodeSpecificationList> (xmlNode, m_specifications);

    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RuleTargetTree ChildNodeRule::GetTargetTree (void) { return m_targetTree; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SubConditionList& ChildNodeRule::GetSubConditions (void) { return m_subConditions;  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecificationList& ChildNodeRule::GetSpecifications (void) { return m_specifications; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRule::RootNodeRule () : ChildNodeRule ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRule::RootNodeRule (WStringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree, bool autoExpand)
    : ChildNodeRule (condition, priority, onlyIfNotHandled, targetTree), m_autoExpand (autoExpand)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP RootNodeRule::_GetXmlElementName ()
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
void RootNodeRule::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeBooleanValue (ROOT_NODE_RULE_XML_ATTRIBUTE_AUTOEXPAND, m_autoExpand);
    ChildNodeRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas             04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootNodeRule::GetAutoExpand (void) { return m_autoExpand; }
