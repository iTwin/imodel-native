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
    //IGeometryPtr GetOuterCurve() const { return GetPropertyValue(PRF_PROP_CustomShapeProfile_OuterCurve); }
    PROFILES_EXPORT void SetOuterCurve(IGeometryPtr val);
    //IGeometryPtr GetInnerCurves() const { return GetPropertyValue(PRF_PROP_CustomShapeProfile_InnerCurves); }
    PROFILES_EXPORT void SetInnerCurves(IGeometryPtr val);

}; // CustomShapeProfile

END_BENTLEY_PROFILES_NAMESPACE
