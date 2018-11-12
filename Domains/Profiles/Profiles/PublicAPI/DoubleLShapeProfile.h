/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/DoubleLShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CompositeProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A CompositeProfile comprised of back-to-back Ls with the horizontal legs at the top of the Profile.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct DoubleLShapeProfile : CompositeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_DoubleLShapeProfile, CompositeProfile);
    friend struct DoubleLShapeProfileHandler;

protected:
    explicit DoubleLShapeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(DoubleLShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(DoubleLShapeProfile)

    PROFILES_EXPORT static DoubleLShapeProfilePtr Create(/*TODO: args*/);

public:
    PROFILES_EXPORT double GetSpacing() const;
    PROFILES_EXPORT void SetSpacing(double val);

    PROFILES_EXPORT int GetType() const;
    PROFILES_EXPORT void SetType(int val);

    }; // DoubleLShapeProfile

//=======================================================================================
//! Handler for DoubleLShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DoubleLShapeProfileHandler : CompositeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_DoubleLShapeProfile, DoubleLShapeProfile, DoubleLShapeProfileHandler, CompositeProfileHandler, PROFILES_EXPORT)

    }; // DoubleLShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
