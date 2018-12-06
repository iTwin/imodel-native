/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/ProfilesGeometry.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//#include <Bentley\Bentley.h>
//#include <DgnPlatform\DgnPlatformApi.h>
#include <Geom\GeomApi.h>
#include <Profiles\ProfilesApi.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

struct ProfilesGeomApi
    {
public:
    ProfilesGeomApi() = delete;

    PROFILES_EXPORT static IGeometryPtr CreateCShape (CShapeProfileCPtr profile);
    PROFILES_EXPORT static IGeometryPtr CreateIShape (IShapeProfileCPtr profile);

    };

END_BENTLEY_PROFILES_NAMESPACE