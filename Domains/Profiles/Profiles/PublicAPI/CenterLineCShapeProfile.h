/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CenterLineCShapeProfile.h $
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
//! A C shaped Profile with rounded corners, similar to cold-formed steel C-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CenterLineCShapeProfile : CenteredProfile, ICShapeProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CenterLineCShapeProfile, CenteredProfile);
    friend struct CenterLineCShapeProfileHandler;

protected:
    explicit CenterLineCShapeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CenterLineCShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CenterLineCShapeProfile)

    PROFILES_EXPORT static CenterLineCShapeProfilePtr Create(Dgn::DgnModelCR model);

public:
    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius(double val);

    PROFILES_EXPORT double GetLipLength() const;
    PROFILES_EXPORT void SetLipLength(double val);

    }; // CenterLineCShapeProfile

//=======================================================================================
//! Handler for CenterLineCShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineCShapeProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CenterLineCShapeProfile, CenterLineCShapeProfile, CenterLineCShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // CenterLineCShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
