/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/ArbitraryShapeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "SinglePerimeterProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! TODO Karolis: Add description
//! @ingroup GROUP_SinglePerimeterProfiles
//=======================================================================================
struct ArbitraryShapeProfile : SinglePerimeterProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_ArbitraryShapeProfile, SinglePerimeterProfile);
    friend struct ArbitraryShapeProfileHandler;

protected:
    //! @private
    explicit ArbitraryShapeProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ArbitraryShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (ArbitraryShapeProfile)

    PROFILES_EXPORT static ArbitraryShapeProfilePtr Create (/*TODO: args*/);

    }; // ArbitraryShapeProfile

//=======================================================================================
//! Handler for ArbitraryShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ArbitraryShapeProfileHandler : SinglePerimeterProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_ArbitraryShapeProfile, ArbitraryShapeProfile, ArbitraryShapeProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)

    }; // ArbitraryShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
