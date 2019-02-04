/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/CenterLineCShapeProfile.h $
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
//! A C shaped Profile with rounded corners, similar to cold-formed steel C-shapes.
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct CenterLineCShapeProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_CenterLineCShapeProfile, ParametricProfile);
    friend struct CenterLineCShapeProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (CenterLineCShapeProfile)

    public:
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double flangeWidth, double depth, double wallThickness, double girth = 0.0, double filletRadius = 0.0);
    public:
        //! Required properties
        double flangeWidth = 0.0;
        double depth = 0.0;
        double wallThickness = 0.0;

        //! Optional properties
        double girth = 0.0;
        double filletRadius = 0.0;
        };

protected:
    explicit CenterLineCShapeProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual bool _CreateGeometry () override;  //!< @private
    IGeometryPtr _CreateShapeGeometry() const;  //!< @private

private:
    IGeometryPtr CreateCenterLineGeometry();

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (CenterLineCShapeProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (CenterLineCShapeProfile)

    //! Creates an instance of CenterLineCShapeProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of CenterLineCShapeProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static CenterLineCShapeProfilePtr Create (CreateParams const& params) { return new CenterLineCShapeProfile (params); }
public:
    PROFILES_EXPORT double GetFlangeWidth() const; //!< Get the value of @ref CreateParams.flangeWidth "FlangeWidth"
    PROFILES_EXPORT void SetFlangeWidth (double value); //!< Set the value for @ref CreateParams.flangeWidth "FlangeWidth"

    PROFILES_EXPORT double GetDepth() const; //!< Get the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Set the value for @ref CreateParams.depth "Depth"

    PROFILES_EXPORT double GetFilletRadius() const; //!< Get the value of @ref CreateParams.filletRadius "FilletRadius"
    PROFILES_EXPORT void SetFilletRadius (double value); //!< Set the value for @ref CreateParams.filletRadius "FilletRadius"

    PROFILES_EXPORT double GetGirth() const; //!< Get the value of @ref CreateParams.girth "Girth"
    PROFILES_EXPORT void SetGirth (double value); //!< Set the value for @ref CreateParams.girth "Girth"
    }; // CenterLineCShapeProfile

//=======================================================================================
//! Handler for CenterLineCShapeProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CenterLineCShapeProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_CenterLineCShapeProfile, CenterLineCShapeProfile, CenterLineCShapeProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)
    }; // CenterLineCShapeProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
