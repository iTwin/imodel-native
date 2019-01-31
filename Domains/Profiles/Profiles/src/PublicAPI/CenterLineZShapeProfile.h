/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/CenterLineZShapeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A Z-shaped Profile with rounded corners, similar to cold-formed steel Z-shapes
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct CenterLineZShapeProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CenterLineZShapeProfile, ParametricProfile);
    friend struct CenterLineZShapeProfileHandler;

protected:
    explicit CenterLineZShapeProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CenterLineZShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CenterLineZShapeProfile)

    PROFILES_EXPORT static CenterLineZShapeProfilePtr Create (CreateParams const& params) { return new CenterLineZShapeProfile (params); }

public:
    PROFILES_EXPORT double GetFlangeWidth() const;
    PROFILES_EXPORT void SetFlangeWidth (double value);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double value);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius (double value);

    PROFILES_EXPORT double GetGirth() const;
    PROFILES_EXPORT void SetGirth (double value);

    }; // CenterLineZShapeProfile

//=======================================================================================
//! Handler for CenterLineZShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineZShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CenterLineZShapeProfile, CenterLineZShapeProfile, CenterLineZShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // CenterLineZShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
