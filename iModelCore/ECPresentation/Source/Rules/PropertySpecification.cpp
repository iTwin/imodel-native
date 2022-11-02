/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/CommonTools.h>
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertySpecification::PropertySpecification(Utf8String propertyName, int overridesPriority, Utf8String labelOverride, std::unique_ptr<PropertyCategoryIdentifier> categoryId, BoolOrString isDisplayed,
    CustomRendererSpecification* rendererOverride, PropertyEditorSpecification* editorOverride, bool doNotHideOtherPropertiesOnDisplayOverride, Nullable<bool> isReadOnly, Nullable<int> priority)
    : m_propertyName(propertyName), m_overridesPriority(overridesPriority), m_labelOverride(labelOverride), m_isDisplayed(isDisplayed),
    m_doNotHideOtherPropertiesOnDisplayOverride(doNotHideOtherPropertiesOnDisplayOverride), m_rendererOverride(rendererOverride), m_editorOverride(editorOverride), m_categoryId(std::move(categoryId)),
    m_isReadOnly(isReadOnly), m_priority(priority)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertySpecification::PropertySpecification(PropertySpecification const& other)
    : T_Super(other), m_propertyName(other.m_propertyName), m_overridesPriority(other.m_overridesPriority), m_labelOverride(other.m_labelOverride), m_isDisplayed(other.m_isDisplayed),
    m_doNotHideOtherPropertiesOnDisplayOverride(other.m_doNotHideOtherPropertiesOnDisplayOverride),
    m_rendererOverride(other.m_rendererOverride ? new CustomRendererSpecification(*other.m_rendererOverride) : nullptr),
    m_editorOverride(other.m_editorOverride ? new PropertyEditorSpecification(*other.m_editorOverride) : nullptr), m_isReadOnly(other.m_isReadOnly), m_priority(other.m_priority)
    {
    if (other.m_categoryId)
        m_categoryId = other.m_categoryId->Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertySpecification::PropertySpecification(PropertySpecification&& other)
    : T_Super(std::move(other)), m_propertyName(std::move(other.m_propertyName)), m_overridesPriority(other.m_overridesPriority), m_labelOverride(std::move(other.m_labelOverride)),
    m_isDisplayed(std::move(other.m_isDisplayed)), m_doNotHideOtherPropertiesOnDisplayOverride(other.m_doNotHideOtherPropertiesOnDisplayOverride),
    m_rendererOverride(other.m_rendererOverride), m_editorOverride(other.m_editorOverride), m_categoryId(std::move(other.m_categoryId)), m_isReadOnly(std::move(other.m_isReadOnly)),
    m_priority(std::move(other.m_priority))
    {
    other.m_rendererOverride = nullptr;
    other.m_editorOverride = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertySpecification::~PropertySpecification()
    {
    DELETE_AND_CLEAR(m_rendererOverride);
    DELETE_AND_CLEAR(m_editorOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertySpecification::_GetJsonElementType() const { return "PropertySpecification"; }

/*---------------------------------------------------------------------------------**//**
* note: this is only needed to support deprecated PropertyEditorsSpecification
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertySpecification::ReadEditorSpecificationJson(JsonValueCR json)
    {
    // required:
    m_propertyName = json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME].asCString();
    m_editorOverride = CommonToolsInternal::LoadRuleFromJson<PropertyEditorSpecification>(json);

    bool hasIssues = (nullptr == m_editorOverride)
        || CommonToolsInternal::CheckRuleIssue(m_propertyName.empty(), _GetJsonElementType(), PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME, json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME], "non-empty string");
    if (hasIssues)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertySpecification::_ReadJson(JsonValueCR json)
    {
    if (!T_Super::_ReadJson(json))
        return false;

    // required:
    m_propertyName = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_NAME].asCString();
    if (CommonToolsInternal::CheckRuleIssue(m_propertyName.empty(), _GetJsonElementType(), PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_NAME, json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_NAME], "non-empty string"))
        return false;

    // optional:
    m_overridesPriority = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_OVERRIDESPRIORITY].asInt(1000);
    m_labelOverride = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_LABELOVERRIDE].asCString("");

    if (json.isMember(PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID))
        m_categoryId = PropertyCategoryIdentifier::Create(json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID]);

    if (json.isMember(PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED))
        {
        if (json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED].isBool())
            m_isDisplayed = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED].asBool();
        else 
            m_isDisplayed = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED].asCString();
        }
    else
        m_isDisplayed = nullptr;

    m_doNotHideOtherPropertiesOnDisplayOverride = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_DONOTHIDEOTHERPROPERTIESONDISPLAYOVERRIDE].asBool(false);

    if (json.isMember(PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISREADONLY))
        m_isReadOnly = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISREADONLY].asBool();
    else
        m_isReadOnly = nullptr;

    if (json.isMember(PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY))
        m_priority = json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY].asInt();
    else
        m_priority = nullptr;

    DELETE_AND_CLEAR(m_rendererOverride);
    if (json.isMember(PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_RENDERER))
        m_rendererOverride = CommonToolsInternal::LoadRuleFromJson<CustomRendererSpecification>(json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_RENDERER]);

    DELETE_AND_CLEAR(m_editorOverride);
    if (json.isMember(PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_EDITOR))
        m_editorOverride = CommonToolsInternal::LoadRuleFromJson<PropertyEditorSpecification>(json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_EDITOR]);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertySpecification::_WriteJson(JsonValueR json) const
    {
    T_Super::_WriteJson(json);
    json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_NAME] = m_propertyName;
    if (m_overridesPriority != 1000)
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_OVERRIDESPRIORITY] = m_overridesPriority;
    if (!m_labelOverride.empty())
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_LABELOVERRIDE] = m_labelOverride;
    if (m_isDisplayed.IsValid())
        {
        if (m_isDisplayed.IsBoolean())
            json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED] = m_isDisplayed.GetBoolean();
        else
            json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED] = m_isDisplayed.GetString();
        if ((m_isDisplayed.IsBoolean() && m_isDisplayed.GetBoolean() || m_isDisplayed.IsString()) && m_doNotHideOtherPropertiesOnDisplayOverride)
            json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_DONOTHIDEOTHERPROPERTIESONDISPLAYOVERRIDE] = m_doNotHideOtherPropertiesOnDisplayOverride;
        }
    if (m_isReadOnly.IsValid())
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISREADONLY] = m_isReadOnly.Value();
    if (m_priority.IsValid())
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY] = m_priority.Value();
    if (nullptr != m_rendererOverride)
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_RENDERER] = m_rendererOverride->WriteJson();
    if (nullptr != m_editorOverride)
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_EDITOR] = m_editorOverride->WriteJson();
    if (m_categoryId)
        json[PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID] = m_categoryId->WriteJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertySpecification::SetRendererOverride(CustomRendererSpecification* value)
    {
    if (value == m_rendererOverride)
        return;

    InvalidateHash();
    DELETE_AND_CLEAR(m_rendererOverride);
    if (value)
        value->SetParent(this);
    m_rendererOverride = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertySpecification::SetEditorOverride(PropertyEditorSpecification* value)
    {
    if (value == m_editorOverride)
        return;

    InvalidateHash();
    DELETE_AND_CLEAR(m_editorOverride);
    if (value)
        value->SetParent(this);
    m_editorOverride = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertySpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_propertyName.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_NAME, m_propertyName);
    if (m_overridesPriority != 1000)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_OVERRIDESPRIORITY, m_overridesPriority);

    if (!m_labelOverride.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_LABELOVERRIDE, m_labelOverride);

    if (m_isDisplayed.IsBoolean())
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED, m_isDisplayed.GetBoolean())
    else if (m_isDisplayed.IsString())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED, m_isDisplayed.GetString())

    if (m_doNotHideOtherPropertiesOnDisplayOverride)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_DONOTHIDEOTHERPROPERTIESONDISPLAYOVERRIDE, m_doNotHideOtherPropertiesOnDisplayOverride);

    if (m_isReadOnly.IsValid())
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISREADONLY, m_isReadOnly.Value());

    if (m_priority.IsValid())
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY, m_priority.Value());

    if (nullptr != m_rendererOverride)
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_RENDERER, m_rendererOverride->GetHash());

    if (nullptr != m_editorOverride)
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_EDITOR, m_editorOverride->GetHash());

    if (nullptr != m_categoryId)
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID, m_categoryId->GetHash());

    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertySpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    PropertySpecification const* otherRule = dynamic_cast<PropertySpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_propertyName == otherRule->m_propertyName
        && m_overridesPriority == otherRule->m_overridesPriority
        && m_labelOverride == otherRule->m_labelOverride
        && m_isDisplayed == otherRule->m_isDisplayed
        && m_doNotHideOtherPropertiesOnDisplayOverride == otherRule->m_doNotHideOtherPropertiesOnDisplayOverride
        && m_categoryId == otherRule->m_categoryId
        && m_isReadOnly == otherRule->m_isReadOnly
        && m_priority == otherRule->m_priority;
    }
