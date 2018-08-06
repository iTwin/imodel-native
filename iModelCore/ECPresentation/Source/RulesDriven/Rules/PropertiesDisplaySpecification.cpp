/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/PropertiesDisplaySpecification.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

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
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, m_isDisplayed ? DISPLAYED_PROPERTIES_SPECIFICATION_XML_NODE_NAME : HIDDEN_PROPERTIES_SPECIFICATION_XML_NODE_NAME, PROPERTIES_DISPLAY_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAMES);
        return false;
        }
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertiesDisplaySpecification::ReadJson(JsonValueCR json)
    {
    //Required
    JsonValueCR propertyNamesJson = json[PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES];
    if (propertyNamesJson.isNull() || !propertyNamesJson.isArray() || 0 == propertyNamesJson.size())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertiesDisplaySpecification", PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES);
        return false;
        }
    for (Json::ArrayIndex i = 0; i < propertyNamesJson.size(); ++i)
        {
        if (!m_propertyNames.empty())
            m_propertyNames.append(",");
        m_propertyNames.append(propertyNamesJson[i].asCString());
        }

    //Optional
    m_isDisplayed = json[PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED].asBool(true);
    m_priority = json[PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY].asInt(1000);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PropertiesDisplaySpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    bvector<Utf8String> propertyNames;
    BeStringUtilities::Split(m_propertyNames.c_str(), ",", propertyNames);
    for (Utf8StringR propertyName : propertyNames)
        json[PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES].append(propertyName.Trim());
    if (!m_isDisplayed)
        json[PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED] = m_isDisplayed;
    if (1000 != m_priority)
        json[PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY] = m_priority;
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertiesDisplaySpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(m_propertyNames.c_str(), m_propertyNames.size());
    md5.Add(&m_priority, sizeof(m_priority));
    md5.Add(&m_isDisplayed, sizeof(m_isDisplayed));
    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));
    return md5;
    }
