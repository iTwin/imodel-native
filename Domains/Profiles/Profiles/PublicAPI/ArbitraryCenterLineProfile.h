/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/ArbitraryCenterLineProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ArbitraryShapeProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct ArbitraryCenterLineProfile : ArbitraryShapeProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_ArbitraryCenterLineProfile, ArbitraryShapeProfile);
    friend struct ArbitraryCenterLineProfileHandler;

protected:
    explicit ArbitraryCenterLineProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ArbitraryCenterLineProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (ArbitraryCenterLineProfile)

    PROFILES_EXPORT static ArbitraryCenterLineProfilePtr Create (/*TODO: args*/);

    }; // ArbitraryCenterLineProfile

//=======================================================================================
//! Handler for ArbitraryCenterLineProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArbitraryCenterLineProfileHandler : ArbitraryShapeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_ArbitraryCenterLineProfile, ArbitraryCenterLineProfile, ArbitraryCenterLineProfileHandler, ArbitraryShapeProfileHandler, PROFILES_EXPORT)

    }; // ArbitraryCenterLineProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
