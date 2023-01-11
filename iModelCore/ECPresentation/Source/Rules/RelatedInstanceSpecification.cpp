/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RelatedInstanceSpecification::_GetXmlElementName() const {return RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceSpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PresentationKey::_ReadXml(xmlNode))
        return false;

    Utf8String relationshipName;
    if (BEXML_Success != xmlNode->GetAttributeStringValue(relationshipName, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString(INVALID_XML, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME));
        return false;
        }

    Utf8String className;
    if (BEXML_Success != xmlNode->GetAttributeStringValue(className, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString(INVALID_XML, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME));
        return false;
        }

    Utf8String requiredDirectionString;
    if (BEXML_Success != xmlNode->GetAttributeStringValue(requiredDirectionString, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPDIRECTION))
        requiredDirectionString = "";
    RequiredRelationDirection direction = CommonToolsInternal::ParseRequiredDirectionString(requiredDirectionString.c_str(), _GetXmlElementName());
    m_relationshipPath.AddStep(*new RelationshipStepSpecification(relationshipName, direction, className));

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_alias, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString(INVALID_XML, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS));
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_isRequired, RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ISREQUIRED))
        m_isRequired = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceSpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    PresentationKey::_WriteXml(xmlNode);
    if (!m_relationshipPath.GetSteps().empty())
        {
        xmlNode->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME, m_relationshipPath.GetSteps().front()->GetTargetClassName().c_str());
        xmlNode->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME, m_relationshipPath.GetSteps().front()->GetRelationshipClassName().c_str());
        xmlNode->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPDIRECTION, CommonToolsInternal::FormatRequiredDirectionString(m_relationshipPath.GetSteps().front()->GetRelationDirection()));
        }
    xmlNode->AddAttributeStringValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS, m_alias.c_str());
    xmlNode->AddAttributeBooleanValue(RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ISREQUIRED, m_isRequired);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RelatedInstanceSpecification::_GetJsonElementType() const { return "RelatedInstanceSpecification"; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceSpecification::_ReadJson(JsonValueCR json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    // required:
    bool hasIssues = false;
    if (json.isMember(RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH))
        {
        m_relationshipPath.ReadJson(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH]);
        hasIssues |= CommonToolsInternal::CheckRuleIssue(m_relationshipPath.GetSteps().empty(), _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH, json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH], "non-empty relationship path");
        }
    else if (json.isMember(RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS) && json.isMember(RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP))
        {
        Utf8String className = CommonToolsInternal::SchemaAndClassNameToString(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS], Utf8PrintfString("%s.%s", _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS).c_str());
        Utf8String relationshipName = CommonToolsInternal::SchemaAndClassNameToString(json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP], Utf8PrintfString("%s.%s", _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP).c_str());
        RequiredRelationDirection direction = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""), _GetJsonElementType());
        hasIssues |= false
            || CommonToolsInternal::CheckRuleIssue(className.empty(), _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS, json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS], "class specification")
            || CommonToolsInternal::CheckRuleIssue(relationshipName.empty(), _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP, json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP], "relationship specification");
        if (!hasIssues)
            m_relationshipPath.AddStep(*new RelationshipStepSpecification(relationshipName, direction, className));
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_WARNING, Utf8PrintfString("Using deprecated attributes of `%s`: `%s`, `%s`, `%s`. It's recommended to switch to `%s`.",
            _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP, COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION,
            RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH));
        }
    else
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString("Missing required `%s` attribute: `%s`.", _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH));
        hasIssues = true;
        }

    m_alias = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS].asCString("");
    hasIssues |= CommonToolsInternal::CheckRuleIssue(m_alias.empty(), _GetJsonElementType(), RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS, json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS], "non-empty string");

    if (hasIssues)
        return false;

    // optional:
    m_isRequired = json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED].asBool(false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceSpecification::_WriteJson(JsonValueR json) const
    {
    PresentationKey::_WriteJson(json);
    json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH] = m_relationshipPath.WriteJson();
    json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS] = m_alias;
    if (m_isRequired)
        json[RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED] = m_isRequired;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelatedInstanceSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_relationshipPath.GetSteps().empty())
        ADD_STR_VALUE_TO_HASH(md5, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH, m_relationshipPath.GetHash());
    if (!m_alias.empty())
        ADD_STR_VALUE_TO_HASH(md5, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS, m_alias);
    if (m_isRequired)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED, m_isRequired);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    RelatedInstanceSpecification const* otherRule = dynamic_cast<RelatedInstanceSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_isRequired == otherRule->m_isRequired
        && m_alias == otherRule->m_alias
        && m_relationshipPath == otherRule->m_relationshipPath;
    }
