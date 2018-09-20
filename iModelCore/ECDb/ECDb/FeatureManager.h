/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/FeatureManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include "ProfileManager.h"
#include "IssueReporter.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//***************************************************************
// ECDb Features
// -------------
// When a new feature is added to ECDb, we no longer assume that older files are upgraded to include the new feature.
// Instead ECDb has to deal with older files not supporting the new feature.
// Therefore for each feature, a required minimum ECDb profile version is defined, which code can query for using
// FeatureManager::IsAvailable.
//
// How to define new Features
// --------------------------
// 1) Add the new feature to the enum Feature
// 2) Add an entry to the FEATURE_MINIMUMVERSIONS define for the new feature with the minimum required profile version

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2018
//+===============+===============+===============+===============+===============+======
enum class Feature
    {
    ECVersions,
    NamedEnumerators,
    UnitsAndFormats,
    SystemPropertiesHaveIdExtendedType // corresponds to ECDbSystem schema version 5.0.1 or newer
    };

//defines the minimum profile versions each feature requires
#define FEATURE_MINIMUMVERSIONS { \
    {Feature::ECVersions, ProfileVersion(4, 0, 0, 2)}, \
    {Feature::NamedEnumerators, ProfileVersion(4, 0, 0, 2)}, \
    {Feature::UnitsAndFormats, ProfileVersion(4, 0, 0, 2)}, \
    {Feature::SystemPropertiesHaveIdExtendedType, ProfileVersion(4, 0, 0, 2)} \
    }

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2018
//+===============+===============+===============+===============+===============+======
struct FeatureManager final
    {
private:
    FeatureManager() = delete;
    ~FeatureManager() = delete;

    //non-POD static members must never be destroyed (Bentley guideline)
    static std::map<Feature, ProfileVersion> const* s_featureMinimumVersions;

    static bool IsAvailable(ProfileVersion const& actualVersion, Feature);

public:
    //! convenience method to check whether EC3.2 features (units, named enumerators) are available in the given file
    static bool IsEC32Available(ECDbCR ecdb) { return IsAvailable(ecdb, Feature::UnitsAndFormats); }
    static bool IsAvailable(ECDbCR ecdb, Feature feature) { return IsAvailable(ecdb.GetECDbProfileVersion(), feature); }
    static bool IsAvailable(ECDbCR, std::vector<Feature> const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
