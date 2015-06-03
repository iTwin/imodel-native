/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/AllRelatedInstanceNodesSpecification.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "CommonTools.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
AllRelatedInstanceNodesSpecification::AllRelatedInstanceNodesSpecification ()
    : ChildNodeSpecification (), m_groupByClass (true), m_groupByRelationship (false), m_groupByLabel (true), 
      m_skipRelatedLevel (0), m_supportedSchemas (L""), m_requiredDirection (RequiredRelationDirection_Both)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
AllRelatedInstanceNodesSpecification::AllRelatedInstanceNodesSpecification
(
int       priority,
bool      alwaysReturnsChildren,
bool      hideNodesInHierarchy,
bool      hideIfNoChildren,
bool      groupByClass,
bool      groupByRelationship,
bool      groupByLabel,
int       skipRelatedLevel,
WStringCR supportedSchemas
) : ChildNodeSpecification (priority, alwaysReturnsChildren, hideNodesInHierarchy, hideIfNoChildren), 
    m_groupByClass (groupByClass), m_groupByRelationship (groupByRelationship), m_groupByLabel (groupByLabel),
    m_skipRelatedLevel (skipRelatedLevel), m_supportedSchemas (supportedSchemas), m_requiredDirection (RequiredRelationDirection_Both)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP AllRelatedInstanceNodesSpecification::_GetXmlElementName ()
    {
    return ALL_RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllRelatedInstanceNodesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByClass, COMMON_XML_ATTRIBUTE_GROUPBYCLASS))
        m_groupByClass = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByRelationship, COMMON_XML_ATTRIBUTE_GROUPBYRELATIONSHIP))
        m_groupByRelationship = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByLabel, COMMON_XML_ATTRIBUTE_GROUPBYLABEL))
        m_groupByLabel = true;

    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_skipRelatedLevel, COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL))
        m_skipRelatedLevel = 0;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_supportedSchemas, COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS))
        m_supportedSchemas = L"";

    WString requiredDirectionString = L"";
    if (BEXML_Success != xmlNode->GetAttributeStringValue (requiredDirectionString, COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION))
        requiredDirectionString = L"";
    else
        m_requiredDirection = CommonTools::ParseRequiredDirectionString (requiredDirectionString.c_str ());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYRELATIONSHIP, m_groupByRelationship);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    xmlNode->AddAttributeInt32Value   (COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL, m_skipRelatedLevel);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS, m_supportedSchemas.c_str ());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonTools::FormatRequiredDirectionString (m_requiredDirection));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllRelatedInstanceNodesSpecification::GetGroupByClass (void) const { return m_groupByClass; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetGroupByClass (bool value) { m_groupByClass = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllRelatedInstanceNodesSpecification::GetGroupByRelationship (void) const { return m_groupByRelationship; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetGroupByRelationship (bool value) { m_groupByRelationship = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllRelatedInstanceNodesSpecification::GetGroupByLabel (void) const { return m_groupByLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetGroupByLabel (bool value) { m_groupByLabel = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int AllRelatedInstanceNodesSpecification::GetSkipRelatedLevel (void) const { return m_skipRelatedLevel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetSkipRelatedLevel (int value) { m_skipRelatedLevel = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR AllRelatedInstanceNodesSpecification::GetSupportedSchemas (void) const { return m_supportedSchemas; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetSupportedSchemas (WString value) { m_supportedSchemas = value; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RequiredRelationDirection AllRelatedInstanceNodesSpecification::GetRequiredRelationDirection (void) const { return m_requiredDirection; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetRequiredRelationDirection (RequiredRelationDirection requiredDirection) { m_requiredDirection = requiredDirection; }