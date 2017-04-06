/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/CalculatedPropertySpecification.cpp $
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
* @bsimethod                                    Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool CalculatedPropertiesSpecification::ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_label, CALCULATED_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_LABEL))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeInt32Value(m_priority, CALCULATED_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;

    if (BEXML_Success != xmlNode->GetContent(m_value))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tautvydas.Zinys                 10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CalculatedPropertiesSpecification::WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP relatedPropertiesNode = parentXmlNode->AddElementStringValue(CALCULATED_PROPERTIES_SPECIFICATION_XML_CHILD_NAME, m_value.c_str());
    relatedPropertiesNode->AddAttributeStringValue(CALCULATED_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_LABEL, m_label.c_str());
    relatedPropertiesNode->AddAttributeInt32Value(CALCULATED_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_PRIORITY, m_priority);
    }