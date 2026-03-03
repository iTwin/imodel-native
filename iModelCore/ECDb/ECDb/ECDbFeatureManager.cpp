/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

// ---------------------------------------------------------------------------
// ECDbFeatureDescriptor
// ---------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
//static
Utf8CP ECDbFeatureDescriptor::StatusToString(ECDbFeatureStatus status)
    {
    switch (status)
        {
        case ECDbFeatureStatus::Experimental: return "Experimental";
        case ECDbFeatureStatus::Stable:       return "Stable";
        case ECDbFeatureStatus::Deprecated:   return "Deprecated";
        default:
            BeAssert(false);
            return "Unknown";
        }
    }

// ---------------------------------------------------------------------------
// ECDbFeatureRegistry
// ---------------------------------------------------------------------------

//! Well-known feature name constants.
/*static*/ Utf8CP ECDbFeatureRegistry::STRICT_SCHEMA_LOADING = "strict-schema-loading";

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
void ECDbFeatureRegistry::RegisterBuiltIns()
    {
    //                        name,                     label,                  description,
    //                        status,                   enabledByDefault, toggleable, ephemeral
    m_features.emplace_back(
        STRICT_SCHEMA_LOADING,
        "Strict Schema Loading",
        "When enabled, the ECSchema XML parser runs in strict mode. Unknown XML attributes "
        "on known elements, unknown XML elements in known positions, and unrecognised enum "
        "values in typed attributes are treated as hard errors rather than being silently "
        "ignored. Catches schema authoring mistakes that currently vanish silently.",
        ECDbFeatureStatus::Experimental,
        /*enabledByDefault*/ false,
        /*toggleable*/       false,
        /*ephemeral*/        false
        );

    // Future features are registered here, e.g.:
    //   m_features.emplace_back("dynamic-types", "Dynamic Types", "...",
    //       ECDbFeatureStatus::Experimental, false, false, false);
    }

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
//static
ECDbFeatureRegistry const& ECDbFeatureRegistry::GetInstance()
    {
    // Non-POD static – intentionally never destroyed (follows Bentley guidelines).
    static ECDbFeatureRegistry* s_instance = new ECDbFeatureRegistry();
    return *s_instance;
    }

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
ECDbFeatureDescriptor const* ECDbFeatureRegistry::Find(Utf8CP name) const
    {
    if (nullptr == name)
        return nullptr;
    for (ECDbFeatureDescriptor const& desc : m_features)
        {
        if (desc.GetName().EqualsIAscii(name))
            return &desc;
        }
    return nullptr;
    }

