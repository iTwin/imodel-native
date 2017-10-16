/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/CalculatedPropertySpecification.cpp $
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 CalculatedPropertiesSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(m_label.c_str(), m_label.size());
    md5.Add(m_value.c_str(), m_value.size());
    md5.Add(&m_priority, sizeof(m_priority));
    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));
    return md5;
    }
