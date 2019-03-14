/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/SchifflerizedLShapeProfile.h $
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
//! An L-shaped Profile (also refered as Angle) with legs bent at 15 degree angle, making the angle between the legs equal to 60 degrees.
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct SchifflerizedLShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_SchifflerizedLShapeProfile, ParametricProfile);
    friend struct SchifflerizedLShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (SchifflerizedLShapeProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double legLength, double thickness,
                                               double legBendOffset = 0.0, double filletRadius = 0.0, double edgeRadius = 0.0);

    public:
        //! @beginGroup
        //! Leg length. @details Both vertical and horizontal legs are defined wiht equal length.
        double legLength = 0.0;
        //! Constant wall thickness of profile.
        double thickness = 0.0;
        //! @endGroup

        //! @beginGroup
        //! Offset defining where legs are bent. @details Offset is measured from start of the angle (leg joint) plus
        //! leg thickness i.e. if legBendOffset is zero, legs will be bent at distance equal to thickness from the leg joint.
        double legBendOffset = 0.0;
        //! Fillet radius between legs. @details 0 if sharp-edged, default 0 if not specified.
        double filletRadius = 0.0;
        //! Radius of flange edges. @details 0 if sharp-edged, default 0 if not specified.
        double edgeRadius = 0.0;
        //! @endGroup
        };

private:
    explicit SchifflerizedLShapeProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    IGeometryPtr _CreateShapeGeometry() const override;

    bool ValidateThickness() const;
    bool ValidateLegBendOffset() const;
    bool ValidateFilletRadius() const;
    bool ValidateEdgeRadius() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (SchifflerizedLShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (SchifflerizedLShapeProfile)

    //! Creates an instance of SchifflerizedLShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of SchifflerizedLShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static SchifflerizedLShapeProfilePtr Create (CreateParams const& params) { return new SchifflerizedLShapeProfile (params); }

    //! @beginGroup
    PROFILES_EXPORT double GetLegLength() const; //!< Get the value of @ref CreateParams.legLength "LegLength"
    PROFILES_EXPORT void SetLegLength (double value); //!< Set the value for @ref CreateParams.legLength "LegLength"

    PROFILES_EXPORT double GetThickness() const; //!< Get the value of @ref CreateParams.thickness "Thickness"
    PROFILES_EXPORT void SetThickness (double value); //!< Set the value for @ref CreateParams.thickness "Thickness"

    PROFILES_EXPORT double GetLegBendOffset() const; //!< Get the value of @ref CreateParams.legBendOffset "LegBendOffset"
    PROFILES_EXPORT void SetLegBendOffset (double value); //!< Set the value for @ref CreateParams.legBendOffset "LegBendOffset"

    PROFILES_EXPORT double GetFilletRadius() const; //!< Get the value of @ref CreateParams.filletRadius "FilletRadius"
    PROFILES_EXPORT void SetFilletRadius (double value); //!< Set the value for @ref CreateParams.filletRadius "FilletRadius"

    PROFILES_EXPORT double GetEdgeRadius() const; //!< Get the value of @ref CreateParams.edgeRadius "EdgeRadius"
    PROFILES_EXPORT void SetEdgeRadius (double value); //!< Set the value for @ref CreateParams.edgeRadius "EdgeRadius"
    //! @endGroup

    }; // SchifflerizedLShapeProfile

//=======================================================================================
//! Handler for SchifflerizedLShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SchifflerizedLShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_SchifflerizedLShapeProfile, SchifflerizedLShapeProfile, SchifflerizedLShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // SchifflerizedLShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
