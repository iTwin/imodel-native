/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/DisplayRelatedItemsSpecification.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

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