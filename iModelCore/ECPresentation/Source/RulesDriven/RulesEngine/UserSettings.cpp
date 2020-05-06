/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/UserSettings.h>
#include "LocalizationHelper.h"
#include "LoggingHelper.h"
#include "RulesEngineTypes.h"

#define USER_SETTINGS_NAMESPACE "RulesEngine.UserSettings"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingValue(Utf8CP id, Utf8CP value)
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        {
        BeAssert(false);
        return;
        }

    if (!m_isInitializing)
        {
        Utf8String currentValue = GetSettingValue(id);
        if (currentValue.Equals(value))
            return;
        }

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), value);
    m_settingIds.insert(bpair<Utf8String, Utf8String>(id, "string"));

    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingIntValue(Utf8CP id, int64_t value)
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        {
        BeAssert(false);
        return;
        }
    
    if (!m_isInitializing)
        {
        int64_t currentValue = GetSettingIntValue(id);
        if (currentValue == value)
            return;
        }

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), Json::Value(value));
    m_settingIds.insert(bpair<Utf8String, Utf8String>(id, "int"));
    
    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<int64_t> VectorFromJsonArray(JsonValueCR json)
    {
    BeAssert(json.isArray());
    bvector<int64_t> vec;
    for (Json::ArrayIndex i = 0; i < json.size(); ++i)
        vec.push_back(json[i].asInt64());
    return vec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value JsonArrayFromVector(bvector<int64_t> const& vec)
    {
    Json::Value json(Json::arrayValue);
    for (int64_t v : vec)
        json.append(Json::Value(v));
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingIntValues(Utf8CP id, bvector<int64_t> const& values)
    {    
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        {
        BeAssert(false);
        return;
        }
    
    if (!m_isInitializing)
        {
        bvector<int64_t> currentValues = GetSettingIntValues(id);
        if (currentValues == values)
            return;
        }

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), JsonArrayFromVector(values));
    m_settingIds.insert(bpair<Utf8String, Utf8String>(id, "ints"));
    
    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_SetSettingBoolValue(Utf8CP id, bool value)
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        {
        BeAssert(false);
        return;
        }
    
    if (!m_isInitializing)
        {
        bool currentValue = GetSettingBoolValue(id);
        if (currentValue == value)
            return;
        }

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    m_localState->SaveJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str(), value);
    m_settingIds.insert(bpair<Utf8String, Utf8String>(id, "bool"));
    
    if (!m_isInitializing && nullptr != m_changeListener)
        m_changeListener->_OnSettingChanged(m_rulesetId.c_str(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UserSettings::_GetSettingValue(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    static Utf8String s_defaultValue = "";
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return s_defaultValue;
        }
    
    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    Json::Value value = m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    return value.isString() ? value.asCString() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t UserSettings::_GetSettingIntValue(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    static int64_t s_defaultValue = 0;
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return s_defaultValue;
        }
    
    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    Json::Value value = m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    return (value.isConvertibleTo(Json::intValue) || value.isConvertibleTo(Json::uintValue)) ? value.asInt64() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<int64_t> UserSettings::_GetSettingIntValues(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    bvector<int64_t> values;
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return values;
        }
    
    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    Json::Value jsonValues = m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    if (!jsonValues.isArray())
        return values;

    return VectorFromJsonArray(jsonValues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettings::_GetSettingBoolValue(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    static bool s_defaultValue = false;
    if (nullptr == m_localState)
        {
        BeAssert(false);
        return s_defaultValue;
        }
    
    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    Json::Value value = m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    return value.isConvertibleTo(Json::booleanValue) ? value.asBool() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value UserSettings::_GetSettingValueAsJson(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        {
        BeAssert(false);
        return Json::Value(Json::nullValue);
        }

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    return m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<bpair<Utf8String, Utf8String>> UserSettings::_GetSettings() const
    {
    return ContainerHelpers::TransformContainer<bvector<bpair<Utf8String, Utf8String>>>(m_settingIds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool UserSettings::_HasSetting(Utf8CP id) const
    {
    BeMutexHolder lock(m_mutex);

    Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), id);
    return !m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str()).isNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::AddValues(JsonValueR groupListJson) const
    {
    for (Json::ArrayIndex i = 0; i < groupListJson.size(); i++)
        {
        JsonValueR groupJson = groupListJson[i];
        if (groupJson.isMember("Items"))
            {
            for (Json::ArrayIndex j = 0; j < groupJson["Items"].size(); j++)
                {
                JsonValueR itemJson = groupJson["Items"][j];
                Utf8CP options = itemJson["Options"].asCString();
                if (0 == strcmp("StringValue", options))
                    itemJson["Value"] = GetSettingValue(itemJson["Id"].asCString()).c_str();
                else if (0 == strcmp("IntValue", options))
                    itemJson["Value"] = Json::Value(GetSettingIntValue(itemJson["Id"].asCString()));
                else
                    itemJson["Value"] = GetSettingBoolValue(itemJson["Id"].asCString());
                }
            }
        if (groupJson.isMember("NestedGroups"))
            AddValues(groupJson["NestedGroups"]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::LocalizeLabels(JsonValueR groupListJson, Utf8StringCR locale) const
    {
    for (Json::ArrayIndex i = 0; i < groupListJson.size(); i++)
        {
        JsonValueR groupJson = groupListJson[i];
        if (groupJson.isMember("Label"))
            groupJson["Label"] = GetLocalizedLabel(groupJson["Label"].asCString(), locale);
        if (groupJson.isMember("Items"))
            {
            for (Json::ArrayIndex j = 0; j < groupJson["Items"].size(); j++)
                {
                JsonValueR itemJson = groupJson["Items"][j];
                if (itemJson.isMember("Label"))
                    itemJson["Label"] = GetLocalizedLabel(itemJson["Label"].asCString(), locale);
                }
            }
        if (groupJson.isMember("NestedGroups"))
            LocalizeLabels(groupJson["NestedGroups"], locale);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value UserSettings::_GetPresentationInfo(Utf8StringCR locale) const
    {
    BeMutexHolder lock(m_mutex);

    if (nullptr == m_localState)
        {
        BeAssert(false);
        return Json::Value::GetNull();
        }

    Json::Value json = m_presentationInfo;
    LocalizeLabels(json, locale);
    AddValues(json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String UserSettings::GetLocalizedLabel(Utf8StringCR nonLocalizedLabel, Utf8StringCR locale) const
    {
    if (nullptr == m_localizationProvider)
        {
        LoggingHelper::LogMessage(Log::Localization, "Localization is not available as the localization provider is not set", NativeLogging::LOG_WARNING, true);
        return nonLocalizedLabel;
        }

    Utf8String localizedLabel = nonLocalizedLabel;
    LocalizationHelper helper(*m_localizationProvider, locale);
    return helper.LocalizeString(localizedLabel) ? localizedLabel : nonLocalizedLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::_InitFrom(UserSettingsGroupList const& rules)
    {
    BeMutexHolder lock(m_mutex);

    if (!rules.empty() && nullptr == m_localState)
        {
        BeAssert(false);
        return;
        }

    if (m_initializedFrom == &rules)
        return;

    m_isInitializing = true;
    m_presentationInfo.clear();
    InitFromJson(rules, m_presentationInfo);
    m_initializedFrom = &rules;
    m_isInitializing = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettings::InitFromJson(UserSettingsGroupList const& rules, JsonValueR presentationInfo)
    {
    presentationInfo = Json::Value(Json::arrayValue);
    for (UserSettingsGroupCP group : rules)
        {
        Json::Value groupJson(Json::objectValue);
        groupJson["Label"] = group->GetCategoryLabel();
        if (!group->GetSettingsItems().empty())
            groupJson["Items"] = Json::Value(Json::arrayValue);

        for (UserSettingsItemCP item : group->GetSettingsItems())
            {
            Utf8PrintfString stringId("%s:%s", m_rulesetId.c_str(), item->GetId().c_str());
            if (m_localState->GetJsonValue(USER_SETTINGS_NAMESPACE, stringId.c_str()).isNull())
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

            // create the presentation info
            Json::Value itemPresentationInfo(Json::objectValue);
            itemPresentationInfo["Id"] = item->GetId();
            itemPresentationInfo["Label"] = item->GetLabel();
            itemPresentationInfo["Options"] = item->GetOptions().empty() ? "TrueFalse" : item->GetOptions();
            groupJson["Items"].append(itemPresentationInfo);
            }

        if (!group->GetNestedSettings().empty())
            InitFromJson(group->GetNestedSettings(), groupJson["NestedGroups"]);

        presentationInfo.append(groupJson);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsManager::UserSettingsManager(BeFileNameCR) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettingsManager::~UserSettingsManager()
    {
    BeMutexHolder lock(GetMutex());
    for (auto iter : m_settings)
        delete iter.second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::_OnLocalStateChanged()
    {
    BeMutexHolder lock(GetMutex());
    for (auto iter : m_settings)
        iter.second->SetLocalState(GetLocalState());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::_OnLocalizationProviderChanged()
    {
    BeMutexHolder lock(GetMutex());
    for (auto iter : m_settings)
        iter.second->SetLocalizationProvider(GetLocalizationProvider());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UserSettings& UserSettingsManager::GetSettings(Utf8StringCR rulesetId) const
    {
    BeMutexHolder lock(GetMutex());
    auto iter = m_settings.find(rulesetId);
    if (m_settings.end() == iter)
        {
        UserSettings* settings = new UserSettings(rulesetId.c_str(), this);
        settings->SetLocalState(GetLocalState());
        settings->SetLocalizationProvider(GetLocalizationProvider());
        iter = m_settings.Insert(rulesetId, settings).first;
        }
    return *iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
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
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UserSettingsManager::_OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const
    {
    BeMutexHolder lock(GetMutex());
    if (nullptr != GetChangesListener())
        GetChangesListener()->_OnSettingChanged(rulesetId, settingId);
    }
