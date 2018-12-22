/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/DerivedProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "SinglePerimeterProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! This class represents a SinglePerimeterProfile that has been converted into a different shape via a matrix transform. Usage outside of IFC compatability is not recommended
//! @ingroup GROUP_Profiles
//=======================================================================================
struct DerivedProfile : SinglePerimeterProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_DerivedProfile, SinglePerimeterProfile);
    friend struct DerivedProfileHandler;

protected:
    explicit DerivedProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (DerivedProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (DerivedProfile)

    PROFILES_EXPORT static DerivedProfilePtr Create (/*TODO: args*/);

public:
    PROFILES_EXPORT bool GetMirrorProfileAboutYAxis() const;
    PROFILES_EXPORT void SetMirrorProfileAboutYAxis (bool val);

    PROFILES_EXPORT DPoint2d GetOffset() const;
    PROFILES_EXPORT void SetOffset (DPoint2dCR val);

    PROFILES_EXPORT DPoint2d GetRotation() const;
    PROFILES_EXPORT void SetRotation (DPoint2dCR val);

    PROFILES_EXPORT DPoint2d GetScale() const;
    PROFILES_EXPORT void SetScale (DPoint2dCR val);

    }; // DerivedProfile

//=======================================================================================
//! Handler for DerivedProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DerivedProfileHandler : SinglePerimeterProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_DerivedProfile, DerivedProfile, DerivedProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)

    }; // DerivedProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
