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
ChildNodeSpecificationP ChildNodeSpecification::Create(JsonValueCR json)
    {
    Utf8CP type = json[COMMON_JSON_ATTRIBUTE_SPECTYPE].asCString("");
    ChildNodeSpecificationP spec = nullptr;
    if (0 == strcmp(ALL_INSTANCE_NODES_SPECIFICATION_JSON_TYPE, type))
        spec = new AllInstanceNodesSpecification();
    else if (0 == strcmp(ALL_RELATED_INSTANCE_NODES_SPECIFICATION_JSON_TYPE, type))
        spec = new AllRelatedInstanceNodesSpecification();
    else if (0 == strcmp(RELATED_INSTANCE_NODES_SPECIFICATION_JSON_TYPE, type))
        spec = new RelatedInstanceNodesSpecification();
    else if (0 == strcmp(INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE, type))
        spec = new InstanceNodesOfSpecificClassesSpecification();
    else if (0 == strcmp(CUSTOM_NODE_SPECIFICATION_JSON_TYPE, type))
        spec = new CustomNodeSpecification();
    else if (0 == strcmp(SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_TYPE, type))
        spec = new SearchResultInstanceNodesSpecification();
    else
        {
        Utf8String msg = json.isMember(COMMON_JSON_ATTRIBUTE_SPECTYPE)
            ? Utf8PrintfString("Invalid `" COMMON_JSON_ATTRIBUTE_SPECTYPE "` attribute value: `%s`", type)
            : Utf8String("Missing required attribute: `" COMMON_JSON_ATTRIBUTE_SPECTYPE "`");
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, msg);
        }
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::ChildNodeSpecification()
    : m_hasChildren(ChildrenHint::Unknown), m_hideNodesInHierarchy(false), m_hideIfNoChildren(false), m_doNotSort(false), m_suppressSimilarAncestorsCheck(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::ChildNodeSpecification (int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy, bool hideIfNoChildren)
    : PresentationRuleSpecification(priority), m_hasChildren(hasChildren), m_hideNodesInHierarchy (hideNodesInHierarchy), m_hideIfNoChildren (hideIfNoChildren), m_doNotSort (false), m_suppressSimilarAncestorsCheck(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::ChildNodeSpecification(ChildNodeSpecificationCR other)
    : PresentationRuleSpecification(other), m_hasChildren(other.m_hasChildren), m_suppressSimilarAncestorsCheck(other.m_suppressSimilarAncestorsCheck),
    m_hideNodesInHierarchy(other.m_hideNodesInHierarchy), m_hideIfNoChildren(other.m_hideIfNoChildren),
    m_doNotSort(other.m_doNotSort), m_hideExpression(other.m_hideExpression)
    {
    CommonToolsInternal::CopyRules(m_relatedInstances, other.m_relatedInstances, this);
    CommonToolsInternal::CopyRules(m_nestedRules, other.m_nestedRules, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecification::~ChildNodeSpecification ()
    {
    CommonToolsInternal::FreePresentationRules(m_relatedInstances);
    CommonToolsInternal::FreePresentationRules(m_nestedRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!PresentationRuleSpecification::_ShallowEqual(other))
        return false;

    ChildNodeSpecificationCP otherRule = dynamic_cast<ChildNodeSpecificationCP>(&other);
    if (nullptr == otherRule)
        return false;

    return m_doNotSort == otherRule->m_doNotSort
        && m_hasChildren == otherRule->m_hasChildren
        && m_hideExpression == otherRule->m_hideExpression
        && m_hideIfNoChildren == otherRule->m_hideIfNoChildren
        && m_hideNodesInHierarchy == otherRule->m_hideNodesInHierarchy
        && m_suppressSimilarAncestorsCheck == otherRule->m_suppressSimilarAncestorsCheck;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ChildrenHint ParseChildrenHint(Utf8CP str, Utf8CP attributeIdentifier)
    {
    if (0 == strcmp("Always", str))
        return ChildrenHint::Always;
    if (0 == strcmp("Never", str))
        return ChildrenHint::Never;
    if (0 == strcmp("Unknown", str))
        return ChildrenHint::Unknown;

    DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Invalid `%s` attribute value: `%s`. Expected \"Always\", \"Never\" or \"Unknown\".", attributeIdentifier, str));
    return ChildrenHint::Unknown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP ChildrenHintToString(ChildrenHint hint)
    {
    switch (hint)
        {
        case ChildrenHint::Always: return "Always";
        case ChildrenHint::Never: return "Never";
        default: return "Unknown";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PresentationRuleSpecification::_ReadXml(xmlNode))
        return false;

    // optional:
    bool alwaysReturnsChildrenValue;
    Utf8String childrenHintValue;
    if (BEXML_Success == xmlNode->GetAttributeStringValue(childrenHintValue, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HASCHILDREN))
        m_hasChildren = ParseChildrenHint(childrenHintValue.c_str(), CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HASCHILDREN);
    else if (BEXML_Success == xmlNode->GetAttributeBooleanValue(alwaysReturnsChildrenValue, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_ALWAYSRETURNSCHILDREN) && alwaysReturnsChildrenValue)
        m_hasChildren = ChildrenHint::Always;
    else
        m_hasChildren = ChildrenHint::Unknown;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_hideNodesInHierarchy, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDENODESINHIERARCHY))
        m_hideNodesInHierarchy = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_hideIfNoChildren, CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDEIFNOCHILDREN))
        m_hideIfNoChildren = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_doNotSort, SORTING_RULE_XML_ATTRIBUTE_DONOTSORT))
        m_doNotSort = false;

    CommonToolsInternal::LoadSpecificationsFromXmlNode<RelatedInstanceSpecification, RelatedInstanceSpecificationList> (xmlNode, m_relatedInstances, RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME, this);
    CommonToolsInternal::LoadRulesFromXmlNode<ChildNodeRule, ChildNodeRuleList>(xmlNode, m_nestedRules, CHILD_NODE_RULE_XML_NODE_NAME, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::_WriteXml(BeXmlNodeP specificationNode) const
    {
    PresentationRuleSpecification::_WriteXml(specificationNode);
    specificationNode->AddAttributeStringValue(CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HASCHILDREN, ChildrenHintToString(m_hasChildren));
    specificationNode->AddAttributeBooleanValue (CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDENODESINHIERARCHY, m_hideNodesInHierarchy);
    specificationNode->AddAttributeBooleanValue (CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDEIFNOCHILDREN, m_hideIfNoChildren);
    specificationNode->AddAttributeBooleanValue (SORTING_RULE_XML_ATTRIBUTE_DONOTSORT, m_doNotSort);
    CommonToolsInternal::WriteRulesToXmlNode<RelatedInstanceSpecification, RelatedInstanceSpecificationList>(specificationNode, m_relatedInstances);
    CommonToolsInternal::WriteRulesToXmlNode<ChildNodeRule, ChildNodeRuleList>(specificationNode, m_nestedRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::_ReadJson(JsonValueCR json)
    {
    if (!PresentationRuleSpecification::_ReadJson(json))
        return false;

    if (json.isMember(CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HASCHILDREN))
        m_hasChildren = ParseChildrenHint(json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HASCHILDREN].asCString(""), CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HASCHILDREN);
    else if (json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_ALWAYSRETURNSCHILDREN].asBool(false))
        {
        m_hasChildren = ChildrenHint::Always;
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_WARNING, Utf8PrintfString("Using deprecated `%s.%s`. It's recommended to switch to `%s`.",
            _GetJsonElementType(), CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_ALWAYSRETURNSCHILDREN, CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HASCHILDREN));
        }

    m_hideNodesInHierarchy = json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDENODESINHIERARCHY].asBool(false);
    m_hideIfNoChildren = json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDEIFNOCHILDREN].asBool(false);
    m_hideExpression = json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDEEXPRESSION].asCString("");
    m_doNotSort = json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_DONOTSORT].asBool(false);
    m_suppressSimilarAncestorsCheck = json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_SUPPRESSSIMILARANCESTORSCHECK].asBool(false);
    if (json.isMember(CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCES))
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCES, json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCES], m_relatedInstances, CommonToolsInternal::LoadRuleFromJson<RelatedInstanceSpecification>, this);
    if (json.isMember(CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRULES))
        CommonToolsInternal::LoadFromJsonByPriority(_GetJsonElementType(), CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRULES, json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRULES], m_nestedRules, CommonToolsInternal::LoadRuleFromJson<ChildNodeRule>, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::_WriteJson(JsonValueR json) const
    {
    PresentationRuleSpecification::_WriteJson(json);
    if (ChildrenHint::Unknown != m_hasChildren)
        json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HASCHILDREN] = ChildrenHintToString(m_hasChildren);
    if (m_hideNodesInHierarchy)
        json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDENODESINHIERARCHY] = m_hideNodesInHierarchy;
    if (m_hideIfNoChildren)
        json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDEIFNOCHILDREN] = m_hideIfNoChildren;
    if (m_doNotSort)
        json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_DONOTSORT] = m_doNotSort;
    if (m_suppressSimilarAncestorsCheck)
        json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_SUPPRESSSIMILARANCESTORSCHECK] = m_suppressSimilarAncestorsCheck;
    if (!m_hideExpression.empty())
        json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDEEXPRESSION] = m_hideExpression;
    if (!m_relatedInstances.empty())
        CommonToolsInternal::WriteRulesToJson<RelatedInstanceSpecification, RelatedInstanceSpecificationList>(json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCES], m_relatedInstances);
    if (!m_nestedRules.empty())
        CommonToolsInternal::WriteRulesToJson<ChildNodeRule, ChildNodeRuleList>(json[CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRULES], m_nestedRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::GetHideNodesInHierarchy (void) const { return m_hideNodesInHierarchy; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetHideNodesInHierarchy (bool value) { m_hideNodesInHierarchy = value; InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::GetHideIfNoChildren (void) const { return m_hideIfNoChildren; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetHideIfNoChildren (bool value) { m_hideIfNoChildren = value; InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChildNodeSpecification::GetDoNotSort (void) const { return m_doNotSort; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::SetDoNotSort (bool doNotSort) { m_doNotSort = doNotSort; InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRuleList const& ChildNodeSpecification::GetNestedRules (void) const { return m_nestedRules; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::AddNestedRule(ChildNodeRuleR rule) { ADD_HASHABLE_CHILD(m_nestedRules, rule); InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ChildNodeSpecification::AddRelatedInstance(RelatedInstanceSpecificationR relatedInstance) { ADD_HASHABLE_CHILD(m_relatedInstances, relatedInstance); InvalidateHash(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ChildNodeSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_hideNodesInHierarchy)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDENODESINHIERARCHY, m_hideNodesInHierarchy);
    if (m_hideIfNoChildren)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDEIFNOCHILDREN, m_hideIfNoChildren);
    if (m_suppressSimilarAncestorsCheck)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_SUPPRESSSIMILARANCESTORSCHECK, m_suppressSimilarAncestorsCheck);
    if (!m_hideExpression.empty())
        ADD_STR_VALUE_TO_HASH(md5, CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDEEXPRESSION, m_hideExpression);
    if (m_doNotSort)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_DONOTSORT, m_doNotSort);
    if (m_hasChildren != ChildrenHint::Unknown)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HASCHILDREN, m_hasChildren);
    ADD_RULES_TO_HASH(md5, CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCES, m_relatedInstances);
    ADD_RULES_TO_HASH(md5, CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRULES, m_nestedRules);
    return md5;
    }
