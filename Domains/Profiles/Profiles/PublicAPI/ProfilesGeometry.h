/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/PublicAPI/ProfilesGeometry.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Geom\GeomApi.h>
#include <Profiles\ProfilesApi.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//*
* @bsiclass                                                                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProfilesGeomApi
    {
public:
    ProfilesGeomApi() = delete;

    // TODO Karolis: We should place this header in PrivateAPI so its not visible for users of the domain
    static IGeometryPtr CreateCShape (CShapeProfileCPtr profile);
    static IGeometryPtr CreateIShape (IShapeProfileCPtr profile);
    static IGeometryPtr CreateAsymmetricIShape (AsymmetricIShapeProfileCPtr profile);
    static IGeometryPtr CreateLShape (LShapeProfileCPtr profile);
    static IGeometryPtr CreateTShape (TShapeProfileCPtr profile);
    static IGeometryPtr CreateZShape (ZShapeProfileCPtr profile);
    static IGeometryPtr CreateCircle (CircleProfileCPtr profile);
    static IGeometryPtr CreateHollowCircle (HollowCircleProfileCPtr profile);
    static IGeometryPtr CreateEllipse (EllipseProfileCPtr profile);
    static IGeometryPtr CreateRectangle (RectangleProfileCPtr profile);

    };

END_BENTLEY_PROFILES_NAMESPACE
