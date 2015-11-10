/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/RelatedInstanceNodesSpecification.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/CommonTools.h>
#include <ECPresentationRules/PresentationRules.h>
#include <ECPresentationRules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification ()
    : ChildNodeSpecification (), m_groupByClass (true), m_groupByRelationship (false), m_groupByLabel (true), m_showEmptyGroups (false),
    m_skipRelatedLevel (0), m_instanceFilter (L""), m_requiredDirection (RequiredRelationDirection_Both),
    m_supportedSchemas (L""), m_relationshipClassNames (L""), m_relatedClassNames (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedInstanceNodesSpecification::RelatedInstanceNodesSpecification 
(
int                        priority, 
bool                       alwaysReturnsChildren, 
bool                       hideNodesInHierarchy, 
bool                       hideIfNoChildren,
bool                       groupByClass,
bool                       groupByRelationship,
bool                       groupByLabel,
bool                       showEmptyGroups,
int                        skipRelatedLevel,
Utf8String                 instanceFilter,
RequiredRelationDirection  requiredDirection,
Utf8String                 supportedSchemas,
Utf8String                 relationshipClassNames,
Utf8String                 relatedClassNames
) : ChildNodeSpecification (priority, alwaysReturnsChildren, hideNodesInHierarchy, hideIfNoChildren), 
    m_groupByClass (groupByClass), m_groupByRelationship (groupByRelationship), m_groupByLabel (groupByLabel), m_showEmptyGroups (showEmptyGroups), 
    m_skipRelatedLevel (skipRelatedLevel), m_instanceFilter (instanceFilter), m_requiredDirection (requiredDirection),
    m_supportedSchemas (supportedSchemas), m_relationshipClassNames (relationshipClassNames),  m_relatedClassNames (relatedClassNames)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP RelatedInstanceNodesSpecification::_GetXmlElementName ()
    {
    return RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceNodesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
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
        m_requiredDirection = CommonTools::ParseRequiredDirectionString (requiredDirectionString.c_str ());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYRELATIONSHIP, m_groupByRelationship);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS, m_showEmptyGroups);
    xmlNode->AddAttributeInt32Value   (COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL, m_skipRelatedLevel);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str ());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS, m_supportedSchemas.c_str ());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES, m_relationshipClassNames.c_str ());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES, m_relatedClassNames.c_str ());
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonTools::FormatRequiredDirectionString (m_requiredDirection));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceNodesSpecification::GetGroupByClass (void) const                { return m_groupByClass; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::SetGroupByClass (bool value)                { m_groupByClass = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceNodesSpecification::GetGroupByRelationship (void) const         { return m_groupByRelationship; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::SetGroupByRelationship (bool value)         { m_groupByRelationship = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceNodesSpecification::GetGroupByLabel (void) const                { return m_groupByLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::SetGroupByLabel (bool value)                { m_groupByLabel = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedInstanceNodesSpecification::GetShowEmptyGroups (void) const             { return m_showEmptyGroups; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::SetShowEmptyGroups (bool value)             { m_showEmptyGroups = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int RelatedInstanceNodesSpecification::GetSkipRelatedLevel (void) const             { return m_skipRelatedLevel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::SetSkipRelatedLevel (int value)             { m_skipRelatedLevel = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR RelatedInstanceNodesSpecification::GetInstanceFilter (void) const         { return m_instanceFilter; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::SetInstanceFilter (Utf8String value)           { m_instanceFilter = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RequiredRelationDirection RelatedInstanceNodesSpecification::GetRequiredRelationDirection (void) const { return m_requiredDirection; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::SetRequiredRelationDirection (RequiredRelationDirection value) { m_requiredDirection = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR RelatedInstanceNodesSpecification::GetSupportedSchemas (void) const       { return m_supportedSchemas; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::SetSupportedSchemas (Utf8String value)         { m_supportedSchemas = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR RelatedInstanceNodesSpecification::GetRelationshipClassNames (void) const { return m_relationshipClassNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::SetRelationshipClassNames (Utf8String value)   { m_relationshipClassNames = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR RelatedInstanceNodesSpecification::GetRelatedClassNames (void) const      { return m_relatedClassNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedInstanceNodesSpecification::SetRelatedClassNames (Utf8String value) { m_relatedClassNames = value; }
