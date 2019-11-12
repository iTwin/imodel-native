/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! An L-shaped Profile (also refered as Angle) similar to rolled steel L-shapes.
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct LShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_LShapeProfile, ParametricProfile);
    friend struct LShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (LShapeProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double width, double depth, double thickness,
                                               double filletRadius = 0.0, double edgeRadius = 0.0, Angle const& legSlope = Angle::FromRadians (0.0));

    public:
        //! @beginGroup
        double width = 0.0; //!< Horizontal leg length. @details Defined parallel to the x axis of the position coordinate system.
        double depth = 0.0; //!< Vertical leg length. @details Defined parallel to the y axis of the position coordinate system.
        double thickness = 0.0; //!< Constant wall thickness of profile.
        //! @endGroup

        //! @beginGroup
        double filletRadius = 0.0; //!< Fillet radius between legs. @details 0 if sharp-edged, default 0 if not specified.
        double edgeRadius = 0.0; //!< Radius of flange edges. @details 0 if sharp-edged, default 0 if not specified.
        Angle legSlope = Angle::FromRadians (0.0); //!< Slope of the inner faces of the legs. @details Non-zero in case of tapered legs, 0 in case of perpendicular legs, default 0 if not specified.
        //! @endGroup
        };

protected:
    explicit LShapeProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateShapeGeometry() const override; //!< @private

    PROFILES_EXPORT virtual Dgn::DgnDbStatus _OnDelete() const override; //!< @private
    PROFILES_EXPORT virtual Dgn::DgnDbStatus _UpdateInDb() override; //!< @private

private:
    bool ValidateThickness() const;
    bool ValidateFilletRadius() const;
    bool ValidateEdgeRadius() const;
    bool ValidateLegSlope() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (LShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (LShapeProfile)

    //! Creates an instance of LShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of LShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static LShapeProfilePtr Create (CreateParams const& params) { return new LShapeProfile (params); }

public:
    //! @beginGroup
    PROFILES_EXPORT double GetWidth() const; //!< Get the value of @ref CreateParams.width "Width"
    PROFILES_EXPORT void SetWidth (double value); //!< Set the value for @ref CreateParams.width "Width"

    PROFILES_EXPORT double GetDepth() const; //!< Get the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Set the value for @ref CreateParams.depth "Depth"

    PROFILES_EXPORT double GetThickness() const; //!< Get the value of @ref CreateParams.thickness "Thickness"
    PROFILES_EXPORT void SetThickness (double value); //!< Set the value for @ref CreateParams.thickness "Thickness"

    PROFILES_EXPORT double GetFilletRadius() const; //!< Get the value of @ref CreateParams.filletRadius "FilletRadius"
    PROFILES_EXPORT void SetFilletRadius (double value); //!< Set the value for @ref CreateParams.filletRadius "FilletRadius"

    PROFILES_EXPORT double GetEdgeRadius() const; //!< Get the value of @ref CreateParams.edgeRadius "EdgeRadius"
    PROFILES_EXPORT void SetEdgeRadius (double value); //!< Set the value for @ref CreateParams.edgeRadius "EdgeRadius"

    PROFILES_EXPORT Angle GetLegSlope() const; //!< Get the value of @ref CreateParams.legSlope "LegSlope"
    PROFILES_EXPORT void SetLegSlope (Angle const& value); //!< Set the value for @ref CreateParams.legSlope "LegSlope"
    //! @endGroup

public:
    PROFILES_EXPORT double GetFlangeInnerFaceLength() const; //!< @private
    PROFILES_EXPORT double GetWebInnerFaceLength() const; //!< @private
    PROFILES_EXPORT double GetHorizontalLegSlopeHeight() const; //!< @private
    PROFILES_EXPORT double GetVerticalLegSlopeHeight() const; //!< @private

    }; // LShapeProfile

//=======================================================================================
//! Handler for LShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_LShapeProfile, LShapeProfile, LShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // LShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
