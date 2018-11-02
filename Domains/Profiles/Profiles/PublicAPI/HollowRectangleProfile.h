/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/HollowRectangleProfile.h $
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
struct HollowRectangleProfile : CenteredProfile, IRectangleShapeProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_HollowRectangleProfile, CenteredProfile);
    friend struct HollowRectangleProfileHandler;

protected:
    explicit HollowRectangleProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(HollowRectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(HollowRectangleProfile)

    PROFILES_EXPORT static HollowRectangleProfilePtr Create(/*TODO: args*/);

public:
    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius(double val);

    }; // HollowRectangleProfile

//=======================================================================================
//! Handler for HollowRectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HollowRectangleProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_HollowRectangleProfile, HollowRectangleProfile, HollowRectangleProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // HollowRectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
