/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/PropertyEditorsSpecification.cpp $
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
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorsSpecification::ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_propertyName, PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAME))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_editorName, PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_EDITORNAME))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorsSpecification::WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP editorNode = parentXmlNode->AddEmptyElement(PROPERTY_EDITORS_SPECIFICATION_XML_CHILD_NAME);
    editorNode->AddAttributeStringValue(PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAME, m_propertyName.c_str());
    editorNode->AddAttributeStringValue(PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_EDITORNAME, m_editorName.c_str());
    }