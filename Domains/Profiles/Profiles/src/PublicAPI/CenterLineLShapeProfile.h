/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/CenterLineLShapeProfile.h $
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
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double width, double depth, double wallThickness, double girth = 0.0, double filletRadius = 0.0);

    public:
        //! Required properties
        double width = 0.0;
        double depth = 0.0;
        double wallThickness = 0.0;

        //! Optional properties
        double girth = 0.0;
        double filletRadius = 0.0;
        };

protected:
    explicit CenterLineLShapeProfile (CreateParams const& params);

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateShapeGeometry() const override; //!< @private
    virtual bool _CreateGeometry() override; //!< @private

private:
    IGeometryPtr CreateCenterLineGeometry();

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CenterLineLShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CenterLineLShapeProfile)

    //! Creates an instance of CenterLineLShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of CenterLineLShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static CenterLineLShapeProfilePtr Create (CreateParams const& params) { return new CenterLineLShapeProfile (params); }

public:
    PROFILES_EXPORT double GetWidth() const; //!< Get the value of @ref CreateParams.width "Width"
    PROFILES_EXPORT void SetWidth (double value); //!< Set the value for @ref CreateParams.width "Width"

    PROFILES_EXPORT double GetDepth() const; //!< Get the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Set the value for @ref CreateParams.depth "Depth"

    PROFILES_EXPORT double GetFilletRadius() const; //!< Get the value of @ref CreateParams.filletRadius "FilletRadius"
    PROFILES_EXPORT void SetFilletRadius (double value); //!< Set the value for @ref CreateParams.filletRadius "FilletRadius"

    PROFILES_EXPORT double GetGirth() const; //!< Get the value of @ref CreateParams.girth "Girth"
    PROFILES_EXPORT void SetGirth (double value); //!< Set the value for @ref CreateParams.girth "Girth"

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
