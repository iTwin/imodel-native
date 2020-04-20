/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
        && m_alias == other.m_alias
        && m_relationshipPath == other.m_relationshipPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceSpecification::ReadXml(BeXmlNodeP xmlNode)
    {
    Utf8String relationshipName;
    if (BEXML_Success != xmlNode->GetAttributeStringValue(relationshipName, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME);
        return false;
        }

    Utf8String className;
    if (BEXML_Success != xmlNode->GetAttributeStringValue(className, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME);
        return false;
        }

    Utf8String requiredDirectionString;
    if (BEXML_Success != xmlNode->GetAttributeStringValue(requiredDirectionString, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPDIRECTION))
        requiredDirectionString = "";
    RequiredRelationDirection direction = CommonToolsInternal::ParseRequiredDirectionString(requiredDirectionString.c_str());
    m_relationshipPath.AddStep(*new RelationshipStepSpecification(relationshipName, direction, className));

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_alias, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_isRequired, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ISREQUIRED))
        m_isRequired = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceSpecification::WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP node = parentXmlNode->AddEmptyElement(RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME);
    if (!m_relationshipPath.GetSteps().empty())
        {
        node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME, m_relationshipPath.GetSteps().front()->GetTargetClassName().c_str());
        node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME, m_relationshipPath.GetSteps().front()->GetRelationshipClassName().c_str());
        node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPDIRECTION, CommonToolsInternal::FormatRequiredDirectionString(m_relationshipPath.GetSteps().front()->GetRelationDirection()));
        }
    node->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS, m_alias.c_str());
    node->AddAttributeBooleanValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ISREQUIRED, m_isRequired);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                   04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceSpecification::ReadJson(JsonValueCR json)
    {
    if (json.isMember(RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH))
        {
        if (!m_relationshipPath.ReadJson(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH]))
            {
            ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "RelatedInstanceSpecification", RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH);
            return false;
            }
        }
    else
        {
        Utf8String className = CommonToolsInternal::SchemaAndClassNameToString(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS]);
        if (className.empty())
            {
            ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "RelatedInstanceSpecification", RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS);
            return false;
            }
        Utf8String relationshipName = CommonToolsInternal::SchemaAndClassNameToString(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP]);
        if (relationshipName.empty())
            {
            ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "RelatedInstanceSpecification", RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP);
            return false;
            }
        RequiredRelationDirection direction = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""));
        m_relationshipPath.AddStep(*new RelationshipStepSpecification(relationshipName, direction, className));
        }

    JsonValueCR aliasJson = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS];
    if (aliasJson.isNull() || !aliasJson.isConvertibleTo(Json::ValueType::stringValue))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "RelatedInstanceSpecification", RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS);
        return false;
        }
    m_alias = aliasJson.asCString();

    m_isRequired = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED].asBool(false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value RelatedInstanceSpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH] = m_relationshipPath.WriteJson();
    json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS] = m_alias;
    if (m_isRequired)
        json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED] = m_isRequired;
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelatedInstanceSpecification::_ComputeHash() const
    {
    MD5 md5;
    md5.Add(m_alias.c_str(), m_alias.size());
    md5.Add(&m_isRequired, sizeof(m_isRequired));

    Utf8StringCR relationshipPathHash = m_relationshipPath.GetHash();
    md5.Add(relationshipPathHash.c_str(), relationshipPathHash.size());

    return md5;
    }
