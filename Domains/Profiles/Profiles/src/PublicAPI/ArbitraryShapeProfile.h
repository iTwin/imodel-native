/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/ArbitraryShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "SinglePerimeterProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct ArbitraryShapeProfile : SinglePerimeterProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_ArbitraryShapeProfile, SinglePerimeterProfile);
    friend struct ArbitraryShapeProfileHandler;

protected:
    explicit ArbitraryShapeProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ArbitraryShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (ArbitraryShapeProfile)

    PROFILES_EXPORT static ArbitraryShapeProfilePtr Create (/*TODO: args*/);

    }; // ArbitraryShapeProfile

//=======================================================================================
//! Handler for ArbitraryShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArbitraryShapeProfileHandler : SinglePerimeterProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_ArbitraryShapeProfile, ArbitraryShapeProfile, ArbitraryShapeProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)

    }; // ArbitraryShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
