/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/TTShapeProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A TT-shaped Profile similar to double-tee concrete shapes.
//! @ingroup GROUP_Profiles
//=======================================================================================
struct TTShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_TTShapeProfile, ParametricProfile);
    friend struct TTShapeProfileHandler;

protected:
    explicit TTShapeProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (TTShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (TTShapeProfile)

    PROFILES_EXPORT static TTShapeProfilePtr Create (CreateParams const& params) { return new TTShapeProfile (params); }

public:
    PROFILES_EXPORT double GetWidth() const;
    PROFILES_EXPORT void SetWidth (double value);

    PROFILES_EXPORT double GetDepth() const;
    PROFILES_EXPORT void SetDepth (double value);

    PROFILES_EXPORT double GetSpan() const;
    PROFILES_EXPORT void SetSpan (double value);

    PROFILES_EXPORT double GetFlangeThickness() const;
    PROFILES_EXPORT void SetFlangeThickness (double value);

    PROFILES_EXPORT double GetWebThickness() const;
    PROFILES_EXPORT void SetWebThickness (double value);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius (double value);

    PROFILES_EXPORT double GetFlangeEdgeRadius() const;
    PROFILES_EXPORT void SetFlangeEdgeRadius (double value);

    PROFILES_EXPORT double GetFlangeSlope() const;
    PROFILES_EXPORT void SetFlangeSlope (double value);

    PROFILES_EXPORT double GetWebEdgeRadius() const;
    PROFILES_EXPORT void SetWebEdgeRadius (double value);

    PROFILES_EXPORT double GetWebSlope() const;
    PROFILES_EXPORT void SetWebSlope (double value);

    }; // TTShapeProfile

//=======================================================================================
//! Handler for TTShapeProfile class
//! @ingroup GROUP_Profiles
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TTShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_TTShapeProfile, TTShapeProfile, TTShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // TTShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
