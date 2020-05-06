/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/UserSettings.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define VARIABLES_CHUNK_SIZE 256

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                04/2020
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
    ECPRESENTATION_EXPORT RulesetVariableEntry& operator=(RulesetVariableEntry const& other);
    Utf8StringCR GetId() const { return m_id; }
    RapidJsonValueCR GetValue() const { return m_value; }
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                04/2020
+===============+===============+===============+===============+===============+======*/
struct RulesetVariables
{
private:
    std::unique_ptr<rapidjson::MemoryPoolAllocator<>> m_allocator;
    rapidjson::Document m_variables;
    mutable Utf8String m_serializedVariables;

public:
    //! Constructor.
    RulesetVariables() : m_allocator(std::make_unique<rapidjson::MemoryPoolAllocator<>>(VARIABLES_CHUNK_SIZE)), m_variables(rapidjson::kObjectType, m_allocator.get()) {}
    ECPRESENTATION_EXPORT RulesetVariables(JsonValueCR variables);
    //! Constructor.
    //! @param[in] variables List of variables entries.
    ECPRESENTATION_EXPORT RulesetVariables(bvector<RulesetVariableEntry> const& variables);
    //! Constructor.
    //! @param[in] variables Variables json string.
    ECPRESENTATION_EXPORT RulesetVariables(Utf8CP variables);
    //! Copy constructor.
    ECPRESENTATION_EXPORT RulesetVariables(RulesetVariables const& other);
    ECPRESENTATION_EXPORT RulesetVariables(RulesetVariables&& other);

    ECPRESENTATION_EXPORT RulesetVariables& operator=(RulesetVariables const& other);
    ECPRESENTATION_EXPORT RulesetVariables& operator=(RulesetVariables&& other);

    ECPRESENTATION_EXPORT bool operator==(RulesetVariables const& other) const;

    //! Check if variables contains supplied variables
    ECPRESENTATION_EXPORT bool Contains(RulesetVariables const& other) const;
    //! Merge variables with supplied used settings.
    ECPRESENTATION_EXPORT void Merge(IUserSettings const& userSettings);

    //! Get variables serialized as json string.
    ECPRESENTATION_EXPORT Utf8StringCR GetSerialized() const;

    //! Check if there is variable with supplied id.
    bool HasValue(Utf8CP id) const { return m_variables.HasMember(id); }
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
