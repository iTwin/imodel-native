/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceSpecification::ShallowEqual(RelatedInstanceSpecificationCR other) const
    {
    return m_isRequired == other.m_isRequired
        && m_direction == other.m_direction
        && m_className == other.m_className
        && m_relationshipName == other.m_relationshipName
        && m_alias == other.m_alias;
    }

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
    m_direction = CommonToolsInternal::ParseRequiredDirectionString(requiredDirectionString.c_str());

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
    node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPDIRECTION, CommonToolsInternal::FormatRequiredDirectionString(m_direction));
    node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS, m_alias.c_str());
    node->AddAttributeBooleanValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ISREQUIRED, m_isRequired);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                   04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceSpecification::ReadJson(JsonValueCR json)
    {
    //Required
    m_className = CommonToolsInternal::SchemaAndClassNameToString(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS]);
    if (m_className.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "RelatedInstanceSpecification", RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS);
        return false;
        }

    m_relationshipName = CommonToolsInternal::SchemaAndClassNameToString(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP]);
    if (m_relationshipName.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "RelatedInstanceSpecification", RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP);
        return false;
        }

    JsonValueCR aliasJson = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS];
    if (aliasJson.isNull() || !aliasJson.isConvertibleTo(Json::ValueType::stringValue))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "RelatedInstanceSpecification", RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS);
        return false;
        }
    m_alias = aliasJson.asCString();

    //Optional
    m_isRequired = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED].asBool(false);
    m_direction = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value RelatedInstanceSpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    if (!m_className.empty())
        json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS] = CommonToolsInternal::SchemaAndClassNameToJson(m_className);
    if (!m_relationshipName.empty())
        json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP] = CommonToolsInternal::SchemaAndClassNameToJson(m_relationshipName);
    json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS] = m_alias;
    if (m_isRequired)
        json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED] = m_isRequired;
    if (RequiredRelationDirection_Both != m_direction)
        json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION] = CommonToolsInternal::FormatRequiredDirectionString(m_direction);
    return json;
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
