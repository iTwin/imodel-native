/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ChildNodeRule.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "CommonTools.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRule::~ChildNodeRule ()
    {
    CommonTools::FreePresentationRules (m_specifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    m_targetTree = TargetTree_MainTree;
    WString ruleTargetTreeString = L"";
    if (BEXML_Success == xmlNode->GetAttributeStringValue (ruleTargetTreeString, CHILD_NODE_RULE_XML_ATTRIBUTE_TARGETTREE))
        m_targetTree = CommonTools::ParseTargetTreeString (ruleTargetTreeString);

    CommonTools::LoadRulesFromXmlNode<AllInstanceNodesSpecification, ChildNodeSpecificationList> (xmlNode, m_specifications, ALL_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode<AllRelatedInstanceNodesSpecification, ChildNodeSpecificationList> (xmlNode, m_specifications, ALL_RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode<CustomNodeSpecification, ChildNodeSpecificationList> (xmlNode, m_specifications, CUSTOM_NODE_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode<InstanceNodesOfSpecificClassesSpecification, ChildNodeSpecificationList> (xmlNode, m_specifications, INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode<RelatedInstanceNodesSpecification, ChildNodeSpecificationList> (xmlNode, m_specifications, RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadRulesFromXmlNode<SearchResultInstanceNodesSpecification, ChildNodeSpecificationList> (xmlNode, m_specifications, SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME);

    return PresentationRule::_ReadXml (xmlNode);
    }