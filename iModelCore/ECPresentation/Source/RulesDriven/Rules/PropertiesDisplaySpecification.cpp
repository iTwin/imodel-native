/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/PropertiesDisplaySpecification.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/CommonTools.h>
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertiesDisplaySpecification::ReadXml(BeXmlNodeP xmlNode)
    {
    if (0 == strcmp(xmlNode->GetName(), DISPLAYED_PROPERTIES_SPECIFICATION_XML_NODE_NAME))
        m_isDisplayed = true;
    else
        m_isDisplayed = false;
    
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_propertyNames, PROPERTIES_DISPLAY_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAMES))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeInt32Value(m_priority, PROPERTIES_DISPLAY_SPECIFICATION_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertiesDisplaySpecification::WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP relatedPropertiesNode;
    if (m_isDisplayed)
        relatedPropertiesNode = parentXmlNode->AddEmptyElement(DISPLAYED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    else
        relatedPropertiesNode = parentXmlNode->AddEmptyElement(HIDDEN_PROPERTIES_SPECIFICATION_XML_NODE_NAME);

    relatedPropertiesNode->AddAttributeStringValue(PROPERTIES_DISPLAY_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAMES, m_propertyNames.c_str());
    relatedPropertiesNode->AddAttributeInt32Value(PROPERTIES_DISPLAY_SPECIFICATION_XML_ATTRIBUTE_PRIORITY, m_priority);
    }