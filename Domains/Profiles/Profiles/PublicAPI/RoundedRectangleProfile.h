/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/RoundedRectangleProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct RoundedRectangleProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_RoundedRectangleProfile, ParametricProfile);
    friend struct RoundedRectangleProfileHandler;

protected:
    explicit RoundedRectangleProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (RoundedRectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (RoundedRectangleProfile)

    PROFILES_EXPORT static RoundedRectangleProfilePtr Create (CreateParams const& params) { return new RoundedRectangleProfile (params); }

public:
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth (double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double val);

    PROFILES_EXPORT double GetRoundingRadius() const;
    PROFILES_EXPORT void SetRoundingRadius (double val);

    }; // RoundedRectangleProfile

//=======================================================================================
//! Handler for RoundedRectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoundedRectangleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_RoundedRectangleProfile, RoundedRectangleProfile, RoundedRectangleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // RoundedRectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
