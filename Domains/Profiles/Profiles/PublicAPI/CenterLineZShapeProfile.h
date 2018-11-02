/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CenterLineZShapeProfile.h $
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
//! A Z-shaped Profile with rounded corners, similar to cold-formed steel Z-shapes
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CenterLineZShapeProfile : CenteredProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CenterLineZShapeProfile, CenteredProfile);
    friend struct CenterLineZShapeProfileHandler;

protected:
    explicit CenterLineZShapeProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CenterLineZShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CenterLineZShapeProfile)

    PROFILES_EXPORT static CenterLineZShapeProfilePtr Create(Dgn::DgnModelCR model);

public:
    PROFILES_EXPORT double GetFlangeWidth() const;
    PROFILES_EXPORT void SetFlangeWidth(double val);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth(double val);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius(double val);

    PROFILES_EXPORT double GetLipLength() const;
    PROFILES_EXPORT void SetLipLength(double val);

    }; // CenterLineZShapeProfile

//=======================================================================================
//! Handler for CenterLineZShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineZShapeProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CenterLineZShapeProfile, CenterLineZShapeProfile, CenterLineZShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // CenterLineZShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
