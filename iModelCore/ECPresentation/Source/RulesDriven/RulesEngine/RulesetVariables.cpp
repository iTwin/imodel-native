/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "RulesetVariables.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariableEntry& RulesetVariableEntry::operator=(RulesetVariableEntry const& other)
    {
    m_id = other.m_id;
    m_value.CopyFrom(other.m_value, m_value.GetAllocator());
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Value GetArrayJson(bvector<int64_t> values, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    rapidjson::Value json(rapidjson::kArrayType);
    for (int64_t value : values)
        json.PushBack(value, allocator);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables::RulesetVariables(JsonValueCR variables): RulesetVariables()
    {
    if (!variables.isArray() || 0 == variables.size())
        return;

    for (Json::ArrayIndex i = 0; i < variables.size(); ++i)
        {
        if (!variables[i].isMember("id") || !variables[i].isMember("type") || !variables[i].isMember("value"))
            continue;

        Utf8String variableId = variables[i]["id"].asString();
        Utf8String variableType = variables[i]["type"].asString();
        JsonValueCR variableValue = variables[i]["value"];
        if (variableType.Equals("bool"))
            SetBoolValue(variableId.c_str(), variableValue.asBool());
        else if (variableType.Equals("string"))
            SetStringValue(variableId.c_str(), variableValue.asCString());
        else if (variableType.Equals("id64"))
            SetIntValue(variableId.c_str(), BeInt64Id::FromString(variableValue.asCString()).GetValue());
        else if (variableType.Equals("id64[]"))
            {
            bvector<int64_t> values;
            for (Json::ArrayIndex i = 0; i < variableValue.size(); i++)
                values.push_back(BeInt64Id::FromString(variableValue[i].asCString()).GetValue());
            SetIntValues(variableId.c_str(), values);
            }
        else if (variableType.Equals("int"))
            SetIntValue(variableId.c_str(), variableValue.asInt64());
        else if (variableType.Equals("int[]"))
            {
            bvector<int64_t> values;
            for (Json::ArrayIndex i = 0; i < variableValue.size(); i++)
                values.push_back(variableValue[i].asInt64());
            SetIntValues(variableId.c_str(), values);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables::RulesetVariables(bvector<RulesetVariableEntry> const& variables)
    : RulesetVariables()
    {
    for (RulesetVariableEntry const& entry : variables)
        m_variables.AddMember(rapidjson::Value(entry.GetId().c_str(), m_variables.GetAllocator()).Move(), rapidjson::Value(entry.GetValue(), m_variables.GetAllocator()).Move(), m_variables.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables::RulesetVariables(Utf8CP variables)
    : RulesetVariables()
    {
    m_variables.Parse(variables);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables::RulesetVariables(RulesetVariables const& other)
    : RulesetVariables()
    {
    m_variables.CopyFrom(other.m_variables, m_variables.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables::RulesetVariables(RulesetVariables&& other)
    : m_allocator(std::move(other.m_allocator)), m_variables(std::move(other.m_variables))
    { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables& RulesetVariables::operator=(RulesetVariables const& other)
    {
    m_variables.CopyFrom(other.m_variables, m_variables.GetAllocator());
    m_serializedVariables.clear();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables& RulesetVariables::operator=(RulesetVariables&& other)
    {
    m_allocator = std::move(other.m_allocator);
    m_variables = std::move(other.m_variables);
    m_serializedVariables.clear();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool RulesetVariables::operator==(RulesetVariables const& other) const
    {
    return m_variables == other.m_variables;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool RulesetVariables::Contains(RulesetVariables const& other) const
    {
    if (other.m_variables.MemberCount() > m_variables.MemberCount())
        return false;

    for (auto otherVariablesIter = other.m_variables.MemberBegin(); other.m_variables.MemberEnd() != otherVariablesIter; ++otherVariablesIter)
        {
        auto variablesIter = m_variables.FindMember(otherVariablesIter->name);
        if (m_variables.MemberEnd() == variablesIter || otherVariablesIter->value != variablesIter->value)
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetVariables::Merge(IUserSettings const& userSettings)
    {
    m_serializedVariables.clear();
    bvector<bpair<Utf8String, Utf8String>> settings = userSettings.GetSettings();
    for (auto setting : settings)
        {
        Utf8CP id = setting.first.c_str();
        if (m_variables.HasMember(id))
            continue;

        rapidjson::Value name(id, m_variables.GetAllocator());
        if (setting.second.Equals("string"))
            m_variables.AddMember(name.Move(), rapidjson::Value(userSettings.GetSettingValue(id).c_str(), m_variables.GetAllocator()), m_variables.GetAllocator());
        else if (setting.second.Equals("int"))
            m_variables.AddMember(name.Move(), userSettings.GetSettingIntValue(id), m_variables.GetAllocator());
        else if (setting.second.Equals("ints"))
            m_variables.AddMember(name.Move(), GetArrayJson(userSettings.GetSettingIntValues(id), m_variables.GetAllocator()), m_variables.GetAllocator());
        else if (setting.second.Equals("bool"))
            m_variables.AddMember(name.Move(), userSettings.GetSettingBoolValue(id), m_variables.GetAllocator());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RulesetVariables::GetStringValue(Utf8CP id) const
    {
    static Utf8CP default_value = "";
    if (!m_variables.HasMember(id))
        return default_value;

    RapidJsonValueCR value = m_variables[id];
    return value.IsString() ? m_variables[id].GetString() : default_value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t RulesetVariables::GetIntValue(Utf8CP id) const
    {
    static int64_t s_defaultValue = 0;
    if (!m_variables.HasMember(id))
        return s_defaultValue;

    RapidJsonValueCR value = m_variables[id];
    return value.IsInt64() ? m_variables[id].GetInt64() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool RulesetVariables::GetBoolValue(Utf8CP id) const
    {
    static bool s_defaultValue = false;
    if (!m_variables.HasMember(id))
        return s_defaultValue;

    RapidJsonValueCR value = m_variables[id];
    return value.IsBool() ? m_variables[id].GetBool() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<int64_t> RulesetVariables::GetIntValues(Utf8CP id) const
    {
    bvector<int64_t> values;
    if (!m_variables.HasMember(id) || !m_variables[id].IsArray())
        return values;

    RapidJsonValueCR jsonArray = m_variables[id];
    for (RapidJsonValueCR value : jsonArray.GetArray())
        values.push_back(value.GetInt64());
    return values;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
RapidJsonValueCR RulesetVariables::GetJsonValue(Utf8CP id) const
    {
    static rapidjson::Value s_defaultValue;
    if (!m_variables.HasMember(id))
        return s_defaultValue;

    return m_variables[id];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> RulesetVariables::GetVariableNames() const
    {
    bvector<Utf8String> names;
    for (auto iter = m_variables.MemberBegin(); m_variables.MemberEnd() != iter; ++iter)
        names.push_back(iter->name.GetString());
    return names;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetVariables::SetStringValue(Utf8CP id, Utf8StringCR value)
    {
    m_serializedVariables.clear();
    m_variables.HasMember(id)
        ? m_variables[id].SetString(value.c_str(), m_variables.GetAllocator())
        : m_variables.AddMember(rapidjson::Value(id, m_variables.GetAllocator()).Move(), rapidjson::Value(value.c_str(), m_variables.GetAllocator()), m_variables.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetVariables::SetIntValue(Utf8CP id, int64_t value)
    {
    m_serializedVariables.clear();
    m_variables.HasMember(id) ? m_variables[id].SetInt64(value) : m_variables.AddMember<int64_t>(rapidjson::Value(id, m_variables.GetAllocator()).Move(), value, m_variables.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetVariables::SetBoolValue(Utf8CP id, bool value)
    {
    m_serializedVariables.clear();
    m_variables.HasMember(id) ? m_variables.SetBool(value) : m_variables.AddMember(rapidjson::Value(id, m_variables.GetAllocator()).Move(), value, m_variables.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetVariables::SetIntValues(Utf8CP id, bvector<int64_t> values)
    {
    m_serializedVariables.clear();
    m_variables.HasMember(id) ? m_variables[id] = GetArrayJson(values, m_variables.GetAllocator()) : m_variables.AddMember(rapidjson::Value(id, m_variables.GetAllocator()).Move(), GetArrayJson(values, m_variables.GetAllocator()), m_variables.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR RulesetVariables::GetSerialized() const
    {
    if (m_serializedVariables.empty())
        m_serializedVariables = BeRapidJsonUtilities::ToString(m_variables);
    return m_serializedVariables;
    }
