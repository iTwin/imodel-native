/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"
#include "ParametricProfile.h"
#include "ICenterLineProfile.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! A Z-shaped Profile with rounded corners, similar to cold-formed steel Z-shapes
//! @ingroup GROUP_ParametricProfiles GROUP_CenterLineProfiles
//=======================================================================================
struct CenterLineZShapeProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CenterLineZShapeProfile, ParametricProfile);
    friend struct CenterLineZShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (CenterLineZShapeProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double flangeWidth, double depth,
                                               double wallThickness, double filletRadius = 0.0, double girth = 0.0);

    public:
        //! @beginGroup
        double flangeWidth = 0.0; //!< Extent of single flange. @details Defined parallel to the x axis of the position coordinate system.
        double depth = 0.0; //!< Extent of the web. @details Defined parallel to the y axis of the position coordinate system.
        double wallThickness = 0.0; //!< Constant thickness of profile walls.
        //! @endGroup

        //! @beginGroup
        double filletRadius = 0.0; //!< The fillet radius between the web and the flanges. @details 0 if sharp-edged, default 0 if not specified.
        double girth = 0.0; //!< Length of lips. @details 0 if profile doesn't have lips, default 0 if not specified.
        //! @endGroup
        };

private:
    explicit CenterLineZShapeProfile (CreateParams const& params);

    virtual bool _Validate() const override;
    virtual bool _CreateGeometry() override;
    virtual IGeometryPtr _CreateShapeGeometry() const override;

    bool ValidateWallThickness() const;
    bool ValidateFilletRadius() const;

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CenterLineZShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CenterLineZShapeProfile)

    //! Creates an instance of CenterLineZShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of CenterLineZShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static CenterLineZShapeProfilePtr Create (CreateParams const& params) { return new CenterLineZShapeProfile (params); }

    PROFILES_EXPORT double GetFlangeWidth() const; //!< Get the value of @ref CreateParams.flangeWidth "FlangeWidth"
    PROFILES_EXPORT void SetFlangeWidth (double value); //!< Set the value for @ref CreateParams.flangeWidth "FlangeWidth"

    PROFILES_EXPORT double GetDepth() const; //!< Get the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Set the value for @ref CreateParams.depth "Depth"

    PROFILES_EXPORT double GetFilletRadius() const; //!< Get the value of @ref CreateParams.filletRadius "FilletRadius"
    PROFILES_EXPORT void SetFilletRadius (double value); //!< Set the value for @ref CreateParams.filletRadius "FilletRadius"

    PROFILES_EXPORT double GetGirth() const; //!< Get the value of @ref CreateParams.girth "Girth"
    PROFILES_EXPORT void SetGirth (double value); //!< Set the value for @ref CreateParams.girth "Girth"

    }; // CenterLineZShapeProfile

//=======================================================================================
//! Handler for CenterLineZShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineZShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CenterLineZShapeProfile, CenterLineZShapeProfile, CenterLineZShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // CenterLineZShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
