/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/RectangleProfile.h $
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
struct RectangleProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_RectangleProfile, ParametricProfile);
    friend struct RectangleProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (RectangleProfile)

    public:
        //! Minimal constructor that initializes all members to zero.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        //! Full constructor to initialize members.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth);

    public:
        //! @beginGroup
        double width = 0.0; //!< Extent of the rectangle in the direction of the x-axis.
        double depth = 0.0; //!< Extent of the rectangle in the direction of the y-axis.
        //! @endGroup
        };

protected:
    explicit RectangleProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateGeometry() const override; //!< @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (RectangleProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (RectangleProfile)

    //! Creates an instance of RectangleProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of RectangleProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static RectangleProfilePtr Create (CreateParams const& params) { return new RectangleProfile (params); }

    PROFILES_EXPORT double GetWidth() const; //!< Get the value of @ref CreateParams.width "Width"
    PROFILES_EXPORT void SetWidth (double value); //!< Set the value for @ref CreateParams.width "Width"

    PROFILES_EXPORT double GetDepth() const; //!< Get the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Set the value for @ref CreateParams.depth "Depth"

    }; // RectangleProfile

//=======================================================================================
//! Handler for RectangleProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RectangleProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_RectangleProfile, RectangleProfile, RectangleProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // RectangleProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
