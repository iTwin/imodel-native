/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/ArbitraryCenterLineProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ArbitraryShapeProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! TODO Karolis: Add description
//! @ingroup GROUP_SinglePerimeterProfiles
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
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArbitraryCenterLineProfileHandler : ArbitraryShapeProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_ArbitraryCenterLineProfile, ArbitraryCenterLineProfile, ArbitraryCenterLineProfileHandler, ArbitraryShapeProfileHandler, PROFILES_EXPORT)

    }; // ArbitraryCenterLineProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
