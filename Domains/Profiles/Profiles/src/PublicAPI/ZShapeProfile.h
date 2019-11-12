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
//! A Z-shaped Profile similar to rolled steel Z-shapes.
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct ZShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_ZShapeProfile, ParametricProfile);
    friend struct ZShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (ZShapeProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness,
                                               double webThickness, double filletRadius = 0.0, double flangeEdgeRadius = 0.0,
                                               Angle const& flangeSlope = Angle::FromRadians (0.0));

    public:
        //! @beginGroup
        double flangeWidth = 0.0; //!< Extent of single flange. @details Defined parallel to the x axis of the position coordinate system.
        double depth = 0.0; //!< Extent of the depth (web length). @details Defined parallel to the y axis of the position coordinate system.
        double flangeThickness = 0.0; //!< Constant wall thickness of flanges.
        double webThickness = 0.0; //!< Constant wall thickness of the web.
        //! @endGroup

        //! @beginGroup
        double filletRadius = 0.0; //!< The fillet radius between the web and the flanges. @details 0 if sharp-edged, default 0 if not specified.
        double flangeEdgeRadius = 0.0; //!< Radius of flange edges. @details 0 if sharp-edged, default 0 if not specified.
        Angle flangeSlope = Angle::FromRadians (0.0); //!< Slope of the lower face of the top flange and of the upper face of the bottom flange. @details Non-zero in case of tapered flanges, 0 in case of parallel flanges, default 0 if not specified.
        //! @endGroup
        };

protected:
    explicit ZShapeProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateShapeGeometry() const override; //!< @private

private:
    bool ValidateFlangeThickness() const;
    bool ValidateWebThickness() const;
    bool ValidateFilletRadius() const;
    bool ValidateFlangeEdgeRadius() const;
    bool ValidateFlangeSlope() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (ZShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (ZShapeProfile)

    //! Creates an instance of ZShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of ZShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static ZShapeProfilePtr Create (CreateParams const& params) { return new ZShapeProfile (params); }

public:
    //! @beginGroup
    PROFILES_EXPORT double GetFlangeWidth() const; //!< Get the value of @ref CreateParams.flangeWidth "FlangeWidth"
    PROFILES_EXPORT void SetFlangeWidth (double value); //!< Set the value for @ref CreateParams.flangeWidth "FlangeWidth"

    PROFILES_EXPORT double GetDepth() const; //!< Get the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Set the value for @ref CreateParams.depth "Depth"

    PROFILES_EXPORT double GetFlangeThickness() const; //!< Get the value of @ref CreateParams.flangeThickness "FlangeThickness"
    PROFILES_EXPORT void SetFlangeThickness (double value); //!< Set the value for @ref CreateParams.flangeThickness "FlangeThickness"

    PROFILES_EXPORT double GetWebThickness() const; //!< Get the value of @ref CreateParams.webThickness "WebThickness"
    PROFILES_EXPORT void SetWebThickness (double value); //!< Set the value for @ref CreateParams.webThickness "WebThickness"

    PROFILES_EXPORT double GetFilletRadius() const; //!< Get the value of @ref CreateParams.filletRadius "FilletRadius"
    PROFILES_EXPORT void SetFilletRadius (double value); //!< Set the value for @ref CreateParams.filletRadius "FilletRadius"

    PROFILES_EXPORT double GetFlangeEdgeRadius() const; //!< Get the value of @ref CreateParams.flangeEdgeRadius "FlangeEdgeRadius"
    PROFILES_EXPORT void SetFlangeEdgeRadius (double value); //!< Set the value for @ref CreateParams.flangeEdgeRadius "FlangeEdgeRadius"

    PROFILES_EXPORT Angle GetFlangeSlope() const; //!< Get the value of @ref CreateParams.flangeSlope "FlangeSlope"
    PROFILES_EXPORT void SetFlangeSlope (Angle const& value); //!< Set the value for @ref CreateParams.flangeSlope "FlangeSlope"
    //! @beginGroup

public:
    PROFILES_EXPORT double GetFlangeInnerFaceLength() const; //!< @private
    PROFILES_EXPORT double GetWebFaceLength() const; //!< @private
    PROFILES_EXPORT double GetFlangeSlopeHeight() const; //!< @private

    }; // ZShapeProfile

//=======================================================================================
//! Handler for ZShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ZShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_ZShapeProfile, ZShapeProfile, ZShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // ZShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
