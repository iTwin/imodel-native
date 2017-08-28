/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/SearchResultInstanceNodesSpecification.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::SearchResultInstanceNodesSpecification ()
    : ChildNodeSpecification (), m_groupByClass (true), m_groupByLabel (true)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::SearchResultInstanceNodesSpecification
(
int priority,
bool alwaysReturnsChildren,
bool hideNodesInHierarchy,
bool hideIfNoChildren,
bool groupByClass,
bool groupByLabel
) : ChildNodeSpecification (priority, alwaysReturnsChildren, hideNodesInHierarchy, hideIfNoChildren), 
    m_groupByClass (groupByClass), m_groupByLabel (groupByLabel)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::SearchResultInstanceNodesSpecification(SearchResultInstanceNodesSpecification const& other)
    : ChildNodeSpecification(other), m_groupByClass(other.m_groupByClass), m_groupByLabel(other.m_groupByLabel)
    {
    for (QuerySpecificationP spec : other.GetQuerySpecifications())
        m_querySpecifications.push_back(spec->Clone());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::~SearchResultInstanceNodesSpecification()
    {
    for (QuerySpecification* spec : m_querySpecifications)
        delete spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP SearchResultInstanceNodesSpecification::_GetXmlElementName () const
    {
    return SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchResultInstanceNodesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByClass, COMMON_XML_ATTRIBUTE_GROUPBYCLASS))
        m_groupByClass = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByLabel, COMMON_XML_ATTRIBUTE_GROUPBYLABEL))
        m_groupByLabel = true;

    CommonTools::LoadSpecificationsFromXmlNode<StringQuerySpecification, QuerySpecificationList>(xmlNode, m_querySpecifications, STRING_QUERY_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadSpecificationsFromXmlNode<ECPropertyValueQuerySpecification, QuerySpecificationList>(xmlNode, m_querySpecifications, ECPROPERTY_VALUE_QUERY_SPECIFICATION_XML_NODE_NAME);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    
    CommonTools::WriteRulesToXmlNode<QuerySpecification, QuerySpecificationList>(xmlNode, m_querySpecifications);
    CommonTools::WriteRulesToXmlNode<QuerySpecification, QuerySpecificationList>(xmlNode, m_querySpecifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchResultInstanceNodesSpecification::GetGroupByClass (void) const { return m_groupByClass; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::SetGroupByClass (bool value) { m_groupByClass = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchResultInstanceNodesSpecification::GetGroupByLabel (void) const { return m_groupByLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::SetGroupByLabel (bool value) { m_groupByLabel = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool QuerySpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_schemaName, SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_SCHEMA_NAME))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_className, SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_CLASS_NAME))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QuerySpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue(SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_SCHEMA_NAME, m_schemaName.c_str());
    xmlNode->AddAttributeStringValue(SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_CLASS_NAME, m_className.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP StringQuerySpecification::_GetXmlElementName() const {return STRING_QUERY_SPECIFICATION_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool StringQuerySpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!QuerySpecification::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetContent(m_query))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void StringQuerySpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    QuerySpecification::_WriteXml(xmlNode);
    xmlNode->SetContent(WString(m_query.c_str(), true).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECPropertyValueQuerySpecification::_GetXmlElementName() const {return ECPROPERTY_VALUE_QUERY_SPECIFICATION_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPropertyValueQuerySpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!QuerySpecification::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_parentPropertyName, ECPROPERTY_VALUE_QUERY_SPECIFICATION_XML_ATTRIBUTE_PARENT_PROPERTY_NAME))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPropertyValueQuerySpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    QuerySpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(ECPROPERTY_VALUE_QUERY_SPECIFICATION_XML_ATTRIBUTE_PARENT_PROPERTY_NAME, m_parentPropertyName.c_str());
    }
