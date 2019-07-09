/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
struct RoundedRectangleProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_RoundedRectangleProfile, ParametricProfile);
    friend struct RoundedRectangleProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (RoundedRectangleProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double width, double depth, double roundingRadius);

    public:
        //! @beginGroup
        double width = 0.0; //!< Extent of the rectangle in the direction of the x-axis.
        double depth = 0.0; //!< Extent of the rectangle in the direction of the y-axis.
        double roundingRadius = 0.0; //!< Radius of the circular arcs by which all four corners of the rectangle are equally rounded.
        //! @endGroup
        };

protected:
    explicit RoundedRectangleProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateShapeGeometry() const override; //!< @private

private:
    bool ValidateWidth() const;
    bool ValidateDepth() const;
    bool ValidateRoundingRadius() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (RoundedRectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (RoundedRectangleProfile)

    //! Creates an instance of RoundedRectangleProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of RoundedRectangleProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static RoundedRectangleProfilePtr Create (CreateParams const& params) { return new RoundedRectangleProfile (params); }

    //! @beginGroup
    PROFILES_EXPORT double GetWidth() const; //!< Get the value of @ref CreateParams.width "Width"
    PROFILES_EXPORT void SetWidth (double value); //!< Set the value for @ref CreateParams.width "Width"

    PROFILES_EXPORT double GetDepth() const; //!< Get the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Set the value for @ref CreateParams.depth "Depth"

    PROFILES_EXPORT double GetRoundingRadius() const; //!< Get the value of @ref CreateParams.roundingRadius "RoundingRadius"
    PROFILES_EXPORT void SetRoundingRadius (double value); //!< Set the value for @ref CreateParams.roundingRadius "RoundingRadius"
    //! @endGroup

    }; // RoundedRectangleProfile

//=======================================================================================
//! Handler for RoundedRectangleProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoundedRectangleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_RoundedRectangleProfile, RoundedRectangleProfile, RoundedRectangleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // RoundedRectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
