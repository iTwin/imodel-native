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
//! 
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct TrapeziumProfile : ParametricProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_TrapeziumProfile, ParametricProfile);
    friend struct TrapeziumProfileHandler;

public:
    struct CreateParams : T_Super::CreateParams
        {
        DECLARE_PROFILES_CREATE_PARAMS_BASE_METHODS (TrapeziumProfile)

    public:
        //! Minimal constructor that initializes Profile members to default values and associates it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName);
        //! Constructor to initialize Profile members and associate it with provided DgnModel.
        //! @param[in] model DgnModel that the Profile will be associated to.
        //! @param[in] pName Name of the Profile.
        PROFILES_EXPORT explicit CreateParams (Dgn::DefinitionModel const& model, Utf8CP pName, double topWidth, double bottomWidth,
                                               double depth, double topOffset);

    public:
        //! @beginGroup
        double topWidth = 0.0; //!< Extent of the top line measured along the implicit x-axis.
        double bottomWidth = 0.0; //!< Extent of the bottom line measured along the implicit x-axis.
        double depth = 0.0; //!< Extent of the distance between the parallel bottom and top lines measured along the implicit y-axis.
        double topOffset = 0.0; //!< Offset from the beginning of the top line to the bottom line, measured along the implicit x-axis.
        //! @endGroup
        };

protected:
    explicit TrapeziumProfile (CreateParams const& params); //!< @private

    virtual bool _Validate() const override; //!< @private
    virtual IGeometryPtr _CreateShapeGeometry() const override; //!< @private

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (TrapeziumProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (TrapeziumProfile)

    //! Creates an instance of TrapeziumProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of TrapeziumProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static TrapeziumProfilePtr Create (CreateParams const& params) { return new TrapeziumProfile (params); }

    //! @beginGroup
    PROFILES_EXPORT double GetTopWidth() const; //!< Get the value of @ref CreateParams.topWidth "TopWidth"
    PROFILES_EXPORT void SetTopWidth (double value); //!< Set the value for @ref CreateParams.topWidth "TopWidth"

    PROFILES_EXPORT double GetBottomWidth() const; //!< Get the value of @ref CreateParams.bottomWidth "BottomWidth"
    PROFILES_EXPORT void SetBottomWidth (double value); //!< Set the value for @ref CreateParams.bottomWidth "BottomWidth"

    PROFILES_EXPORT double GetDepth() const; //!< Get the value of @ref CreateParams.depth "Depth"
    PROFILES_EXPORT void SetDepth (double value); //!< Set the value for @ref CreateParams.depth "Depth"

    PROFILES_EXPORT double GetTopOffset() const; //!< Get the value of @ref CreateParams.topOffset "TopOffset"
    PROFILES_EXPORT void SetTopOffset (double value); //!< Set the value for @ref CreateParams.topOffset "TopOffset"
    //! @endGroup

    }; // TrapeziumProfile

//=======================================================================================
//! Handler for TrapeziumProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE TrapeziumProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_TrapeziumProfile, TrapeziumProfile, TrapeziumProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // TrapeziumProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
