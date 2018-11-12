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
struct CenterLineLShapeProfile : CenteredProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CenterLineLShapeProfile, CenteredProfile);
    friend struct CenterLineLShapeProfileHandler;

protected:
    explicit CenterLineLShapeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CenterLineLShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CenterLineLShapeProfile)

    PROFILES_EXPORT static CenterLineLShapeProfilePtr Create(Dgn::DgnModelCR model);

public:
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth(double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth(double val);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius(double val);

    PROFILES_EXPORT double GetLipLength() const;
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
