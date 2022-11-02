/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRelatedInstancesSpecification::ContentRelatedInstancesSpecification ()
    : ContentSpecification(), m_skipRelatedLevel(0), m_isRecursive(false), m_requiredDirection(RequiredRelationDirection_Both)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRelatedInstancesSpecification::ContentRelatedInstancesSpecification(ContentRelatedInstancesSpecification const& other)
    : ContentSpecification(other), m_skipRelatedLevel(other.m_skipRelatedLevel), m_isRecursive(other.m_isRecursive),
    m_instanceFilter(other.m_instanceFilter), m_requiredDirection(other.m_requiredDirection),
    m_relationshipClassNames(other.m_relationshipClassNames), m_relatedClassNames(other.m_relatedClassNames)
    {
    CommonToolsInternal::CopyRules(m_relationshipPaths, other.m_relationshipPaths, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRelatedInstancesSpecification::ContentRelatedInstancesSpecification(ContentRelatedInstancesSpecification&& other)
    : ContentSpecification(std::move(other)), m_skipRelatedLevel(other.m_skipRelatedLevel), m_isRecursive(other.m_isRecursive),
    m_instanceFilter(std::move(other.m_instanceFilter)), m_requiredDirection(other.m_requiredDirection),
    m_relationshipClassNames(std::move(other.m_relationshipClassNames)), m_relatedClassNames(std::move(other.m_relatedClassNames))
    {
    m_relationshipPaths.swap(other.m_relationshipPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRelatedInstancesSpecification::ContentRelatedInstancesSpecification(int priority, int skipRelatedLevel, bool isRecursive,
    Utf8String instanceFilter, RequiredRelationDirection requiredDirection, Utf8String relationshipClassNames, Utf8String relatedClassNames)
    : ContentSpecification (priority), m_skipRelatedLevel (skipRelatedLevel), m_isRecursive(isRecursive), m_instanceFilter (instanceFilter),
    m_requiredDirection (requiredDirection), m_relationshipClassNames (relationshipClassNames), m_relatedClassNames (relatedClassNames)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRelatedInstancesSpecification::~ContentRelatedInstancesSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_relationshipPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentRelatedInstancesSpecification::_GetXmlElementName () const
    {
    return CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRelatedInstancesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ContentSpecification::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_skipRelatedLevel, COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL))
        m_skipRelatedLevel = 0;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_isRecursive, CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ISRECURSIVE))
        m_isRecursive = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_instanceFilter, COMMON_XML_ATTRIBUTE_INSTANCEFILTER))
        m_instanceFilter = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relationshipClassNames, COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES))
        m_relationshipClassNames = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relatedClassNames, COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES))
        m_relatedClassNames = "";

    Utf8String requiredDirectionString = "";
    if (BEXML_Success != xmlNode->GetAttributeStringValue (requiredDirectionString, COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION))
        requiredDirectionString = "";
    else
        m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString (requiredDirectionString.c_str (), _GetXmlElementName());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ContentSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeInt32Value  (COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL, m_skipRelatedLevel);
    xmlNode->AddAttributeBooleanValue (CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ISRECURSIVE, m_isRecursive);
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES, m_relationshipClassNames.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES, m_relatedClassNames.c_str ());
    xmlNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonToolsInternal::FormatRequiredDirectionString (m_requiredDirection));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentRelatedInstancesSpecification::_GetJsonElementType() const
    {
    return CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRelatedInstancesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ContentSpecification::_ReadJson(json))
        return false;

    // required:
    bool hasIssues = false;
    if (json.isMember(COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS))
        {
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS, json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS], m_relationshipPaths, &CommonToolsInternal::LoadRuleFromJson<RepeatableRelationshipPathSpecification>, this);
        hasIssues |= CommonToolsInternal::CheckRuleIssue(m_relationshipPaths.empty(), _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS, json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS], "at least one relationship path");
        }
    else if (json.isMember(COMMON_JSON_ATTRIBUTE_RELATIONSHIPS) || json.isMember(COMMON_JSON_ATTRIBUTE_RELATEDCLASSES))
        {
        m_skipRelatedLevel = json[COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL].asInt(0);
        m_isRecursive = json[CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ISRECURSIVE].asBool(false);
        m_relationshipClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS]);
        m_relatedClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES]);
        m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""), _GetJsonElementType());
        if (m_relationshipClassNames.empty() && m_relatedClassNames.empty())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid `%s` specification - either `%s` or `%s` must be specified, but both are empty.",
                _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_RELATIONSHIPS, COMMON_JSON_ATTRIBUTE_RELATEDCLASSES));
            hasIssues = true;
            }
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_WARNING, Utf8PrintfString("Using deprecated attributes of `%s`: `%s`, `%s`, `%s`, `%s`, `%s`. It's recommended to switch to `%s`.",
            _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL, CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ISRECURSIVE, COMMON_JSON_ATTRIBUTE_RELATIONSHIPS,
            COMMON_JSON_ATTRIBUTE_RELATEDCLASSES, COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION, COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS));
        }
    else
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Missing required `%s` attribute: `%s`.", _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS));
        hasIssues = true;
        }

    if (hasIssues)
        return false;

    // optional:
    m_instanceFilter = json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER].asCString("");

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::_WriteJson(JsonValueR json) const
    {
    ContentSpecification::_WriteJson(json);
    if (0 != m_skipRelatedLevel)
        json[COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL] = m_skipRelatedLevel;
    if (m_isRecursive)
        json[CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ISRECURSIVE] = m_isRecursive;
    if (!m_instanceFilter.empty())
        json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER] = m_instanceFilter;
    if (!m_relationshipClassNames.empty())
        json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS] = CommonToolsInternal::SchemaAndClassNamesToJson(m_relationshipClassNames);
    if (!m_relatedClassNames.empty())
        json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES] = CommonToolsInternal::SchemaAndClassNamesToJson(m_relatedClassNames);
    if (RequiredRelationDirection_Both != m_requiredDirection)
        json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION] = CommonToolsInternal::FormatRequiredDirectionString(m_requiredDirection);
    if (!m_relationshipPaths.empty())
        CommonToolsInternal::WriteRulesToJson<RepeatableRelationshipPathSpecification, bvector<RepeatableRelationshipPathSpecification*>>(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS], m_relationshipPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentRelatedInstancesSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_skipRelatedLevel != 0)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL, m_skipRelatedLevel);
    if (m_isRecursive)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ISRECURSIVE, m_isRecursive);
    if (!m_instanceFilter.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter);
    if (m_requiredDirection != RequiredRelationDirection_Both)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION, m_requiredDirection);
    if (!m_relationshipClassNames.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_RELATIONSHIPS, m_relationshipClassNames);
    if (!m_relatedClassNames.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_RELATEDCLASSES, m_relatedClassNames);
    ADD_RULES_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS, m_relationshipPaths);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRelatedInstancesSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ContentSpecification::_ShallowEqual(other))
        return false;

    ContentRelatedInstancesSpecification const* otherRule = dynamic_cast<ContentRelatedInstancesSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_skipRelatedLevel == otherRule->m_skipRelatedLevel
        && m_isRecursive == otherRule->m_isRecursive
        && m_instanceFilter == otherRule->m_instanceFilter
        && m_requiredDirection == otherRule->m_requiredDirection
        && m_relationshipClassNames == otherRule->m_relationshipClassNames
        && m_relatedClassNames == otherRule->m_relatedClassNames;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::ClearRelationshipPaths()
    {
    InvalidateHash();
    CommonToolsInternal::FreePresentationRules(m_relationshipPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentRelatedInstancesSpecification::AddRelationshipPath(RepeatableRelationshipPathSpecification& relationship)
    {
    ADD_HASHABLE_CHILD(m_relationshipPaths, relationship);
    }
