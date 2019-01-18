/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/SinglePerimeterProfile.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesLogging.h>
#include <ProfilesInternal\ProfilesQuery.h>
#include <ProfilesInternal\ArbitraryCompositeProfileAspect.h>
#include <Profiles\SinglePerimeterProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (SinglePerimeterProfileHandler)

/*---------------------------------------------------------------------------------**//**
* Update geometry of all ArbitraryCompositeProfiles that are referencing this profile.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SinglePerimeterProfile::_UpdateInDb()
    {
    DgnDbStatus dbStatus;
    bvector<ArbitraryCompositeProfilePtr> compositeProfiles = ProfilesQuery::SelectByAspectNavigationProperty<ArbitraryCompositeProfile> (m_dgndb, m_elementId,
        PRF_CLASS_ArbitraryCompositeProfileAspect, PRF_PROP_ArbitraryCompositeProfileAspect_SingleProfile, &dbStatus);
    if (dbStatus != DgnDbStatus::Success)
        return dbStatus;

    for (auto const& compositeProfilePtr : compositeProfiles)
        {
        dbStatus = compositeProfilePtr->UpdateGeometry (*this);
        if (dbStatus != DgnDbStatus::Success)
            return dbStatus;

        compositeProfilePtr->Update (&dbStatus);
        if (dbStatus != DgnDbStatus::Success)
            return dbStatus;
        }
    return T_Super::_UpdateInDb();
    }

/*---------------------------------------------------------------------------------**//**
* Prohibit deleteion of this profile if it is being referenced by ArbitraryCompositeProfile.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SinglePerimeterProfile::_OnDelete() const
    {
    DgnDbStatus dbStatus;
    DgnElementId compositeProfileId = ProfilesQuery::SelectFirstByAspectNavigationProperty (m_dgndb, m_elementId,
        PRF_CLASS_ArbitraryCompositeProfileAspect, PRF_PROP_ArbitraryCompositeProfileAspect_SingleProfile, &dbStatus);
    if (dbStatus != DgnDbStatus::Success)
        return dbStatus;

    if (compositeProfileId.IsValid())
        {
        ProfilesLog::FailedDelete_ProfileHasReference (PRF_CLASS_SinglePerimeterProfile, m_elementId, PRF_CLASS_ArbitraryCompositeProfile, compositeProfileId);
        return DgnDbStatus::ForeignKeyConstraint;
        }

    return T_Super::_OnDelete();
    }

END_BENTLEY_PROFILES_NAMESPACE
