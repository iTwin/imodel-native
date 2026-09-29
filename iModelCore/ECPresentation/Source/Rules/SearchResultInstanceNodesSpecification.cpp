/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::SearchResultInstanceNodesSpecification ()
    : ChildNodeSpecification (), m_groupByClass (true), m_groupByLabel (true)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::SearchResultInstanceNodesSpecification(int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy,
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel)
    : SearchResultInstanceNodesSpecification(priority, alwaysReturnsChildren ? ChildrenHint::Always : ChildrenHint::Unknown, hideNodesInHierarchy,
        hideIfNoChildren, groupByClass, groupByLabel)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::SearchResultInstanceNodesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy,
    bool hideIfNoChildren, bool groupByClass, bool groupByLabel)
    : ChildNodeSpecification(priority, hasChildren, hideNodesInHierarchy, hideIfNoChildren), m_groupByClass(groupByClass), m_groupByLabel(groupByLabel)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::SearchResultInstanceNodesSpecification(SearchResultInstanceNodesSpecification const& other)
    : ChildNodeSpecification(other), m_groupByClass(other.m_groupByClass), m_groupByLabel(other.m_groupByLabel)
    {
    CommonToolsInternal::CloneRules(m_querySpecifications, other.m_querySpecifications, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SearchResultInstanceNodesSpecification::~SearchResultInstanceNodesSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_querySpecifications);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SearchResultInstanceNodesSpecification::_GetJsonElementType() const
    {
    return SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchResultInstanceNodesSpecification::_ReadJson(BeJsConst json)
    {
    if (!ChildNodeSpecification::_ReadJson(json))
        return false;

    m_groupByClass = json[COMMON_JSON_ATTRIBUTE_GROUPBYCLASS].asBool(true);
    m_groupByLabel = json[COMMON_JSON_ATTRIBUTE_GROUPBYLABEL].asBool(true);

    if (json.isMember(SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_ATTRIBUTE_QUERIES))
        {
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_ATTRIBUTE_QUERIES, json[SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_ATTRIBUTE_QUERIES],
            m_querySpecifications, QuerySpecification::Create, this);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::_WriteJson(BeJsValue json) const
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchResultInstanceNodesSpecification::GetGroupByClass(void) const {return m_groupByClass;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::SetGroupByClass(bool value) {m_groupByClass = value; InvalidateHash();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchResultInstanceNodesSpecification::GetGroupByLabel(void) const {return m_groupByLabel;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::SetGroupByLabel(bool value) {m_groupByLabel = value; InvalidateHash();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchResultInstanceNodesSpecification::AddQuerySpecification(QuerySpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_querySpecifications.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 SearchResultInstanceNodesSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_groupByClass)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_GROUPBYCLASS, m_groupByClass);
    if (!m_groupByLabel)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_GROUPBYLABEL, m_groupByLabel);
    ADD_RULES_TO_HASH(md5, SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_ATTRIBUTE_QUERIES, m_querySpecifications);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QuerySpecification* QuerySpecification::Create(BeJsConst json)
    {
    Utf8CP type = json[COMMON_JSON_ATTRIBUTE_SPECTYPE].asCString("");
    QuerySpecification* spec = nullptr;
    if (0 == strcmp(STRING_QUERY_SPECIFICATION_JSON_TYPE, type))
        spec = new StringQuerySpecification();
    else if (0 == strcmp(ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_TYPE, type))
        spec = new ECPropertyValueQuerySpecification();
    else
        {
        Utf8String msg = json.isMember(COMMON_JSON_ATTRIBUTE_SPECTYPE)
            ? Utf8PrintfString("Invalid `" COMMON_JSON_ATTRIBUTE_SPECTYPE "` attribute value: `%s`", type)
            : Utf8String("Missing required attribute: `" COMMON_JSON_ATTRIBUTE_SPECTYPE "`");
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, msg);
        }
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP QuerySpecification::_GetJsonElementTypeAttributeName() const {return COMMON_JSON_ATTRIBUTE_SPECTYPE;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool QuerySpecification::_ReadJson(BeJsConst json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    // required:
    CommonToolsInternal::ParseSchemaAndClassName(m_schemaName, m_className, json[COMMON_JSON_ATTRIBUTE_CLASS], Utf8PrintfString("%s.%s", _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_CLASS).c_str());
    if (CommonToolsInternal::CheckRuleIssue(m_schemaName.empty() || m_className.empty(), _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_CLASS, json[COMMON_JSON_ATTRIBUTE_CLASS], "class specification"))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QuerySpecification::_WriteJson(BeJsValue json) const
    {
    PresentationKey::_WriteJson(json);
    CommonToolsInternal::WriteSchemaAndClassNameToJson(json[COMMON_JSON_ATTRIBUTE_CLASS], m_schemaName, m_className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 QuerySpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_schemaName.empty())
        ADD_STR_VALUE_TO_HASH(md5, SCHEMA_CLASS_SPECIFICATION_SCHEMANAME, m_schemaName);
    if (!m_className.empty())
        ADD_STR_VALUE_TO_HASH(md5, SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME, m_className);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP StringQuerySpecification::_GetJsonElementType() const {return STRING_QUERY_SPECIFICATION_JSON_TYPE;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool StringQuerySpecification::_ReadJson(BeJsConst json)
    {
    if (!QuerySpecification::_ReadJson(json))
        return false;

    // required:
    m_query = json[STRING_QUERY_SPECIFICATION_JSON_ATTRIBUTE_QUERY].asCString("");
    if (CommonToolsInternal::CheckRuleIssue(m_query.empty(), _GetJsonElementType(), STRING_QUERY_SPECIFICATION_JSON_ATTRIBUTE_QUERY, json[STRING_QUERY_SPECIFICATION_JSON_ATTRIBUTE_QUERY], "non-empty string"))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void StringQuerySpecification::_WriteJson(BeJsValue json) const
    {
    QuerySpecification::_WriteJson(json);
    json[STRING_QUERY_SPECIFICATION_JSON_ATTRIBUTE_QUERY] = m_query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 StringQuerySpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_query.empty())
        ADD_STR_VALUE_TO_HASH(md5, STRING_QUERY_SPECIFICATION_JSON_ATTRIBUTE_QUERY, m_query);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECPropertyValueQuerySpecification::_GetJsonElementType() const {return ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_TYPE;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPropertyValueQuerySpecification::_ReadJson(BeJsConst json)
    {
    if (!QuerySpecification::_ReadJson(json))
        return false;

    // required:
    m_parentPropertyName = json[ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_ATTRIBUTE_PARENT_PROPERTY_NAME].asCString("");
    if (CommonToolsInternal::CheckRuleIssue(m_parentPropertyName.empty(), _GetJsonElementType(), ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_ATTRIBUTE_PARENT_PROPERTY_NAME, json[ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_ATTRIBUTE_PARENT_PROPERTY_NAME], "non-empty string"))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPropertyValueQuerySpecification::_WriteJson(BeJsValue json) const
    {
    QuerySpecification::_WriteJson(json);
    json[ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_ATTRIBUTE_PARENT_PROPERTY_NAME] = m_parentPropertyName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ECPropertyValueQuerySpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_parentPropertyName.empty())
        ADD_STR_VALUE_TO_HASH(md5, ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_ATTRIBUTE_PARENT_PROPERTY_NAME, m_parentPropertyName);
    return md5;
    }
