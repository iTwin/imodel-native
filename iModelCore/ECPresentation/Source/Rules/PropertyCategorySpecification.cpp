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
std::unique_ptr<PropertyCategoryIdentifier> PropertyCategoryIdentifier::CreateForRoot()
    {
    return std::unique_ptr<PropertyCategoryIdentifier>(new PropertyCategoryIdentifier(PropertyCategoryIdentifierType::Root));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<PropertyCategoryIdentifier> PropertyCategoryIdentifier::CreateForDefaultParent()
    {
    return std::unique_ptr<PropertyCategoryIdentifier>(new PropertyCategoryIdentifier(PropertyCategoryIdentifierType::DefaultParent));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<PropertyCategoryIdentifier> PropertyCategoryIdentifier::CreateForId(Utf8String id)
    {
    return std::unique_ptr<IdPropertyCategoryIdentifier>(new IdPropertyCategoryIdentifier(id));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<PropertyCategoryIdentifier> PropertyCategoryIdentifier::Create(JsonValueCR json)
    {
    if (json.isObject() && 0 == strcmp(PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_NONE, json[PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_TYPE].asCString()))
        return nullptr;

    std::unique_ptr<PropertyCategoryIdentifier> identifier;
    if (json.isString() || json.isObject() && 0 == strcmp(PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_ID, json[PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_TYPE].asCString()))
        identifier = std::unique_ptr<IdPropertyCategoryIdentifier>(new IdPropertyCategoryIdentifier());
    else
        identifier = std::unique_ptr<PropertyCategoryIdentifier>(new PropertyCategoryIdentifier(PropertyCategoryIdentifierType::Root));
    return identifier->ReadJson(json) ? std::move(identifier) : nullptr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyCategoryIdentifier::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!PresentationKey::_ShallowEqual(other))
        return false;

    auto otherRule = static_cast<PropertyCategoryIdentifier const&>(other);
    return m_type == otherRule.m_type;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyCategoryIdentifier::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_TYPE, m_type);
    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyCategoryIdentifier::_GetJsonElementType() const {return "PropertyCategoryIdentifier";}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyCategoryIdentifier::_ReadJson(JsonValueCR json)
    {
    if (!json.isObject() || json.isNull())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid value for `%s`: `%s`. Expected %s.",
            _GetJsonElementType(), json.ToString().c_str(), "an object with a `type` attribute"));
        return false;
        }

    Utf8CP type = json[PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_TYPE].asCString("");
    if (0 == strcmp(PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_ROOT, type))
        m_type = PropertyCategoryIdentifierType::Root;
    else if (0 == strcmp(PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_DEFAULTPARENT, type))
        m_type = PropertyCategoryIdentifierType::DefaultParent;
    else if (0 == strcmp(PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_ID, type))
        m_type = PropertyCategoryIdentifierType::Id;
    else
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid value for `%s.%s`: `%s`. Expected %s.",
            _GetJsonElementType(), PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_TYPE, json.ToString().c_str(), "one of \"Root\", \"DefaultParent\" or \"Id\""));
        return false;
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCategoryIdentifier::_WriteJson(JsonValueR json) const
    {
    switch (m_type)
        {
        case PropertyCategoryIdentifierType::Root:
            json[PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_TYPE] = PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_ROOT;
            break;
        case PropertyCategoryIdentifierType::DefaultParent:
            json[PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_TYPE] = PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_DEFAULTPARENT;
            break;
        case PropertyCategoryIdentifierType::Id:
            json[PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_TYPE] = PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_ID;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IdPropertyCategoryIdentifier::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!PropertyCategoryIdentifier::_ShallowEqual(other))
        return false;

    auto otherRule = static_cast<IdPropertyCategoryIdentifier const&>(other);
    return m_categoryId.Equals(otherRule.GetCategoryId());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 IdPropertyCategoryIdentifier::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_categoryId.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID, m_categoryId);
    return md5;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IdPropertyCategoryIdentifier::_ReadJson(JsonValueCR json)
    {
    if (json.isString())
        {
        m_categoryId = json.asCString("");
        if (m_categoryId.empty())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_INFO, LOG_ERROR, Utf8PrintfString("Invalid value for `%s`: `%s`. Expected %s.",
                _GetJsonElementType(), json.ToString().c_str(), "non-empty string or an object"));
            return false;
            }
        return true;
        }

    if (!PropertyCategoryIdentifier::_ReadJson(json))
        return false;

    m_categoryId = json[PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID].asCString("");
    if (CommonToolsInternal::CheckRuleIssue(m_categoryId.empty(), _GetJsonElementType(), PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID, json[PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID], "non-empty string"))
        return false;

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void IdPropertyCategoryIdentifier::_WriteJson(JsonValueR json) const
    {
    PropertyCategoryIdentifier::_WriteJson(json);
    json[PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID] = m_categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyCategorySpecification::PropertyCategorySpecification(Utf8String id, Utf8String label, Utf8String description,
    int priority, bool autoExpand, std::unique_ptr<CustomRendererSpecification> rendererOverride, std::unique_ptr<PropertyCategoryIdentifier> parentId)
    : m_id(id), m_label(label), m_description(description), m_rendererOverride(std::move(rendererOverride)), m_autoExpand(autoExpand), m_parentId(std::move(parentId))
    {
    SetPriority(priority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyCategorySpecification::PropertyCategorySpecification(PropertyCategorySpecification const& other)
    : T_Super(other), m_id(other.m_id), m_label(other.m_label),
    m_description(other.m_description), m_rendererOverride(nullptr), m_autoExpand(other.m_autoExpand)
    {
    if (other.m_parentId != nullptr)
        m_parentId = other.m_parentId->Clone();
    if (other.m_rendererOverride != nullptr)
        m_rendererOverride = std::make_unique<CustomRendererSpecification>(*other.m_rendererOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyCategorySpecification::PropertyCategorySpecification(PropertyCategorySpecification&& other)
    : T_Super(std::move(other)), m_id(std::move(other.m_id)), m_parentId(std::move(other.m_parentId)), m_label(std::move(other.m_label)),
    m_description(std::move(other.m_description)), m_autoExpand(other.m_autoExpand), m_rendererOverride(std::move(other.m_rendererOverride))
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyCategorySpecification& PropertyCategorySpecification::operator=(PropertyCategorySpecification const& other)
    {
    m_id = other.m_id;
    m_parentId = other.m_parentId != nullptr ? other.m_parentId->Clone() : nullptr;
    m_label = other.m_label;
    m_description = other.m_description;
    m_rendererOverride = other.m_rendererOverride != nullptr
        ? std::make_unique<CustomRendererSpecification>(*other.m_rendererOverride)
        : nullptr;
    m_autoExpand = other.m_autoExpand;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyCategorySpecification& PropertyCategorySpecification::operator=(PropertyCategorySpecification&& other)
    {
    m_id = std::move(other.m_id);
    m_parentId = std::move(other.m_parentId);
    m_label = std::move(other.m_label);
    m_description = std::move(other.m_description);
    m_rendererOverride = std::move(other.m_rendererOverride);
    m_autoExpand = other.m_autoExpand;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyCategorySpecification::_GetJsonElementType() const {return "PropertyCategorySpecification";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyCategorySpecification::_ReadJson(JsonValueCR json)
    {
    if (!T_Super::_ReadJson(json))
        return false;

    // required:
    m_id = json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_ID].asCString("");
    m_label = json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_LABEL].asCString("");

    bool hasIssues = false
        || CommonToolsInternal::CheckRuleIssue(m_id.empty(), _GetJsonElementType(), PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_ID, json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_ID], "non-empty string")
        || CommonToolsInternal::CheckRuleIssue(m_label.empty(), _GetJsonElementType(), PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_LABEL, json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_LABEL], "non-empty string");
    if (hasIssues)
        return false;

    // optional:
    if (json.isMember(PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_PARENTID))
        m_parentId = PropertyCategoryIdentifier::Create(json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_PARENTID]);
    else
        m_parentId = PropertyCategoryIdentifier::CreateForDefaultParent();

    m_description = json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION].asCString("");
    m_autoExpand = json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_AUTOEXPAND].asBool();

    m_rendererOverride.reset(nullptr);
    if (json.isMember(PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_RENDERER))
        m_rendererOverride.reset(CommonToolsInternal::LoadRuleFromJson<CustomRendererSpecification>(json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_RENDERER]));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyCategorySpecification::_WriteJson(JsonValueR json) const
    {
    T_Super::_WriteJson(json);
    json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_ID] = m_id;
    if (m_parentId)
        json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_PARENTID] = m_parentId->WriteJson();
    json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_LABEL] = m_label;
    if (!m_description.empty())
        json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION] = m_description;
    if (m_autoExpand)
        json[PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_AUTOEXPAND] = m_autoExpand;
    if (m_rendererOverride)
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_RENDERER] = m_rendererOverride->WriteJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyCategorySpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();

    if (!m_id.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_ID, m_id);

    if (nullptr != m_parentId)
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_PARENTID, m_parentId->GetHash());

    if (!m_label.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_LABEL, m_label);

    if (!m_description.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION, m_description);

    if (nullptr != m_rendererOverride)
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_RENDERER, m_rendererOverride->GetHash());

    if (m_autoExpand)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_AUTOEXPAND, m_autoExpand);

    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyCategorySpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!T_Super::_ShallowEqual(other))
        return false;

    PropertyCategorySpecification const* otherRule = dynamic_cast<PropertyCategorySpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_id == otherRule->m_id
        && m_parentId == otherRule->m_parentId
        && m_label == otherRule->m_label
        && m_description == otherRule->m_description
        && m_autoExpand == otherRule->m_autoExpand;
    }
