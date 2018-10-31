/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/TrapeziumProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CenteredProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct TrapeziumProfile : CenteredProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_TrapeziumProfile, CenteredProfile);
    friend struct TrapeziumProfileHandler;

protected:
    explicit TrapeziumProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(TrapeziumProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(TrapeziumProfile)

    PROFILES_EXPORT static TrapeziumProfilePtr Create(/*TODO: args*/);

public:
    double GetTopWidth() const { return GetPropertyValueDouble(PRF_PROP_TrapeziumProfile_TopWidth); }
    PROFILES_EXPORT void SetTopWidth(double val);
    double GetBottomWidth() const { return GetPropertyValueDouble(PRF_PROP_TrapeziumProfile_BottomWidth); }
    PROFILES_EXPORT void SetBottomWidth(double val);
    double GetDepth() const { return GetPropertyValueDouble(PRF_PROP_TrapeziumProfile_Depth); }
    PROFILES_EXPORT void SetDepth(double val);
    double GetTopOffset() const { return GetPropertyValueDouble(PRF_PROP_TrapeziumProfile_TopOffset); }
    PROFILES_EXPORT void SetTopOffset(double val);

    }; // TrapeziumProfile

//=======================================================================================
//! Handler for TrapeziumProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TrapeziumProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_TrapeziumProfile, TrapeziumProfile, TrapeziumProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // TrapeziumProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
