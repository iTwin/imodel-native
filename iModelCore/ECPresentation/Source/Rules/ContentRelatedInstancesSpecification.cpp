/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
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
Utf8CP ContentRelatedInstancesSpecification::_GetJsonElementType() const
    {
    return CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentRelatedInstancesSpecification::_ReadJson(BeJsConst json)
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
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Invalid `%s` specification - either `%s` or `%s` must be specified, but both are empty.",
                _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_RELATIONSHIPS, COMMON_JSON_ATTRIBUTE_RELATEDCLASSES));
            hasIssues = true;
            }
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_WARNING, Utf8PrintfString("Using deprecated attributes of `%s`: `%s`, `%s`, `%s`, `%s`, `%s`. It's recommended to switch to `%s`.",
            _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL, CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ISRECURSIVE, COMMON_JSON_ATTRIBUTE_RELATIONSHIPS,
            COMMON_JSON_ATTRIBUTE_RELATEDCLASSES, COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION, COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS));
        }
    else
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Missing required `%s` attribute: `%s`.", _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS));
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
void ContentRelatedInstancesSpecification::_WriteJson(BeJsValue json) const
    {
    ContentSpecification::_WriteJson(json);
    if (0 != m_skipRelatedLevel)
        json[COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL] = m_skipRelatedLevel;
    if (m_isRecursive)
        json[CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ISRECURSIVE] = m_isRecursive;
    if (!m_instanceFilter.empty())
        json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER] = m_instanceFilter;
    if (!m_relationshipClassNames.empty())
        CommonToolsInternal::WriteSchemaAndClassNamesToJson(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS], m_relationshipClassNames);
    if (!m_relatedClassNames.empty())
        CommonToolsInternal::WriteSchemaAndClassNamesToJson(json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES], m_relatedClassNames);
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
