/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/AsymmetricIShapeProfile.h $
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
//! An asymmetric (about x-axis) I shaped Profile similar to rolled steel I-shapes.
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct AsymmetricIShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_AsymmetricIShapeProfile, ParametricProfile);
    friend struct AsymmetricIShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (AsymmetricIShapeProfile)

    public:
        //! Minimal constructor that initializes all members to zero.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        //! Full constructor to initialize members.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double topFlangeWidth, double bottomFlangeWidth, double depth,
                                               double topFlangeThickness, double bottomFlangeThickness, double webThickness, double topFlangeFilletRadius = 0.0,
                                               double topFlangeEdgeRadius = 0.0, Angle const& topFlangeSlope = Angle::FromRadians (0.0), double bottomFlangeFilletRadius = 0.0,
                                               double bottomFlangeEdgeRadius = 0.0, Angle const& bottomFlangeSlope = Angle::FromRadians (0.0));

    public:
        //! @beginGroup
        double topFlangeWidth = 0.0; //!< Extent of the top flange. @details Defined parallel to the x axis of the position coordinate system.
        double bottomFlangeWidth = 0.0; //!< Extent of the bottom flange. @details Defined parallel to the x axis of the position coordinate system.
        double depth = 0.0; //!< Extent of the depth (web length). @details Defined parallel to the y axis of the position coordinate system.
        double topFlangeThickness = 0.0; //!< Thickness of the top flange.
        double bottomFlangeThickness = 0.0; //!< Thickness of the bottom flange.
        double webThickness = 0.0; //!< Thickness of the web. @details The web is centred on the x-axis and the y-axis of the position coordinate system.
        //! @endGroup

        //! @beginGroup
        double topFlangeFilletRadius = 0.0; //!< The fillet radius between the web and the top flange. @details 0 if sharp-edged, default 0 if not specified.
        double topFlangeEdgeRadius = 0.0; //!< Radius of the lower edges of the top flange. @details 0 if sharp-edged, default 0 if not specified.
        Angle topFlangeSlope = Angle::FromRadians (0.0); //!< Slope of the lower faces of the top flange. @details Non-zero in case of of tapered top flange, 0 in case of parallel top flange, default 0 if not specified.
        double bottomFlangeFilletRadius = 0.0; //!< The fillet radius between the web and the bottom flange. @details 0 if sharp-edged, default 0 if not specified.
        double bottomFlangeEdgeRadius = 0.0; //!< Radius of the upper edges of the bottom flange. @details 0 if sharp-edged, default 0 if not specified.
        Angle bottomFlangeSlope = Angle::FromRadians (0.0); //!< Slope of the upper faces of the bottom flange. @details Non-zero in case of of tapered bottom flange, 0 in case of parallel bottom flange, default 0 if not specified.
        //! @endGroup
        };

protected:
    explicit AsymmetricIShapeProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateGeometry() const override; //!< @private

