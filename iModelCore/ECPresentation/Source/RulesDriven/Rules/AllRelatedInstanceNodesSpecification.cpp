/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/AllRelatedInstanceNodesSpecification.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
AllRelatedInstanceNodesSpecification::AllRelatedInstanceNodesSpecification ()
    : ChildNodeSpecification (), m_groupByClass (true), m_groupByRelationship (false), m_groupByLabel (true), 
      m_skipRelatedLevel (0), m_requiredDirection (RequiredRelationDirection_Both)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
AllRelatedInstanceNodesSpecification::AllRelatedInstanceNodesSpecification(int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, 
    bool hideIfNoChildren, bool groupByClass, bool groupByRelationship, bool groupByLabel, int skipRelatedLevel, Utf8StringCR supportedSchemas) 
    : AllRelatedInstanceNodesSpecification(priority, alwaysReturnsChildren ? ChildrenHint::Always : ChildrenHint::Unknown, hideNodesInHierarchy, 
        hideIfNoChildren, groupByClass, groupByLabel, skipRelatedLevel, supportedSchemas)
    {
    m_groupByRelationship = groupByRelationship;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AllRelatedInstanceNodesSpecification::AllRelatedInstanceNodesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy, 
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel, int skipRelatedLevel, Utf8StringCR supportedSchemas) 
    : ChildNodeSpecification(priority, hasChildren, hideNodesInHierarchy, hideIfNoChildren), m_groupByClass(groupByClass), m_groupByRelationship(false), 
    m_groupByLabel(groupByLabel), m_skipRelatedLevel(skipRelatedLevel), m_supportedSchemas(supportedSchemas), m_requiredDirection(RequiredRelationDirection_Both)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP AllRelatedInstanceNodesSpecification::_GetXmlElementName () const
    {
    return ALL_RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllRelatedInstanceNodesSpecification::_ReadXml (BeXmlNodeP xmlNode)
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

    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_skipRelatedLevel, COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL))
        m_skipRelatedLevel = 0;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_supportedSchemas, COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS))
        m_supportedSchemas = "";

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
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP AllRelatedInstanceNodesSpecification::_GetJsonElementType() const
    {
    return ALL_RELATED_INSTANCE_NODES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllRelatedInstanceNodesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ChildNodeSpecification::_ReadJson(json))
        return false;
    m_groupByClass = json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS].asBool(true);
    m_groupByLabel = json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL].asBool(true);
    m_skipRelatedLevel = json[COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL].asInt(0);
    m_supportedSchemas = CommonToolsInternal::SupportedSchemasToString(json[COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS]);
    m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""));
    return true;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::_WriteJson(JsonValueR json) const
    {
    ChildNodeSpecification::_WriteJson(json);
    if (!m_groupByClass)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS] = m_groupByClass;
    if (!m_groupByLabel)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL] = m_groupByLabel;
    if (0 != m_skipRelatedLevel)
        json[COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL] = m_skipRelatedLevel;
    if (!m_supportedSchemas.empty())
        json[COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS] = CommonToolsInternal::SupportedSchemasToJson(m_supportedSchemas);
    if (RequiredRelationDirection_Both != m_requiredDirection)
        json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION] = CommonToolsInternal::FormatRequiredDirectionString(m_requiredDirection);
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
Utf8StringCR AllRelatedInstanceNodesSpecification::GetSupportedSchemas (void) const { return m_supportedSchemas; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetSupportedSchemas (Utf8String value) { m_supportedSchemas = value; }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RequiredRelationDirection AllRelatedInstanceNodesSpecification::GetRequiredRelationDirection (void) const { return m_requiredDirection; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AllRelatedInstanceNodesSpecification::SetRequiredRelationDirection (RequiredRelationDirection requiredDirection) { m_requiredDirection = requiredDirection; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 AllRelatedInstanceNodesSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ChildNodeSpecification::_ComputeHash(parentHash);
    md5.Add(&m_groupByClass, sizeof(m_groupByClass));
    md5.Add(&m_groupByRelationship, sizeof(m_groupByRelationship));
    md5.Add(&m_groupByLabel, sizeof(m_groupByLabel));
    md5.Add(&m_skipRelatedLevel, sizeof(m_skipRelatedLevel));
    md5.Add(m_supportedSchemas.c_str(), m_supportedSchemas.size());
    md5.Add(&m_requiredDirection, sizeof(m_requiredDirection));
    return md5;
    }
