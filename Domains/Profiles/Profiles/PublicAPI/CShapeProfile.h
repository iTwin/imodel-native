/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/CShapeProfile.h $
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
//! A C shaped Profile similar to rolled steel C-shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CShapeProfile : CenteredProfile, ICShapeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_CShapeProfile, CenteredProfile);
    friend struct CShapeProfileHandler;

protected:
    explicit CShapeProfile(CreateParams const& params) : T_Super(params) {}

protected:
    virtual Dgn::DgnElementR _ICShapeProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(CShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(CShapeProfile)

    PROFILES_EXPORT static CShapeProfilePtr Create(/*TODO: args*/);

public:
    PROFILES_EXPORT double GetFlangeThickness() const;
    PROFILES_EXPORT void SetFlangeThickness(double val);

    PROFILES_EXPORT double GetWebThickness() const;
    PROFILES_EXPORT void SetWebThickness(double val);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius(double val);

    PROFILES_EXPORT double GetEdgeRadius() const;
    PROFILES_EXPORT void SetEdgeRadius(double val);

    PROFILES_EXPORT double GetFlangeSlope() const;
    PROFILES_EXPORT void SetFlangeSlope(double val);

    }; // CShapeProfile

//=======================================================================================
//! Handler for CShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CShapeProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_CShapeProfile, CShapeProfile, CShapeProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // CShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
