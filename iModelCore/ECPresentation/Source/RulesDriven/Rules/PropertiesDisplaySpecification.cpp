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
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertiesDisplaySpecification::ReadJson(JsonValueCR json)
    {
    //Required
    m_propertyNames = json[PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES].asCString("");
    if (m_propertyNames.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, PROPERTIES_DISPLAY_SPECIFICATION_JSON_NAME, PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES);
        return false;
        }

    //Optional
    m_isDisplayed = json[PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED].asBool(true);
    m_priority = json[PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY].asInt(1000);

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
