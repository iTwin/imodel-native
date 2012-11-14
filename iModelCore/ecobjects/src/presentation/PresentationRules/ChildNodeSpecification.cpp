/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ChildNodeSpecification.cpp $
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
int ChildNodeSpecification::GetNewSpecificationId ()
    {
    static int counter = 0;
    return counter++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::ChildNodeSpecification ()
: m_priority (1000), m_alwaysReturnsChildren (false), m_hideNodesInHierarchy (false), m_hideIfNoChildren (false)
    {
    m_id = GetNewSpecificationId ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::ChildNodeSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren)
: m_priority (priority), m_alwaysReturnsChildren (alwaysReturnsChildren), m_hideNodesInHierarchy (hideNodesInHierarchy), m_hideIfNoChildren (hideIfNoChildren)
    {
    m_id = GetNewSpecificationId ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::~ChildNodeSpecification ()
    {
    CommonTools::FreePresentationRules (m_nestedRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_priority, COMMON_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_alwaysReturnsChildren, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_ALWAYSRETURNSCHILDREN))
        m_alwaysReturnsChildren = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_hideNodesInHierarchy, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDENODESINHIERARCHY))
        m_hideNodesInHierarchy = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_hideIfNoChildren, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDEIFNOCHILDREN))
        m_hideIfNoChildren = false;

    CommonTools::LoadRulesFromXmlNode<ChildNodeRule, ChildNodeRuleList> (xmlNode, m_nestedRules, CHILD_NODE_RULE_XML_NODE_NAME);

    return _ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::WriteXml (BeXmlNodeP parentXmlNode)
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement (_GetXmlElementName ());

    specificationNode->AddAttributeInt32Value   (COMMON_XML_ATTRIBUTE_PRIORITY, m_priority);
    specificationNode->AddAttributeBooleanValue (CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_ALWAYSRETURNSCHILDREN, m_alwaysReturnsChildren);
    specificationNode->AddAttributeBooleanValue (CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDENODESINHIERARCHY, m_hideNodesInHierarchy);
    specificationNode->AddAttributeBooleanValue (CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDEIFNOCHILDREN, m_hideIfNoChildren);

    CommonTools::WriteRulesToXmlNode<ChildNodeRule, ChildNodeRuleList> (specificationNode, m_nestedRules);

    //Make sure we call protected override
    _WriteXml (specificationNode);
    }