/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! An L-shaped Profile with rounded corners, similar to cold-formed steel L-shapes.
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct CenterLineLShapeProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CenterLineLShapeProfile, ParametricProfile);
    friend struct CenterLineLShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (CenterLineLShapeProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        //! @param[in] width Profile's width.
        //! @param[in] depth Profile's depth.
        //! @param[in] girth Profile's girth.
        //! @param[in] filletRadius Profile's inner fillet radius.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double width, double depth,
            double wallThickness, double girth = 0.0, double filletRadius = 0.0);

    public:
        //! @beginGroup
        double width = 0.0; //!< Horizontal leg length. @details Defined parallel to the x axis of the position coordinate system.
        double depth = 0.0; //!< Vertical leg length. @details defined parallel to the y axis of the position coordinate system.
        double wallThickness = 0.0; //!< Constant thickness of profile walls.
        double filletRadius = 0.0; //!< Inner fillet radius. @details Default 0 if not specified.
        //! @endGroup

        //! @beginGroup
        double girth = 0.0; //!< Length of lips. @details 0 if profile doesn't have lips, default 0 if not specified.
        //! @endGroup
        };

private:
    explicit CenterLineLShapeProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual bool _CreateGeometry() override;
    virtual IGeometryPtr _CreateShapeGeometry() const override;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CenterLineLShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CenterLineLShapeProfile)

    //! Creates an instance of CenterLineLShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of CenterLineLShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static CenterLineLShapeProfilePtr Create (CreateParams const& params) { return new CenterLineLShapeProfile (params); }

    //! @beginGroup
    PROFILES_EXPORT double GetWidth() const; //!< Get the value of @ref CreateParams.width "Width"
    PROFILES_EXPORT void SetWidth (double value); //!< Set the value for @ref CreateParams.width "Width"

    PROFILES_EXPORT double GetDepth() const; //!< Get the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Set the value for @ref CreateParams.depth "Depth"

    PROFILES_EXPORT double GetFilletRadius() const; //!< Get the value of @ref CreateParams.filletRadius "FilletRadius"
    PROFILES_EXPORT void SetFilletRadius (double value); //!< Set the value for @ref CreateParams.filletRadius "FilletRadius"

    PROFILES_EXPORT double GetGirth() const; //!< Get the value of @ref CreateParams.girth "Girth"
    PROFILES_EXPORT void SetGirth (double value); //!< Set the value for @ref CreateParams.girth "Girth"
    //! @endGroup

    }; // CenterLineLShapeProfile

//=======================================================================================
//! Handler for CenterLineLShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineLShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CenterLineLShapeProfile, CenterLineLShapeProfile, CenterLineLShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)
    }; // CenterLineLShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
