/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/UserSettings.h>
#include "RulesEngineTypes.h"

#define USER_SETTINGS_NAMESPACE "RulesEngine.UserSettings"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
BeJsDocument convertToBeJsValue(T value)
    {
    BeJsDocument doc;
    (BeJsValue)doc = value;
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_UnsetValue(Utf8CP id)
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    if (!HasSetting(id))
        return;

    Utf8PrintfString settingId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveValue(USER_SETTINGS_NAMESPACE, settingId.c_str(), BeJsDocument::Null());
    m_settingIds.erase(std::remove_if(m_settingIds.begin(), m_settingIds.end(), [&settingId](auto const& entry){return entry.first.Equals(settingId);}), m_settingIds.end());

    if (nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingValue(Utf8CP id, Utf8CP value)
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    if (!m_isInitializing && HasSetting(id) && GetSettingValue(id).Equals(value))
        return;

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), convertToBeJsValue(value));
    m_settingIds.insert(bpair<Utf8String, Utf8String>(id, "string"));

    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingIntValue(Utf8CP id, int64_t value)
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    if (!m_isInitializing && HasSetting(id) && GetSettingIntValue(id) == value)
        return;

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), convertToBeJsValue(value));
    m_settingIds.insert(bpair<Utf8String, Utf8String>(id, "int"));

    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<int64_t> VectorFromJsonArray(BeJsConst json)
    {
    bvector<int64_t> vec;
    if (!json.isArray())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Expected a JSON array");
    for (BeJsConst::ArrayIndex i = 0; i < json.size(); ++i)
        vec.push_back(json[i].asInt64());
    return vec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BeJsDocument JsonArrayFromVector(bvector<int64_t> const& vec)
    {
    BeJsDocument json;
    json.toArray();
    for (int64_t v : vec)
        json[json.size()] = v;
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values)
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    if (!m_isInitializing && HasSetting(id) && GetSettingIntValues(id) == values)
        return;

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), JsonArrayFromVector(values));
    m_settingIds.insert(bpair<Utf8String, Utf8String>(id, "ints"));

    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingBoolValue(Utf8CP id, bool value)
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    if (!m_isInitializing && HasSetting(id) && GetSettingBoolValue(id) == value)
        return;

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), convertToBeJsValue(value));
    m_settingIds.insert(bpair<Utf8String, Utf8String>(id, "bool"));

    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UserSettings::_GetSettingValue(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    static Utf8String s_defaultValue = "";
    if (nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    BeJsDocument value = m_localState->GetValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    return value.isString() ? value.asCString() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t UserSettings::_GetSettingIntValue(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    static int64_t s_defaultValue = 0;
    if (nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    BeJsDocument value = m_localState->GetValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    return value.isNumeric() ? value.asInt64() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<int64_t> UserSettings::_GetSettingIntValues(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    bvector<int64_t> values;
    if (nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    BeJsDocument jsonValues = m_localState->GetValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    if (!jsonValues.isArray())
        return values;

    return VectorFromJsonArray(jsonValues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettings::_GetSettingBoolValue(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    static bool s_defaultValue = false;
    if (nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    BeJsDocument value = m_localState->GetValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    return value.isBool() ? value.asBool() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeJsDocument UserSettings::_GetSettingValueAsJson(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    return m_localState->GetValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<bpair<Utf8String, Utf8String>> UserSettings::_GetSettings() const
    {
    return ContainerHelpers::TransformContainer<bvector<bpair<Utf8String, Utf8String>>>(m_settingIds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettings::_HasSetting(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    BeJsDocument json = m_localState->GetValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    return !json.isNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::AddValues(BeJsValue groupListJson) const
    {
    for (BeJsValue::ArrayIndex i = 0; i < groupListJson.size(); i++)
        {
        BeJsValue groupJson = groupListJson[i];
        if (groupJson.isMember("Items"))
            {
            for (BeJsValue::ArrayIndex j = 0; j < groupJson["Items"].size(); j++)
                {
                BeJsValue itemJson = groupJson["Items"][j];
                Utf8CP options = itemJson["Options"].asCString();
                if (0 == strcmp("StringValue", options))
                    itemJson["Value"] = GetSettingValue(itemJson["Id"].asCString()).c_str();
                else if (0 == strcmp("IntValue", options))
                    itemJson["Value"] = GetSettingIntValue(itemJson["Id"].asCString());
                else
                    itemJson["Value"] = GetSettingBoolValue(itemJson["Id"].asCString());
                }
            }
        if (groupJson.isMember("NestedGroups"))
            AddValues(groupJson["NestedGroups"]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeJsDocument UserSettings::_GetPresentationInfo() const
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    BeJsDocument json;
    json.From(m_presentationInfo);
    AddValues(json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_InitFrom(UserSettingsGroupList const& rules)
    {
    BeMutexHolder lock(m_mutex);

    if (!rules.empty() && nullptr == m_localState)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::RulesetVariables, "Local state not set up");

    if (m_initializedFrom == &rules)
        return;

    m_isInitializing = true;
    m_presentationInfo.SetNull();
    InitFromJson(rules, m_presentationInfo);
    m_initializedFrom = &rules;
    m_isInitializing = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetSettingType(Utf8StringCR options)
    {
    if (options.Equals("StringValue"))
        return "string";
    if (options.Equals("IntValue"))
        return "int";

    return "bool";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::InitFromJson(UserSettingsGroupList const& rules, BeJsValue presentationInfo)
    {
    presentationInfo.toArray();
    for (UserSettingsGroupCP group : rules)
        {
        BeJsValue groupJson = presentationInfo[presentationInfo.size()];
        groupJson.toObject();
        groupJson["Label"] = group->GetCategoryLabel();
        if (!group->GetSettingsItems().empty())
            groupJson["Items"].toArray();

        for (UserSettingsItemCP item : group->GetSettingsItems())
            {
            Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), item->GetId().c_str());
            if (m_localState->GetValue(USER_SETTINGS_NAMESPACE, stringId.c_str()).isNull())
                {
                // save the value in local state
                if (item->GetOptions().Equals("StringValue"))
                    {
                    SetSettingValue(item->GetId().c_str(), item->GetDefaultValue().c_str());
                    }
                else if (item->GetOptions().Equals("IntValue"))
                    {
                    int value = atoi(item->GetDefaultValue().c_str());
                    SetSettingIntValue(item->GetId().c_str(), value);
                    }
                else
                    {
                    bool value = item->GetDefaultValue().EqualsI("true") || item->GetDefaultValue().Equals("1");
                    SetSettingBoolValue(item->GetId().c_str(), value);
                    }
                }
            else
                {
                m_settingIds.insert(bpair<Utf8String, Utf8String>(item->GetId(), GetSettingType(item->GetOptions())));
                }

            // create the presentation info
            BeJsValue groupJsonItems = groupJson["Items"];
            BeJsValue itemPresentationInfo = groupJsonItems[groupJsonItems.size()];
            itemPresentationInfo["Id"] = item->GetId();
            itemPresentationInfo["Label"] = item->GetLabel();
            itemPresentationInfo["Options"] = item->GetOptions().empty() ? "TrueFalse" : item->GetOptions();
            }

        if (!group->GetNestedSettings().empty())
            InitFromJson(group->GetNestedSettings(), groupJson["NestedGroups"]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsManager::UserSettingsManager(BeFileNameCR) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsManager::~UserSettingsManager()
    {
    BeMutexHolder lock(GetMutex());
    for (auto iter : m_settings)
        delete iter.second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::_OnLocalStateChanged()
    {
    BeMutexHolder lock(GetMutex());
    for (auto iter : m_settings)
        iter.second->SetLocalState(GetLocalState());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettings& UserSettingsManager::GetSettings(Utf8StringCR rulesetId) const
    {
    BeMutexHolder lock(GetMutex());
    auto iter = m_settings.find(rulesetId);
    if (m_settings.end() == iter)
        {
        UserSettings* settings = new UserSettings(rulesetId.c_str(), this);
        settings->SetLocalState(GetLocalState());
        iter = m_settings.Insert(rulesetId, settings).first;
        }
    return *iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::_OnRulesetDispose(RuleSetLocaterCR, PresentationRuleSetR ruleset)
    {
    BeMutexHolder lock(GetMutex());
    auto iter = m_settings.find(ruleset.GetRuleSetId());
    if (m_settings.end() != iter)
        {
        delete iter->second;
        m_settings.erase(iter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::_OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const
    {
    BeMutexHolder lock(GetMutex());
    if (nullptr != GetChangesListener())
        GetChangesListener()->_OnSettingChanged(rulesetId, settingId);
    }
