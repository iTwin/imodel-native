/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/HollowRectangleProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct HollowRectangleProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_HollowRectangleProfile, ParametricProfile);
    friend struct HollowRectangleProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (HollowRectangleProfile)

    public:
        //! Minimal constructor that initializes all members to zero.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        //! Full constructor to initialize members.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth, double wallThickness,
                                               double innerFilletRadius = 0.0, double outerFilletRadisu = 0.0);

    public:
        //! @beginGroup
        double width = 0.0; //!< Extent of the rectangle in the direction of the x-axis.
        double depth = 0.0; //!< Extent of the rectangle in the direction of the y-axis.
        double wallThickness = 0.0; //!< Constant thickness of profile walls.
        //! @endGroup

        //! @beginGroup
        double innerFilletRadius = 0.0; //!< Radius of the circular arcs, by which all four corners of the inner contour of rectangle are equally rounded. @details Default 0 (= no rounding arcs) if not specified.
        double outerFilletRadius = 0.0; //!< Radius of the circular arcs, by which all four corners of the outer contour of rectangle are equally rounded. @details Default 0 (= no rounding arcs) if not specified.
        //! @endGroup
        };

protected:
    explicit HollowRectangleProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateGeometry() const override; //!< @private

private:
    bool ValidateWallThickness() const;
    bool ValidateInnerFilletRadius() const;
    bool ValidateOuterFilletRadius() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (HollowRectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (HollowRectangleProfile)

    //! Creates an instance of HollowRectangleProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of HollowRectangleProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static HollowRectangleProfilePtr Create (CreateParams const& params) { return new HollowRectangleProfile (params); }

    //! @beginGroup
    PROFILES_EXPORT double GetWidth() const; //!< Get the value of @ref CreateParams.width "Width"
    PROFILES_EXPORT void SetWidth (double value); //!< Set the value for @ref CreateParams.width "Width"

    PROFILES_EXPORT double GetDepth() const; //!< Get the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Set the value for @ref CreateParams.depth "Depth"

    PROFILES_EXPORT double GetInnerFilletRadius() const; //!< Get the value of @ref CreateParams.innerFilletRadius "InnerFilletRadius"
    PROFILES_EXPORT void SetInnerFilletRadius (double value); //!< Set the value for @ref CreateParams.innerFilletRadius "InnerFilletRadius"

    PROFILES_EXPORT double GetOuterFilletRadius() const; //!< Get the value of @ref CreateParams.outerFilletRadius "OuterFilletRadius"
    PROFILES_EXPORT void SetOuterFilletRadius (double value); //!< Set the value for @ref CreateParams.outerFilletRadius "OuterFilletRadius"
    //! @endGroup

    }; // HollowRectangleProfile

//=======================================================================================
//! Handler for HollowRectangleProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HollowRectangleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_HollowRectangleProfile, HollowRectangleProfile, HollowRectangleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // HollowRectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
