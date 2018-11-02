/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/RectangleProfile.h $
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
struct RectangleProfile : CenteredProfile, IRectangleShapeProfile
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_RectangleProfile, CenteredProfile);
    friend struct RectangleProfileHandler;

protected:
    explicit RectangleProfile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(RectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS(RectangleProfile)

    PROFILES_EXPORT static RectangleProfilePtr Create(Dgn::DgnModelCR model);

    }; // RectangleProfile

//=======================================================================================
//! Handler for RectangleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RectangleProfileHandler : CenteredProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_RectangleProfile, RectangleProfile, RectangleProfileHandler, CenteredProfileHandler, PROFILES_EXPORT)

    }; // RectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
