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
AllRelatedInstanceNodesSpecification::AllRelatedInstanceNodesSpecification ()
    : ChildNodeSpecification (), m_groupByClass (true), m_groupByRelationship (false), m_groupByLabel (true),
      m_skipRelatedLevel (0), m_requiredDirection (RequiredRelationDirection_Both)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AllRelatedInstanceNodesSpecification::AllRelatedInstanceNodesSpecification(int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy,
    bool hideIfNoChildren, bool groupByClass, bool groupByRelationship, bool groupByLabel, int skipRelatedLevel, Utf8StringCR supportedSchemas)
    : AllRelatedInstanceNodesSpecification(priority, alwaysReturnsChildren ? ChildrenHint::Always : ChildrenHint::Unknown, hideNodesInHierarchy,
        hideIfNoChildren, groupByClass, groupByLabel, skipRelatedLevel, supportedSchemas)
    {
    m_groupByRelationship = groupByRelationship;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AllRelatedInstanceNodesSpecification::AllRelatedInstanceNodesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy,
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel, int skipRelatedLevel, Utf8StringCR supportedSchemas)
    : ChildNodeSpecification(priority, hasChildren, hideNodesInHierarchy, hideIfNoChildren), m_groupByClass(groupByClass), m_groupByRelationship(false),
    m_groupByLabel(groupByLabel), m_skipRelatedLevel(skipRelatedLevel), m_supportedSchemas(supportedSchemas), m_requiredDirection(RequiredRelationDirection_Both)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP AllRelatedInstanceNodesSpecification::_GetXmlElementName () const
    {
    return ALL_RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllRelatedInstanceNodesSpecification::_ReadXml (BeXmlNodeP xmlNode)
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

    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_skipRelatedLevel, COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL))
        m_skipRelatedLevel = 0;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_supportedSchemas, COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS))
        m_supportedSchemas = "";

    Utf8String requiredDirectionString = "";
    if (BEXML_Success != xmlNode->GetAttributeStringValue (requiredDirectionString, COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION))
        requiredDirectionString = "";
    else
        m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString (requiredDirectionString.c_str(), _GetXmlElementName());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ChildNodeSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYRELATIONSHIP, m_groupByRelationship);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    xmlNode->AddAttributeInt32Value   (COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL, m_skipRelatedLevel);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS, m_supportedSchemas.c_str ());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonToolsInternal::FormatRequiredDirectionString (m_requiredDirection));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP AllRelatedInstanceNodesSpecification::_GetJsonElementType() const
    {
    return ALL_RELATED_INSTANCE_NODES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllRelatedInstanceNodesSpecification::_ReadJson(BeJsConst json)
    {
    if (!ChildNodeSpecification::_ReadJson(json))
        return false;

    m_groupByClass = json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS].asBool(true);
    m_groupByLabel = json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL].asBool(true);
    m_skipRelatedLevel = json[COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL].asInt(0);
    m_supportedSchemas = CommonToolsInternal::SupportedSchemasToString(json[COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS]);
    m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""), _GetJsonElementType());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::_WriteJson(BeJsValue json) const
    {
    ChildNodeSpecification::_WriteJson(json);
    if (!m_groupByClass)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS] = m_groupByClass;
    if (!m_groupByLabel)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL] = m_groupByLabel;
    if (0 != m_skipRelatedLevel)
        json[COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL] = m_skipRelatedLevel;
    if (!m_supportedSchemas.empty())
        CommonToolsInternal::WriteSupportedSchemasToJson(json[COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS], m_supportedSchemas);
    if (RequiredRelationDirection_Both != m_requiredDirection)
        json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION] = CommonToolsInternal::FormatRequiredDirectionString(m_requiredDirection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllRelatedInstanceNodesSpecification::GetGroupByClass (void) const { return m_groupByClass; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetGroupByClass (bool value) { m_groupByClass = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllRelatedInstanceNodesSpecification::GetGroupByRelationship (void) const { return m_groupByRelationship; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetGroupByRelationship (bool value) { m_groupByRelationship = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllRelatedInstanceNodesSpecification::GetGroupByLabel (void) const { return m_groupByLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetGroupByLabel (bool value) { m_groupByLabel = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int AllRelatedInstanceNodesSpecification::GetSkipRelatedLevel (void) const { return m_skipRelatedLevel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetSkipRelatedLevel (int value) { m_skipRelatedLevel = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR AllRelatedInstanceNodesSpecification::GetSupportedSchemas (void) const { return m_supportedSchemas; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetSupportedSchemas (Utf8String value) { m_supportedSchemas = value; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RequiredRelationDirection AllRelatedInstanceNodesSpecification::GetRequiredRelationDirection (void) const { return m_requiredDirection; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetRequiredRelationDirection (RequiredRelationDirection requiredDirection) { m_requiredDirection = requiredDirection; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 AllRelatedInstanceNodesSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_groupByClass)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    if (m_groupByRelationship)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_XML_ATTRIBUTE_GROUPBYRELATIONSHIP, m_groupByRelationship);
    if (!m_groupByLabel)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    if (m_skipRelatedLevel != 0)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL, m_skipRelatedLevel);
    if (!m_supportedSchemas.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS, m_supportedSchemas);
    if (m_requiredDirection != RequiredRelationDirection_Both)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION, m_requiredDirection);
    return md5;
    }
