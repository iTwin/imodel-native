/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "Profile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A Profile with a single outer perimiter.
//! @ingroup GROUP_SinglePerimeterProfiles
//=======================================================================================
struct SinglePerimeterProfile : Profile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_SinglePerimeterProfile, Profile);
    friend struct SinglePerimeterProfileHandler;

protected:
    explicit SinglePerimeterProfile (CreateParams const& params) : T_Super (params) {}

    PROFILES_EXPORT virtual Dgn::DgnDbStatus _UpdateInDb() override; //!< @private
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnDelete() const override; //!< @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (SinglePerimeterProfile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS (SinglePerimeterProfile)

    }; // SinglePerimeterProfile

//=======================================================================================
//! Handler for SinglePerimeterProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SinglePerimeterProfileHandler : ProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS_ABSTRACT (PRF_CLASS_SinglePerimeterProfile, SinglePerimeterProfile, SinglePerimeterProfileHandler, ProfileHandler, PROFILES_EXPORT)

    }; // SinglePerimeterProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
