/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/HiddenPropertiesSpecification.cpp $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/CommonTools.h>
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool HiddenPropertiesSpecification::ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_fullClassName, HIDDEN_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_propertyNames, HIDDEN_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAME))
        return false;
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HiddenPropertiesSpecification::WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP relatedPropertiesNode = parentXmlNode->AddEmptyElement(HIDDEN_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    relatedPropertiesNode->AddAttributeStringValue(HIDDEN_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME, m_fullClassName.c_str());
    relatedPropertiesNode->AddAttributeStringValue(HIDDEN_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAME, m_propertyNames.c_str());
    }