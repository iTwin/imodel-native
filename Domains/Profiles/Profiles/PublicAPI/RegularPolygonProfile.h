/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/RegularPolygonProfile.h $
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
struct RegularPolygonProfile : CenteredProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_RegularPolygonProfile, CenteredProfile);
    friend struct RegularPolygonProfileHandler;

protected:
    explicit RegularPolygonProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(RegularPolygonProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(RegularPolygonProfile)

    PROFILES_EXPORT static RegularPolygonProfilePtr Create(/*TODO: args*/);

    }; // RegularPolygonProfile

//=======================================================================================
//! Handler for RegularPolygonProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularPolygonProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_RegularPolygonProfile, RegularPolygonProfile, RegularPolygonProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // RegularPolygonProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
