/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    01/2018
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
// @bsimethod                                                    Krischan.Eberle    01/2018
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
// @bsimethod                                                    Krischan.Eberle    01/2018
//+---------------+---------------+---------------+---------------+---------------+--------
//static
std::map<Feature, ProfileVersion> const* FeatureManager::s_featureMinimumVersions = new std::map<Feature, ProfileVersion>(FEATURE_MINIMUMVERSIONS);

END_BENTLEY_SQLITE_EC_NAMESPACE