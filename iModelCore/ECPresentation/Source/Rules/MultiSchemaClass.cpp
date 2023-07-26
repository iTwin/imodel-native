/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include <ECPresentation/Rules/PresentationRuleSet.h>
#include <ECPresentation/Rules/MultiSchemaClass.h>
#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MultiSchemaClass::MultiSchemaClass(MultiSchemaClass const& other)
    : m_schemaName(other.m_schemaName), m_classNames(other.m_classNames), m_arePolymorphic(other.m_arePolymorphic)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MultiSchemaClass::MultiSchemaClass(MultiSchemaClass&& other)
    : m_schemaName(std::move(other.m_schemaName)), m_classNames(std::move(other.m_classNames)), m_arePolymorphic(other.m_arePolymorphic)
    { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 MultiSchemaClass::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();

    if (!m_schemaName.empty())
        ADD_STR_VALUE_TO_HASH(md5, SCHEMA_CLASS_SPECIFICATION_SCHEMANAME, m_schemaName);
    if (!m_classNames.empty())
        ADD_STR_VALUES_TO_HASH(md5, MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES, m_classNames);
    if (m_arePolymorphic)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC, m_arePolymorphic);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP MultiSchemaClass::_GetJsonElementType() const
    {
    return "MultiSchemaClassesSpecification";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiSchemaClass::_ReadJson(BeJsConst json)
    {
    bool hasIssues = false
        || CommonToolsInternal::CheckRuleIssue(!(json.isMember(SCHEMA_CLASS_SPECIFICATION_SCHEMANAME) && json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME].isString()), _GetJsonElementType(),
            SCHEMA_CLASS_SPECIFICATION_SCHEMANAME, json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME], "schema name string")
        || CommonToolsInternal::CheckRuleIssue(!json.isMember(MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES) || !json[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES].isArray() || json[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES].empty(),
            _GetJsonElementType(), MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES, json[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES], "array of class names");
    if (hasIssues)
        return false;

    if (json.isMember(COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC) && json[COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC].isBool())
        m_arePolymorphic = json[COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC].asBool();

    m_schemaName = json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME].asCString();

    json["classNames"].ForEachArrayMember(
        [&](BeJsConst::ArrayIndex i, BeJsConst className)
        {
        m_classNames.push_back(className.asCString());
        return false;
        }
    );

    InvalidateHash();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiSchemaClass::_WriteJson(BeJsValue json) const
    {
    T_Super::_WriteJson(json);
    if (!m_schemaName.empty())
        json[SCHEMA_CLASS_SPECIFICATION_SCHEMANAME] = m_schemaName;
    if (!m_classNames.empty())
        {
        BeJsValue classNames = json[MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES];
        for (auto const& className : m_classNames)
            classNames[classNames.size()] = className.c_str();
        }
    if (m_arePolymorphic)
        json[COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC] = m_arePolymorphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MultiSchemaClass* MultiSchemaClass::LoadFromJson(BeJsConst json, bool defaultPolymorphism)
    {
    MultiSchemaClass* rule = new MultiSchemaClass();
    rule->SetArePolymorphic(defaultPolymorphism);

    if (!rule->ReadJson(json))
        DELETE_AND_CLEAR(rule);
    return rule;
    }
