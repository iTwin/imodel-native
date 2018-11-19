/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/ICenterLineProfile.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "ProfilesDefinitions.h"

BEGIN_BENTLEY_PROFILES_NAMESPACE

//=======================================================================================
//! 
//! @ingroup GROUP_Profiles
//=======================================================================================
struct ICenterLineProfile : NonCopyableClass
    {
protected:
    virtual ~ICenterLineProfile() = default;

public:
    PROFILES_EXPORT IGeometryPtr GetCenterLine() const;
    PROFILES_EXPORT void SetCenterLine (IGeometryPtr val);

    PROFILES_EXPORT double GetWallThickness() const;
    PROFILES_EXPORT void SetWallThickness (double val);

    }; // ICenterLineProfile

END_BENTLEY_PROFILES_NAMESPACE
