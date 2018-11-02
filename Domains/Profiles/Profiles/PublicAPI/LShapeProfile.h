/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/LShapeProfile.h $
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
//! An L-shaped Profile similar to rolled steel L-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct LShapeProfile : CenteredProfile, ILShapeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_LShapeProfile, CenteredProfile);
    friend struct LShapeProfileHandler;

protected:
    explicit LShapeProfile(CreateParams const& params) : T_Super(params) {}

protected:
    virtual Dgn::DgnElementR _ILShapeProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(LShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(LShapeProfile)

    PROFILES_EXPORT static LShapeProfilePtr Create(/*TODO: args*/);

public:
    PROFILES_EXPORT double GetThickness() const;
    PROFILES_EXPORT void SetThickness(double val);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius(double val);

    PROFILES_EXPORT double GetEdgeRadius() const;
    PROFILES_EXPORT void SetEdgeRadius(double val);

    PROFILES_EXPORT double GetHorizontalLegSlope() const;
    PROFILES_EXPORT void SetHorizontalLegSlope(double val);

    PROFILES_EXPORT double GetVerticalLegSlope() const;
    PROFILES_EXPORT void SetVerticalLegSlope(double val);

    }; // LShapeProfile

//=======================================================================================
//! Handler for LShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LShapeProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_LShapeProfile, LShapeProfile, LShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // LShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
