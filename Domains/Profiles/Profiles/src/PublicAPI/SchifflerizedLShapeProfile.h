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

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER(SchifflerizedLShapeProfile::T_Super::CreateParams);
        explicit CreateParams (DgnElement::CreateParams const& params) : T_Super (params) {}

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double legLength, double thickness,
                                               double legBendOffset = 0.0, double filletRadius = 0.0, double edgeRadius = 0.0);

    public:
        //! Required properties
        double legLength = 0.0;
        double thickness = 0.0;

        //! Optional properties
        double legBendOffset = 0.0;
        double filletRadius = 0.0;
        double edgeRadius = 0.0;
        };

protected:
    explicit SchifflerizedLShapeProfile (CreateParams const& params);

    virtual bool _Validate() const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (SchifflerizedLShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (SchifflerizedLShapeProfile)

    PROFILES_EXPORT static SchifflerizedLShapeProfilePtr Create (CreateParams const& params) { return new SchifflerizedLShapeProfile (params); }

private:
    bool ValidateThickness() const;
    bool ValidateLegBendOffset() const;
    bool ValidateFilletRadius() const;
    bool ValidateEdgeRadius() const;

public:
    PROFILES_EXPORT double GetLegLength() const;
    PROFILES_EXPORT void SetLegLength (double value);

    PROFILES_EXPORT double GetThickness() const;
    PROFILES_EXPORT void SetThickness (double value);

    PROFILES_EXPORT double GetLegBendOffset() const;
    PROFILES_EXPORT void SetLegBendOffset (double value);

    PROFILES_EXPORT double GetFilletRadius() const;
    PROFILES_EXPORT void SetFilletRadius (double value);

    PROFILES_EXPORT double GetEdgeRadius() const;
    PROFILES_EXPORT void SetEdgeRadius (double value);

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
