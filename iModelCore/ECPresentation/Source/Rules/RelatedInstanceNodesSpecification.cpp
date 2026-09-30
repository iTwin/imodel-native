/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>
#include <ECPresentation/Rules/RelationshipPathSpecification.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification()
    : ChildNodeSpecification(), m_groupByClass(true), m_groupByLabel(true),
    m_skipRelatedLevel(0), m_requiredDirection(RequiredRelationDirection_Both)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification(RelatedInstanceNodesSpecification const& other)
    : ChildNodeSpecification(other), m_groupByClass(other.m_groupByClass),
    m_groupByLabel(other.m_groupByLabel), m_skipRelatedLevel(other.m_skipRelatedLevel),
    m_instanceFilter(other.m_instanceFilter), m_supportedSchemas(other.m_supportedSchemas), m_requiredDirection(other.m_requiredDirection),
    m_relationshipClassNames(other.m_relationshipClassNames), m_relatedClassNames(other.m_relatedClassNames)
    {
    CommonToolsInternal::CopyRules(m_relationshipPaths, other.m_relationshipPaths, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification(RelatedInstanceNodesSpecification&& other)
    : ChildNodeSpecification(std::move(other)), m_groupByClass(other.m_groupByClass),
    m_groupByLabel(other.m_groupByLabel), m_skipRelatedLevel(other.m_skipRelatedLevel),
    m_instanceFilter(std::move(other.m_instanceFilter)), m_supportedSchemas(std::move(other.m_supportedSchemas)), m_requiredDirection(other.m_requiredDirection),
    m_relationshipClassNames(std::move(other.m_relationshipClassNames)), m_relatedClassNames(std::move(other.m_relatedClassNames))
    {
    m_relationshipPaths.swap(other.m_relationshipPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification(int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy,
    bool hideIfNoChildren, bool groupByClass, bool groupByRelationship, bool groupByLabel, bool, int skipRelatedLevel,
    Utf8String instanceFilter, RequiredRelationDirection requiredDirection, Utf8String supportedSchemas, Utf8String relationshipClassNames,
    Utf8String relatedClassNames)
    : RelatedInstanceNodesSpecification(priority, alwaysReturnsChildren ? ChildrenHint::Always : ChildrenHint::Unknown, hideNodesInHierarchy, hideIfNoChildren,
    groupByClass, groupByLabel, skipRelatedLevel, instanceFilter, requiredDirection, supportedSchemas, relationshipClassNames, relatedClassNames)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy,
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel, int skipRelatedLevel, Utf8String instanceFilter,
    RequiredRelationDirection requiredDirection, Utf8String supportedSchemas, Utf8String relationshipClassNames, Utf8String relatedClassNames)
    : ChildNodeSpecification(priority, hasChildren, hideNodesInHierarchy, hideIfNoChildren), m_groupByClass(groupByClass),
    m_groupByLabel(groupByLabel), m_skipRelatedLevel(skipRelatedLevel), m_instanceFilter(instanceFilter),
    m_supportedSchemas(supportedSchemas), m_requiredDirection(requiredDirection), m_relationshipClassNames(relationshipClassNames), m_relatedClassNames(relatedClassNames)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::~RelatedInstanceNodesSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_relationshipPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::SetRelationshipPaths(bvector<RepeatableRelationshipPathSpecification*> paths)
    {
    CommonToolsInternal::FreePresentationRules(m_relationshipPaths);
    m_relationshipPaths = paths;
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RelatedInstanceNodesSpecification::_GetJsonElementType() const
    {
    return RELATED_INSTANCE_NODES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceNodesSpecification::_ReadJson(BeJsConst json)
    {
    if (!ChildNodeSpecification::_ReadJson(json))
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
        m_supportedSchemas = CommonToolsInternal::SupportedSchemasToString(json[COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS]);
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
    m_groupByClass = json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS].asBool(true);
    m_groupByLabel = json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL].asBool(true);
    m_instanceFilter = json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER].asCString("");

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::_WriteJson(BeJsValue json) const
    {
    ChildNodeSpecification::_WriteJson(json);
    if (!m_groupByClass)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS] = m_groupByClass;
    if (!m_groupByLabel)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL] = m_groupByLabel;
    if (0 != m_skipRelatedLevel)
        json[COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL] = m_skipRelatedLevel;
    if (!m_instanceFilter.empty())
        json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER] = m_instanceFilter;
    if (!m_supportedSchemas.empty())
        CommonToolsInternal::WriteSupportedSchemasToJson(json[COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS], m_supportedSchemas);
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
MD5 RelatedInstanceNodesSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_groupByClass)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    if (!m_groupByLabel)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    if (m_skipRelatedLevel != 0)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL, m_skipRelatedLevel);
    if (!m_instanceFilter.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter);
    if (m_requiredDirection != RequiredRelationDirection_Both)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION, m_requiredDirection);
    if (!m_supportedSchemas.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS, m_supportedSchemas);
    if (!m_relationshipClassNames.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_RELATIONSHIPS, m_relationshipClassNames);
    if (!m_relatedClassNames.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_RELATEDCLASSES, m_relatedClassNames);
    ADD_RULES_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS, m_relationshipPaths);
    return md5;
    }
