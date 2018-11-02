/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/RoundedRectangleProfile.h $
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
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct RoundedRectangleProfile : CenteredProfile, IRectangleShapeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_RoundedRectangleProfile, CenteredProfile);
    friend struct RoundedRectangleProfileHandler;

protected:
    explicit RoundedRectangleProfile(CreateParams const& params) : T_Super(params) {}

protected:
    virtual Dgn::DgnElementR _IRectangleShapeProfileToDgnElement() override { return *this; }

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(RoundedRectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(RoundedRectangleProfile)

    PROFILES_EXPORT static RoundedRectangleProfilePtr Create(/*TODO: args*/);

public:
    PROFILES_EXPORT double GetRoundingRadius() const;
    PROFILES_EXPORT void SetRoundingRadius(double val);

    }; // RoundedRectangleProfile

//=======================================================================================
//! Handler for RoundedRectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoundedRectangleProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_RoundedRectangleProfile, RoundedRectangleProfile, RoundedRectangleProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // RoundedRectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
