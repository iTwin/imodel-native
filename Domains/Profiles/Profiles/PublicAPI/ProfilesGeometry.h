/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/ProfilesGeometry.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley\Bentley.h>
#include <DgnPlatform\DgnPlatformApi.h>
#include <Profiles\ProfilesApi.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

struct ProfilesGeomApi : RefCountedBase
    {
    protected: 
        ProfilesGeomApi() {};
    public:
        PROFILES_EXPORT static IGeometryPtr CreateIShape(double overallWidth, double overallDepth, double flangeThickness, double webThickness);
        PROFILES_EXPORT static IGeometryPtr CreateIShapeWithFillet(double overallWidth, double overallDepth, double flangeThickness, double webThickness, double filletRadius);
        PROFILES_EXPORT static IGeometryPtr CreateCShape (CShapeProfileCPtr profile);
    };

END_BENTLEY_PROFILES_NAMESPACE