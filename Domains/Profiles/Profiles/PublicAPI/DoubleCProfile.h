/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/DoubleCProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CompositeProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A CompositeProfile comprised of back-to-back Cs.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct DoubleCShapeProfile : CompositeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_DoubleCShapeProfile, CompositeProfile);
    friend struct DoubleCShapeProfileHandler;

protected:
    explicit DoubleCShapeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(DoubleCShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(DoubleCShapeProfile)

    PROFILES_EXPORT static DoubleCShapeProfilePtr Create(/*TODO: args*/);

public:
    PROFILES_EXPORT double GetSpacing() const;
    PROFILES_EXPORT void SetSpacing(double val);

    }; // DoubleCShapeProfile

//=======================================================================================
//! Handler for DoubleCShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoubleCShapeProfileHandler : CompositeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_DoubleCShapeProfile, DoubleCShapeProfile, DoubleCShapeProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)

    }; // DoubleCShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
