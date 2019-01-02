/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/CapsuleProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct CapsuleProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CapsuleProfile, ParametricProfile);
    friend struct CapsuleProfileHandler;

protected:
    explicit CapsuleProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CapsuleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CapsuleProfile)

    PROFILES_EXPORT static CapsuleProfilePtr Create (CreateParams const& params) { return new CapsuleProfile (params); }

    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth (double value);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double value);

    }; // CapsuleProfile

//=======================================================================================
//! Handler for CapsuleProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CapsuleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CapsuleProfile, CapsuleProfile, CapsuleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // CapsuleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
