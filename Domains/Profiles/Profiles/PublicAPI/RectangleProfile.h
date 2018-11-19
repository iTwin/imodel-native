/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/RectangleProfile.h $
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
struct RectangleProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_RectangleProfile, ParametricProfile);
    friend struct RectangleProfileHandler;

protected:
    explicit RectangleProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (RectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (RectangleProfile)

    PROFILES_EXPORT static RectangleProfilePtr Create (CreateParams const& params) { return new RectangleProfile (params); }

    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth (double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double val);

    }; // RectangleProfile

//=======================================================================================
//! Handler for RectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RectangleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_RectangleProfile, RectangleProfile, RectangleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // RectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
