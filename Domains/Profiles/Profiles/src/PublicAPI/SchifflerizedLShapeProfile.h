/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/SchifflerizedLShapeProfile.h $
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
struct SchifflerizedLShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_SchifflerizedLShapeProfile, ParametricProfile);
    friend struct SchifflerizedLShapeProfileHandler;

protected:
    explicit SchifflerizedLShapeProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (SchifflerizedLShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (SchifflerizedLShapeProfile)

    PROFILES_EXPORT static SchifflerizedLShapeProfilePtr Create (CreateParams const& params) { return new SchifflerizedLShapeProfile (params); }

public:
    PROFILES_EXPORT double GetLegLength() const;
    PROFILES_EXPORT void SetLegLength (double value);

    PROFILES_EXPORT double GetThickness() const;
    PROFILES_EXPORT void SetThickness (double value);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius (double value);

    PROFILES_EXPORT double GetEdgeRadius() const;
    PROFILES_EXPORT void SetEdgeRadius (double value);

    PROFILES_EXPORT double GetLegSlope() const;
    PROFILES_EXPORT void SetLegSlope (double value);

    }; // SchifflerizedLShapeProfile

//=======================================================================================
//! Handler for SchifflerizedLShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SchifflerizedLShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_SchifflerizedLShapeProfile, SchifflerizedLShapeProfile, SchifflerizedLShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // SchifflerizedLShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
