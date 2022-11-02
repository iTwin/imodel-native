/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/UserSettings.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define VARIABLES_CHUNK_SIZE 128

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesetVariableEntry
{
private:
    Utf8String m_id;
    rapidjson::MemoryPoolAllocator<> m_allocator;
    rapidjson::Document m_value;
public:
    RulesetVariableEntry(Utf8String id, RapidJsonValueCR value) : m_id(id), m_allocator(VARIABLES_CHUNK_SIZE), m_value(&m_allocator) { m_value.CopyFrom(value, m_value.GetAllocator()); }
    RulesetVariableEntry(RulesetVariableEntry const& other) : m_id(other.m_id), m_allocator(VARIABLES_CHUNK_SIZE), m_value(&m_allocator) { m_value.CopyFrom(other.m_value, m_value.GetAllocator()); }
    RulesetVariableEntry(Utf8String id, bool value) : RulesetVariableEntry(id, rapidjson::Value(value)) {}
    RulesetVariableEntry(Utf8String id, Utf8CP value) : RulesetVariableEntry(id, rapidjson::Value()) { m_value.SetString(value, m_allocator); }
    RulesetVariableEntry(Utf8String id, Utf8StringCR value) : RulesetVariableEntry(id, value.c_str()) {}
    RulesetVariableEntry(Utf8String id, int64_t value) : RulesetVariableEntry(id, rapidjson::Value(value)) {}
    RulesetVariableEntry(Utf8String id, BeInt64Id const& value) : RulesetVariableEntry(id, rapidjson::Value(value.GetValueUnchecked())) {}
    ECPRESENTATION_EXPORT RulesetVariableEntry& operator=(RulesetVariableEntry const& other);
    Utf8StringCR GetId() const { return m_id; }
    RapidJsonValueCR GetValue() const { return m_value; }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesetVariables
{
private:
    mutable std::unique_ptr<rapidjson::MemoryPoolAllocator<>> m_allocator;
    mutable std::unique_ptr<rapidjson::Document> m_variables;
    mutable Utf8String m_serializedVariables;

private:
    void InitJsonDocument() const
        {
        if (m_variables != nullptr)
            return;

        m_allocator = std::make_unique<rapidjson::MemoryPoolAllocator<>>(VARIABLES_CHUNK_SIZE);
        m_variables = std::make_unique<rapidjson::Document>(rapidjson::kObjectType, m_allocator.get());
        }
    rapidjson::Document& GetJsonDocument() {InitJsonDocument(); return *m_variables;}
    rapidjson::Document const& GetJsonDocument() const {InitJsonDocument(); return *m_variables;}
    static RulesetVariables FromInternalJsonObject(RapidJsonValueCR);

public:
    //! Constructor.
    RulesetVariables() {}
    ECPRESENTATION_EXPORT RulesetVariables(bvector<RulesetVariableEntry> const&);
    ECPRESENTATION_EXPORT RulesetVariables(std::initializer_list<RulesetVariableEntry>);
    ECPRESENTATION_EXPORT RulesetVariables(RulesetVariables const&);
    ECPRESENTATION_EXPORT RulesetVariables(RulesetVariables&&);

    ECPRESENTATION_EXPORT static RulesetVariables FromSerializedInternalJsonObjectString(Utf8CP);
    ECPRESENTATION_EXPORT Utf8StringCR GetSerializedInternalJsonObjectString() const;

    ECPRESENTATION_EXPORT RulesetVariables& operator=(RulesetVariables const& other);
    ECPRESENTATION_EXPORT RulesetVariables& operator=(RulesetVariables&& other);

    ECPRESENTATION_EXPORT bool operator==(RulesetVariables const& other) const;
    bool operator!=(RulesetVariables const& other) const {return !operator==(other);}

    //! Check if variables contains supplied variables
    ECPRESENTATION_EXPORT bool Contains(RulesetVariables const& other, bool nullEqualsUndefined = true) const;
    //! Merge variables with supplied used settings.
    ECPRESENTATION_EXPORT void Merge(IUserSettings const& userSettings);
    //! Merge variables with supplied ruleset variables.
    ECPRESENTATION_EXPORT void Merge(RulesetVariables const& otherVariables);
    //! Check if there are no variables.
    bool Empty() const {return !m_variables || 0 == m_variables->MemberCount();}

    //! Check if there is variable with supplied id.
    bool HasValue(Utf8CP id) const { return m_variables && m_variables->HasMember(id); }
    //! Get string variable value.
    ECPRESENTATION_EXPORT Utf8CP GetStringValue(Utf8CP id) const;
    //! Get int variable value.
    ECPRESENTATION_EXPORT int64_t GetIntValue(Utf8CP id) const;
    //! Get bool variable value.
    ECPRESENTATION_EXPORT bool GetBoolValue(Utf8CP id) const;
    //! Get int array variable value.
    ECPRESENTATION_EXPORT bvector<int64_t> GetIntValues(Utf8CP id) const;
    //! Get variable value as json.
    ECPRESENTATION_EXPORT RapidJsonValueCR GetJsonValue(Utf8CP id) const;
    //! Get all variables names.
    ECPRESENTATION_EXPORT bvector<Utf8String> GetVariableNames() const;
    //! Get all variables entries.
    ECPRESENTATION_EXPORT bvector<RulesetVariableEntry> GetVariableEntries() const;

    //! Set string variables value.
    ECPRESENTATION_EXPORT void SetStringValue(Utf8CP id, Utf8StringCR value);
    //! Set int variable value.
    ECPRESENTATION_EXPORT void SetIntValue(Utf8CP id, int64_t value);
    //! Set bool variable value.
    ECPRESENTATION_EXPORT void SetBoolValue(Utf8CP id, bool value);
    //! Set int array variable value.
    ECPRESENTATION_EXPORT void SetIntValues(Utf8CP id, bvector<int64_t> values);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
