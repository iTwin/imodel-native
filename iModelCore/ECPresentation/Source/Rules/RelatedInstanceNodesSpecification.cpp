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
#include <ECPresentation/Rules/RelationshipPathSpecification.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification()
    : ChildNodeSpecification(), m_groupByClass(true), m_groupByRelationship(false), m_groupByLabel(true),
    m_showEmptyGroups(false), m_skipRelatedLevel(0), m_requiredDirection(RequiredRelationDirection_Both)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification(RelatedInstanceNodesSpecification const& other)
    : ChildNodeSpecification(other), m_groupByClass(other.m_groupByClass), m_showEmptyGroups(other.m_showEmptyGroups),
    m_groupByRelationship(other.m_groupByRelationship), m_groupByLabel(other.m_groupByLabel), m_skipRelatedLevel(other.m_skipRelatedLevel),
    m_instanceFilter(other.m_instanceFilter), m_supportedSchemas(other.m_supportedSchemas), m_requiredDirection(other.m_requiredDirection),
    m_relationshipClassNames(other.m_relationshipClassNames), m_relatedClassNames(other.m_relatedClassNames)
    {
    CommonToolsInternal::CopyRules(m_relationshipPaths, other.m_relationshipPaths, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification(RelatedInstanceNodesSpecification&& other)
    : ChildNodeSpecification(std::move(other)), m_groupByClass(other.m_groupByClass), m_showEmptyGroups(other.m_showEmptyGroups),
    m_groupByRelationship(other.m_groupByRelationship), m_groupByLabel(other.m_groupByLabel), m_skipRelatedLevel(other.m_skipRelatedLevel),
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
    m_groupByRelationship = groupByRelationship;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy,
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel, int skipRelatedLevel, Utf8String instanceFilter,
    RequiredRelationDirection requiredDirection, Utf8String supportedSchemas, Utf8String relationshipClassNames, Utf8String relatedClassNames)
    : ChildNodeSpecification(priority, hasChildren, hideNodesInHierarchy, hideIfNoChildren), m_groupByClass(groupByClass), m_showEmptyGroups(false),
    m_groupByRelationship(false), m_groupByLabel(groupByLabel), m_skipRelatedLevel(skipRelatedLevel), m_instanceFilter(instanceFilter),
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
bool RelatedInstanceNodesSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ChildNodeSpecification::_ShallowEqual(other))
        return false;

    RelatedInstanceNodesSpecificationCP otherRule = dynamic_cast<RelatedInstanceNodesSpecificationCP>(&other);
    if (nullptr == otherRule)
        return false;

    return m_groupByClass == otherRule->m_groupByClass
        && m_groupByLabel == otherRule->m_groupByLabel
        && m_groupByRelationship == otherRule->m_groupByRelationship
        && m_requiredDirection == otherRule->m_requiredDirection
        && m_showEmptyGroups == otherRule->m_showEmptyGroups
        && m_skipRelatedLevel == otherRule->m_skipRelatedLevel
        && m_relatedClassNames == otherRule->m_relatedClassNames
        && m_relationshipClassNames == otherRule->m_relationshipClassNames
        && m_supportedSchemas == otherRule->m_supportedSchemas
        && m_instanceFilter == otherRule->m_instanceFilter
        && m_relationshipPaths.size() == otherRule->m_relationshipPaths.size()
        && std::equal(m_relationshipPaths.begin(), m_relationshipPaths.end(), otherRule->m_relationshipPaths.begin(),
            [](RepeatableRelationshipPathSpecification const* lhs, RepeatableRelationshipPathSpecification const* rhs){return *lhs == *rhs;});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RelatedInstanceNodesSpecification::_GetXmlElementName () const
    {
    return RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceNodesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ChildNodeSpecification::_ReadXml(xmlNode))
        return false;

    // optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByClass, COMMON_XML_ATTRIBUTE_GROUPBYCLASS))
        m_groupByClass = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByRelationship, COMMON_XML_ATTRIBUTE_GROUPBYRELATIONSHIP))
        m_groupByRelationship = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByLabel, COMMON_XML_ATTRIBUTE_GROUPBYLABEL))
        m_groupByLabel = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_showEmptyGroups, COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS))
        m_showEmptyGroups = false;

    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_skipRelatedLevel, COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL))
        m_skipRelatedLevel = 0;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_instanceFilter, COMMON_XML_ATTRIBUTE_INSTANCEFILTER))
        m_instanceFilter = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_supportedSchemas, COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS))
        m_supportedSchemas = "";

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
void RelatedInstanceNodesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ChildNodeSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYRELATIONSHIP, m_groupByRelationship);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS, m_showEmptyGroups);
    xmlNode->AddAttributeInt32Value   (COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL, m_skipRelatedLevel);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str ());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS, m_supportedSchemas.c_str ());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES, m_relationshipClassNames.c_str ());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES, m_relatedClassNames.c_str ());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonToolsInternal::FormatRequiredDirectionString (m_requiredDirection));
    }

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
bool RelatedInstanceNodesSpecification::_ReadJson(JsonValueCR json)
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
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid `%s` specification - either `%s` or `%s` must be specified, but both are empty.",
                _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_RELATIONSHIPS, COMMON_JSON_ATTRIBUTE_RELATEDCLASSES));
            hasIssues = true;
            }
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_WARNING, Utf8PrintfString("Using deprecated attributes of `%s`: `%s`, `%s`, `%s`, `%s`, `%s`. It's recommended to switch to `%s`.",
            _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL, CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ISRECURSIVE, COMMON_JSON_ATTRIBUTE_RELATIONSHIPS,
            COMMON_JSON_ATTRIBUTE_RELATEDCLASSES, COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION, COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS));
        }
    else
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString("Missing required `%s` attribute: `%s`.", _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS));
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
void RelatedInstanceNodesSpecification::_WriteJson(JsonValueR json) const
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
        json[COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS] = CommonToolsInternal::SupportedSchemasToJson(m_supportedSchemas);
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
MD5 RelatedInstanceNodesSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_groupByClass)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    if (m_groupByRelationship)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_XML_ATTRIBUTE_GROUPBYRELATIONSHIP, m_groupByRelationship);
    if (!m_groupByLabel)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    if (m_showEmptyGroups)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS, m_showEmptyGroups);
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
