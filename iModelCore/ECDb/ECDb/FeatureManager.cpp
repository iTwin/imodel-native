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

END_BENTLEY_SQLITE_EC_NAMESPACE