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
struct DoubleCProfile : CompositeProfile
{
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_DoubleCProfile, CompositeProfile);
    friend struct DoubleCProfileHandler;

protected:
    explicit DoubleCProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(DoubleCProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(DoubleCProfile)

    PROFILES_EXPORT static DoubleCProfilePtr Create(/*TODO: args*/);

public:
    double GetSpacing() const { return GetPropertyValueDouble(PRF_PROP_DoubleCProfile_Spacing); }
    PROFILES_EXPORT void SetSpacing(double val);

}; // DoubleCProfile

END_BENTLEY_PROFILES_NAMESPACE
