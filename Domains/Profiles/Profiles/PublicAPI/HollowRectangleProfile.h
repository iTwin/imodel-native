/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/HollowRectangleProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct HollowRectangleProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_HollowRectangleProfile, ParametricProfile);
    friend struct HollowRectangleProfileHandler;

protected:
    explicit HollowRectangleProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (HollowRectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (HollowRectangleProfile)

    PROFILES_EXPORT static HollowRectangleProfilePtr Create (CreateParams const& params) { return new HollowRectangleProfile (params); }

public:
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth (double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double val);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius (double val);

    }; // HollowRectangleProfile

//=======================================================================================
//! Handler for HollowRectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HollowRectangleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_HollowRectangleProfile, HollowRectangleProfile, HollowRectangleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // HollowRectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
