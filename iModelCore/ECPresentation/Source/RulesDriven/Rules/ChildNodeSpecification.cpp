/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ChildNodeSpecification.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/CommonTools.h>
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_EC

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
: m_priority (1000), m_alwaysReturnsChildren (false), m_hideNodesInHierarchy (false), m_hideIfNoChildren (false), m_extendedData (L""), m_doNotSort (false)
    {
    m_id = GetNewSpecificationId ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::ChildNodeSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren)
: m_priority (priority), m_alwaysReturnsChildren (alwaysReturnsChildren), m_hideNodesInHierarchy (hideNodesInHierarchy), m_hideIfNoChildren (hideIfNoChildren), m_extendedData (L""), m_doNotSort (false)
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

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_extendedData, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_EXTENDEDDATA))
        m_extendedData = L"";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_doNotSort, SORTING_RULE_XML_ATTRIBUTE_DONOTSORT))
        m_doNotSort = false;

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
    specificationNode->AddAttributeStringValue  (CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_EXTENDEDDATA, m_extendedData.c_str ());
    specificationNode->AddAttributeBooleanValue (SORTING_RULE_XML_ATTRIBUTE_DONOTSORT, m_doNotSort);

    CommonTools::WriteRulesToXmlNode<ChildNodeRule, ChildNodeRuleList> (specificationNode, m_nestedRules);

    //Make sure we call protected override
    _WriteXml (specificationNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ChildNodeSpecification::GetPriority (void) const
    {
    return m_priority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetPriority (int value) { m_priority = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ChildNodeSpecification::GetId (void) const
    {
    return m_id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::GetAlwaysReturnsChildren (void) const
    {
    return m_alwaysReturnsChildren;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetAlwaysReturnsChildren (bool value) { m_alwaysReturnsChildren = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::GetHideNodesInHierarchy (void) const
    {
    return m_hideNodesInHierarchy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetHideNodesInHierarchy (bool value) { m_hideNodesInHierarchy = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::GetHideIfNoChildren (void)
    {
    return m_hideIfNoChildren;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetHideIfNoChildren (bool value) { m_hideIfNoChildren = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ChildNodeSpecification::GetExtendedData (void)
    {
    return m_extendedData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetExtendedData (WStringCR extendedData)
    {
    m_extendedData = extendedData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::GetDoNotSort (void)
    {
    return m_doNotSort;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetDoNotSort (bool doNotSort)
    {
    m_doNotSort = doNotSort;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRuleList& ChildNodeSpecification::GetNestedRules (void)
    {
    return m_nestedRules;
    }
