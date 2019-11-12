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
//! A TT-shaped Profile similar to double-tee concrete shapes.
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct TTShapeProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_TTShapeProfile, ParametricProfile);
    friend struct TTShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (TTShapeProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double flangeWidth, double depth, double flangeThickness, double webThickness,
                                               double webSpacing, double filletRadius = 0.0, double flangeEdgeRadius = 0.0, Angle const& flangeSlope = Angle::FromRadians (0.0),
                                               double webEdgeRadius = 0.0, Angle const& webSlope = Angle::FromRadians (0.0));

    public:
        //! @beginGroup
        double flangeWidth = 0.0; //!< Extent of the width (top flange). @details Defined parallel to the x axis of the position coordinate system.
        double depth = 0.0; //!< Extent of the depth (web length). @details Defined parallel to the y axis of the position coordinate system.
        double flangeThickness = 0.0; //!< Constant wall thickness of the flange.
        double webThickness = 0.0; //!< Constant wall thickness of the web.
        double webSpacing = 0.0; //!< Spacing between the two webs.
        //! @endGroup

        //! @beginGroup
        double filletRadius = 0.0; //!< The fillet radius between the web and the flange. @details 0 if sharp-edged, default 0 if not specified.
        double flangeEdgeRadius = 0.0; //!< Radius of flange edges. @details 0 if sharp-edged, default 0 if not specified.
        Angle flangeSlope = Angle::FromRadians (0.0); //!< Slope of the lower face of the flange. @details Non-zero in case of tapered flange, 0 in case of non-tapered flange, default 0 if not specified.
        double webEdgeRadius = 0.0; //!< Radius of web edges. @details 0 if sharp-edged, default 0 if not specified.
        Angle webSlope = Angle::FromRadians (0.0); //!< Slope of the web. @details Non-zero in case of tapered web, 0 in case of non-tapered web, default 0 if not specified.
        //! @endGroup
        };

protected:
    explicit TTShapeProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateShapeGeometry() const override;  //!< @private

private:
    bool ValidateFlangeThickness() const;
    bool ValidateWebThickness() const;
    bool ValidateWebSpacing() const;
    bool ValidateFilletRadius() const;
    bool ValidateFlangeEdgeRadius() const;
    bool ValidateFlangeSlope() const;
    bool ValidateWebEdgeRadius() const;
    bool ValidateWebSlope() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (TTShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (TTShapeProfile)

    //! Creates an instance of TTShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of TTShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static TTShapeProfilePtr Create (CreateParams const& params) { return new TTShapeProfile (params); }

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

    PROFILES_EXPORT double GetWebSpacing() const; //!< Get the value of @ref CreateParams.webSpacing "WebSpacing"
    PROFILES_EXPORT void SetWebSpacing (double value); //!< Set the value for @ref CreateParams.webSpacing "WebSpacing"

    PROFILES_EXPORT double GetFilletRadius() const; //!< Get the value of @ref CreateParams.filletRadius "FilletRadius"
    PROFILES_EXPORT void SetFilletRadius (double value); //!< Set the value for @ref CreateParams.filletRadius "FilletRadius"

    PROFILES_EXPORT double GetFlangeEdgeRadius() const; //!< Get the value of @ref CreateParams.flangeEdgeRadius "FlangeEdgeRadius"
    PROFILES_EXPORT void SetFlangeEdgeRadius (double value); //!< Set the value for @ref CreateParams.flangeEdgeRadius "FlangeEdgeRadius"

    PROFILES_EXPORT Angle GetFlangeSlope() const; //!< Get the value of @ref CreateParams.flangeSlope "FlangeSlope"
    PROFILES_EXPORT void SetFlangeSlope (Angle const& value); //!< Set the value for @ref CreateParams.flangeSlope "FlangeSlope"

    PROFILES_EXPORT double GetWebEdgeRadius() const; //!< Get the value of @ref CreateParams.webEdgeRadius "WebEdgeRadius"
    PROFILES_EXPORT void SetWebEdgeRadius (double value); //!< Set the value for @ref CreateParams.webEdgeRadius "WebEdgeRadius"

    PROFILES_EXPORT Angle GetWebSlope() const; //!< Get the value of @ref CreateParams.webSlope "WebSlope"
    PROFILES_EXPORT void SetWebSlope (Angle const& value); //!< Set the value for @ref CreateParams.webSlope "WebSlope"
    //! @endGroup

public:
    PROFILES_EXPORT double GetFlangeInnerFaceLength() const; //!< @private
    PROFILES_EXPORT double GetFlangeSlopeHeight() const; //!< @private
    PROFILES_EXPORT double GetWebInnerFaceLength() const; //!< @private
    PROFILES_EXPORT double GetWebOuterFaceLength() const; //!< @private
    PROFILES_EXPORT double GetWebInnerSlopeHeight() const; //!< @private
    PROFILES_EXPORT double GetWebOuterSlopeHeight() const; //!< @private

    }; // TTShapeProfile

//=======================================================================================
//! Handler for TTShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TTShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_TTShapeProfile, TTShapeProfile, TTShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // TTShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
