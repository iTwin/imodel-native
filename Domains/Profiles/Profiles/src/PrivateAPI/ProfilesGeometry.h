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
struct ProfilesGeometry
    {
public:
    ProfilesGeometry() = delete;

    static IGeometryPtr CreateCShape (CShapeProfile const& profile);
    static IGeometryPtr CreateIShape (IShapeProfile const& profile);
    static IGeometryPtr CreateAsymmetricIShape (AsymmetricIShapeProfile const& profile);
    static IGeometryPtr CreateLShape (LShapeProfile const& profile);
    static IGeometryPtr CreateTShape (TShapeProfile const& profile);
    static IGeometryPtr CreateTTShape (TTShapeProfile const& profile);
    static IGeometryPtr CreateZShape (ZShapeProfile const& profile);
    static IGeometryPtr CreateCenterLineCShape (CenterLineCShapeProfile const& profile);
    static IGeometryPtr CreateCenterLineForCShape (CenterLineCShapeProfile const& profile);
    static IGeometryPtr CreateCenterLineForLShape (CenterLineLShapeProfile const& profile);
    static IGeometryPtr CreateCenterLineLShape (CenterLineLShapeProfile const& profile);
    static IGeometryPtr CreateCircle (CircleProfile const& profile);
    static IGeometryPtr CreateHollowCircle (HollowCircleProfile const& profile);
    static IGeometryPtr CreateEllipse (EllipseProfile const& profile);
    static IGeometryPtr CreateRectangle (RectangleProfile const& profile);
    static IGeometryPtr CreateRoundedRectangle (RoundedRectangleProfile const& profile);
    static IGeometryPtr CreateHollowRectangle (HollowRectangleProfile const& profile);
    static IGeometryPtr CreateTrapezium (TrapeziumProfile const& profile);
    static IGeometryPtr CreateRegularPolygon (RegularPolygonProfile const& profile);
    static IGeometryPtr CreateDoubleLShape (DoubleLShapeProfile const& doubleProfile, LShapeProfile const& singleProfile);
    static IGeometryPtr CreateDoubleCShape (DoubleCShapeProfile const& doubleProfile, CShapeProfile const& singleProfile);
    static IGeometryPtr CreateArbitraryCompositeShape (ArbitraryCompositeProfile const& profile, SinglePerimeterProfileCPtr updatedProfilePtr);
    static IGeometryPtr CreateDerivedShape (DerivedProfile const& profile, SinglePerimeterProfile const& baseProfile);

    };

END_BENTLEY_PROFILES_NAMESPACE
