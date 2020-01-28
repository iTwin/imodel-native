/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include <ECPresentation/RulesDriven/Rules/RelationshipPathSpecification.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification()
    : ChildNodeSpecification(), m_groupByClass(true), m_groupByRelationship(false), m_groupByLabel(true),
    m_showEmptyGroups(false), m_skipRelatedLevel(0), m_requiredDirection(RequiredRelationDirection_Both)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
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
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification(RelatedInstanceNodesSpecification&& other)
    : ChildNodeSpecification(other), m_groupByClass(other.m_groupByClass), m_showEmptyGroups(other.m_showEmptyGroups),
    m_groupByRelationship(other.m_groupByRelationship), m_groupByLabel(other.m_groupByLabel), m_skipRelatedLevel(other.m_skipRelatedLevel),
    m_instanceFilter(std::move(other.m_instanceFilter)), m_supportedSchemas(std::move(other.m_supportedSchemas)), m_requiredDirection(other.m_requiredDirection),
    m_relationshipClassNames(std::move(other.m_relationshipClassNames)), m_relatedClassNames(std::move(other.m_relatedClassNames))
    {
    m_relationshipPaths.swap(other.m_relationshipPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
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
* @bsimethod                                    Grigas.Petraitis                10/2018
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
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::~RelatedInstanceNodesSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_relationshipPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceNodesSpecification::_ShallowEqual(PresentationRuleSpecification const& other) const
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
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RelatedInstanceNodesSpecification::_GetXmlElementName () const
    {
    return RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceNodesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ChildNodeSpecification::_ReadXml(xmlNode))
        return false;

    //Optional:
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
        m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString (requiredDirectionString.c_str ());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
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
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RelatedInstanceNodesSpecification::_GetJsonElementType() const
    {
    return RELATED_INSTANCE_NODES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceNodesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ChildNodeSpecification::_ReadJson(json))
        return false;

    if (json.isMember(COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS))
        {
        CommonToolsInternal::LoadFromJson(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS], m_relationshipPaths, &CommonToolsInternal::LoadRuleFromJson<RepeatableRelationshipPathSpecification>, this);
        }
    else
        {
        // all of this is deprecated in favor of relationship paths:
        m_skipRelatedLevel = json[COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL].asInt(0);
        m_supportedSchemas = CommonToolsInternal::SupportedSchemasToString(json[COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS]);
        m_relationshipClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS]);
        m_relatedClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES]);
        m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""));
        }
    m_groupByClass = json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS].asBool(true);
    m_groupByLabel = json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL].asBool(true);
    m_instanceFilter = json[COMMON_JSON_ATTRIBUTE_INSTANCEFILTER].asCString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
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
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelatedInstanceNodesSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ChildNodeSpecification::_ComputeHash(parentHash);
    md5.Add(&m_groupByClass, sizeof(m_groupByClass));
    md5.Add(&m_groupByRelationship, sizeof(m_groupByRelationship));
    md5.Add(&m_groupByLabel, sizeof(m_groupByLabel));
    md5.Add(&m_showEmptyGroups, sizeof(m_showEmptyGroups));
    md5.Add(&m_skipRelatedLevel, sizeof(m_skipRelatedLevel));
    md5.Add(m_instanceFilter.c_str(), m_instanceFilter.size());
    md5.Add(&m_requiredDirection, sizeof(m_requiredDirection));
    md5.Add(m_supportedSchemas.c_str(), m_supportedSchemas.size());
    md5.Add(m_relationshipClassNames.c_str(), m_relationshipClassNames.size());
    md5.Add(m_relatedClassNames.c_str(), m_relatedClassNames.size());

    Utf8String currentHash = md5.GetHashString();
    for (RepeatableRelationshipPathSpecification const* spec : m_relationshipPaths)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }

    return md5;
    }
