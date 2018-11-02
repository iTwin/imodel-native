/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CustomShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "SinglePerimeterProfile.h"
#include "ProfileMixins.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CustomShapeProfile : SinglePerimeterProfile, ICustomProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CustomShapeProfile, SinglePerimeterProfile);
    friend struct CustomShapeProfileHandler;

protected:
    explicit CustomShapeProfile(CreateParams const& params) : T_Super(params) {}

protected:
    virtual Dgn::DgnElementR _ICustomProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CustomShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CustomShapeProfile)

    PROFILES_EXPORT static CustomShapeProfilePtr Create(/*TODO: args*/);

public:
    PROFILES_EXPORT IGeometryPtr GetOuterCurve() const;
    PROFILES_EXPORT void SetOuterCurve(IGeometryPtr val);

    PROFILES_EXPORT IGeometryPtr GetInnerCurves() const;
    PROFILES_EXPORT void SetInnerCurves(IGeometryPtr val);

    }; // CustomShapeProfile

//=======================================================================================
//! Handler for CustomShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CustomShapeProfileHandler : SinglePerimeterProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CustomShapeProfile, CustomShapeProfile, CustomShapeProfileHandler, SinglePerimeterProfileHandler, PROFILES_EXPORT)

    }; // CustomShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
