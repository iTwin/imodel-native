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
UserSettingsGroup::UserSettingsGroup()
    : PrioritizedPresentationKey()
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroup::UserSettingsGroup (Utf8StringCR categoryLabel)
    : m_categoryLabel (categoryLabel)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroup::UserSettingsGroup(UserSettingsGroupCR other)
    : PrioritizedPresentationKey(other)
    {
    CommonToolsInternal::CopyRules(m_nestedSettings, other.m_nestedSettings, this);
    CommonToolsInternal::CopyRules(m_settingsItems, other.m_settingsItems, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroup::~UserSettingsGroup (void)
    {
    CommonToolsInternal::FreePresentationRules (m_nestedSettings);
    CommonToolsInternal::FreePresentationRules (m_settingsItems);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP UserSettingsGroup::_GetJsonElementType() const {return USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettingsGroup::_ReadJson(BeJsConst json)
    {
    if (!PrioritizedPresentationKey::_ReadJson(json))
        return false;

    m_categoryLabel = json[USER_SETTINGS_JSON_ATTRIBUTE_CATEGORY_LABEL].asCString("");
    if (json.isMember(USER_SETTINGS_JSON_ATTRIBUTE_SETTINGS_ITEMS))
        {
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), USER_SETTINGS_JSON_ATTRIBUTE_SETTINGS_ITEMS, json[USER_SETTINGS_JSON_ATTRIBUTE_SETTINGS_ITEMS],
            m_settingsItems, CommonToolsInternal::LoadRuleFromJson<UserSettingsItem>, this);
        }
    if (json.isMember(USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS))
        {
        CommonToolsInternal::LoadFromJsonByPriority(_GetJsonElementType(), USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS, json[USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS],
            m_nestedSettings, CommonToolsInternal::LoadRuleFromJson<UserSettingsGroup>, this);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsGroup::_WriteJson(BeJsValue json) const
    {
    PrioritizedPresentationKey::_WriteJson(json);
    json[USER_SETTINGS_JSON_ATTRIBUTE_CATEGORY_LABEL] = m_categoryLabel;
    if (!m_settingsItems.empty())
        CommonToolsInternal::WriteRulesToJson<UserSettingsItem, UserSettingsItemList>(json[USER_SETTINGS_JSON_ATTRIBUTE_SETTINGS_ITEMS], m_settingsItems);
    if (!m_nestedSettings.empty())
        CommonToolsInternal::WriteRulesToJson<UserSettingsGroup, UserSettingsGroupList>(json[USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS], m_nestedSettings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UserSettingsGroup::GetCategoryLabel (void) const { return m_categoryLabel; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsGroup::AddSettingsItem(UserSettingsItemR item)
    {
    ADD_HASHABLE_CHILD(m_settingsItems, item);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsGroup::AddNestedSettings(UserSettingsGroupR group)
    {
    ADD_HASHABLE_CHILD(m_nestedSettings, group);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsItemList const& UserSettingsGroup::GetSettingsItems (void) const { return m_settingsItems; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsGroupList const& UserSettingsGroup::GetNestedSettings (void) const { return m_nestedSettings; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 UserSettingsGroup::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_categoryLabel.empty())
        ADD_STR_VALUE_TO_HASH(md5, USER_SETTINGS_JSON_ATTRIBUTE_CATEGORY_LABEL, m_categoryLabel);
    ADD_RULES_TO_HASH(md5, USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS, m_nestedSettings);
    ADD_RULES_TO_HASH(md5, USER_SETTINGS_JSON_ATTRIBUTE_SETTINGS_ITEMS, m_settingsItems);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsItem::UserSettingsItem() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsItem::UserSettingsItem(Utf8StringCR id, Utf8StringCR label, Utf8StringCR options, Utf8StringCR defaultValue)
    : m_id(id), m_label(label), m_options(options), m_defaultValue(defaultValue)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP UserSettingsItem::_GetJsonElementType() const { return "UserSettingsItem"; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettingsItem::_ReadJson(BeJsConst json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    // required:
    m_id = json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_ID].asCString("");
    m_label = json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_LABEL].asCString("");

    bool hasIssues = false
        || CommonToolsInternal::CheckRuleIssue(m_id.empty(), USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS "." USER_SETTINGS_JSON_ATTRIBUTE_SETTINGS_ITEMS, USER_SETTINGS_ITEM_JSON_ATTRIBUTE_ID, json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_ID], "non-empty string")
        || CommonToolsInternal::CheckRuleIssue(m_label.empty(), USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS "." USER_SETTINGS_JSON_ATTRIBUTE_SETTINGS_ITEMS, USER_SETTINGS_ITEM_JSON_ATTRIBUTE_LABEL, json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_LABEL], "non-empty string");
    if (hasIssues)
        return false;

    // optional:
    m_options = json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_OPTIONS].asCString("");
    m_defaultValue = json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_DEFAULT_VALUE].asCString("");

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsItem::_WriteJson(BeJsValue json) const
    {
    PresentationKey::_WriteJson(json);
    json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_ID] = m_id;
    json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_LABEL] = m_label;
    if (!m_options.empty())
        json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_OPTIONS] = m_options;
    if (!m_defaultValue.empty())
        json[USER_SETTINGS_ITEM_JSON_ATTRIBUTE_DEFAULT_VALUE] = m_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UserSettingsItem::GetId (void) const               { return m_id; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UserSettingsItem::GetLabel (void) const            { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UserSettingsItem::GetOptions (void) const          { return m_options; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR UserSettingsItem::GetDefaultValue (void) const     { return m_defaultValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 UserSettingsItem::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_id.empty())
        md5.Add(m_id.c_str(), m_id.size());
    if (!m_label.empty())
        md5.Add(m_label.c_str(), m_label.size());
    if (!m_options.empty())
        md5.Add(m_options.c_str(), m_options.size());
    if (!m_defaultValue.empty())
        md5.Add(m_defaultValue.c_str(), m_defaultValue.size());
    return md5;
    }
