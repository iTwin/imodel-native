/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    static IGeometryPtr CreateSchifflerizedLShape (SchifflerizedLShapeProfile const& profile);
    static IGeometryPtr CreateTShape (TShapeProfile const& profile);
    static IGeometryPtr CreateTTShape (TTShapeProfile const& profile);
    static IGeometryPtr CreateZShape (ZShapeProfile const& profile);
    static IGeometryPtr CreateCenterLineCShape (CenterLineCShapeProfile const& profile);
    static IGeometryPtr CreateCenterLineForCShape (CenterLineCShapeProfile const& profile);
    static IGeometryPtr CreateCenterLineLShape (CenterLineLShapeProfile const& profile);
    static IGeometryPtr CreateCenterLineForLShape (CenterLineLShapeProfile const& profile);
    static IGeometryPtr CreateCenterLineZShape (CenterLineZShapeProfile const& profile);
    static IGeometryPtr CreateCenterLineForZShape (CenterLineZShapeProfile const& profile);
    static IGeometryPtr CreateBentPlateShape (BentPlateProfile const& profile);
    static IGeometryPtr CreateBentPlateCenterLine (BentPlateProfile const& profile);
    static IGeometryPtr CreateArbitraryCenterLineShape (IGeometry const& centerLine, double wallThickness, Angle const& arcAngle, Angle const& chamferAngle);
    static IGeometryPtr CreateCircle (CircleProfile const& profile);
    static IGeometryPtr CreateHollowCircle (HollowCircleProfile const& profile);
    static IGeometryPtr CreateEllipse (EllipseProfile const& profile);
    static IGeometryPtr CreateRectangle (RectangleProfile const& profile);
    static IGeometryPtr CreateRoundedRectangle (RoundedRectangleProfile const& profile);
    static IGeometryPtr CreateHollowRectangle (HollowRectangleProfile const& profile);
    static IGeometryPtr CreateTrapezium (TrapeziumProfile const& profile);
    static IGeometryPtr CreateCapsule (CapsuleProfile const& profile);
    static IGeometryPtr CreateRegularPolygon (RegularPolygonProfile const& profile);
    static IGeometryPtr CreateDoubleLShape (DoubleLShapeProfile const& doubleProfile, LShapeProfile const& singleProfile);
    static IGeometryPtr CreateDoubleCShape (DoubleCShapeProfile const& doubleProfile, CShapeProfile const& singleProfile);
    static IGeometryPtr CreateArbitraryCompositeShape (ArbitraryCompositeProfile const& profile, SinglePerimeterProfileCPtr updatedProfilePtr);
    static IGeometryPtr CreateDerivedShape (DerivedProfile const& profile, SinglePerimeterProfile const& baseProfile);

    static bool ValidateRangeXY (IGeometry const& geometry, Utf8CP pProfileClassName);
    static bool ValidateCurveVectorContinious (CurveVector const& curveVector, bool checkRegion, Utf8CP pProfileClassName);
    };

END_BENTLEY_PROFILES_NAMESPACE