// ---------------------------------------------------------------------------
// ECDbFeatureManager
// ---------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
//static
PropertySpec ECDbFeatureManager::GetPropertySpec()
    {
    return PropertySpec("Features", ECDB_PROPSPEC_NAMESPACE);
    }

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
BentleyStatus ECDbFeatureManager::LoadFromDb() const
    {
    m_enabledFeatures.Clear();

    Utf8String json;
    DbResult stat = m_ecdb.QueryProperty(json, GetPropertySpec());
    if (stat == BE_SQLITE_ERROR)
        {
        // be_prop table does not exist yet – treat as empty set (pre-feature file).
        return SUCCESS;
        }
    if (stat != BE_SQLITE_ROW || json.empty())
        return SUCCESS;  // Property exists but is empty or not yet written.

    // Parse JSON array: ["feature-name-1", "feature-name-2", ...]
    Json::Value arr;
    Json::Reader reader;
    if (!reader.parse(json.c_str(), json.c_str() + json.size(), arr, false))
        {
        m_ecdb.GetImpl().Issues().Report(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0742,
            "ECDb features property contains invalid JSON. The feature set could not be loaded.");
        return ERROR;
        }

    if (!arr.isArray())
        {
        m_ecdb.GetImpl().Issues().Report(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0742,
            "ECDb features property must be a JSON array. The feature set could not be loaded.");
        return ERROR;
        }

    for (Json::ArrayIndex i = 0; i < arr.size(); ++i)
        {
        if (arr[i].isString())
            m_enabledFeatures.Add(arr[i].asString());
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
BentleyStatus ECDbFeatureManager::SaveToDb() const
    {
    // Build JSON array ["name1", "name2", ...]
    Json::Value arr(Json::arrayValue);
    for (Utf8StringCR name : m_enabledFeatures.GetAll())
        arr.append(name.c_str());

    Utf8String json = Json::FastWriter().write(arr);

    DbResult stat = m_ecdb.SavePropertyString(GetPropertySpec(), json);
    if (stat != BE_SQLITE_OK)
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0743,
            "Failed to persist ECDb features to be_prop: %s",
            m_ecdb.GetLastError().c_str());
        return ERROR;
        }
    return SUCCESS;
    }

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
BentleyStatus ECDbFeatureManager::OnProfileCreated() const
    {
    // On new file creation: enable all Stable features that are opt-in by default.
    ECDbFeatureRegistry const& registry = ECDbFeatureRegistry::GetInstance();
    for (ECDbFeatureDescriptor const& desc : registry.GetAll())
        {
        if (desc.GetStatus() == ECDbFeatureStatus::Stable && desc.IsEnabledByDefault())
            m_enabledFeatures.Add(desc.GetName());
        }

    // Only write to be_prop if there's actually something to persist.
    // Avoid making the connection dirty on new empty databases (no default features).
    if (m_enabledFeatures.GetAll().empty())
        return SUCCESS;

    return SaveToDb();
    }

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
BentleyStatus ECDbFeatureManager::OnDbOpened() const
    {
    return LoadFromDb();
    }

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
ECDbFeatureSet const& ECDbFeatureManager::GetEnabledFeatures() const
    {
    LoadFromDb();
    return m_enabledFeatures;
    }

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
BentleyStatus ECDbFeatureManager::EnableFeature(Utf8CP name) const
    {
    if (nullptr == name || *name == '\0')
        {
        m_ecdb.GetImpl().Issues().Report(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0744,
            "ECDb::EnableFeature called with a null or empty feature name.");
        return ERROR;
        }

    ECDbFeatureRegistry const& registry = ECDbFeatureRegistry::GetInstance();
    ECDbFeatureDescriptor const* descriptor = registry.Find(name);
    if (nullptr == descriptor)
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0745,
            "Cannot enable unknown feature '%s'. Only features registered in the "
            "built-in registry are supported. Check ECDbFeatureRegistry::GetInstance() "
            "for available features.",
            name);
        return ERROR;
        }

    if (!m_ecdb.IsDbOpen())
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0746,
            "Cannot enable feature '%s': the ECDb file is not open.",
            name);
        return ERROR;
        }

    if (m_ecdb.IsReadonly())
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0747,
            "Cannot enable feature '%s': the ECDb file is open read-only.",
            name);
        return ERROR;
        }

    if (!m_ecdb.GetDefaultTransaction()->IsActive())
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0748,
            "Cannot enable feature '%s': no active transaction. Begin a write transaction first.",
            name);
        return ERROR;
        }

    // Idempotent – if already enabled, nothing to do.
    if (GetEnabledFeatures().IsEnabled(name))
        return SUCCESS;

    m_enabledFeatures.Add(Utf8String(name));
    if (SUCCESS != SaveToDb())
        {
        m_enabledFeatures.Remove(Utf8String(name));
        return ERROR;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------
BentleyStatus ECDbFeatureManager::DisableFeature(Utf8CP name) const
    {
    if (nullptr == name || *name == '\0')
        {
        m_ecdb.GetImpl().Issues().Report(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0744,
            "ECDb::DisableFeature called with a null or empty feature name.");
        return ERROR;
        }

    ECDbFeatureRegistry const& registry = ECDbFeatureRegistry::GetInstance();
    ECDbFeatureDescriptor const* descriptor = registry.Find(name);
    if (nullptr == descriptor)
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0745,
            "Cannot disable unknown feature '%s'.",
            name);
        return ERROR;
        }

    if (!descriptor->IsToggleable())
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0749,
            "Cannot disable feature '%s': this feature is not toggleable. "
            "Disabling it could risk data integrity.",
            name);
        return ERROR;
        }

    if (!m_ecdb.IsDbOpen())
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0746,
            "Cannot disable feature '%s': the ECDb file is not open.",
            name);
        return ERROR;
        }

    if (m_ecdb.IsReadonly())
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0747,
            "Cannot disable feature '%s': the ECDb file is open read-only.",
            name);
        return ERROR;
        }

    if (!m_ecdb.GetDefaultTransaction()->IsActive())
        {
        m_ecdb.GetImpl().Issues().ReportV(
            IssueSeverity::Error,
            IssueCategory::BusinessProperties,
            IssueType::ECDbIssue,
            ECDbIssueId::ECDb_0748,
            "Cannot disable feature '%s': no active transaction.",
            name);
        return ERROR;
        }

    // Idempotent – if not enabled, nothing to do.
    if (!GetEnabledFeatures().IsEnabled(name))
        return SUCCESS;

    m_enabledFeatures.Remove(Utf8String(name));
    if (SUCCESS != SaveToDb())
        {
        m_enabledFeatures.Add(Utf8String(name));
        return ERROR;
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
