/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesetVariables.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariableEntry& RulesetVariableEntry::operator=(RulesetVariableEntry const& other)
    {
    m_id = other.m_id;
    m_value.CopyFrom(other.m_value, m_value.GetAllocator());
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TSourceContainer> static void PushVariables(rapidjson::Document& target, TSourceContainer const& container)
    {
    for (RulesetVariableEntry const& entry : container)
        target.AddMember(rapidjson::Value(entry.GetId().c_str(), target.GetAllocator()).Move(), rapidjson::Value(entry.GetValue(), target.GetAllocator()).Move(), target.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables::RulesetVariables(bvector<RulesetVariableEntry> const& entries)
    : RulesetVariables()
    {
    PushVariables(GetJsonDocument(), entries);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables::RulesetVariables(std::initializer_list<RulesetVariableEntry> entries)
    : RulesetVariables()
    {
    PushVariables(GetJsonDocument(), entries);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables RulesetVariables::FromInternalJsonObject(RapidJsonValueCR json)
    {
    RulesetVariables variables;
    if (json.IsObject())
        {
        auto& doc = variables.GetJsonDocument();
        doc.CopyFrom(json, doc.GetAllocator());
        }
    return variables;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables RulesetVariables::FromSerializedInternalJsonObjectString(Utf8CP str)
    {
    if (!str || !*str || 0 == strcmp("{}", str) || 0 == strcmp("[]", str))
        return RulesetVariables();

    rapidjson::Document json;
    json.Parse(str);
    return FromInternalJsonObject(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR RulesetVariables::GetSerializedInternalJsonObjectString() const
    {
    if (m_serializedVariables.empty())
        {
        if (m_variables)
            m_serializedVariables = BeRapidJsonUtilities::ToString(*m_variables);
        else
            m_serializedVariables = "{}";
        }
    return m_serializedVariables;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables::RulesetVariables(RulesetVariables const& other)
    : RulesetVariables()
    {
    auto& variables = GetJsonDocument();
    variables.CopyFrom(other.GetJsonDocument(), variables.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables::RulesetVariables(RulesetVariables&& other)
    : m_allocator(std::move(other.m_allocator)), m_variables(std::move(other.m_variables))
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables& RulesetVariables::operator=(RulesetVariables const& other)
    {
    auto& variables = GetJsonDocument();
    variables.CopyFrom(other.GetJsonDocument(), variables.GetAllocator());
    m_serializedVariables.clear();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RulesetVariables& RulesetVariables::operator=(RulesetVariables&& other)
    {
    m_allocator = std::move(other.m_allocator);
    m_variables = std::move(other.m_variables);
    m_serializedVariables.clear();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RulesetVariables::operator==(RulesetVariables const& other) const
    {
    return GetJsonDocument() == other.GetJsonDocument();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RulesetVariables::Contains(RulesetVariables const& other, bool nullEqualsUndefined) const
    {
    if (!other.m_variables)
        return true;

    for (auto otherVariablesIter = other.GetJsonDocument().MemberBegin(); other.GetJsonDocument().MemberEnd() != otherVariablesIter; ++otherVariablesIter)
        {
        auto variablesIter = GetJsonDocument().FindMember(otherVariablesIter->name);
        if (GetJsonDocument().MemberEnd() != variablesIter && otherVariablesIter->value != variablesIter->value)
            return false;
        if (GetJsonDocument().MemberEnd() == variablesIter && (!otherVariablesIter->value.IsNull() || !nullEqualsUndefined))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Value GetArrayJson(bvector<int64_t> const& values, rapidjson::MemoryPoolAllocator<>& allocator)
    {
    rapidjson::Value json(rapidjson::kArrayType);
    for (int64_t value : values)
        json.PushBack(value, allocator);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetVariables::Merge(IUserSettings const& userSettings)
    {
    auto& variables = GetJsonDocument();
    m_serializedVariables.clear();
    bvector<bpair<Utf8String, Utf8String>> settings = userSettings.GetSettings();
    for (auto const& setting : settings)
        {
        Utf8CP id = setting.first.c_str();
        if (variables.HasMember(id))
            continue;

        rapidjson::Value name(id, variables.GetAllocator());
        if (setting.second.Equals("string"))
            variables.AddMember(name.Move(), rapidjson::Value(userSettings.GetSettingValue(id).c_str(), variables.GetAllocator()), variables.GetAllocator());
        else if (setting.second.Equals("int"))
            variables.AddMember(name.Move(), userSettings.GetSettingIntValue(id), variables.GetAllocator());
        else if (setting.second.Equals("ints"))
            variables.AddMember(name.Move(), GetArrayJson(userSettings.GetSettingIntValues(id), variables.GetAllocator()), variables.GetAllocator());
        else if (setting.second.Equals("bool"))
            variables.AddMember(name.Move(), userSettings.GetSettingBoolValue(id), variables.GetAllocator());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetVariables::Merge(RulesetVariables const& otherVariables)
    {
    auto& variables = GetJsonDocument();
    m_serializedVariables.clear();
    for (auto otherVariablesIter = otherVariables.GetJsonDocument().MemberBegin(); otherVariables.GetJsonDocument().MemberEnd() != otherVariablesIter; ++otherVariablesIter)
        {
        rapidjson::Value value(otherVariablesIter->value, variables.GetAllocator());
        if (variables.HasMember(otherVariablesIter->name))
            variables[otherVariablesIter->name] = value;
        else
            variables.AddMember(rapidjson::Value(otherVariablesIter->name, variables.GetAllocator()), value, variables.GetAllocator());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RulesetVariables::GetStringValue(Utf8CP id) const
    {
    static Utf8CP default_value = "";
    if (!HasValue(id))
        return default_value;

    auto const& variables = GetJsonDocument();
    RapidJsonValueCR value = variables[id];
    return value.IsString() ? variables[id].GetString() : default_value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t RulesetVariables::GetIntValue(Utf8CP id) const
    {
    static int64_t s_defaultValue = 0;
    if (!HasValue(id))
        return s_defaultValue;

    auto const& variables = GetJsonDocument();
    RapidJsonValueCR value = variables[id];
    return value.IsInt64() ? variables[id].GetInt64() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RulesetVariables::GetBoolValue(Utf8CP id) const
    {
    static bool s_defaultValue = false;
    if (!HasValue(id))
        return s_defaultValue;

    auto const& variables = GetJsonDocument();
    RapidJsonValueCR value = variables[id];
    return value.IsBool() ? variables[id].GetBool() : s_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<int64_t> RulesetVariables::GetIntValues(Utf8CP id) const
    {
    bvector<int64_t> values;
    if (!HasValue(id))
        return values;

    auto const& variables = GetJsonDocument();
    if (!variables[id].IsArray())
        return values;

    RapidJsonValueCR jsonArray = variables[id];
    for (RapidJsonValueCR value : jsonArray.GetArray())
        values.push_back(value.GetInt64());
    return values;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RapidJsonValueCR RulesetVariables::GetJsonValue(Utf8CP id) const
    {
    static rapidjson::Value s_defaultValue;
    if (!HasValue(id))
        return s_defaultValue;

    auto const& variables = GetJsonDocument();
    return variables[id];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> RulesetVariables::GetVariableNames() const
    {
    bvector<Utf8String> names;
    if (!m_variables)
        return names;

    auto const& variables = GetJsonDocument();
    for (auto iter = variables.MemberBegin(); variables.MemberEnd() != iter; ++iter)
        names.push_back(iter->name.GetString());
    return names;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RulesetVariableEntry> RulesetVariables::GetVariableEntries() const
    {
    bvector<RulesetVariableEntry> entries;
    if (!m_variables)
        return entries;

    auto const& variables = GetJsonDocument();
    for (auto iter = variables.MemberBegin(); variables.MemberEnd() != iter; ++iter)
        entries.push_back(RulesetVariableEntry(iter->name.GetString(), iter->value));
    return entries;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetVariables::SetStringValue(Utf8CP id, Utf8StringCR value)
    {
    auto& variables = GetJsonDocument();
    m_serializedVariables.clear();
    variables.HasMember(id)
        ? variables[id].SetString(value.c_str(), variables.GetAllocator())
        : variables.AddMember(rapidjson::Value(id, variables.GetAllocator()).Move(), rapidjson::Value(value.c_str(), variables.GetAllocator()), variables.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetVariables::SetIntValue(Utf8CP id, int64_t value)
    {
    auto& variables = GetJsonDocument();
    m_serializedVariables.clear();
    variables.HasMember(id) ? variables[id].SetInt64(value) : variables.AddMember<int64_t>(rapidjson::Value(id, variables.GetAllocator()).Move(), value, variables.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetVariables::SetBoolValue(Utf8CP id, bool value)
    {
    auto& variables = GetJsonDocument();
    m_serializedVariables.clear();
    variables.HasMember(id) ? variables[id].SetBool(value) : variables.AddMember(rapidjson::Value(id, variables.GetAllocator()).Move(), value, variables.GetAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesetVariables::SetIntValues(Utf8CP id, bvector<int64_t> values)
    {
    m_serializedVariables.clear();

    auto& variables = GetJsonDocument();
    auto valuesJson = GetArrayJson(values, variables.GetAllocator());
    if (variables.HasMember(id))
        variables[id] = valuesJson;
    else
        variables.AddMember(rapidjson::Value(id, variables.GetAllocator()).Move(), valuesJson, variables.GetAllocator());
    }
