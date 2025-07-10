/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifiersList::ContentModifiersList(ContentModifiersList const& other)
    : HashableBase(other)
    {
    CommonToolsInternal::CopyRules(m_relatedProperties, other.m_relatedProperties, this);
    CommonToolsInternal::CopyRules(m_calculatedProperties, other.m_calculatedProperties, this);
    CommonToolsInternal::CopyRules(m_propertyOverrides, other.m_propertyOverrides, this);
    CommonToolsInternal::CopyRules(m_propertyCategories, other.m_propertyCategories, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifiersList::ContentModifiersList(ContentModifiersList&& other)
    : HashableBase(std::move(other))
    {
    CommonToolsInternal::SwapRules(m_relatedProperties, other.m_relatedProperties, this);
    CommonToolsInternal::SwapRules(m_calculatedProperties, other.m_calculatedProperties, this);
    CommonToolsInternal::SwapRules(m_propertyOverrides, other.m_propertyOverrides, this);
    CommonToolsInternal::SwapRules(m_propertyCategories, other.m_propertyCategories, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifiersList::~ContentModifiersList()
    {
    CommonToolsInternal::FreePresentationRules(m_relatedProperties);
    CommonToolsInternal::FreePresentationRules(m_calculatedProperties);
    CommonToolsInternal::FreePresentationRules(m_propertyOverrides);
    CommonToolsInternal::FreePresentationRules(m_propertyCategories);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::AddRelatedProperty(RelatedPropertiesSpecificationR specification) { ADD_HASHABLE_CHILD(m_relatedProperties, specification); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::ClearRelatedProperties()
    {
    InvalidateHash();
    CommonToolsInternal::FreePresentationRules(m_relatedProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::AddCalculatedProperty(CalculatedPropertiesSpecificationR specification) { ADD_HASHABLE_CHILD(m_calculatedProperties, specification); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::ClearCalculatedProperties()
    {
    InvalidateHash();
    CommonToolsInternal::FreePresentationRules(m_calculatedProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::AddPropertyOverride(PropertySpecificationR specification) { ADD_HASHABLE_CHILD(m_propertyOverrides, specification); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::ClearPropertyOverrides()
    {
    InvalidateHash();
    CommonToolsInternal::FreePresentationRules(m_propertyOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::AddPropertyCategory(PropertyCategorySpecificationR specification) { ADD_HASHABLE_CHILD(m_propertyCategories, specification); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::ClearPropertyCategories()
    {
    InvalidateHash();
    CommonToolsInternal::FreePresentationRules(m_propertyCategories);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PropertySpecificationP CreatePropertyEditorSpecification(BeJsConst json)
    {
    return CommonToolsInternal::LoadRuleFromJson(json, &PropertySpecification::ReadEditorSpecificationJson);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void LoadPropertyDisplaySpecifications(BeJsConst json, bvector<PropertySpecificationP>& specs, HashableBase const* parentSpec)
    {
    for (BeJsConst::ArrayIndex i = 0; i < json.size(); ++i)
        {
        BeJsConst propertyNamesJson = json[i][PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES];
        if (propertyNamesJson.isNull() || !propertyNamesJson.isArray() || 0 == propertyNamesJson.size())
            continue;

        int priority = json[i][PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY].asInt(1000);
        bool isDisplayed = json[i][PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED].asBool(true);
        bool doNotHideOtherPropertiesOnDisplayOverride = json[i][PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_DONOTHIDEOTHERPROPERTIESONDISPLAYOVERRIDE].asBool(false);

        for (BeJsConst::ArrayIndex j = 0; j < propertyNamesJson.size(); ++j)
            {
            Utf8CP propertyName = propertyNamesJson[j].asCString();
            specs.push_back(new PropertySpecification(propertyName, priority, "", nullptr, isDisplayed, nullptr, nullptr, doNotHideOtherPropertiesOnDisplayOverride));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentModifiersList::ReadJson(BeJsConst json)
    {
    if (json.isMember(CONTENTMODIFIER_JSON_ATTRIBUTE_RELATEDPROPERTIES))
        {
        CommonToolsInternal::LoadFromJson(CONTENTMODIFIER_RULE_JSON_TYPE, CONTENTMODIFIER_JSON_ATTRIBUTE_RELATEDPROPERTIES, json[CONTENTMODIFIER_JSON_ATTRIBUTE_RELATEDPROPERTIES],
            m_relatedProperties, CommonToolsInternal::LoadRuleFromJson<RelatedPropertiesSpecification>, this);
        }
    if (json.isMember(CONTENTMODIFIER_JSON_ATTRIBUTE_CALCULATEDPROPERTIES))
        {
        CommonToolsInternal::LoadFromJson(CONTENTMODIFIER_RULE_JSON_TYPE, CONTENTMODIFIER_JSON_ATTRIBUTE_CALCULATEDPROPERTIES, json[CONTENTMODIFIER_JSON_ATTRIBUTE_CALCULATEDPROPERTIES],
            m_calculatedProperties, CommonToolsInternal::LoadRuleFromJson<CalculatedPropertiesSpecification>, this);
        }
    if (json.isMember(CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYCATEGORIES))
        {
        CommonToolsInternal::LoadFromJson(CONTENTMODIFIER_RULE_JSON_TYPE, CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYCATEGORIES, json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYCATEGORIES],
            m_propertyCategories, CommonToolsInternal::LoadRuleFromJson<PropertyCategorySpecification>, this);
        }
    if (json.isMember(CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYOVERRIDES))
        {
        CommonToolsInternal::LoadFromJson(CONTENTMODIFIER_RULE_JSON_TYPE, CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYOVERRIDES, json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYOVERRIDES],
            m_propertyOverrides, CommonToolsInternal::LoadRuleFromJson<PropertySpecification>, this);
        }
    if (json.isMember(CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYEDITORS))
        {
        CommonToolsInternal::LoadFromJson(CONTENTMODIFIER_RULE_JSON_TYPE, CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYEDITORS, json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYEDITORS], m_propertyOverrides, &CreatePropertyEditorSpecification, this);
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_WARNING, Utf8PrintfString("Using deprecated `%s.%s`. It's recommended to switch to `%s`.",
            CONTENTMODIFIER_RULE_JSON_TYPE, CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYEDITORS, CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYOVERRIDES));
        }
    if (json.isMember(CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYDISPLAYSPECIFICATIONS))
        {
        if (CommonToolsInternal::ValidateJsonArrayValueType(CONTENTMODIFIER_RULE_JSON_TYPE, CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYDISPLAYSPECIFICATIONS, json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYDISPLAYSPECIFICATIONS]))
            LoadPropertyDisplaySpecifications(json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYDISPLAYSPECIFICATIONS], m_propertyOverrides, this);
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_WARNING, Utf8PrintfString("Using deprecated `%s.%s`. It's recommended to switch to `%s`.",
            CONTENTMODIFIER_RULE_JSON_TYPE, CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYDISPLAYSPECIFICATIONS, CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYOVERRIDES));
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifiersList::WriteJson(BeJsValue json) const
    {
    if (!m_calculatedProperties.empty())
        {
        CommonToolsInternal::WriteRulesToJson<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>
            (json[CONTENTMODIFIER_JSON_ATTRIBUTE_CALCULATEDPROPERTIES], m_calculatedProperties);
        }
    if (!m_relatedProperties.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>
            (json[CONTENTMODIFIER_JSON_ATTRIBUTE_RELATEDPROPERTIES], m_relatedProperties);
        }
    if (!m_propertyCategories.empty())
        {
        CommonToolsInternal::WriteRulesToJson<PropertyCategorySpecification, PropertyCategorySpecificationsList>
            (json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYCATEGORIES], m_propertyCategories);
        }
    if (!m_propertyOverrides.empty())
        {
        CommonToolsInternal::WriteRulesToJson<PropertySpecification, PropertySpecificationsList>
            (json[CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYOVERRIDES], m_propertyOverrides);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentModifiersList::_ComputeHash() const
    {
    MD5 md5;
    ADD_RULES_TO_HASH(md5, CONTENTMODIFIER_JSON_ATTRIBUTE_RELATEDPROPERTIES, m_relatedProperties);
    ADD_RULES_TO_HASH(md5, CONTENTMODIFIER_JSON_ATTRIBUTE_CALCULATEDPROPERTIES, m_calculatedProperties);
    ADD_RULES_TO_HASH(md5, CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYCATEGORIES, m_propertyCategories);
    ADD_RULES_TO_HASH(md5, CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYOVERRIDES, m_propertyOverrides);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::ContentModifier(ContentModifier const& other)
    : PrioritizedPresentationKey(other), m_schemaName(other.m_schemaName), m_className(other.m_className), m_modifiers(other.m_modifiers), m_applyOnNestedContent(other.m_applyOnNestedContent)
    {
    CommonToolsInternal::CopyRules(m_requiredSchemas, other.m_requiredSchemas, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::ContentModifier(ContentModifier&& other)
    : PrioritizedPresentationKey(std::move(other)), m_schemaName(std::move(other.m_schemaName)), m_className(std::move(other.m_className)), m_modifiers(std::move(other.m_modifiers)), m_applyOnNestedContent(other.m_applyOnNestedContent)
    {
    CommonToolsInternal::SwapRules(m_requiredSchemas, other.m_requiredSchemas, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::~ContentModifier()
    {
    CommonToolsInternal::FreePresentationRules(m_requiredSchemas);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentModifier::_GetJsonElementTypeAttributeName() const {return COMMON_JSON_ATTRIBUTE_RULETYPE;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ContentModifier::_GetJsonElementType() const
    {
    return CONTENTMODIFIER_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentModifier::_ReadJson(BeJsConst json)
    {
    if (!PrioritizedPresentationKey::_ReadJson(json))
        return false;

    if (json.isMember(COMMON_JSON_ATTRIBUTE_CLASS))
        CommonToolsInternal::ParseSchemaAndClassName(m_schemaName, m_className, json[COMMON_JSON_ATTRIBUTE_CLASS], Utf8PrintfString("%s.%s", _GetJsonElementType(), COMMON_JSON_ATTRIBUTE_CLASS).c_str());
    if (json.isMember(COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS))
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS, json[COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS], m_requiredSchemas, CommonToolsInternal::LoadRuleFromJson<RequiredSchemaSpecification>, this);
    m_applyOnNestedContent = json[CONTENTMODIFIER_JSON_ATTRIBUTE_APPLYONNESTEDCONTENT].asBool(false);
    return m_modifiers.ReadJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::_WriteJson(BeJsValue json) const
    {
    PrioritizedPresentationKey::_WriteJson(json);

    if (!m_schemaName.empty() && !m_className.empty())
        CommonToolsInternal::WriteSchemaAndClassNameToJson(json[COMMON_JSON_ATTRIBUTE_CLASS], m_schemaName, m_className);
    if (m_applyOnNestedContent)
        json[CONTENTMODIFIER_JSON_ATTRIBUTE_APPLYONNESTEDCONTENT] = m_applyOnNestedContent;

    m_modifiers.WriteJson(json);

    if (!m_requiredSchemas.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RequiredSchemaSpecification, RequiredSchemaSpecificationsList>
            (json[COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS], m_requiredSchemas);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ContentModifier::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_schemaName.empty())
        ADD_STR_VALUE_TO_HASH(md5, SCHEMA_CLASS_SPECIFICATION_SCHEMANAME, m_schemaName);
    if (!m_className.empty())
        ADD_STR_VALUE_TO_HASH(md5, SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME, m_className);
    ADD_RULES_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS, m_requiredSchemas);

    Utf8StringCR modifiersHash = m_modifiers.GetHash();
    md5.Add(modifiersHash.c_str(), modifiersHash.size());

    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::ClearRequiredSchemaSpecifications()
    {
    CommonToolsInternal::FreePresentationRules(m_requiredSchemas);
    InvalidateHash();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::AddRequiredSchemaSpecification(RequiredSchemaSpecificationR spec) {ADD_HASHABLE_CHILD(m_requiredSchemas, spec);}
