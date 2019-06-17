/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include <BeJsonCpp/BeJsonUtilities.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
AllInstanceNodesSpecification::AllInstanceNodesSpecification ()
    : ChildNodeSpecification(), m_groupByClass(true), m_groupByLabel(true)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
AllInstanceNodesSpecification::AllInstanceNodesSpecification(int priority, bool alwaysReturnsChildren, 
    bool hideNodesInHierarchy, bool hideIfNoChildren, bool groupByClass, bool groupByLabel, Utf8StringCR supportedSchemas) 
    : AllInstanceNodesSpecification(priority, alwaysReturnsChildren ? ChildrenHint::Always : ChildrenHint::Unknown, 
        hideNodesInHierarchy, hideIfNoChildren, groupByClass, groupByLabel, supportedSchemas)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AllInstanceNodesSpecification::AllInstanceNodesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy, 
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel, Utf8StringCR supportedSchemas) 
    : ChildNodeSpecification (priority, hasChildren, hideNodesInHierarchy, hideIfNoChildren), 
    m_groupByClass(groupByClass), m_groupByLabel(groupByLabel), m_supportedSchemas(supportedSchemas)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AllInstanceNodesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP AllInstanceNodesSpecification::_GetXmlElementName () const
    {
    return ALL_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllInstanceNodesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ChildNodeSpecification::_ReadXml(xmlNode))
        return false;

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByClass, COMMON_XML_ATTRIBUTE_GROUPBYCLASS))
        m_groupByClass = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByLabel, COMMON_XML_ATTRIBUTE_GROUPBYLABEL))
        m_groupByLabel = true;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_supportedSchemas, COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS))
        m_supportedSchemas = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void AllInstanceNodesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ChildNodeSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS, m_supportedSchemas.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP AllInstanceNodesSpecification::_GetJsonElementType() const
    {
    return ALL_INSTANCE_NODES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllInstanceNodesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ChildNodeSpecification::_ReadJson(json))
        return false;
    m_groupByClass = json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS].asBool(true);
    m_groupByLabel = json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL].asBool(true);
    m_supportedSchemas = CommonToolsInternal::SupportedSchemasToString(json[COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS]);
    return true;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void AllInstanceNodesSpecification::_WriteJson(JsonValueR json) const
    {
    ChildNodeSpecification::_WriteJson(json);
    if (!m_groupByClass)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS] = m_groupByClass;
    if (!m_groupByLabel)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL] = m_groupByLabel;
    if (!m_supportedSchemas.empty())
        json[COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS] = CommonToolsInternal::SupportedSchemasToJson(m_supportedSchemas);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllInstanceNodesSpecification::GetGroupByClass (void) const { return m_groupByClass; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AllInstanceNodesSpecification::SetGroupByClass (bool value) { m_groupByClass = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool AllInstanceNodesSpecification::GetGroupByLabel (void) const { return m_groupByLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AllInstanceNodesSpecification::SetGroupByLabel (bool value) { m_groupByLabel = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR AllInstanceNodesSpecification::GetSupportedSchemas (void) const { return m_supportedSchemas; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AllInstanceNodesSpecification::SetSupportedSchemas (Utf8StringCR value) { m_supportedSchemas = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 AllInstanceNodesSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ChildNodeSpecification::_ComputeHash(parentHash);
    md5.Add(&m_groupByClass, sizeof(m_groupByClass));
    md5.Add(&m_groupByLabel, sizeof(m_groupByLabel));
    md5.Add(m_supportedSchemas.c_str(), m_supportedSchemas.size());
    return md5;
    }
