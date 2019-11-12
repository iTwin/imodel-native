/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesLogging.h>
#include <ProfilesInternal\ProfilesQuery.h>
#include <ProfilesInternal\ArbitraryCompositeProfileAspect.h>
#include <Profiles\DerivedProfile.h>
#include <Profiles\SinglePerimeterProfile.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

HANDLER_DEFINE_MEMBERS (SinglePerimeterProfileHandler)

/*---------------------------------------------------------------------------------**//**
* Update geometry of all ArbitraryCompositeProfiles that are referencing 'profile'.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus updateGeometryForCompositeProfiles (SinglePerimeterProfile const& profile)
    {
    DgnDbStatus status;
    bvector<ArbitraryCompositeProfilePtr> compositeProfiles = ProfilesQuery::SelectByAspectNavigationProperty<ArbitraryCompositeProfile>
        (profile.GetDgnDb(), profile.GetElementId(), PRF_CLASS_ArbitraryCompositeProfileAspect, PRF_PROP_ArbitraryCompositeProfileAspect_SingleProfile, &status);
    if (status != DgnDbStatus::Success)
        return status;

    for (ArbitraryCompositeProfilePtr const& compositeProfilePtr : compositeProfiles)
        {
        status = compositeProfilePtr->UpdateGeometry (profile);
        if (status != DgnDbStatus::Success)
            return status;

        compositeProfilePtr->Update (&status);
        if (status != DgnDbStatus::Success)
            return status;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Update geometry of all DerivedPorifles that are referencing 'profile'.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus updateGeometryForDerivedProfiles (SinglePerimeterProfile const& profile)
    {
    DgnDbStatus status;
    DgnElementIdSet childrenIds = profile.QueryChildren();

    for (DgnElementId const& childId : childrenIds)
        {
        DerivedProfilePtr const& derivedProfilePtr = DerivedProfile::GetForEdit (profile.GetDgnDb(), childId);
        if (derivedProfilePtr.IsNull())
            continue;

        status = derivedProfilePtr->UpdateGeometry (profile);
        if (status != DgnDbStatus::Success)
            return status;

        derivedProfilePtr->Update (&status);
        if (status != DgnDbStatus::Success)
            return status;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Update geometry of Profiles that are referencing this profile.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SinglePerimeterProfile::_UpdateInDb()
    {
    DgnDbStatus status = updateGeometryForCompositeProfiles (*this);
    if (status != DgnDbStatus::Success)
        return status;

    status = updateGeometryForDerivedProfiles (*this);
    if (status != DgnDbStatus::Success)
        return status;

    return T_Super::_UpdateInDb();
    }

/*---------------------------------------------------------------------------------**//**
* Prohibit deleteion of 'profile' if it is being referenced by any ArbitraryCompositeProfile.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus checkForeignKeyConstraintForCompositeProfiles (SinglePerimeterProfile const& profile)
    {
    DgnDbStatus status;
    DgnElementId compositeProfileId = ProfilesQuery::SelectFirstByAspectNavigationProperty (profile.GetDgnDb(), profile.GetElementId(),
        PRF_CLASS_ArbitraryCompositeProfileAspect, PRF_PROP_ArbitraryCompositeProfileAspect_SingleProfile, &status);
    if (status != DgnDbStatus::Success)
        return status;

    if (compositeProfileId.IsValid())
        {
        ProfilesLog::FailedDelete_ProfileHasReference (PRF_CLASS_SinglePerimeterProfile, profile.GetElementId(), PRF_CLASS_ArbitraryCompositeProfile, compositeProfileId);
        return DgnDbStatus::ForeignKeyConstraint;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Prohibit deleteion of 'profile' if it is being referenced by any DerivedProfile.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus checkForeignKeyConstraintForDerivedProfiles (SinglePerimeterProfile const& profile)
    {
    DgnElementIdSet childrenIds = profile.QueryChildren();

    if (!childrenIds.empty())
        {
        ProfilesLog::FailedDelete_ProfileHasReference (PRF_CLASS_SinglePerimeterProfile, profile.GetElementId(), PRF_CLASS_DerivedProfile, *childrenIds.begin());
        return DgnDbStatus::ForeignKeyConstraint;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Prohibit deleteion of this profile if it is being referenced by other Profiles.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SinglePerimeterProfile::_OnDelete() const
    {
    DgnDbStatus status = checkForeignKeyConstraintForCompositeProfiles (*this);
    if (status != DgnDbStatus::Success)
        return status;

    status = checkForeignKeyConstraintForDerivedProfiles (*this);
    if (status != DgnDbStatus::Success)
        return status;

    return T_Super::_OnDelete();
    }

END_BENTLEY_PROFILES_NAMESPACE
