/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
// static
bool FeatureManager::SchemaRequiresProfileUpgrade(ECDbCR ecdb, ECN::ECSchemaCR ecSchema)
    {
    // NOTE this method must be updated if new EC versions are added that require a new ECDb profile version
    return ecSchema.OriginalECXmlVersionAtLeast(ECN::ECVersion::V3_2) ? !IsEC32Available(ecdb) : false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool FeatureManager::IsAvailable(ECDbCR ecdb, std::vector<Feature> const& features)
    {

    ProfileVersion const& actualVersion = ecdb.GetECDbProfileVersion();
    for (Feature feature : features)
        {
        if (!IsAvailable(actualVersion, feature))
            return false;
        }

    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
bool FeatureManager::IsAvailable(ProfileVersion const& actualVersion, Feature feature)
    {
    auto it = s_featureMinimumVersions->find(feature);
    if (it == s_featureMinimumVersions->end())
        return false;

    return actualVersion >= it->second;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
std::map<Feature, ProfileVersion> const* FeatureManager::s_featureMinimumVersions = new std::map<Feature, ProfileVersion>(FEATURE_MINIMUMVERSIONS);

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
std::map<Utf8String, FeatureInfo> const* FeatureManager::s_knownFeatures = new std::map<Utf8String, FeatureInfo>(KNOWN_FEATURES);

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
FeatureInfo const* FeatureManager::FindKnownFeature(Utf8StringCR featureName)
    {
    if (auto it = s_knownFeatures->find(featureName); it != s_knownFeatures->end())
        return &it->second;
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool FeatureManager::IsFeatureKnown(Utf8StringCR featureName)
    {
    return s_knownFeatures->find(featureName) != s_knownFeatures->end();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
std::vector<FeatureInfo const*> FeatureManager::GetAllKnownFeatures()
    {
    std::vector<FeatureInfo const*> features;
    for (auto const& [featureName, featureInfo] : *s_knownFeatures)
        features.push_back(&featureInfo);
    return features;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
Utf8CP FeatureManager::FeatureCompatToString(Compat compat)
    {
    switch (compat)
        {
        case Compat::Warn:     return "Warn";
        case Compat::ReadOnly: return "ReadOnly";
        case Compat::NoSchemaImport: return "NoSchemaImport";
        case Compat::Refuse:   return "Refuse";
        default:
            BeAssert(false && "Unhandled Compat value");
            return "Warn";
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus FeatureManager::InsertFeature(ECDbCR ecdb, Utf8StringCR featureName)
    {
    const FeatureInfo* info = FindKnownFeature(featureName);
    if (info == nullptr)
        {
        ecdb.GetImpl().Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, ECDbIssueId::ECDb_0745,
            "Feature '%s' is not a known feature and cannot be inserted into the database.", featureName.c_str());
        return BentleyStatus::ERROR;
        }

    // Insert the row into the ECDb file
    Statement stmt;
    if (stmt.Prepare(ecdb, "INSERT OR IGNORE INTO " TABLE_Feature " (Name, Description, Compat) VALUES (?,?,?)") != BE_SQLITE_OK)
        return BentleyStatus::ERROR;

    stmt.BindText(1, info->name, Statement::MakeCopy::No);
    stmt.BindText(2, info->description, Statement::MakeCopy::No);
    stmt.BindText(3, FeatureCompatToString(info->compat), Statement::MakeCopy::No);

    if (stmt.Step() == BE_SQLITE_DONE)
        return BentleyStatus::SUCCESS;

    return BentleyStatus::ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus FeatureManager::DeleteFeature(ECDbCR ecdb, Utf8StringCR featureName)
    {
    Statement stmt;
    if (stmt.Prepare(ecdb, "DELETE FROM " TABLE_Feature " WHERE Name=?") != BE_SQLITE_OK)
        return BentleyStatus::ERROR;

    stmt.BindText(1, featureName.c_str(), Statement::MakeCopy::No);
    if (stmt.Step() == BE_SQLITE_DONE)
        return BentleyStatus::SUCCESS;

    return BentleyStatus::ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
//static
BentleyStatus FeatureManager::ReconcileSchemaFeatures(ECDbCR ecdb)
    {
    if (!ecdb.TableExists(TABLE_Feature))
        return BentleyStatus::SUCCESS;

    for (auto const& [name, info] : *s_knownFeatures)
        {
        if (info.featureDetector == nullptr)
            continue;

        const auto status = info.featureDetector(ecdb) ? InsertFeature(ecdb, name) : DeleteFeature(ecdb, name);
        if (status != BentleyStatus::SUCCESS)
            return status;
        }

    return BentleyStatus::SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE