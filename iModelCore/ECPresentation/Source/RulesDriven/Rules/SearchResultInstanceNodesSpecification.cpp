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
SearchResultInstanceNodesSpecification::SearchResultInstanceNodesSpecification(int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, 
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel) 
    : SearchResultInstanceNodesSpecification(priority, alwaysReturnsChildren ? ChildrenHint::Always : ChildrenHint::Unknown, hideNodesInHierarchy, 
        hideIfNoChildren, groupByClass, groupByLabel)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::SearchResultInstanceNodesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy, 
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel) 
    : ChildNodeSpecification(priority, hasChildren, hideNodesInHierarchy, hideIfNoChildren), m_groupByClass(groupByClass), m_groupByLabel(groupByLabel)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::SearchResultInstanceNodesSpecification(SearchResultInstanceNodesSpecification const& other)
    : ChildNodeSpecification(other), m_groupByClass(other.m_groupByClass), m_groupByLabel(other.m_groupByLabel)
    {
    CommonToolsInternal::CloneRules(m_querySpecifications, other.m_querySpecifications, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::~SearchResultInstanceNodesSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_querySpecifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchResultInstanceNodesSpecification::_ShallowEqual(PresentationRuleSpecification const& other) const
    {
    if (!ChildNodeSpecification::_ShallowEqual(other))
        return false;

    SearchResultInstanceNodesSpecificationCP otherRule = dynamic_cast<SearchResultInstanceNodesSpecificationCP>(&other);
    if (nullptr == otherRule)
        return false;

    return m_groupByClass == otherRule->m_groupByClass
        && m_groupByLabel == otherRule->m_groupByLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SearchResultInstanceNodesSpecification::_GetXmlElementName () const
    {
    return SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchResultInstanceNodesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ChildNodeSpecification::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByClass, COMMON_XML_ATTRIBUTE_GROUPBYCLASS))
        m_groupByClass = true;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_groupByLabel, COMMON_XML_ATTRIBUTE_GROUPBYLABEL))
        m_groupByLabel = true;

    CommonToolsInternal::LoadSpecificationsFromXmlNode<StringQuerySpecification, QuerySpecificationList>(xmlNode, m_querySpecifications, STRING_QUERY_SPECIFICATION_XML_NODE_NAME, this);
    CommonToolsInternal::LoadSpecificationsFromXmlNode<ECPropertyValueQuerySpecification, QuerySpecificationList>(xmlNode, m_querySpecifications, ECPROPERTY_VALUE_QUERY_SPECIFICATION_XML_NODE_NAME, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ChildNodeSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);    
    CommonToolsInternal::WriteRulesToXmlNode<QuerySpecification, QuerySpecificationList>(xmlNode, m_querySpecifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SearchResultInstanceNodesSpecification::_GetJsonElementType() const
    {
    return SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchResultInstanceNodesSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ChildNodeSpecification::_ReadJson(json))
        return false;

    m_groupByClass = json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS].asBool(true);
    m_groupByLabel = json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL].asBool(true);
    CommonToolsInternal::LoadFromJson(json[SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_ATTRIBUTE_QUERIES], m_querySpecifications, QuerySpecification::Create, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::_WriteJson(JsonValueR json) const
    {
    ChildNodeSpecification::_WriteJson(json);
    if (!m_groupByClass)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS] = m_groupByClass;
    if (!m_groupByLabel)
        json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL] = m_groupByLabel;
    if (!m_querySpecifications.empty())
        CommonToolsInternal::WriteRulesToJson<QuerySpecification, QuerySpecificationList>(json[SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_ATTRIBUTE_QUERIES], m_querySpecifications);
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
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
QuerySpecification* QuerySpecification::Create(JsonValueCR json)
    {
    Utf8CP type = json[COMMON_JSON_ATTRIBUTE_SPECTYPE].asCString("");
    QuerySpecification* spec = nullptr;
    if (0 == strcmp(STRING_QUERY_SPECIFICATION_JSON_TYPE, type))
        spec = new StringQuerySpecification();
    else if (0 == strcmp(ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_TYPE, type))
        spec = new ECPropertyValueQuerySpecification();
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
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
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QuerySpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue(SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_SCHEMA_NAME, m_schemaName.c_str());
    xmlNode->AddAttributeStringValue(SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_CLASS_NAME, m_className.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool QuerySpecification::_ReadJson(JsonValueCR json)
    {
    if (!json.isMember(COMMON_JSON_ATTRIBUTE_SPECTYPE) || !json[COMMON_JSON_ATTRIBUTE_SPECTYPE].isString())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "QuerySpecification", COMMON_JSON_ATTRIBUTE_SPECTYPE);
        return false;
        }
    if (0 != strcmp(json[COMMON_JSON_ATTRIBUTE_SPECTYPE].asCString(), _GetJsonElementType()))
        return false;

    CommonToolsInternal::ParseSchemaAndClassName(m_schemaName, m_className, json[COMMON_JSON_ATTRIBUTE_CLASS]);
    if (m_schemaName.empty() || m_className.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "QuerySpecification", COMMON_JSON_ATTRIBUTE_CLASS);
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void QuerySpecification::_WriteJson(JsonValueR json) const
    {
    json[COMMON_JSON_ATTRIBUTE_SPECTYPE] = _GetJsonElementType();
    json[COMMON_JSON_ATTRIBUTE_CLASS] = CommonToolsInternal::SchemaAndClassNameToJson(m_schemaName, m_className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 QuerySpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(m_schemaName.c_str(), m_schemaName.size());
    md5.Add(m_className.c_str(), m_className.size());
    Utf8CP name = _GetXmlElementName();
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
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void StringQuerySpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    BeXmlNodeP ruleNode = xmlNode->AddEmptyElement(_GetXmlElementName());
    QuerySpecification::_WriteXml(ruleNode);
    ruleNode->SetContent(WString(m_query.c_str(), true).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP StringQuerySpecification::_GetJsonElementType() const
    {
    return STRING_QUERY_SPECIFICATION_JSON_TYPE;
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
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "StringQuerySpecification", STRING_QUERY_SPECIFICATION_JSON_ATTRIBUTE_QUERY);
        return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StringQuerySpecification::_WriteJson(JsonValueR json) const
    {
    QuerySpecification::_WriteJson(json);
    json[STRING_QUERY_SPECIFICATION_JSON_ATTRIBUTE_QUERY] = m_query;
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
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPropertyValueQuerySpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    BeXmlNodeP ruleNode = xmlNode->AddEmptyElement(_GetXmlElementName());
    QuerySpecification::_WriteXml(ruleNode);
    ruleNode->AddAttributeStringValue(ECPROPERTY_VALUE_QUERY_SPECIFICATION_XML_ATTRIBUTE_PARENT_PROPERTY_NAME, m_parentPropertyName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECPropertyValueQuerySpecification::_GetJsonElementType() const
    {
    return ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_TYPE;
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
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "ECPropertyValueQuerySpecification", ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_ATTRIBUTE_PARENT_PROPERTY_NAME);
        return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPropertyValueQuerySpecification::_WriteJson(JsonValueR json) const
    {
    QuerySpecification::_WriteJson(json);
    json[ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_ATTRIBUTE_PARENT_PROPERTY_NAME] = m_parentPropertyName;
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
