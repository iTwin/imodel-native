/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/Profile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A resource defining one or more 2D areas that may have voids.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct Profile : Dgn::DefinitionElement
    {
    DGNELEMENT_DECLARE_MEMBERS(PRF_CLASS_Profile, Dgn::DefinitionElement);
    friend struct ProfileHandler;

protected:
    explicit Profile(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS(Profile)
    DECLARE_PROFILES_ELEMENT_BASE_GET_METHODS(Profile)

public:
    PROFILES_EXPORT Utf8String GetName() const;
    PROFILES_EXPORT void SetName(Utf8String val);

    PROFILES_EXPORT IGeometryPtr GetShape() const;
    PROFILES_EXPORT void SetShape(IGeometryPtr val);

    }; // Profile


//=======================================================================================
//! Handler for Profile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ProfileHandler : Dgn::dgn_ElementHandler::Definition
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(PRF_CLASS_Profile, Profile, ProfileHandler, Dgn::dgn_ElementHandler::Definition, PROFILES_EXPORT)

    }; // ProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
