/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/RelatedInstanceSpecification.cpp $
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
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceSpecification::ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_relationshipName, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_className, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_alias, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS);
        return false;
        }
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_isRequired, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ISREQUIRED))
        m_isRequired = false;
    
    Utf8String requiredDirectionString;
    if (BEXML_Success != xmlNode->GetAttributeStringValue (requiredDirectionString, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPDIRECTION))
        requiredDirectionString = "";
    m_direction = CommonTools::ParseRequiredDirectionString(requiredDirectionString.c_str());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                   04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceSpecification::ReadJson(JsonValueCR json)
    {
    //Required
    JsonValueCR classNameJson = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASSNAME];
    if (classNameJson.isNull() || !classNameJson.isConvertibleTo(Json::ValueType::stringValue))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, RELATED_INSTANCE_SPECIFICATION_JSON_NAME, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASSNAME);
        return false;
        }
    m_className = classNameJson.asCString();

    JsonValueCR relationshipNameJson = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPNAME];
    if (relationshipNameJson.isNull() || !relationshipNameJson.isConvertibleTo(Json::ValueType::stringValue))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, RELATED_INSTANCE_SPECIFICATION_JSON_NAME, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPNAME);
        return false;
        }
    m_relationshipName = relationshipNameJson.asCString();

    JsonValueCR aliasJson = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS];
    if (aliasJson.isNull() || !aliasJson.isConvertibleTo(Json::ValueType::stringValue))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, RELATED_INSTANCE_SPECIFICATION_JSON_NAME, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPNAME);
        return false;
        }
    m_alias = aliasJson.asCString();

    //Optional
    m_isRequired = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED].asBool(false);
    m_direction = CommonTools::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceSpecification::WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP node = parentXmlNode->AddEmptyElement(RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME);
    node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME, m_className.c_str());
    node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME, m_relationshipName.c_str());
    node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPDIRECTION, CommonTools::FormatRequiredDirectionString(m_direction));
    node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS, m_alias.c_str());
    node->AddAttributeBooleanValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ISREQUIRED, m_isRequired);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelatedInstanceSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(&m_direction, sizeof(m_direction));
    md5.Add(m_relationshipName.c_str(), m_relationshipName.size());
    md5.Add(m_className.c_str(), m_className.size());
    md5.Add(m_alias.c_str(), m_alias.size());
    md5.Add(&m_isRequired, sizeof(m_isRequired));
    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));

    return md5;
    }
