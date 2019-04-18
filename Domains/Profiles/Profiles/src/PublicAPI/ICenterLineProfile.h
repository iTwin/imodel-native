/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! Mixin class used to decorate "CenterLine" profiles.
//! @details See CenterLineLShapeProfile, CenterLineCShapeProfile, CenterLine
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct ICenterLineProfile : NonCopyableClass
    {
protected:
    virtual ~ICenterLineProfile() = default;

public:
    //! @beginGroup
    //! Get the IGeometry defining the center line of Profile.
    //! @details Geometry is created during a db Insert or Update operation.
    PROFILES_EXPORT IGeometryPtr GetCenterLine() const;

    PROFILES_EXPORT double GetWallThickness() const; //!< Get the value of "WallThickness" property.
    PROFILES_EXPORT void SetWallThickness (double value); //!< Set the value for "WallThickness" property
    //! @endGroup

protected:
    PROFILES_EXPORT void SetCenterLine (IGeometry const& val); //! @private

    }; // ICenterLineProfile

END_BENTLEY_PROFILES_NAMESPACE
