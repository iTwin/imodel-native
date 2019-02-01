/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/EllipseProfile.h $
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
struct EllipseProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_EllipseProfile, ParametricProfile);
    friend struct EllipseProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (EllipseProfile)

    public:
        //! Minimal constructor that initializes all members to zero.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName);
        //! Full constructor to initialize members.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DgnModel const& model, Utf8CP pName, double xRadius, double yRadius);

    public:
        //! @beginGroup
        double xRadius = 0.0; //!< The first radius of the ellipse. @details It is measured along the direction of x axis of the position coordinate system.
        double yRadius = 0.0; //!< The second radius of the ellipse. @details It is measured along the direction of y axis of the position coordinate system.
        //! @endGroup
        };

protected:
    explicit EllipseProfile (CreateParams const& params); //! @private

    virtual bool _Validate() const override; //! @private
    virtual IGeometryPtr _CreateGeometry() const override; //! @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (EllipseProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (EllipseProfile)

    //! Creates an instance of EllipseProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of EllipseProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static EllipseProfilePtr Create (CreateParams const& params) { return new EllipseProfile (params); }

    //! @beginGroup
    PROFILES_EXPORT double GetXRadius() const; //!< Get the value of @ref CreateParams.xRadius "XRadius"
    PROFILES_EXPORT void SetXRadius (double value); //!< Set the value for @ref CreateParams.xRadius "XRadius"

    PROFILES_EXPORT double GetYRadius() const; //!< Get the value of @ref CreateParams.yRadius "YRadius"
    PROFILES_EXPORT void SetYRadius (double value); //!< Set the value for @ref CreateParams.yRadius "YRadius"
    //! @endGroup

    }; // EllipseProfile

//=======================================================================================
//! Handler for EllipseProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EllipseProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_EllipseProfile, EllipseProfile, EllipseProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // EllipseProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
