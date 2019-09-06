/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
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
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME, CALCULATED_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_LABEL);
        return false;
        }
    if (BEXML_Success != xmlNode->GetAttributeInt32Value(m_priority, CALCULATED_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;

    if (BEXML_Success != xmlNode->GetContent(m_value) || m_value.empty())
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
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CalculatedPropertiesSpecification::ReadJson(JsonValueCR json)
    {
    //Required
    m_value = json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_VALUE].asCString("");
    if (m_value.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "CalculatedPropertiesSpecification", CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_VALUE);
        return false;
        }

    JsonValueCR jsonValue = json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_LABEL];
    if (jsonValue.isNull() || !jsonValue.isConvertibleTo(Json::ValueType::stringValue))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "CalculatedPropertiesSpecification", CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_LABEL);
        return false;
        }
    m_label = jsonValue.asCString("");

    //Optional
    m_priority = json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY].asInt(1000);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CalculatedPropertiesSpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_VALUE] = m_value;
    json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_LABEL] = m_label;
    if (1000 != m_priority)
        json[CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY] = m_priority;
    return json;
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
