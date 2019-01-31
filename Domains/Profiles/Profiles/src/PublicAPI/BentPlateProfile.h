/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/BentPlateProfile.h $
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
struct BentPlateProfile : ParametricProfile, ICenterLineProfile
    {
    DGNELEMENT_DECLARE_MEMBERS (PRF_CLASS_BentPlateProfile, ParametricProfile);
    friend struct BentPlateProfileHandler;

protected:
    explicit BentPlateProfile (CreateParams const& params) : T_Super (params) {}

public:
    DECLARE_PROFILES_QUERYCLASS_METHODS (BentPlateProfile)
    DECLARE_PROFILES_ELEMENT_BASE_METHODS (BentPlateProfile)

    //! Creates an instance of BentPlateProfile.
    //! @param params CreateParams used to populate instance properties.
    //! @return Instance of BentPlateProfile.
    //! Note that you must call instance.Insert() to persist it in the `DgnDb`
    PROFILES_EXPORT static BentPlateProfilePtr Create (CreateParams const& params) { return new BentPlateProfile (params); }

public:
    PROFILES_EXPORT double GetWidth() const; //!< Get the value of @ref CreateParams.width "Width"
    PROFILES_EXPORT void SetWidth (double value); //!< Set the value for @ref CreateParams.width "Width"

    PROFILES_EXPORT double GetBendAngle() const; //!< Get the value of @ref CreateParams.bendAngle "BendAngle"
    PROFILES_EXPORT void SetBendAngle (double value); //!< Set the value for @ref CreateParams.bendAngle "BendAngle"

    PROFILES_EXPORT double GetBendRadius() const; //!< Get the value of @ref CreateParams.bendRadius "BendRadius"
    PROFILES_EXPORT void SetBendRadius (double value); //!< Set the value for @ref CreateParams.bendRadius "BendRadius"

    PROFILES_EXPORT double GetBendOffset() const; //!< Get the value of @ref CreateParams.bendOffset "BendOffset"
    PROFILES_EXPORT void SetBendOffset (double value); //!< Set the value for @ref CreateParams.bendOffset "BendOffset"

    }; // BentPlateProfile

//=======================================================================================
//! Handler for BentPlateProfile class
//! @private
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE BentPlateProfileHandler : ParametricProfileHandler
    {
    ELEMENTHANDLER_DECLARE_MEMBERS (PRF_CLASS_BentPlateProfile, BentPlateProfile, BentPlateProfileHandler, ParametricProfileHandler, PROFILES_EXPORT)

    }; // BentPlateProfileHandler

END_BENTLEY_PROFILES_NAMESPACE
