/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/PrivateAPI/ProfilesGeometry.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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

    static IGeometryPtr CreateCShape (CShapeProfileCPtr profile);
    static IGeometryPtr CreateIShape (IShapeProfileCPtr profile);
    static IGeometryPtr CreateAsymmetricIShape (AsymmetricIShapeProfileCPtr profile);
    static IGeometryPtr CreateLShape (LShapeProfileCPtr profile);
    static IGeometryPtr CreateTShape (TShapeProfileCPtr profile);
    static IGeometryPtr CreateZShape (ZShapeProfileCPtr profile);
    static IGeometryPtr CreateCenterLineCShape (CenterLineCShapeProfileCPtr profile);
    static IGeometryPtr CreateCircle (CircleProfileCPtr profile);
    static IGeometryPtr CreateHollowCircle (HollowCircleProfileCPtr profile);
    static IGeometryPtr CreateEllipse (EllipseProfileCPtr profile);
    static IGeometryPtr CreateRectangle (RectangleProfileCPtr profile);
    static IGeometryPtr CreateRoundedRectangle (RoundedRectangleProfileCPtr profile);
    static IGeometryPtr CreateHollowRectangle (HollowRectangleProfileCPtr profile);
    static IGeometryPtr CreateTrapezium (TrapeziumProfileCPtr profile);
    static IGeometryPtr CreateDoubleLShape (DoubleLShapeProfileCPtr profile);

    };

END_BENTLEY_PROFILES_NAMESPACE
