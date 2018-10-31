/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CenterLineLShapeProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "CenteredProfile.h"
#include "ProfileMixins.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! An L-shaped Profile with rounded corners, similar to cold-formed steel L-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CenterLineLShapeProfile : CenteredProfile, ILShapeProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CenterLineLShapeProfile, CenteredProfile);
    friend struct CenterLineLShapeProfileHandler;

protected:
    explicit CenterLineLShapeProfile(CreateParams const& params) : T_Super(params) {}

protected:
    virtual Dgn::DgnElementR _ILShapeProfileToDgnElement() override { return *this; }
    virtual Dgn::DgnElementR _ICenterLineProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CenterLineLShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CenterLineLShapeProfile)

    PROFILES_EXPORT static CenterLineLShapeProfilePtr Create(/*TODO: args*/);

public:
    double GetFilletRadius() const { return GetPropertyValueDouble(PRF_PROP_CenterLineLShapeProfile_FilletRadius); }
    PROFILES_EXPORT void SetFilletRadius(double val);
    double GetLipLength() const { return GetPropertyValueDouble(PRF_PROP_CenterLineLShapeProfile_LipLength); }
    PROFILES_EXPORT void SetLipLength(double val);

    }; // CenterLineLShapeProfile

//=======================================================================================
//! Handler for CenterLineLShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineLShapeProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CenterLineLShapeProfile, CenterLineLShapeProfile, CenterLineLShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // CenterLineLShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
