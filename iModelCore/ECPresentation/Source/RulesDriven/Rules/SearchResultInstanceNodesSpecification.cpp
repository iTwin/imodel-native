/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/SearchResultInstanceNodesSpecification.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
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
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchResultInstanceNodesSpecification::_ReadJson(JsonValueCR json)
    {
    m_groupByClass = json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS].asBool(true);
    m_groupByLabel = json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL].asBool(true);
    
    JsonValueCR queriesJson = json[SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_ATTRIBUTE_QUERIES];
    CommonTools::LoadSpecificationsFromJson<StringQuerySpecification, QuerySpecificationList>
        (queriesJson, m_querySpecifications, STRING_QUERY_SPECIFICATION_JSON_TYPE);
    CommonTools::LoadSpecificationsFromJson<ECPropertyValueQuerySpecification, QuerySpecificationList>
        (queriesJson, m_querySpecifications, ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_TYPE);

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
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::AddQuerySpecification(QuerySpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_querySpecifications.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 SearchResultInstanceNodesSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ChildNodeSpecification::_ComputeHash(parentHash);
    md5.Add(&m_groupByClass, sizeof(m_groupByClass));
    md5.Add(&m_groupByLabel, sizeof(m_groupByLabel));

    Utf8String currentHash = md5.GetHashString();
    for (QuerySpecificationP spec : m_querySpecifications)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool QuerySpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_schemaName, SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_SCHEMA_NAME))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, _GetXmlElementName(), SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_SCHEMA_NAME);
        return false;
        }

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_className, SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_CLASS_NAME))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, _GetXmlElementName(), SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_CLASS_NAME);
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool QuerySpecification::_ReadJson(JsonValueCR json)
    {
    //Required
    m_schemaName = json[SEARCH_QUERY_SPECIFICATION_JSON_ATTRIBUTE_SCHEMA_NAME].asCString("");
    if (m_schemaName.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, SEARCH_QUERY_SPECIFICATION_JSON_NAME, SEARCH_QUERY_SPECIFICATION_JSON_ATTRIBUTE_SCHEMA_NAME);
        return false;
        }

    m_className = json[SEARCH_QUERY_SPECIFICATION_JSON_ATTRIBUTE_CLASS_NAME].asCString("");
    if (m_className.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, SEARCH_QUERY_SPECIFICATION_JSON_NAME, SEARCH_QUERY_SPECIFICATION_JSON_ATTRIBUTE_CLASS_NAME);
        return false;
        }

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
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 QuerySpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(m_schemaName.c_str(), m_schemaName.size());
    md5.Add(m_className.c_str(), m_className.size());
    CharCP name = _GetXmlElementName();
    md5.Add(name, strlen(name));
    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));
    return md5;
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

    if (BEXML_Success != xmlNode->GetContent(m_query) || m_query.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, STRING_QUERY_SPECIFICATION_XML_NODE_NAME, "Query");
        return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool StringQuerySpecification::_ReadJson(JsonValueCR json)
    {
    if (!QuerySpecification::_ReadJson(json))
        return false;

    //Required
    m_query = json[STRING_QUERY_SPECIFICATION_JSON_ATTRIBUTE_QUERY].asCString("");
    if (m_query.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, STRING_QUERY_SPECIFICATION_JSON_NAME, STRING_QUERY_SPECIFICATION_JSON_ATTRIBUTE_QUERY);
        return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void StringQuerySpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    BeXmlNodeP ruleNode = xmlNode->AddEmptyElement(_GetXmlElementName());
    QuerySpecification::_WriteXml(ruleNode);
    ruleNode->SetContent(WString(m_query.c_str(), true).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 StringQuerySpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = QuerySpecification::_ComputeHash(parentHash);
    md5.Add(m_query.c_str(), m_query.size());
    return md5;
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
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, ECPROPERTY_VALUE_QUERY_SPECIFICATION_XML_NODE_NAME, ECPROPERTY_VALUE_QUERY_SPECIFICATION_XML_ATTRIBUTE_PARENT_PROPERTY_NAME);
        return false;
        }
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPropertyValueQuerySpecification::_ReadJson(JsonValueCR json)
    {
    if (!QuerySpecification::_ReadJson(json))
        return false;

    //Required
    m_parentPropertyName = json[ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_ATTRIBUTE_PARENT_PROPERTY_NAME].asCString("");
    if (m_parentPropertyName.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_NAME, ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_ATTRIBUTE_PARENT_PROPERTY_NAME);
        return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPropertyValueQuerySpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    BeXmlNodeP ruleNode = xmlNode->AddEmptyElement(_GetXmlElementName());
    QuerySpecification::_WriteXml(ruleNode);
    ruleNode->AddAttributeStringValue(ECPROPERTY_VALUE_QUERY_SPECIFICATION_XML_ATTRIBUTE_PARENT_PROPERTY_NAME, m_parentPropertyName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ECPropertyValueQuerySpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = QuerySpecification::_ComputeHash(parentHash);
    md5.Add(m_parentPropertyName.c_str(), m_parentPropertyName.size());
    return md5;
    }
