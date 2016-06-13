/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/SearchResultInstanceNodesSpecification.cpp $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>
#include <ECPresentationRules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_EC

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
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::~SearchResultInstanceNodesSpecification()
    {
    for (SearchQuerySpecification* spec : m_querySpecifications)
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

    for (BeXmlNodeP child = xmlNode->GetFirstChild(BEXMLNODE_Element); NULL != child; child = child->GetNextSibling(BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp(child->GetName(), SEARCH_QUERY_SPECIFICATION_XML_NODE_NAME))
            {
            SearchQuerySpecification* spec = new SearchQuerySpecification();
            if (spec->ReadXml(child))
                m_querySpecifications.push_back(spec);
            else
                delete spec;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    
    for (SearchQuerySpecification* spec : m_querySpecifications)
        spec->WriteXml(xmlNode);
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
bvector<SearchQuerySpecification*>& SearchResultInstanceNodesSpecification::GetQuerySpecificationsR() {return m_querySpecifications;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SearchQuerySpecification*> const& SearchResultInstanceNodesSpecification::GetQuerySpecifications() const {return m_querySpecifications;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP SearchQuerySpecification::GetXmlElementName() const {return SEARCH_QUERY_SPECIFICATION_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchQuerySpecification::ReadXml (BeXmlNodeP xmlNode)
    {    
    if (BEXML_Success != xmlNode->GetContent(m_query))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_schemaName, SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_SCHEMA_NAME))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_className, SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_CLASS_NAME))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchQuerySpecification::WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->SetContent(WString(m_query.c_str(), true).c_str());
    xmlNode->AddAttributeStringValue(SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_SCHEMA_NAME, m_schemaName.c_str());
    xmlNode->AddAttributeStringValue(SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_CLASS_NAME, m_className.c_str());
    }