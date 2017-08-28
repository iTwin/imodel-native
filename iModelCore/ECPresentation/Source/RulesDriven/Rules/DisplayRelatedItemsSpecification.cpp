/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/DisplayRelatedItemsSpecification.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas              11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayRelatedItemsSpecification::DisplayRelatedItemsSpecification ()
    : m_logicalChildren (false), m_nestingDepth (0), m_relationshipClasses (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas              11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayRelatedItemsSpecification::DisplayRelatedItemsSpecification (bool logicalChildren, int nestingDepth, Utf8StringCR relationshipClasses)
    : m_logicalChildren (logicalChildren), m_nestingDepth (nestingDepth), m_relationshipClasses (relationshipClasses)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas              11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayRelatedItemsSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_nestingDepth, DISPLAYRELATEDITEMS_SPECIFICATION_XML_ATTRIBUTE_NESTINGDEPTH))
        m_nestingDepth = 0;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_logicalChildren, DISPLAYRELATEDITEMS_SPECIFICATION_XML_ATTRIBUTE_LOGICALCHILDREN))
        m_logicalChildren = true;

    if (BEXML_Success != xmlNode->GetAttributeStringValue  (m_relationshipClasses, DISPLAYRELATEDITEMS_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPCLASSES))
        m_relationshipClasses = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas              11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayRelatedItemsSpecification::WriteXml (BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP displayRelatedItemsNode = parentXmlNode->AddEmptyElement (DISPLAYRELATEDITEMS_SPECIFICATION_XML_NODE_NAME);

    displayRelatedItemsNode->AddAttributeInt32Value (DISPLAYRELATEDITEMS_SPECIFICATION_XML_ATTRIBUTE_NESTINGDEPTH, m_nestingDepth);
    displayRelatedItemsNode->AddAttributeBooleanValue (DISPLAYRELATEDITEMS_SPECIFICATION_XML_ATTRIBUTE_LOGICALCHILDREN, m_logicalChildren);
    displayRelatedItemsNode->AddAttributeStringValue (DISPLAYRELATEDITEMS_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPCLASSES, m_relationshipClasses.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas              11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayRelatedItemsSpecification::GetLogicalChildren (void) const { return m_logicalChildren; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas              11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int DisplayRelatedItemsSpecification::GetNestingDepth (void) const { return m_nestingDepth; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas              11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR DisplayRelatedItemsSpecification::GetRelationshipClasses (void) const { return m_relationshipClasses; }
