/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PublicAPI/ICenterLineProfile.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include "ProfilesDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_ParametricProfiles
//=======================================================================================
struct ICenterLineProfile : NonCopyableClass
    {
protected:
    virtual ~ICenterLineProfile() = default;

public:
    PROFILES_EXPORT IGeometryPtr GetCenterLine() const;

    // TODO Karolis: Add docs
    PROFILES_EXPORT double GetWallThickness() const;
    // TODO Karolis: Add docs
    PROFILES_EXPORT void SetWallThickness (double value);

protected:
    //! @private
    PROFILES_EXPORT void SetCenterLine (IGeometry const& val);

    }; // ICenterLineProfile

END_BENTLEY_PROFILES_NAMESPACE
