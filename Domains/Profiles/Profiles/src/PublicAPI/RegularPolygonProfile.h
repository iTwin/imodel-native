/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/RegularPolygonProfile.h $
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
struct RegularPolygonProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_RegularPolygonProfile, ParametricProfile);
    friend struct RegularPolygonProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (RegularPolygonProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, int32_t sideCount, double sideLength);

    public:
        //! @beginGroup
        int32_t sideCount = 0; //!< Count of sides (edges) the polygon is made of. @details Value cannot be less than 3 or greater than 32.
        double sideLength = 0.0; //!< Constant length of each polygon side (edge).
        //! @endGroup
        };

protected:
    explicit RegularPolygonProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateShapeGeometry() const override; //!< @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (RegularPolygonProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (RegularPolygonProfile)

    //! Creates an instance of RegularPolygonProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of RegularPolygonProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static RegularPolygonProfilePtr Create (CreateParams const& params) { return new RegularPolygonProfile (params); }

    //! @beginGroup
    PROFILES_EXPORT int32_t GetSideCount() const; //!< Get the value of @ref CreateParams.sideCount "SideCount"
    PROFILES_EXPORT void SetSideCount (int32_t value); //!< Set the value for @ref CreateParams.sideCount "SideCount"

    PROFILES_EXPORT double GetSideLength() const; //!< Get the value of @ref CreateParams.sideLength "SideLength"
    PROFILES_EXPORT void SetSideLength (double value); //!< Set the value for @ref CreateParams.sideLength "SideLength"
    //! @endGroup

    }; // RegularPolygonProfile

//=======================================================================================
//! Handler for RegularPolygonProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RegularPolygonProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_RegularPolygonProfile, RegularPolygonProfile, RegularPolygonProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // RegularPolygonProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
