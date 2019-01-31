/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/RegularPolygonProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct RegularPolygonProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_RegularPolygonProfile, ParametricProfile);
    friend struct RegularPolygonProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(RegularPolygonProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, uint64_t sideCount, double sideLength);

    public:
        //! Required properties
        uint64_t sideCount = 0;
        double sideLength = 0.0;
        };

protected:
    explicit RegularPolygonProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual IGeometryPtr _CreateGeometry() const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (RegularPolygonProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (RegularPolygonProfile)

    PROFILES_EXPORT static RegularPolygonProfilePtr Create (CreateParams const& params) { return new RegularPolygonProfile (params); }

    PROFILES_EXPORT uint64_t GetSideCount() const;
    PROFILES_EXPORT void SetSideCount (uint64_t value);

    PROFILES_EXPORT double GetSideLength() const;
    PROFILES_EXPORT void SetSideLength (double value);

    }; // RegularPolygonProfile

//=======================================================================================
//! Handler for RegularPolygonProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularPolygonProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_RegularPolygonProfile, RegularPolygonProfile, RegularPolygonProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // RegularPolygonProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