private:
    bool ValidateTopFlangeThickness() const;
    bool ValidateBottomFlangeThickness() const;
    bool ValidateWebThickness() const;
    bool ValidateTopFlangeFilletRadius() const;
    bool ValidateTopFlangeEdgeRadius() const;
    bool ValidateTopFlangeSlope() const;
    bool ValidateBottomFlangeFilletRadius() const;
    bool ValidateBottomFlangeEdgeRadius() const;
    bool ValidateBottomFlangeSlope() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (AsymmetricIShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (AsymmetricIShapeProfile)

    //! Creates an instance of AsymmetricIShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of AsymmetricIShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static AsymmetricIShapeProfilePtr Create (CreateParams const& params) { return new AsymmetricIShapeProfile (params); }

public:
    //! @beginGroup
    PROFILES_EXPORT double GetTopFlangeWidth() const; //!< Gets the value of  @ref CreateParams.topFlangeWidth "TopFlangeWidth"
    PROFILES_EXPORT void SetTopFlangeWidth (double value); //!< Sets the value for @ref CreateParams.topFlangeWidth "TopFlangeWidth"

    PROFILES_EXPORT double GetBottomFlangeWidth() const; //!< Gets the value of @ref CreateParams.bottomFlangeWidth "BottomFlangeWidth"
    PROFILES_EXPORT void SetBottomFlangeWidth (double value); //!< Sets the value for @ref CreateParams.bottomFlangeWidth "BottomFlangeWidth"

    PROFILES_EXPORT double GetDepth() const; //!< Gets the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Sets the value for @ref CreateParams.depth "Depth"

    PROFILES_EXPORT double GetTopFlangeThickness() const; //!< Gets the value of @ref CreateParams.topFlangeThickness "TopFlangeThickness"
    PROFILES_EXPORT void SetTopFlangeThickness (double value); //!< Sets the value for @ref CreateParams.topFlangeThickness "TopFlangeThickness"

    PROFILES_EXPORT double GetBottomFlangeThickness() const; //!< Gets the value of @ref CreateParams.bottomFlangeThickness "BottomFlangeThickness"
    PROFILES_EXPORT void SetBottomFlangeThickness (double value); //!< Sets the value for @ref CreateParams.bottomFlangeThickness "BottomFlangeThickness"

    PROFILES_EXPORT double GetWebThickness() const; //!< Gets the value of @ref CreateParams.webThickness "WebThickness"
    PROFILES_EXPORT void SetWebThickness (double value); //!< Sets the value for @ref CreateParams.webThickness "WebThickness"

    PROFILES_EXPORT double GetTopFlangeFilletRadius() const; //!< Gets the value of @ref CreateParams.topFlangeFilletRadius "TopFlangeFilletRadius"
    PROFILES_EXPORT void SetTopFlangeFilletRadius (double value); //!< Sets the value for @ref CreateParams.topFlangeFilletRadius "TopFlangeFilletRadius"

    PROFILES_EXPORT double GetTopFlangeEdgeRadius() const; //!< Gets the value of @ref CreateParams.topFlangeEdgeRadius "TopFlangeEdgeRadius"
    PROFILES_EXPORT void SetTopFlangeEdgeRadius (double value); //!< Sets the value for @ref CreateParams.topFlangeEdgeRadius "TopFlangeEdgeRadius"

    PROFILES_EXPORT Angle GetTopFlangeSlope() const; //!< Gets the value of @ref CreateParams.topFlangeSlope "TopFlangeSlope"
    PROFILES_EXPORT void SetTopFlangeSlope (Angle const& value); //!< Sets the value for @ref CreateParams.topFlangeSlope "TopFlangeSlope"

    PROFILES_EXPORT double GetBottomFlangeFilletRadius() const; //!< Gets the value of @ref CreateParams.bottomFlangeFilletRadius "BottomFlangeFilletRadius"
    PROFILES_EXPORT void SetBottomFlangeFilletRadius (double value); //!< Sets the value for @ref CreateParams.bottomFlangeFilletRadius "BottomFlangeFilletRadius"

    PROFILES_EXPORT double GetBottomFlangeEdgeRadius() const; //!< Gets the value of @ref CreateParams.bottomFlangeEdgeRadius "BottomFlangeEdgeRadius"
    PROFILES_EXPORT void SetBottomFlangeEdgeRadius (double value); //!< Sets the value for @ref CreateParams.bottomFlangeEdgeRadius "BottomFlangeEdgeRadius"

    PROFILES_EXPORT Angle GetBottomFlangeSlope() const; //!< Gets the value of @ref CreateParams.bottomFlangeSlope "BottomFlangeSlope"
    PROFILES_EXPORT void SetBottomFlangeSlope (Angle const& value);//!< Sets the value for @ref CreateParams.bottomFlangeSlope "BottomFlangeSlope"
    //! @endGroup

public:
    PROFILES_EXPORT double GetTopFlangeInnerFaceLength() const; //!< @private
    PROFILES_EXPORT double GetBottomFlangeInnerFaceLength() const; //!< @private
    PROFILES_EXPORT double GetWebInnerFaceLength() const; //!< @private
    PROFILES_EXPORT double GetTopFlangeSlopeHeight() const; //!< @private
    PROFILES_EXPORT double GetBottomFlangeSlopeHeight() const; //!< @private

    }; // AsymmetricIShapeProfile

//=======================================================================================
//! Handler for AsymmetricIShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AsymmetricIShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_AsymmetricIShapeProfile, AsymmetricIShapeProfile, AsymmetricIShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // AsymmetricIShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
