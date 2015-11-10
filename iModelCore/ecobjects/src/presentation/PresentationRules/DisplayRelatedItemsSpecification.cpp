/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/DisplayRelatedItemsSpecification.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>
#include <ECPresentationRules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_EC

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
DisplayRelatedItemsSpecification::DisplayRelatedItemsSpecification (bool logicalChildren, int nestingDepth, WStringCR relationshipClasses)
    : m_logicalChildren (logicalChildren), m_nestingDepth (nestingDepth), m_relationshipClasses (relationshipClasses)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayRelatedItemsSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

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
        m_relationshipClasses = L"";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    dmitrijus.tiazlovas              11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayRelatedItemsSpecification::WriteXml (BeXmlNodeP parentXmlNode)
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
WStringCR DisplayRelatedItemsSpecification::GetRelationshipClasses (void) const { return m_relationshipClasses; }
