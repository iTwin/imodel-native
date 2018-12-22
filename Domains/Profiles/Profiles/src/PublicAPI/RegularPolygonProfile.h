/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/RegularPolygonProfile.h $
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
struct RegularPolygonProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_RegularPolygonProfile, ParametricProfile);
    friend struct RegularPolygonProfileHandler;

protected:
    explicit RegularPolygonProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (RegularPolygonProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (RegularPolygonProfile)

    PROFILES_EXPORT static RegularPolygonProfilePtr Create (CreateParams const& params) { return new RegularPolygonProfile (params); }

    PROFILES_EXPORT double GetSideCount() const;
    PROFILES_EXPORT void SetSideCount (double val);

    PROFILES_EXPORT double GetSideLength() const;
    PROFILES_EXPORT void SetSideLength (double val);

    }; // RegularPolygonProfile

//=======================================================================================
//! Handler for RegularPolygonProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularPolygonProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_RegularPolygonProfile, RegularPolygonProfile, RegularPolygonProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // RegularPolygonProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
